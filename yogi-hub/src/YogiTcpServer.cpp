#include "YogiTcpServer.hpp"
#include "helpers.hpp"

#include <QtDebug>

#include <yogi_core.h>


QVector<YogiTcpServer*> YogiTcpServer::ms_instances;

void YogiTcpServer::start_accept()
{
    ++m_activeAsyncOperations;

    try {
        m_server->async_accept(m_timeout, [=](auto& result, auto connection) {
            this->on_accept(result, std::shared_ptr<yogi::TcpConnection>(connection.release()));
        });
    }
    catch (...) {
        --m_activeAsyncOperations;
        throw;
    }
}

void YogiTcpServer::on_accept(const yogi::Result& res, std::shared_ptr<yogi::TcpConnection> connection)
{
    if (res) {
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

            emit(connection_changed(connection, info));
        }
        catch(const yogi::Failure& failure) {
            YOGI_LOG_WARNING(m_logger, "Could not assign pending connection: " << failure);
        }

        try {
            connection->async_await_death([=](auto& failure) {
                this->on_connection_died(failure, connection);
            });
            ++m_activeAsyncOperations;
        }
        catch(const yogi::Failure& failure) {
            YOGI_LOG_WARNING(m_logger, "Could not assign pending connection: " << failure);
        }
    }
    else if (res == yogi::errors::Timeout()) {
        YOGI_LOG_WARNING(m_logger, "Accepting TCP connection failed: " << res);
    }
    else if (res == yogi::errors::Canceled()) {
        --m_activeAsyncOperations;
        return;
    }
    else {
        YOGI_LOG_FATAL(m_logger, "Accepting TCP connection failed: " << res);
        --m_activeAsyncOperations;
        return;
    }

    start_accept();

    --m_activeAsyncOperations;
}

void YogiTcpServer::on_connection_died(const yogi::Failure& failure, std::shared_ptr<yogi::TcpConnection> connection)
{
    if (failure != yogi::errors::Canceled()) {
        YOGI_LOG_INFO(m_logger, "Connection " << connection->description() << " died: " << failure);
        emit(connection_dead(connection));
    }

    --m_activeAsyncOperations;
}

void YogiTcpServer::on_connection_changed(std::shared_ptr<yogi::TcpConnection> connection, ClientInformation info)
{
    if (info.connected) {
        m_connections.insert(connection, info);
    }
}

void YogiTcpServer::on_connection_dead(std::shared_ptr<yogi::TcpConnection> connection)
{
    auto info = m_connections.take(connection);
    info.connected = false;
    info.stateChangedTime = std::chrono::system_clock::now();

    m_connectionsToDestroy.push_back(connection);

    emit(connection_changed(connection, info));
}

void YogiTcpServer::on_periodic_cleanup()
{
    for (auto it = m_connectionsToDestroy.begin(); it != m_connectionsToDestroy.end(); ) {
        if (it->unique()) {
            YOGI_LOG_DEBUG(m_logger, "Destroying dead connection " << (*it)->description());
            it = m_connectionsToDestroy.erase(it);
        }
        else {
            ++it;
        }
    }
}

YogiTcpServer::YogiTcpServer(yogi::ConfigurationChild& config, yogi::Node& node)
: m_enabled(config.get<bool>("enabled"))
, m_address(QString::fromStdString(config.get<std::string>("address")))
, m_port(config.get<unsigned>("port"))
, m_identification(QString::fromStdString(config.get<std::string>("identification")))
, m_timeout(helpers::float_to_timeout(config.get<float>("timeout", 0)))
, m_logger("YOGI TCP Server")
, m_node(node)
, m_activeAsyncOperations(0)
, m_cleanupTimer(new QTimer(this))
{
    if (!m_enabled) {
        YOGI_LOG_DEBUG(m_logger, "Disabled YOGI TCP server listening on address " << m_address << " port " << m_port);
        return;
    }

    m_server = std::make_unique<yogi::TcpServer>(node.scheduler(), m_address.toStdString(), m_port, config.get_optional<std::string>("identification"));

    qRegisterMetaType<ClientInformation>("ClientInformation");
    qRegisterMetaType<std::shared_ptr<yogi::TcpConnection>>("std::shared_ptr<yogi::TcpConnection>");
    connect(this, &YogiTcpServer::connection_changed, this, &YogiTcpServer::on_connection_changed);
    connect(this, &YogiTcpServer::connection_dead,    this, &YogiTcpServer::on_connection_dead);
    connect(m_cleanupTimer, &QTimer::timeout, this, &YogiTcpServer::on_periodic_cleanup);

    ms_instances.push_back(this);

    start_accept();
    m_cleanupTimer->start(500);

    YOGI_LOG_INFO(m_logger, "YOGI TCP server listening on address " << m_address << " port " << m_port);
}

YogiTcpServer::~YogiTcpServer()
{
    if (ms_instances.indexOf(this) != -1) {
        ms_instances.remove(ms_instances.indexOf(this));
    }

    m_server->cancel_accept();
    m_connections.clear();

    while (m_activeAsyncOperations) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
