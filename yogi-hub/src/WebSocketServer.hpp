#ifndef WEBSOCKETSERVER_HPP
#define WEBSOCKETSERVER_HPP

#include <yogi.hpp>

#include <QWebSocketServer>
#include <QWebSocket>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QQueue>


class YogiSession;

class WebSocketServer : public QObject
{
    Q_OBJECT

    struct Client {
        YogiSession*       session;
        QQueue<QByteArray> notificationMessages;
    };

private:
    static QVector<WebSocketServer*> ms_instances;

    yogi::Node&               m_node;
    yogi::Logger              m_logger;
    QWebSocketServer*         m_server;
    QMap<QWebSocket*, Client> m_clients;
    QTimer*                   m_updateClientsTimer;

private:
    QByteArray make_batch_message(QByteArray msg);

private Q_SLOTS:
    void on_new_connection();
    void on_binary_message_received(QByteArray msg);
    void on_connection_closed();
    void on_notify_client(QWebSocket* socket, QByteArray msg);
    void update_clients();

public:
    static const QVector<WebSocketServer*>& instances()
    {
        return ms_instances;
    }

    WebSocketServer(const yogi::ConfigurationChild& config, yogi::Node& node, QObject* parent = Q_NULLPTR);
    ~WebSocketServer();
};

#endif // WEBSOCKETSERVER_HPP
