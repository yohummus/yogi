#ifndef WEB_SERVERS_WEBSOCKETSERVER_HH
#define WEB_SERVERS_WEBSOCKETSERVER_HH

#include "../yogi_network/YogiSession.hh"

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
    typedef std::vector<std::shared_ptr<yogi_network::YogiTcpServer>> yogi_servers_vector;
    typedef std::vector<std::shared_ptr<yogi_network::YogiTcpClient>> yogi_clients_vector;

    WebSocketServer(const yogi::ConfigurationChild& config, yogi::Node& node,
        const yogi_servers_vector& yogiServers, const yogi_clients_vector& yogiClients);
    ~WebSocketServer();

private:
    struct Client {
        yogi_network::YogiSession* session;
        QQueue<QByteArray>         notificationMessages;
    };

    yogi::Node&               m_node;
    const yogi_servers_vector m_yogiServers;
    const yogi_clients_vector m_yogiClients;
    yogi::Logger              m_logger;
    QWebSocketServer*         m_server;
    QMap<QWebSocket*, Client> m_clients;
    QTimer*                   m_updateClientsTimer;

    static QString make_client_identification(const QWebSocket* socket);
    static QByteArray make_batch_message(QByteArray msg);

private Q_SLOTS:
    void on_new_connection();
    void on_binary_message_received(QByteArray msg);
    void on_connection_closed();
    void on_notify_client(QWebSocket* socket, QByteArray msg);
    void update_clients();
};

} // namespace web_servers

#endif // WEB_SERVERS_WEBSOCKETSERVER_HH
