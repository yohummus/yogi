#ifndef YOGI_NETWORK_YOGISESSION_HH
#define YOGI_NETWORK_YOGISESSION_HH

#include "YogiTcpClient.hh"
#include "YogiTcpServer.hh"
#include "../session_services/Service.hh"
#include "../commands/CustomCommandService.hh"

#include <yogi.hpp>

#include <QVector>
#include <QMap>
#include <QWebSocket>
#include <QMetaObject>
#include <QMutex>
#include <QHostInfo>

#include <chrono>
#include <unordered_map>
#include <vector>


namespace yogi_network {

class YogiSession : public QObject
{
    Q_OBJECT
    struct TerminalInfo;

public:
    typedef std::vector<std::shared_ptr<yogi_network::YogiTcpServer>> yogi_servers_vector;
    typedef std::vector<std::shared_ptr<yogi_network::YogiTcpClient>> yogi_clients_vector;

    YogiSession(QWebSocket* socket, yogi::Node& node, const QString& clientIdentification,
        const yogi_servers_vector& yogiServers, const yogi_clients_vector& yogiClients, QObject* parent);
    ~YogiSession();

    yogi::Node& node();
    const QWebSocket& socket() const;

    QByteArray handle_request(QByteArray* request);
    void notify_client(session_services::Service::response_type notificationType, const QByteArray& data);

Q_SIGNALS:
    void received_sg_scatter_message(TerminalInfo*, std::shared_ptr<yogi::RawScatterGatherTerminal::ScatteredMessage>);
    void received_sg_gather_message(TerminalInfo*, std::shared_ptr<yogi::RawScatterGatherTerminal::GatheredMessage>);
    void received_sc_request(TerminalInfo*, std::shared_ptr<yogi::RawServiceTerminal::Request>);
    void received_sc_response(TerminalInfo*, std::shared_ptr<yogi::RawClientTerminal::Response>);
    void notify_client(QWebSocket*, QByteArray);

private:
    enum ConnectionFactoryType {
        CFT_TCP_CLIENT = 0,
        CFT_TCP_SERVER
    };

    typedef std::unordered_map<yogi::raw_operation_id, yogi::RawScatterGatherTerminal::Operation>        sg_operations_map;
    typedef std::unordered_map<yogi::raw_operation_id, yogi::RawScatterGatherTerminal::ScatteredMessage> sg_scattered_messages_map;
    typedef std::unordered_map<yogi::raw_operation_id, yogi::RawClientTerminal::Operation>               sc_operations_map;
    typedef std::unordered_map<yogi::raw_operation_id, yogi::RawServiceTerminal::Request>                sc_requests_map;

    struct TerminalInfo {
        int                                         id;
        std::unique_ptr<yogi::Terminal>             terminal;
        std::unique_ptr<yogi::BindingObserver>      bindingObserver;
        std::unique_ptr<yogi::SubscriptionObserver> subscriptionObserver;
        std::unique_ptr<yogi::Observer>             messageObserver;

        sg_operations_map                           sgOperations;
        sg_scattered_messages_map                   sgScatteredMessages;
        sc_operations_map                           scOperations;
        sc_requests_map                             scRequests;

        TerminalInfo(yogi::Leaf& leaf, yogi::terminal_type type, const char* name, yogi::Signature signature);
    };

    struct BindingInfo {
        int                                    id;
        yogi::Binding                          binding;
        std::unique_ptr<yogi::BindingObserver> observer;

        BindingInfo(yogi::PrimitiveTerminal& terminal, const char* targets);
    };

    typedef QMap<unsigned, std::shared_ptr<TerminalInfo>>                            terminal_lut;
    typedef QMap<unsigned, std::shared_ptr<BindingInfo>>                             binding_lut;
    typedef QMap<unsigned, std::shared_ptr<commands::CustomCommandService::Command>> command_lut;

    yogi::Logger                     m_logger;
    const QString                    m_logPrefix;
    const QString                    m_clientIdentification;
    QWebSocket*                      m_socket;
    yogi::Node&                      m_node;
    yogi::Leaf                       m_leaf;
    yogi::LocalConnection            m_connection;
    const yogi_servers_vector        m_yogiServers;
    const yogi_clients_vector        m_yogiClients;
    bool                             m_monitoringConnections;
    bool                             m_monitoringKnownTerminals;
    QByteArray                       m_monitorKnownTerminalsBuffer;
    QVector<QMetaObject::Connection> m_qtConnections;

    unsigned                         m_lastTerminalId;
    terminal_lut                     m_terminalLut;
    QMutex                           m_terminalLutMutex;

    unsigned                         m_lastBindingId;
    binding_lut                      m_bindingLut;
    QMutex                           m_bindingLutMutex;

    unsigned                         m_lastCommandId;
    command_lut                      m_commandLut;
    QMutex                           m_commandLutMutex;

    std::vector<session_services::service_ptr>              m_services;
    std::vector<session_services::Service::request_handler> m_requestHandlerLut;

