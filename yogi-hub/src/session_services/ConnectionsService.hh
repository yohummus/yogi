#ifndef SESSION_SERVICES_CONNECTIONSSERVICE_HH
#define SESSION_SERVICES_CONNECTIONSSERVICE_HH

#include "Service.hh"
#include "../yogi_network/YogiSession.hh"
#include "../yogi_network/YogiTcpClient.hh"
#include "../yogi_network/YogiTcpServer.hh"
#include "../helpers/LookupTable.hh"

#include <QStringList>
#include <QMap>
#include <QProcess>


namespace session_services {

class ConnectionsService : public Service
{
    Q_OBJECT

public:
    typedef std::vector<std::shared_ptr<yogi_network::YogiTcpServer>> yogi_servers_vector;
    typedef std::vector<std::shared_ptr<yogi_network::YogiTcpClient>> yogi_clients_vector;

    static void register_factories(const yogi_servers_vector& servers, const yogi_clients_vector& clients);

    ConnectionsService(yogi_network::YogiSession& session);
    ~ConnectionsService();

    virtual request_handlers_map make_request_handlers() override;

private:
    enum ConnectionFactoryType {
        CFT_TCP_CLIENT = 0,
        CFT_TCP_SERVER
    };

    static yogi_servers_vector       ms_yogiServers;
    static yogi_clients_vector       ms_yogiClients;

    yogi_network::YogiSession&       m_session;
    yogi::Logger                     m_logger;
    bool                             m_monitoringConnections;
    QVector<QMetaObject::Connection> m_qtConnections;

    response_pair handle_connection_factories_request(const QByteArray& request);
    response_pair handle_connections_request(const QByteArray& request);
    response_pair handle_monitor_connections_request(const QByteArray& request);

    QByteArray to_byte_array(yogi_network::YogiTcpClient::ServerInformation info);
    QByteArray to_byte_array(yogi_network::YogiTcpServer::ClientInformation info);
    QByteArray make_connections_byte_array();
    char make_idx(const std::shared_ptr<yogi_network::YogiTcpClient>& client);
    char make_idx(const std::shared_ptr<yogi_network::YogiTcpServer>& server);
};

} // namespace session_services

#endif // SESSION_SERVICES_CONNECTIONSSERVICE_HH
