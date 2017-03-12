#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <yogi_cpp.hpp>

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QMimeDatabase>
#include <QVector>
#include <QDateTime>


class HttpServer : public QObject
{
    Q_OBJECT

    enum ReceiveState {
        RCV_REQUEST_LINE,
        RCV_HEADER,
        RCV_CONTENT,
        RCV_DONE
    };

    enum RequestType {
        REQ_GET,
        REQ_POST
    };

    struct Request {
        ReceiveState           receiveState = RCV_REQUEST_LINE;
        RequestType            type;
        QString                uri;
        QMap<QString, QString> header;
        int                    contentLength = 0;
        QByteArray             content;
    };

    struct FileCacheEntry {
        QDateTime  lastModified;
        QString    contentType;
        QByteArray content;
        bool       compressed;
    };

private:
    static QVector<HttpServer*>   ms_instances;

    yogi::Logger              m_logger;
    QMimeDatabase                 m_mimeDb;
    QTcpServer*                   m_server;
    QMap<QTcpSocket*, Request>    m_clients;
    QMap<QString, QString>        m_queries;
    QMap<QString, QString>        m_routes;
	bool						  m_gzipEnabled;
	QString                       m_gzipExecutable;
    QMap<QString, FileCacheEntry> m_fileCache;

private Q_SLOTS:
    void on_new_connection();
    void on_ready_read();
    void on_connection_closed();

private:
    QMap<QString, QString> extract_map_from_config(const yogi::ConfigurationChild& config, const char* childName);
    void setup(const yogi::ConfigurationChild& config);
    bool parse_request_line(const QString& requestLine, Request* request);
    void handle_get_request(QTcpSocket* client, const Request& request);
    void handle_post_request(QTcpSocket* client, const Request& request);
    QString uri_to_query_response(const QString& uri);
    QString uri_to_file_path(const QString& uri);
    void respond(QTcpSocket* client, int httpStatusCode, const QByteArray& content = QByteArray(),
        const QString& contentType = "text/plain", bool compressed = false);
    void respond_with_file_content(QTcpSocket* client, const QString& filePath);
	void check_gzip_exists();
    FileCacheEntry* update_file_cache(const QString& filePath);
    bool compress_file(const QString& filePath, QByteArray* compressedContent);

public:
    HttpServer(const yogi::ConfigurationChild& config, QObject* parent = Q_NULLPTR);
    ~HttpServer();

    static const QVector<HttpServer*>& instances()
    {
        return ms_instances;
    }

};

#endif // HTTPSERVER_HPP