    static QByteArray make_response(session_services::Service::response_type status = session_services::Service::RES_OK);
    QByteArray to_byte_array(YogiTcpClient::ServerInformation info);
    QByteArray to_byte_array(YogiTcpServer::ClientInformation info);
    QByteArray make_connections_byte_array();
    char make_idx(const std::shared_ptr<YogiTcpClient>& client);
    char make_idx(const std::shared_ptr<YogiTcpServer>& server);
    template <typename Service> void add_service();
    template <typename Fn> QByteArray use_terminal(QByteArray request, Fn fn);
    template <typename Terminal> void create_message_observer_and_add_callback(TerminalInfo& info, void (YogiSession::*fn)(TerminalInfo&, const std::vector<char>&, yogi::cached_flag));
    template <typename Terminal, typename ScatteredMessage> void create_message_observer_and_add_callback(TerminalInfo& info, void (YogiSession::*fn)(TerminalInfo&, ScatteredMessage&&));
    template <typename Terminal> yogi::Optional<std::vector<char>> get_cached_message(TerminalInfo& info);
    template <typename MessageMap> QByteArray respond_to_scattered_message(MessageMap& messages, yogi::raw_operation_id opId, const QByteArray& request);
    template <typename MessageMap> QByteArray ignore_scattered_message(MessageMap& messages, yogi::raw_operation_id opId);

    QByteArray handle_version_request(const QByteArray& request);
    QByteArray handle_current_time_request(const QByteArray& request);
    QByteArray handle_test_command(const QByteArray& request);
    QByteArray handle_connection_factories_request(const QByteArray& request);
    QByteArray handle_connections_request(const QByteArray& request);
    QByteArray handle_monitor_connections_request(const QByteArray& request);
    QByteArray handle_client_address_request(const QByteArray& request);
	QByteArray handle_dns_lookup_request(const QByteArray& request);
    QByteArray handle_create_terminal_request(const QByteArray& request);
    QByteArray handle_destroy_terminal_request(const QByteArray& request);
    QByteArray handle_create_binding_request(const QByteArray& request);
    QByteArray handle_destroy_binding_request(const QByteArray& request);
    QByteArray handle_monitor_binding_state_request(const QByteArray& request);
	QByteArray handle_monitor_builtin_binding_state_request(const QByteArray& request);
    QByteArray handle_monitor_subscription_state_request(const QByteArray& request);
    QByteArray handle_publish_message_request(const QByteArray& request);
    QByteArray handle_monitor_received_publish_messages_request(const QByteArray& request);
    QByteArray handle_scatter_gather_request(const QByteArray& request);
    QByteArray handle_monitor_received_scatter_messages_request(const QByteArray& request);
    QByteArray handle_respond_to_scattered_message_request(const QByteArray& request);
    QByteArray handle_ignore_scattered_message_request(const QByteArray& request);
    QByteArray handle_start_custom_command_request(const QByteArray& request);
    QByteArray handle_terminate_custom_command_request(const QByteArray& request);
    QByteArray handle_write_custom_command_stdin(const QByteArray& request);

    void on_binding_state_changed(BindingInfo& info, yogi::binding_state state);
    void on_builtin_binding_state_changed(TerminalInfo& info, yogi::binding_state state);
    void on_subscription_state_changed(TerminalInfo& info, yogi::subscription_state state);
    void on_published_message_received(TerminalInfo& info, const std::vector<char>& payload, yogi::cached_flag cached);
    void on_cached_published_message_received(TerminalInfo& info, const std::vector<char>& payload, yogi::cached_flag cached);
    void on_scattered_message_received(TerminalInfo& info, yogi::RawScatterGatherTerminal::ScatteredMessage&& msg);
    yogi::control_flow on_gathered_message_received(TerminalInfo& info, const yogi::Result& result, yogi::RawScatterGatherTerminal::GatheredMessage&& msg);
    void on_request_received(TerminalInfo& info, yogi::RawServiceTerminal::Request&& request);
    yogi::control_flow on_response_received(TerminalInfo& info, const yogi::Result& result, yogi::RawClientTerminal::Response&& response);

private Q_SLOTS:
	void on_dns_lookup_finished(QHostInfo info);
    void on_process_update(commands::CustomCommandService::Command* command, QProcess::ProcessState state, QByteArray out, QByteArray err, int exitCode, QProcess::ProcessError error);
    void handle_received_sg_scatter_message(TerminalInfo* info, std::shared_ptr<yogi::RawScatterGatherTerminal::ScatteredMessage> msg);
    void handle_received_sg_gather_message(TerminalInfo* info, std::shared_ptr<yogi::RawScatterGatherTerminal::GatheredMessage> msg);
    void handle_received_sc_request(TerminalInfo* info, std::shared_ptr<yogi::RawServiceTerminal::Request> request);
    void handle_received_sc_response(TerminalInfo* info, std::shared_ptr<yogi::RawClientTerminal::Response> response);
};

} // namespace yogi_network

#endif // YOGI_NETWORK_YOGISESSION_HH
