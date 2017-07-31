#include "YogiTcpServer.hh"
#include "../helpers/time.hh"
#include "../helpers/ostream.hh"

#include <QMetaObject>

#include <yogi_core.h>


namespace yogi_network {

YogiTcpServer::YogiTcpServer(yogi::ConfigurationChild& config, yogi::Node& node)
: m_address(QString::fromStdString(config.get<std::string>("address")))
, m_port(config.get<unsigned>("port"))
, m_identification(QString::fromStdString(config.get<std::string>("identification")))
, m_timeout(helpers::float_to_timeout(config.get<float>("timeout", 0)))
, m_logger("Yogi TCP Server")
, m_node(node)
{
    m_server = std::make_unique<yogi::TcpServer>(node.scheduler(), m_address.toStdString(), m_port, config.get_optional<std::string>("identification"));

    qRegisterMetaType<ClientInformation>("ClientInformation");
    qRegisterMetaType<std::shared_ptr<yogi::TcpConnection>>("std::shared_ptr<yogi::TcpConnection>");
    qRegisterMetaType<std::weak_ptr<yogi::TcpConnection>>("std::weak_ptr<yogi::TcpConnection>");

    start_accept();

    YOGI_LOG_INFO(m_logger, "YOGI TCP server listening on address " << m_address << " port " << m_port);
}

YogiTcpServer::~YogiTcpServer()
{
    m_server.reset();
    m_connections.clear();
}

QString YogiTcpServer::address() const
{
    return m_address;
}

unsigned YogiTcpServer::port() const
{
    return m_port;
}

QList<YogiTcpServer::ClientInformation> YogiTcpServer::connections() const
{
    QMutexLocker lock(&m_mutex);
    return m_connections.values();
}

void YogiTcpServer::start_accept()
{
    m_server->async_accept(m_timeout, [=](auto& result, auto connection) {
        this->on_accept(result, std::shared_ptr<yogi::TcpConnection>(connection.release()));
    });
}

void YogiTcpServer::on_accept(const yogi::Result& res, std::shared_ptr<yogi::TcpConnection> connection)
{
    if (res == yogi::errors::Canceled()) {
        return;
    }

    if (res) {
        start_await_death(connection);
        assign_connection(connection);
        start_accept();
    }
    else if (res == yogi::errors::Timeout()) {
        YOGI_LOG_WARNING(m_logger, "Accepting TCP connection failed: " << res);
        start_accept();
    }
    else {
        YOGI_LOG_ERROR(m_logger, "Accepting TCP connection failed: " << res);
    }
}

void YogiTcpServer::assign_connection(std::shared_ptr<yogi::TcpConnection> connection)
{
    try {
        connection->assign(m_node, m_timeout);

        ClientInformation info;
        info.description          = QString::fromStdString(connection->description());
        info.remoteVersion        = QString::fromStdString(connection->remote_version());
        info.remoteIdentification = connection->remote_identification() ? QString::fromStdString(*connection->remote_identification()) : QString();
        info.connected            = true;
        info.stateChangedTime     = std::chrono::system_clock::now();

        YOGI_LOG_INFO(m_logger, "Accepted TCP connection " << connection->description()
            << " with remote YOGI version " << connection->remote_version());

        {{
            QMutexLocker lock(&m_mutex);
            m_connections.insert(connection, info);
        }}

        auto weakConnection = std::weak_ptr<yogi::TcpConnection>(connection);
        emit(connection_changed(weakConnection, info));
    }
    catch(const yogi::Failure& failure) {
        YOGI_LOG_WARNING(m_logger, "Could not assign pending connection: " << failure);
    }
}

void YogiTcpServer::start_await_death(std::shared_ptr<yogi::TcpConnection> connection)
{
    try {
        connection->async_await_death([=](auto& failure) {
            this->on_connection_died(failure, connection);
        });
    }
    catch (const yogi::Failure& failure) {
        YOGI_LOG_WARNING(m_logger, "Could not wait for connection to die: " << failure);
    }
}

void YogiTcpServer::on_connection_died(const yogi::Failure& failure, std::shared_ptr<yogi::TcpConnection> connection)
{
    if (failure != yogi::errors::Canceled()) {
        YOGI_LOG_INFO(m_logger, "Connection " << connection->description() << " died: " << failure);

        ClientInformation info;

        {{
            QMutexLocker lock(&m_mutex);
            info = m_connections.take(connection);
        }}

        info.connected = false;
        info.stateChangedTime = std::chrono::system_clock::now();

        auto weakConnection = std::weak_ptr<yogi::TcpConnection>(connection);
        emit(connection_changed(weakConnection, info));

        QMetaObject::invokeMethod(this, "destroy_connection_later", Qt::QueuedConnection,
            Q_ARG(std::shared_ptr<yogi::TcpConnection>, connection));
    }
}

void YogiTcpServer::destroy_connection_later(std::shared_ptr<yogi::TcpConnection>)
{
    // the shared pointer will go out of scope once the caller of this function
    // returns which will destroy the connection
}

} // namespace yogi_network
