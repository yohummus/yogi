#include "FileService.hh"
#include "../helpers/config.hh"
#include "../helpers/ostream.hh"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QProcess>
#include <QMutex>
#include <QMimeDatabase>

using namespace std::string_literals;


namespace http_services {



FileService::FileService(const yogi::ConfigurationChild& config)
: m_logger("File Service")
, m_routes(helpers::extract_map_from_config(config, "routes"))
{
    setup_gzip();
}

void FileService::async_handle_request(request_type type, const QString& path,
	const QMap<QString, QString>& header, const QByteArray& content, completion_handler completionHandler)
{
	if (type != HTTP_GET) {
		completionHandler(HTTP_404, {}, {}, false);
        return;
	}

    auto uri = path;
    if (uri == "/") {
        uri = "/index.html";
    };

    auto filePath = uri_to_file_path(uri);
    if (filePath.isNull()) {
        completionHandler(HTTP_404, "404 Not found", {}, false);
        return;
    }

    respond_with_file_content(filePath, completionHandler);
}

QString FileService::get_mime_type_for_file(const QString& filePath)
{
    static QMimeDatabase db;
    return db.mimeTypeForFile(filePath).name();
}

void FileService::setup_gzip()
{
    m_gzipEnabled = yogi::ProcessInterface::config().get<bool>("http-compression.enabled");
    m_gzipExecutable = QString::fromStdString(yogi::ProcessInterface::config().get<std::string>("http-compression.gzip-executable"));

    static bool logged = false;
    if (!logged) {
        YOGI_LOG_INFO(m_logger, "Compression " << (m_gzipEnabled ? "enabled" : "disabled"));
        logged = true;
    }

    if (m_gzipEnabled) {
        check_gzip_exists();
    }
}

void FileService::check_gzip_exists()
{
    if (!QFile::exists(m_gzipExecutable)) {
		throw std::runtime_error("Cannot find gzip executable '"s + m_gzipExecutable.toStdString() + "'");
	}
}

QString FileService::uri_to_file_path(const QString& uri)
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

FileService::FileCacheEntry* FileService::update_file_cache(const QString& filePath)
{
    static QMutex mutex;
    static QMap<QString, FileCacheEntry> cache;

    if (!m_gzipEnabled) {
        return nullptr;
    }

    auto lastModified = QFileInfo(filePath).lastModified();

    {{
        QMutexLocker lock(&mutex);
        auto entryIt = cache.find(filePath);
        if (entryIt != cache.end() && entryIt->lastModified == lastModified) {
            return &*entryIt;
        }
    }}

    FileCacheEntry entry;
    entry.lastModified = lastModified;
    entry.contentType = get_mime_type_for_file(filePath);

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

    QMutexLocker lock(&mutex);
    auto entryIt = cache.insert(filePath, entry);

    return &*entryIt;
}

void FileService::respond_with_file_content(const QString& filePath, completion_handler completionHandler)
{
    auto lastModified = QFileInfo(filePath).lastModified();

    auto cacheEntry = update_file_cache(filePath);
    if (cacheEntry) {
        YOGI_LOG_TRACE(m_logger, "Serving file " << filePath << (cacheEntry->compressed ? " with gzip" : " without") << " compression...");
        completionHandler(HTTP_200, cacheEntry->content, cacheEntry->contentType, cacheEntry->compressed);
    }
    else {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            YOGI_LOG_TRACE(m_logger, "Serving file " << filePath << " without compression...");
            auto contentType = get_mime_type_for_file(filePath);
            completionHandler(HTTP_200, file.readAll(), contentType, false);
        }
        else {
            YOGI_LOG_WARNING(m_logger, "Could not open " << filePath << ": " << file.errorString());
            completionHandler(HTTP_403, {}, {}, false);
        }
    }
}

bool FileService::compress_file(const QString& filePath, QByteArray* compressedContent)
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
    YOGI_LOG_TRACE(m_logger, "Compressed " << filePath);

    return true;
}

} // namespace http_services
