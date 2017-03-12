#include "helpers.hpp"
#include "HttpServer.hpp"
#include "ProtoCompiler.hpp"

#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QProcess>

#include <string>
using namespace std::string_literals;


QVector<HttpServer*> HttpServer::ms_instances;

void HttpServer::on_new_connection()
{
    auto socket = m_server->nextPendingConnection();

    connect(socket, SIGNAL(readyRead()),    this, SLOT(on_ready_read()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(on_connection_closed()));

    m_clients.insert(socket, Request{});
    YOGI_LOG_INFO(m_logger, "HTTP client " << socket->peerAddress().toString() << " connected");
}

void HttpServer::on_ready_read()
{
    auto client = qobject_cast<QTcpSocket*>(sender());
    auto& request = m_clients[client];

    if (request.receiveState == RCV_REQUEST_LINE) {
        if (client->canReadLine()) {
            auto line = client->readLine();
            if (!parse_request_line(line, &request)) {
                YOGI_LOG_WARNING(m_logger, "Invalid HTTP request line received from " << client->peerAddress().toString() << ": " << QString(line));
                client->close();
                return;
            }

            request.receiveState = RCV_HEADER;
        }
    }

    if (request.receiveState == RCV_HEADER) {
        while (client->canReadLine()) {
            auto line = client->readLine();
            if (line.trimmed().isEmpty()) {
                request.contentLength = request.header.value("content-length", "0").toInt();
                request.receiveState = RCV_CONTENT;
                break;
            }

            int pos = line.indexOf(':');
            request.header[line.left(pos).toLower()] = line.mid(pos + 2);
        }
    }

    if (request.receiveState == RCV_CONTENT) {
        while ((request.content.size() != request.contentLength) && client->isReadable()) {
            auto sizeBeforeRead = request.content.size();
            request.content += client->read(request.contentLength - request.content.size());
            if (request.content.size() == sizeBeforeRead) {
                break;
            }
        }

        if (request.content.size() == request.contentLength) {
            request.receiveState = RCV_DONE;
        }
    }

    if (request.receiveState == RCV_DONE) {
        switch (request.type) {
        case REQ_GET:
            handle_get_request(client, request);
            break;

        case REQ_POST:
            handle_post_request(client, request);
            break;
        }

        request = Request{};
    }
}

void HttpServer::on_connection_closed()
{
    auto client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        YOGI_LOG_DEBUG("HTTP client " << client->peerAddress().toString() << " disconnected.");
        m_clients.erase(m_clients.find(client));
        client->deleteLater();
    }
}

QMap<QString, QString> HttpServer::extract_map_from_config(const yogi::ConfigurationChild& config, const char* childName)
{
    QMap<QString, QString> map;
    for (auto entry : config.get_child(childName)) {
        map[QString::fromStdString(entry.first)] = QString::fromStdString(entry.second.get_value<std::string>());
    }

    return map;
}

void HttpServer::setup(const yogi::ConfigurationChild& config)
{
    auto addrStr = config.get<std::string>("address");
    auto address = (addrStr == "any" || addrStr == "0.0.0.0" || addrStr == "::") ? QHostAddress::Any : QHostAddress(QString::fromStdString(addrStr));
    auto port    = config.get<std::uint16_t>("port");
    auto infoStr = addrStr + ":" + std::to_string(port);

    if (!config.get<bool>("enabled")) {
        YOGI_LOG_DEBUG(m_logger, "Disabled HTTP server listening on " << infoStr);
        return;
    }

    if (m_server->listen(address, port)) {
        YOGI_LOG_INFO(m_logger, "HTTP server listening on " << infoStr);
        connect(m_server, SIGNAL(newConnection()), this, SLOT(on_new_connection()));
        ms_instances.push_back(this);
    }
    else {
        YOGI_LOG_ERROR(m_logger, "Disabled HTTP server since listening on " << infoStr << " failed: " << m_server->errorString());
    }
}

bool HttpServer::parse_request_line(const QString& requestLine, Request* request)
{
    if (requestLine.startsWith("GET ")) {
        request->type = REQ_GET;
    }
    else if (requestLine.startsWith("POST ")) {
        request->type = REQ_POST;
    }
    else {
        return false;
    }

    QRegExp rx("^[A-Z]+ (/.*)(\\?.*) HTTP/1.1");
    if (rx.indexIn(requestLine) != 0) {
        rx = QRegExp("^[A-Z]+ (/.*) HTTP/1.1");
        if (rx.indexIn(requestLine) != 0) {
            return false;
        }
    }

    request->uri = rx.cap(1);

    return true;
}

