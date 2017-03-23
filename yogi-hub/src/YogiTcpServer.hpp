#ifndef YOGITCPSERVER_HPP
#define YOGITCPSERVER_HPP

#include <yogi.hpp>

#include <QObject>
#include <QMap>
#include <QList>
#include <QVector>
#include <QTimer>

#include <chrono>
#include <memory>
#include <atomic>


class YogiTcpServer : public QObject
{
    Q_OBJECT

public:
    struct ClientInformation {
        QString                               description;
        QString                               remoteVersion;
        QString                               remoteIdentification;
        bool                                  connected;
        std::chrono::system_clock::time_point stateChangedTime;
    };

private:
    static QVector<YogiTcpServer*>      ms_instances;

    const bool                           m_enabled;
    const QString                        m_address;
    const unsigned                       m_port;
    const QString                        m_identification;
    const std::chrono::milliseconds      m_timeout;
    yogi::Logger                     m_logger;
    yogi::Node&                      m_node;
    std::unique_ptr<yogi::TcpServer> m_server;
    std::atomic<int>                     m_activeAsyncOperations;
    QTimer*                              m_cleanupTimer;

    QMap<std::shared_ptr<yogi::TcpConnection>, ClientInformation> m_connections;
	QVector<std::shared_ptr<yogi::TcpConnection>>                 m_connectionsToDestroy;

private:
    void start_accept();
    void on_accept(const yogi::Result& res, std::shared_ptr<yogi::TcpConnection> connection);
    void on_connection_died(const yogi::Failure& failure, std::shared_ptr<yogi::TcpConnection>);

private Q_SLOTS:
    void on_connection_changed(std::shared_ptr<yogi::TcpConnection> connection, ClientInformation info);
    void on_connection_dead(std::shared_ptr<yogi::TcpConnection> connection);
    void on_periodic_cleanup();

Q_SIGNALS:
    void connection_changed(std::shared_ptr<yogi::TcpConnection> connection, ClientInformation);
    void connection_dead(std::shared_ptr<yogi::TcpConnection> connection);

public:
    YogiTcpServer(yogi::ConfigurationChild& config, yogi::Node& node);
    virtual ~YogiTcpServer();

    static const QVector<YogiTcpServer*>& instances()
    {
        return ms_instances;
    }

    bool enabled() const
    {
        return m_enabled;
    }

    QString address() const
    {
        return m_address;
    }

    unsigned port() const
    {
        return m_port;
    }

    QList<ClientInformation> connections() const
    {
        return m_connections.values();
    }
};

#endif // YOGITCPSERVER_HPP
