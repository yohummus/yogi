#ifndef WEB_SERVERS_WEBSOCKETSERVER_HH
#define WEB_SERVERS_WEBSOCKETSERVER_HH

#include "../yogi_network/YogiSession.hh"
#include "../session_services/Service.hh"

#include <yogi.hpp>

#include <QWebSocketServer>
#include <QWebSocket>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QQueue>


namespace web_servers {

class WebSocketServer : public QObject
{
    Q_OBJECT

public:
    WebSocketServer(const yogi::ConfigurationChild& config, yogi::Node& node);
    ~WebSocketServer();

    void add_service(const session_services::service_ptr& service);
    void start();

private:
    struct Client {
        yogi_network::YogiSession* session;
        QQueue<QByteArray>         notificationMessages;
    };

    const yogi::ConfigurationChild         m_config;
    yogi::Node&                            m_node;
    QVector<session_services::service_ptr> m_services;
    yogi::Logger                           m_logger;
    QWebSocketServer*                      m_server;
    QMap<QWebSocket*, Client>              m_clients;
    QTimer*                                m_updateClientsTimer;

    static QString make_client_identification(const QWebSocket* socket);
    static QByteArray make_batch_message(QByteArray msg);

    void start_listening();
    void start_update_clients_timer();

private Q_SLOTS:
    void on_new_connection();
    void on_binary_message_received(QByteArray msg);
    void on_connection_closed();
    void on_notify_client(QWebSocket* socket, QByteArray msg);
    void update_clients();
};

} // namespace web_servers

#endif // WEB_SERVERS_WEBSOCKETSERVER_HH
