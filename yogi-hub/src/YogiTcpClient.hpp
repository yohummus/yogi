#ifndef YOGITCPCLIENT_HPP
#define YOGITCPCLIENT_HPP

#include <yogi.hpp>

#include <QObject>
#include <QVector>

#include <mutex>
#include <chrono>
#include <memory>


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

private:
    static QVector<YogiTcpClient*> ms_instances;

    const bool         m_enabled;
    const QString      m_host;
    const unsigned     m_port;
    yogi::Logger       m_logger;
    yogi::Endpoint&    m_endpoint;
    ServerInformation  m_info;
    mutable std::mutex m_mutex;

    std::unique_ptr<yogi::AutoConnectingTcpClient> m_client;

private:
    void on_connected(const yogi::Result& res, const std::unique_ptr<yogi::TcpConnection>& connection);
    void on_disconnected(const yogi::Failure& failure);

private Q_SLOTS:
    void on_connection_changed(ServerInformation info);
    void on_connection_dead();

Q_SIGNALS:
    void connection_changed(ServerInformation);
    void connection_dead();

public:
    YogiTcpClient(yogi::ConfigurationChild& config, yogi::Endpoint& endpoint);
    virtual ~YogiTcpClient();

    static const QVector<YogiTcpClient*>& instances()
    {
        return ms_instances;
    }

    bool enabled() const
    {
        return m_enabled;
    }

    QString host() const
    {
        return m_host;
    }

    unsigned port() const
    {
        return m_port;
    }

    ServerInformation connection() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_info;
    }
};

#endif // YOGITCPCLIENT_HPP
