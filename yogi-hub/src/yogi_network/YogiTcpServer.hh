#ifndef YOGI_NETWORK_YOGITCPSERVER_HH
#define YOGI_NETWORK_YOGITCPSERVER_HH

#include <yogi.hpp>

#include <QObject>
#include <QMap>
#include <QList>
#include <QVector>
#include <QMutex>

#include <chrono>
#include <memory>
#include <atomic>


namespace yogi_network {

class YogiTcpServer : public QObject
{
    Q_OBJECT

public:
    typedef std::weak_ptr<yogi::TcpConnection> weak_connection_ptr;

    struct ClientInformation {
        QString                               description;
        QString                               remoteVersion;
        QString                               remoteIdentification;
        bool                                  connected;
        std::chrono::system_clock::time_point stateChangedTime;
    };

    YogiTcpServer(yogi::ConfigurationChild& config, yogi::Node& node);
    virtual ~YogiTcpServer();

    QString address() const;
    unsigned port() const;
    QList<ClientInformation> connections() const;

Q_SIGNALS:
    void connection_changed(weak_connection_ptr, ClientInformation);

private:
    typedef std::shared_ptr<yogi::TcpConnection>    connection_ptr;
    typedef QMap<connection_ptr, ClientInformation> connections_map;

    const QString                    m_address;
    const unsigned                   m_port;
    const QString                    m_identification;
    const std::chrono::milliseconds  m_timeout;
    yogi::Logger                     m_logger;
    yogi::Node&                      m_node;
    std::unique_ptr<yogi::TcpServer> m_server;
    connections_map                  m_connections;
    mutable QMutex                   m_mutex;

private:
    void start_accept();
    void on_accept(const yogi::Result& res, connection_ptr connection);
    void assign_connection(connection_ptr connection);
    void start_await_death(connection_ptr connection);
    void on_connection_died(const yogi::Failure& failure, weak_connection_ptr connection);

private Q_SLOTS:
    void handle_connection_death_in_qt_thread(weak_connection_ptr connection);
};

} // namespace yogi_network

#endif // YOGI_NETWORK_YOGITCPSERVER_HH
