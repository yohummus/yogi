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
    qRegisterMetaType<weak_connection_ptr>("weak_connection_ptr");

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

void YogiTcpServer::on_accept(const yogi::Result& res, connection_ptr connection)
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

void YogiTcpServer::assign_connection(connection_ptr connection)
{
    try {
        connection->assign(m_node, m_timeout);

        auto remoteIdentification = connection->remote_identification()
                                  ? QString::fromStdString(*connection->remote_identification())
                                  : QString();

        ClientInformation info;
        info.description          = QString::fromStdString(connection->description());
        info.remoteVersion        = QString::fromStdString(connection->remote_version());
        info.remoteIdentification = remoteIdentification;
        info.connected            = true;
        info.stateChangedTime     = std::chrono::system_clock::now();

        YOGI_LOG_INFO(m_logger, "Accepted TCP connection " << connection->description()
            << " with remote YOGI version " << connection->remote_version());

        {{
            QMutexLocker lock(&m_mutex);
            m_connections.insert(connection, info);
        }}

        auto weakConnection = weak_connection_ptr(connection);
        emit(connection_changed(weakConnection, info));
    }
    catch(const yogi::Failure& failure) {
        YOGI_LOG_WARNING(m_logger, "Could not assign pending connection: " << failure);
    }
}

void YogiTcpServer::start_await_death(connection_ptr connection)
{
    try {
        auto conn = weak_connection_ptr(connection);
        connection->async_await_death([=](auto& failure) {
            this->on_connection_died(failure, conn);
        });
    }
    catch (const yogi::Failure& failure) {
        YOGI_LOG_WARNING(m_logger, "Could not wait for connection to die: " << failure);
    }
}

void YogiTcpServer::on_connection_died(const yogi::Failure& failure, weak_connection_ptr connection)
{
    if (failure == yogi::errors::Canceled()) {
        return;
    }

    {{
        auto conn = connection.lock();
        if (!conn) {
            return;
        }

        YOGI_LOG_INFO(m_logger, "Connection " << conn->description() << " died: " << failure);

        QMutexLocker lock(&m_mutex);
        auto& info = m_connections[conn];
        info.connected = false;
        info.stateChangedTime = std::chrono::system_clock::now();
    }}

    // we may not hold any shared_ptr's for connection any more at this point, otherwise this
    // function might return after handle_connection_death_in_qt_thread, resulting in a dead
    // lock since this function is called from within the async_await_dead completion handler
    QMetaObject::invokeMethod(this, "handle_connection_death_in_qt_thread", Qt::QueuedConnection,
        Q_ARG(weak_connection_ptr, connection));
}

void YogiTcpServer::handle_connection_death_in_qt_thread(weak_connection_ptr connection)
{
    auto conn = connection.lock();
    if (conn) {
        auto infoIt = m_connections.find(conn);
        emit(connection_changed(connection, *infoIt));
        m_connections.erase(infoIt);
    }
}

} // namespace yogi_network
