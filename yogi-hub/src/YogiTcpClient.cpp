#include "YogiTcpClient.hpp"
#include "helpers.hpp"

#include <QtDebug>

#include <yogi_core.h>


QVector<YogiTcpClient*> YogiTcpClient::ms_instances;


void YogiTcpClient::on_connected(const yogi::Result& res, const std::unique_ptr<yogi::TcpConnection>& connection)
{
    if (res) {
        ServerInformation info;
        info.description          = QString::fromStdString(connection->description());
        info.remoteVersion        = QString::fromStdString(connection->remote_version());
        info.remoteIdentification = connection->remote_identification() ? QString::fromStdString(*connection->remote_identification()) : QString();
        info.connected            = true;
        info.stateChangedTime     = std::chrono::system_clock::now();

        YOGI_LOG_INFO(m_logger, "Connected to " << m_host << " port " << m_port);

        emit(connection_changed(info));
    }
    else if (res != yogi::errors::Canceled()) {
        YOGI_LOG_DEBUG(m_logger, "Could not connect to " << m_host << " port " << m_port << ": " << res);
    }
}

void YogiTcpClient::on_disconnected(const yogi::Failure& failure)
{
    YOGI_LOG_ERROR(m_logger, "Connection to " << m_host << " port " << m_port << " died: " << failure);
    emit(connection_dead());
}

void YogiTcpClient::on_connection_changed(ServerInformation info)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_info = info;
}

void YogiTcpClient::on_connection_dead()
{
    auto info = m_info;
    info.connected = false;
    info.stateChangedTime = std::chrono::system_clock::now();

    emit(connection_changed(info));
}

YogiTcpClient::YogiTcpClient(yogi::ConfigurationChild& config, yogi::Endpoint& endpoint)
: m_enabled(config.get<bool>("enabled"))
, m_host(QString::fromStdString(config.get<std::string>("host")))
, m_port(config.get<unsigned>("port"))
, m_logger("YOGI TCP Client")
, m_endpoint(endpoint)
{
    if (!m_enabled) {
        YOGI_LOG_DEBUG(m_logger, "Disabled YOGI TCP client connecting to address " << m_host << " port " << m_port);
        return;
    }

    m_client = std::make_unique<yogi::AutoConnectingTcpClient>(endpoint, m_host.toStdString(), m_port,
        helpers::float_to_timeout(config.get<float>("timeout", 0)), config.get_optional<std::string>("identification"));

    qRegisterMetaType<ServerInformation>("ServerInformation");
    connect(this, &YogiTcpClient::connection_changed, this, &YogiTcpClient::on_connection_changed);
    connect(this, &YogiTcpClient::connection_dead,    this, &YogiTcpClient::on_connection_dead);

    m_info.connected = false;
    m_info.stateChangedTime = std::chrono::system_clock::time_point(); // defaults to epoch

    ms_instances.push_back(this);

    m_client->set_connect_observer([&](auto& result, auto& connection) {
        this->on_connected(result, connection);
    });
    m_client->set_disconnect_observer([&](auto& failure) {
        this->on_disconnected(failure);
    });
    m_client->start();

    YOGI_LOG_INFO(m_logger, "YOGI TCP client connecting to host " << m_host << " port " << m_port);
}

YogiTcpClient::~YogiTcpClient()
{
    if (ms_instances.indexOf(this) != -1) {
        ms_instances.remove(ms_instances.indexOf(this));
    }

    m_client.reset();
}
