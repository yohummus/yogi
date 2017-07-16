#include "ConnectionsService.hh"
#include "../helpers/to_byte_array.hh"
#include "../helpers/read_from_stream.hh"
#include "../helpers/ostream.hh"
#include "../helpers/time.hh"

#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QDataStream>

using namespace std::string_literals;


namespace session_services {

void ConnectionsService::register_factories(const yogi_servers_vector& servers, const yogi_clients_vector& clients)
{
    ms_yogiServers = servers;
    ms_yogiClients = clients;
}

ConnectionsService::ConnectionsService(yogi_network::YogiSession& session)
: m_session(session)
, m_logger("Connections Service")
, m_monitoringConnections(false)
{
}

ConnectionsService::~ConnectionsService()
{
    for (auto connection : m_qtConnections) {
        disconnect(connection);
    }
}

ConnectionsService::request_handlers_map ConnectionsService::make_request_handlers()
{
    return {{
        REQ_CONNECTION_FACTORIES, [this](auto request) {
            return this->handle_connection_factories_request(request);
        }}, {
        REQ_CONNECTIONS, [this](auto request) {
            return this->handle_connections_request(request);
        }}, {
        REQ_MONITOR_CONNECTIONS, [this](auto request) {
            return this->handle_monitor_connections_request(request);
        }}
    };
}

ConnectionsService::yogi_servers_vector ConnectionsService::ms_yogiServers;
ConnectionsService::yogi_clients_vector ConnectionsService::ms_yogiClients;

ConnectionsService::response_pair ConnectionsService::handle_connection_factories_request(QByteArray* request)
{
    QByteArray data;

    for (auto client : ms_yogiClients) {
        data += static_cast<char>(CFT_TCP_CLIENT);
        data += make_idx(client);
        data += helpers::to_byte_array(client->port());
        data += client->host() + '\0';
    }

    for (auto server : ms_yogiServers) {
        data += static_cast<char>(CFT_TCP_SERVER);
        data += make_idx(server);
        data += helpers::to_byte_array(server->port());
        data += server->address() + '\0';
    }

    return {RES_OK, data};
}

ConnectionsService::response_pair ConnectionsService::handle_connections_request(QByteArray* request)
{
    return {RES_OK, make_connections_byte_array()};
}

ConnectionsService::response_pair ConnectionsService::handle_monitor_connections_request(QByteArray* request)
{
    if (m_monitoringConnections) {
        return {RES_ALREADY_MONITORING, {}};
    }

    for (auto client : ms_yogiClients) {
        m_qtConnections.append(connect(&*client, &yogi_network::YogiTcpClient::connection_changed,
            [=](yogi_network::YogiTcpClient::ServerInformation info) {
                QByteArray data;
                data += make_idx(client);
                data += to_byte_array(info);
                m_session.notify_client(MON_CONNECTION_CHANGED, data);
            }
        ));
    }

    for (auto server : ms_yogiServers) {
        m_qtConnections.append(connect(&*server, &yogi_network::YogiTcpServer::connection_changed,
            [=](std::weak_ptr<yogi::TcpConnection>, yogi_network::YogiTcpServer::ClientInformation info) {
                QByteArray data;
                data += make_idx(server);
                data += to_byte_array(info);
                m_session.notify_client(MON_CONNECTION_CHANGED, data);
            }
        ));
    }

    m_monitoringConnections = true;

    return {RES_OK, make_connections_byte_array()};
}

QByteArray ConnectionsService::to_byte_array(yogi_network::YogiTcpClient::ServerInformation info)
{
    QByteArray data;
    data += static_cast<char>(info.connected ? 1 : 0);
    data += info.description + '\0';
    data += info.remoteVersion + '\0';
    data += helpers::to_byte_array(info.stateChangedTime);
    return data;
}

QByteArray ConnectionsService::to_byte_array(yogi_network::YogiTcpServer::ClientInformation info)
{
    QByteArray data;
    data += static_cast<char>(info.connected ? 1 : 0);
    data += info.description + '\0';
    data += info.remoteVersion + '\0';
    data += helpers::to_byte_array(info.stateChangedTime);
    return data;
}

QByteArray ConnectionsService::make_connections_byte_array()
{
    QByteArray data;

    for (auto client : ms_yogiClients) {
        data += make_idx(client);
        data += to_byte_array(client->connection());
    }

    for (auto server : ms_yogiServers) {
        auto idx = make_idx(server);
        for (auto connection : server->connections()) {
            data += idx;
            data += to_byte_array(connection);
        }
    }

    return data;
}

char ConnectionsService::make_idx(const std::shared_ptr<yogi_network::YogiTcpClient>& client)
{
    auto begin = ms_yogiClients.begin();
    auto end = ms_yogiClients.end();
    return static_cast<char>(1 + std::distance(begin, std::find(begin, end, client)));
}

char ConnectionsService::make_idx(const std::shared_ptr<yogi_network::YogiTcpServer>& server)
{
    auto begin = ms_yogiServers.begin();
    auto end = ms_yogiServers.end();
    return static_cast<char>(1 + ms_yogiClients.size() + std::distance(begin, std::find(begin, end, server)));
}

} // namespace session_services