void HttpServer::handle_get_request(QTcpSocket* client, const Request& request)
{
    auto uri = request.uri;
    YOGI_LOG_DEBUG(m_logger, "HTTP GET request for " << uri << " from " << client->peerAddress().toString());

    if (uri == "/") {
        uri = "/index.html";
    }

    QStringList header;
    QByteArray content;
    QString contentType;

    auto queryResponse = uri_to_query_response(uri);
    if (!queryResponse.isNull()) {
        respond(client, 201, queryResponse.toUtf8());
        return;
    }

    auto filePath = uri_to_file_path(uri);
    if (filePath.isNull()) {
        respond(client, 404, "404 Not found");
        return;
    }

    respond_with_file_content(client, filePath);
}

void HttpServer::handle_post_request(QTcpSocket* client, const Request& request)
{
    YOGI_LOG_DEBUG(m_logger, "HTTP POST request for " << request.uri << " from " << client->peerAddress().toString())

    try {
        if (request.uri.startsWith("/compile/")) {
            if (yogi::ProcessInterface::config().get<bool>("proto-compiler.enabled")) {
                ProtoCompiler::Language language;
                if (request.uri == "/compile/python") {
                    language = ProtoCompiler::LNG_PYTHON;
                }
                else if (request.uri == "/compile/cpp") {
                    language = ProtoCompiler::LNG_CPP;
                }
                else if (request.uri == "/compile/csharp") {
                    language = ProtoCompiler::LNG_CSHARP;
                }
                else {
                    respond(client, 404);
                    return;
                }

                auto files = ProtoCompiler::instance().compile(request.content, language);
                QByteArray json = "{";
                for (auto& key : files.keys()) {
                    auto fileContent = files[key];
                    fileContent.replace('"', "\\\"");
                    json += "\"" + key + "\":\"" + fileContent + "\",";
                }
                json[json.size() - 1] = '}';

                respond(client, 201, json);
            }
            else {
                respond(client, 403, "The Proto compiler on the server is disabled");
            }
        }
        else {
            respond(client, 404);
        }
    }
    catch (const std::exception& e) {
        respond(client, 500, e.what());
    }
}

QString HttpServer::uri_to_query_response(const QString& uri)
{
    for (auto route : m_queries.keys()) {
        if (uri == route) {
            return m_queries[route];
        }
    }

    return QString();
}

QString HttpServer::uri_to_file_path(const QString& uri)
{
    auto routes = QStringList(m_routes.keys());
    routes.sort();

    while (!routes.empty()) {
        auto route = routes.takeLast();

        QString prefix = QDir::cleanPath(route);
        if (!QDir(route).isRoot()) {
            prefix += "/";
        }

        if (uri.startsWith(prefix)) {
            auto routeBase = QDir::cleanPath(m_routes[route]);
            auto filePath = routeBase + "/" + uri.right(uri.length() - prefix.length());

            if (QFileInfo(filePath).isFile()) {
                return filePath;
            }
            else if (!uri.contains('.')) {
                // walk our way back up the file system and look for index.html
                auto path = filePath;
                while (path.length() >= routeBase.length()) {
                    auto indexPath = path + "/index.html";
                    if (QFileInfo(path).isDir() && QFileInfo(indexPath).isFile()) {
                        YOGI_LOG_INFO(m_logger, "Could not find requested URI. Serving " << indexPath << " instead.");
                        return indexPath;
                    }

                    if (path.length() == routeBase.length()) {
                        break;
                    }

                    path = path.left(path.lastIndexOf('/'));
                }
            }
        }
    }

    YOGI_LOG_WARNING(m_logger, "Could not find requested URI " << uri);
    return QString();
}

void HttpServer::respond(QTcpSocket* client, int httpStatusCode, const QByteArray& content, const QString& contentType, bool compressed)
{
    QStringList header;

    switch (httpStatusCode) {
    case 200:
        header += "HTTP/1.1 200 OK";
        break;

    case 201:
        header += "HTTP/1.1 201 OK";
        break;

    case 400:
        header += "HTTP/1.1 400 Bad Request";
        break;

    case 403:
        header += "HTTP/1.1 403 Forbidden";
        break;

    case 404:
        header += "HTTP/1.1 404 Not found";
        break;

    case 500:
        header += "HTTP/1.1 500 Internal Server Error";
        break;

    default:
        YOGI_LOG_FATAL(m_logger, "HTTP status code " << httpStatusCode << " handling not implemented");
        return;
    }

    header += "Access-Control-Allow-Origin: *";

    QString contentTypeHeader = QString("Content-Type: ") + contentType;
    if (contentType.startsWith("text/")) {
        contentTypeHeader += "; charset=UTF-8";
    }

    header += contentTypeHeader;
    header += "Connection: Keep-Alive";
    header += "Content-Length: " + QString::number(content.length());

    if (compressed) {
        header += "Content-Encoding: gzip";
    }

    QByteArray data;
    data += header.join("\r\n") + "\r\n\r\n";
    data += content;

    if (client->write(data) != data.length()) {
        YOGI_LOG_WARNING(m_logger, "Not all data has been sent to " << client->peerAddress().toString());
        client->close();
    }
}

