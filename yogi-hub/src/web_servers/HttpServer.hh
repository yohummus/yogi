#ifndef WEB_SERVERS_HTTPSERVER_HH
#define WEB_SERVERS_HTTPSERVER_HH

#include "../http_services/Service.hh"

#include <QTcpServer>
#include <QTcpSocket>

#include <yogi.hpp>


namespace web_servers {

class HttpServer : public QObject
{
    Q_OBJECT

public:
    HttpServer(const yogi::ConfigurationChild& config);
    ~HttpServer();

    const yogi::ConfigurationChild& config() const;
    void add_service(const http_services::service_ptr& service, const QString& uriRoot);
    void start();

private:
    enum receive_state {
        RCV_REQUEST_LINE,
        RCV_HEADER,
        RCV_CONTENT,
        RCV_DONE
    };

    struct Request {
        receive_state               receiveState = RCV_REQUEST_LINE;
        http_services::request_type type;
        QString                     uri;
        QMap<QString, QString>      header;
        int                         contentLength = 0;
        QByteArray                  content;
    };

    const yogi::ConfigurationChild            m_config;
    QMap<QString, http_services::service_ptr> m_services;
    yogi::Logger                              m_logger;
    QTcpServer*                               m_server;
    QMap<QTcpSocket*, Request>                m_clients;

    static bool parse_request_line(const QString& requestLine, Request* request);
    static const QString& status_code_to_header(http_services::status_code status);

    void start_listening();
    void handle_receive_state_request_line(QTcpSocket* client, Request* request);
    void handle_receive_state_header(QTcpSocket* client, Request* request);
    void handle_receive_state_content(QTcpSocket* client, Request* request);
    void handle_receive_state_done(QTcpSocket* client, Request* request);
    bool try_redirect_to_service(const QString& uriRoot, QTcpSocket* client, Request* request);

private Q_SLOTS:
    void on_new_connection();
    void on_ready_read();
    void on_connection_closed();
    void respond(QTcpSocket* client, http_services::status_code status, QByteArray content = QByteArray(),
        QString contentType = "text/plain", bool compressed = false);
};

} // namespace web_servers

#endif // WEB_SERVERS_HTTPSERVER_HH
