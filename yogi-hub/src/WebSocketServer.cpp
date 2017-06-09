#include "helpers.hpp"
#include "WebSocketServer.hpp"
#include "YogiSession.hpp"


namespace {

QString make_client_identification(const QWebSocket* socket)
{
    return socket->peerAddress().toString() + ':' + QString::number(socket->peerPort());
}

} // anonymous namespace

QVector<WebSocketServer*> WebSocketServer::ms_instances;

QByteArray WebSocketServer::make_batch_message(QByteArray msg)
{
    return helpers::to_byte_array(msg.size()) + msg;
}

void WebSocketServer::on_new_connection()
{
    auto socket = m_server->nextPendingConnection();

    connect(socket, SIGNAL(binaryMessageReceived(const QByteArray&)), this, SLOT(on_binary_message_received(const QByteArray&)));
    connect(socket, SIGNAL(disconnected()),                           this, SLOT(on_connection_closed()));

    Client client;
    client.session = new YogiSession(socket, m_node, make_client_identification(socket), this);
    m_clients.insert(socket, client);

    connect(client.session, SIGNAL(notify_client(QWebSocket*, QByteArray)),
        this, SLOT(on_notify_client(QWebSocket*, QByteArray)));
    YOGI_LOG_INFO(m_logger, "WS client " << make_client_identification(socket) << " connected");
}

void WebSocketServer::on_binary_message_received(QByteArray msg)
{
    auto client = qobject_cast<QWebSocket*>(sender());
	assert(m_clients.count(client));
    auto response = m_clients[client].session->handle_request(msg);
    client->sendBinaryMessage(make_batch_message(response));
}

void WebSocketServer::on_connection_closed()
{
    auto client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        YOGI_LOG_DEBUG("WS client " << make_client_identification(client) << " disconnected.");
        auto it = m_clients.find(client);
        it.value().session->deleteLater();
        client->deleteLater();
        m_clients.erase(it);
    }
}

void WebSocketServer::on_notify_client(QWebSocket* socket, QByteArray msg)
{
    if (m_updateClientsTimer->isActive()) {
		auto clientIt = m_clients.find(socket);
		if (clientIt != m_clients.end()) {
			clientIt->notificationMessages.enqueue(msg);
		}
    }
    else {
        socket->sendBinaryMessage(make_batch_message(msg));
    }
}

void WebSocketServer::update_clients()
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        auto& socket = it.key();
        auto& messages = it->notificationMessages;

        QByteArray batchMessage;
        while (!messages.empty()) {
            batchMessage += make_batch_message(messages.dequeue());
        }

		if (!batchMessage.isEmpty()) {
			socket->sendBinaryMessage(batchMessage);
		}
    }
}

WebSocketServer::WebSocketServer(const yogi::ConfigurationChild& config, yogi::Node& node, QObject* parent)
: QObject(parent)
, m_node(node)
, m_logger("WS Server")
, m_server(new QWebSocketServer("YOGI Hub", QWebSocketServer::NonSecureMode, this))
, m_updateClientsTimer(new QTimer(this))
{
    auto addrStr = config.get<std::string>("address");
    auto address = (addrStr == "any" || addrStr == "0.0.0.0" || addrStr == "::") ? QHostAddress::Any : QHostAddress(QString::fromStdString(addrStr));
    auto port    = config.get<std::uint16_t>("port");
    auto infoStr = addrStr + ":" + std::to_string(port);

    if (!config.get<bool>("enabled")) {
        YOGI_LOG_DEBUG(m_logger, "Disabled WS server listening on " << infoStr);
        return;
    }

    if (m_server->listen(address, port)) {
        YOGI_LOG_INFO(m_logger, "WS server listening on " << infoStr);
        connect(m_server, SIGNAL(newConnection()), this, SLOT(on_new_connection()));
        ms_instances.push_back(this);
    }

    connect(m_updateClientsTimer, SIGNAL(timeout()), this, SLOT(update_clients()));
    auto interval = config.get<float>("update-interval");
    if (interval > 0.0) {
        int n = static_cast<int>(interval * 1000);
        m_updateClientsTimer->setInterval(std::max(n, 1));
        m_updateClientsTimer->start();
    }
}

WebSocketServer::~WebSocketServer()
{
    if (ms_instances.indexOf(this) != -1) {
        ms_instances.remove(ms_instances.indexOf(this));
    }

    m_server->close();

    auto sockets = m_clients.keys();
    qDeleteAll(sockets.begin(), sockets.end());

    for (auto& client : m_clients) {
        delete client.session;
    }
}
