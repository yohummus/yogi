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
    static const QVector<WebSocketServer*>& instances();

    WebSocketServer(const yogi::ConfigurationChild& config, yogi::Node& node, QObject* parent = Q_NULLPTR);
    ~WebSocketServer();

private:
    struct Client {
        yogi_network::YogiSession* session;
        QQueue<QByteArray>         notificationMessages;
    };

    static QVector<WebSocketServer*> ms_instances;

    yogi::Node&                      m_node;
    yogi::Logger                     m_logger;
    QWebSocketServer*                m_server;
    QMap<QWebSocket*, Client>        m_clients;
    QTimer*                          m_updateClientsTimer;

    QByteArray make_batch_message(QByteArray msg);

private Q_SLOTS:
    void on_new_connection();
    void on_binary_message_received(QByteArray msg);
    void on_connection_closed();
    void on_notify_client(QWebSocket* socket, QByteArray msg);
    void update_clients();
};

} // namespace web_servers

#endif // WEB_SERVERS_WEBSOCKETSERVER_HH
