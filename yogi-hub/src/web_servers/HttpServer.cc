#include "HttpServer.hh"
#include "../helpers/ostream.hh"

#include <QRegExp>
#include <QMetaObject>

#include <string>
using namespace std::string_literals;


namespace web_servers {

HttpServer::HttpServer(const yogi::ConfigurationChild& config)
: m_config(config)
, m_logger("HTTP Server")
, m_server(new QTcpServer(this))
{
    qRegisterMetaType<http_services::status_code>("http_services::status_code");

    connect(m_server, SIGNAL(newConnection()), this, SLOT(on_new_connection()));
}

HttpServer::~HttpServer()
{
    m_server->close();

    auto sockets = m_clients.keys();
    qDeleteAll(sockets.begin(), sockets.end());
}

const yogi::ConfigurationChild& HttpServer::config() const
{
    return m_config;
}

void HttpServer::add_service(const http_services::service_ptr& service, const QString& uriRoot)
{
    if (!uriRoot.startsWith('/')) {
        throw std::runtime_error("Invalid URI root.");
    }

    if (m_services.contains(uriRoot)) {
        throw std::runtime_error("Cannot add service since URI root '"s + uriRoot.toStdString() + "' is already registered.");
    }

    m_services.insert(uriRoot, service);
}

void HttpServer::start()
{
    start_listening();
}

bool HttpServer::parse_request_line(const QString& requestLine, Request* request)
{
    if (requestLine.startsWith("GET ")) {
        request->type = http_services::HTTP_GET;
    }
    else if (requestLine.startsWith("POST ")) {
        request->type = http_services::HTTP_POST;
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

const QString& HttpServer::status_code_to_header(http_services::status_code status)
{
    switch (status) {
    case http_services::HTTP_200:
        {{
            static QString line = "HTTP/1.1 200 OK";
            return line;
        }}

    case http_services::HTTP_201:
        {{
            static QString line = "HTTP/1.1 201 OK";
            return line;
        }}

    case http_services::HTTP_400:
        {{
            static QString line = "HTTP/1.1 400 Bad Request";
            return line;
        }}

    case http_services::HTTP_403:
        {{
            static QString line = "HTTP/1.1 403 Forbidden";
            return line;
        }}

    case http_services::HTTP_404:
        {{
            static QString line = "HTTP/1.1 404 Not found";
            return line;
        }}

    case http_services::HTTP_500:
        {{
            static QString line = "HTTP/1.1 500 Internal Server Error";
            return line;
        }}
    }

    throw std::runtime_error("Unsupported HTTP status code: "s + std::to_string(status));
}

void HttpServer::start_listening()
{
    auto addrStr = m_config.get<std::string>("address");
    auto address = (addrStr == "any" || addrStr == "0.0.0.0" || addrStr == "::")
                 ? QHostAddress::Any
                 : QHostAddress(QString::fromStdString(addrStr));
    auto port    = m_config.get<std::uint16_t>("port");
    auto infoStr = addrStr + ":" + std::to_string(port);

    if (m_server->listen(address, port)) {
        YOGI_LOG_INFO(m_logger, "HTTP server listening on " << infoStr);
    }
    else {
        throw std::runtime_error("Listening on "s + infoStr + " failed: " + m_server->errorString().toStdString());
    }
}

void HttpServer::handle_receive_state_request_line(QTcpSocket* client, Request* request)
{
    if (client->canReadLine()) {
        auto line = client->readLine();
        if (!parse_request_line(line, request)) {
            YOGI_LOG_WARNING(m_logger, "Invalid HTTP request line received from " << client->peerAddress().toString() << ": " << QString(line));
            client->close();
            return;
        }

        request->receiveState = RCV_HEADER;
    }
}

void HttpServer::handle_receive_state_header(QTcpSocket* client, Request* request)
{
    while (client->canReadLine()) {
        auto line = client->readLine();
        if (line.trimmed().isEmpty()) {
            request->contentLength = request->header.value("content-length", "0").toInt();
            request->receiveState = RCV_CONTENT;
            break;
        }

        int pos = line.indexOf(':');
        request->header[line.left(pos).toLower()] = line.mid(pos + 2);
    }
}

void HttpServer::handle_receive_state_content(QTcpSocket* client, Request* request)
{
    while ((request->content.size() != request->contentLength) && client->isReadable()) {
        auto sizeBeforeRead = request->content.size();
        request->content += client->read(request->contentLength - request->content.size());
        if (request->content.size() == sizeBeforeRead) {
            break;
        }
    }

    if (request->content.size() == request->contentLength) {
        request->receiveState = RCV_DONE;
    }
}

void HttpServer::handle_receive_state_done(QTcpSocket* client, Request* request)
{
    YOGI_LOG_DEBUG(m_logger, request->type << " request for " << request->uri << " from " << client->peerAddress().toString());
    bool handled = false;
    for (auto& uriRoot : m_services.keys()) {
        if (uriRoot != "/") {
            handled = try_redirect_to_service(uriRoot, client, request);
            if (handled) {
                break;
            }
        }
    }

    if (!handled && m_services.contains("/")) {
        handled = try_redirect_to_service("/", client, request);
    }

    if (!handled) {
        YOGI_LOG_WARNING(m_logger, "Could not handle " << request->type << " request for " << request->uri << ": Bad request");
        respond(client, http_services::HTTP_400);
    }

    *request = {};
}

bool HttpServer::try_redirect_to_service(const QString& uriRoot, QTcpSocket* client, Request* request)
{
    if (request->uri != uriRoot && !request->uri.startsWith(uriRoot == "/" ? uriRoot : uriRoot + '/')) {
        return false;
    }

    auto path = request->uri.mid(uriRoot.size());
    if (!path.startsWith('/')) {
        path.prepend('/');
    }

    try {
        m_services.value(uriRoot)->async_handle_request(request->type, path, request->header, request->content,
            [=](auto status, auto& content, auto& contentType, bool compressed) {
                QMetaObject::invokeMethod(this, "respond", Qt::AutoConnection,
                    Q_ARG(QTcpSocket*, client),
                    Q_ARG(http_services::status_code, status),
                    Q_ARG(QByteArray, content),
                    Q_ARG(QString, contentType),
                    Q_ARG(bool, compressed)
                );
            }
        );
    }
    catch (const std::exception& e) {
        respond(client, http_services::HTTP_500, e.what());
    }

    return true;
}

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
        handle_receive_state_request_line(client, &request);
    }

    if (request.receiveState == RCV_HEADER) {
        handle_receive_state_header(client, &request);
    }

    if (request.receiveState == RCV_CONTENT) {
        handle_receive_state_content(client, &request);
    }

    if (request.receiveState == RCV_DONE) {
        handle_receive_state_done(client, &request);
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

void HttpServer::respond(QTcpSocket* client, http_services::status_code status, QByteArray content, QString contentType, bool compressed)
{
    QStringList header;

    header += status_code_to_header(status);
    header += "Access-Control-Allow-Origin: *";

    if (contentType.isEmpty()) {
        contentType = "text/plain";
    }

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
        YOGI_LOG_WARNING(m_logger, "Not all data could be sent to " << client->peerAddress().toString());
        client->close();
    }
}

} // namespace web_servers
