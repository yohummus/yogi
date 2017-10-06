#include "YogiTcpClient.hh"
#include "../helpers/ostream.hh"
#include "../helpers/time.hh"

#include <QtDebug>

#include <yogi_core.h>


namespace yogi_network {

YogiTcpClient::YogiTcpClient(yogi::ConfigurationChild& config, yogi::Endpoint& endpoint)
: m_host(QString::fromStdString(config.get<std::string>("host")))
, m_port(config.get<unsigned>("port"))
, m_logger("Yogi TCP Client")
, m_endpoint(endpoint)
{
    m_client = std::make_unique<yogi::AutoConnectingTcpClient>(endpoint, m_host.toStdString(), m_port,
        helpers::float_to_timeout(config.get<float>("timeout", 0)), config.get_optional<std::string>("identification"));

    qRegisterMetaType<ServerInformation>("ServerInformation");

    m_info.connected = false;
    m_info.stateChangedTime = std::chrono::system_clock::time_point(); // defaults to epoch

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
    m_client.reset();
}

QString YogiTcpClient::host() const
{
    return m_host;
}

unsigned YogiTcpClient::port() const
{
    return m_port;
}

YogiTcpClient::ServerInformation YogiTcpClient::connection() const
{
    QMutexLocker lock(&m_mutex);
    return m_info;
}

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

        QMutexLocker lock(&m_mutex);
        m_info = info;

        emit(connection_changed(info));
    }
    else if (res != yogi::errors::Canceled()) {
        YOGI_LOG_DEBUG(m_logger, "Could not connect to " << m_host << " port " << m_port << ": " << res);
    }
}

void YogiTcpClient::on_disconnected(const yogi::Failure& failure)
{
    YOGI_LOG_ERROR(m_logger, "Connection to " << m_host << " port " << m_port << " died: " << failure);

    QMutexLocker lock(&m_mutex);

    m_info.connected = false;
    m_info.stateChangedTime = std::chrono::system_clock::now();

    emit(connection_changed(m_info));
}

} // namespace yogi_network
