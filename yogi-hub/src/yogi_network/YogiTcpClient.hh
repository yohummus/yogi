#ifndef YOGI_NETWORK_YOGITCPCLIENT_HH
#define YOGI_NETWORK_YOGITCPCLIENT_HH

#include <yogi.hpp>

#include <QObject>

#include <mutex>
#include <chrono>
#include <memory>


namespace yogi_network {

class YogiTcpClient : public QObject
{
    Q_OBJECT

public:
    struct ServerInformation {
        QString                               description;
        QString                               remoteVersion;
        QString                               remoteIdentification;
        bool                                  connected;
        std::chrono::system_clock::time_point stateChangedTime;
    };

    YogiTcpClient(yogi::ConfigurationChild& config, yogi::Endpoint& endpoint);
    virtual ~YogiTcpClient();

    QString host() const;
    unsigned port() const;
    ServerInformation connection() const;

Q_SIGNALS:
    void connection_changed(ServerInformation);
    void connection_dead();

private:
    const QString                                  m_host;
    const unsigned                                 m_port;
    yogi::Logger                                   m_logger;
    yogi::Endpoint&                                m_endpoint;
    ServerInformation                              m_info;
    mutable std::mutex                             m_mutex;
    std::unique_ptr<yogi::AutoConnectingTcpClient> m_client;

    void on_connected(const yogi::Result& res, const std::unique_ptr<yogi::TcpConnection>& connection);
    void on_disconnected(const yogi::Failure& failure);

private Q_SLOTS:
    void on_connection_changed(ServerInformation info);
    void on_connection_dead();
};

} // namespace yogi_network

#endif // YOGI_NETWORK_YOGITCPCLIENT_HH