void HttpServer::respond_with_file_content(QTcpSocket* client, const QString& filePath)
{
    auto lastModified = QFileInfo(filePath).lastModified();

    auto cacheEntry = update_file_cache(filePath);
    if (cacheEntry) {
        YOGI_LOG_DEBUG(m_logger, "Serving file " << filePath << (cacheEntry->compressed ? " with gzip" : " without") << " compression ");
        respond(client, 200, cacheEntry->content, cacheEntry->contentType, cacheEntry->compressed);
    }
    else {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            YOGI_LOG_DEBUG(m_logger, "Serving file " << filePath << " without compression ");
            auto contentType = m_mimeDb.mimeTypeForFile(filePath).name();
            respond(client, 200, file.readAll(), contentType);
        }
        else {
            YOGI_LOG_WARNING(m_logger, "Could not open " << filePath << ": " << file.errorString());
            respond(client, 403);
        }
    }
}

void HttpServer::check_gzip_exists()
{
	if (!QFile::exists(m_gzipExecutable)) {
		auto msg = "Cannot find gzip executable '"s + m_gzipExecutable.toStdString() + "'";
		YOGI_LOG_ERROR(m_logger, msg);
		throw std::runtime_error(msg);
	}
}

HttpServer::FileCacheEntry* HttpServer::update_file_cache(const QString& filePath)
{
    if (!m_gzipEnabled) {
        return nullptr;
    }

    auto lastModified = QFileInfo(filePath).lastModified();

    auto entryIt = m_fileCache.find(filePath);
    if (entryIt != m_fileCache.end() && entryIt->lastModified == lastModified) {
        return &*entryIt;
    }

    FileCacheEntry entry;
    entry.lastModified = lastModified;
    entry.contentType = m_mimeDb.mimeTypeForFile(filePath).name();

    static const QStringList uncompressableTypes{
        "video/",
        "image/",
        "audio/",
        "application/x-bzip2",
        "application/gzip",
        "application/x-7z-compressed",
        "application/zip",
        "application/x-gtar",
        "application/x-rar-compressed"
    };

    for (auto& prefix : uncompressableTypes) {
        if (entry.contentType.startsWith(prefix)) {
            YOGI_LOG_TRACE(m_logger, "Not compressing " << filePath << " since it is already compressed");
            return nullptr;
        }
    }

    if (!compress_file(filePath, &entry.content)) {
        return nullptr;
    }

    entry.compressed = true;
    entryIt = m_fileCache.insert(filePath, entry);
    return &*entryIt;
}

bool HttpServer::compress_file(const QString& filePath, QByteArray* compressedContent)
{
    QStringList args;
    args << "--stdout";
    args << filePath;

    QProcess gzip;
    gzip.start(m_gzipExecutable, args);

    if (!gzip.waitForStarted(1000) || !gzip.waitForFinished(3000)) {
        YOGI_LOG_ERROR(m_logger, "Could not execute gzip: " << gzip.errorString().toStdString());
        gzip.kill();
        return false;
    }

    if (gzip.exitCode() != 0) {
        auto err = gzip.readAllStandardError();
        YOGI_LOG_ERROR(m_logger, "Could not compress file: " << err.data());
        return false;
    }

    *compressedContent = gzip.readAllStandardOutput();
    YOGI_LOG_DEBUG(m_logger, "Compressed " << filePath);
    return true;
}

HttpServer::HttpServer(const yogi::ConfigurationChild& config, QObject* parent)
: QObject(parent)
, m_logger("HTTP Server")
, m_server(new QTcpServer(this))
{
	static bool gzipChecked;
	if (!gzipChecked) {
		gzipChecked = true;

		m_gzipEnabled = yogi::ProcessInterface::config().get<bool>("http-compression.enabled");
		m_gzipExecutable = QString::fromStdString(yogi::ProcessInterface::config().get<std::string>("http-compression.gzip-executable"));

		if (m_gzipEnabled) {
			check_gzip_exists();
		}

		YOGI_LOG_INFO(m_logger, "Compression " << (m_gzipEnabled ? "enabled" : "disabled"));
	}

    m_queries = extract_map_from_config(config, "queries");
    m_routes  = extract_map_from_config(config, "routes");
    setup(config);
}

HttpServer::~HttpServer()
{
    if (ms_instances.indexOf(this) != -1) {
        ms_instances.remove(ms_instances.indexOf(this));
    }

    m_server->close();

    auto sockets = m_clients.keys();
    qDeleteAll(sockets.begin(), sockets.end());
}
