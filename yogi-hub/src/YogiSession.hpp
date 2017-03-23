#ifndef YOGISESSION_HPP
#define YOGISESSION_HPP

#include "YogiTcpClient.hpp"
#include "YogiTcpServer.hpp"
#include "CustomCommandService.hpp"

#include <yogi.hpp>

#include <QVector>
#include <QMap>
#include <QWebSocket>
#include <QMetaObject>
#include <QMutex>
#include <QHostInfo>

#include <chrono>
#include <unordered_map>


class YogiSession : public QObject
{
    Q_OBJECT

    enum RequestType {
        REQ_VERSION = 0,
        REQ_CURRENT_TIME,
        REQ_TEST_COMMAND,
        REQ_KNOWN_TERMINALS,
        REQ_KNOWN_TERMINALS_SUBTREE,
		REQ_FIND_KNOWN_TERMINALS,
        REQ_MONITOR_KNOWN_TERMINALS,
        REQ_CONNECTION_FACTORIES,
        REQ_CONNECTIONS,
        REQ_MONITOR_CONNECTIONS,
        REQ_CLIENT_ADDRESS,
		REQ_START_DNS_LOOKUP,
        REQ_CREATE_TERMINAL,
        REQ_DESTROY_TERMINAL,
        REQ_CREATE_BINDING,
        REQ_DESTROY_BINDING,
        REQ_MONITOR_BINDING_STATE,
		REQ_MONITOR_BUILTIN_BINDING_STATE,
        REQ_MONITOR_SUBSCRIPTION_STATE,
        REQ_PUBLISH_MESSAGE,
        REQ_MONITOR_RECEIVED_PUBLISH_MESSAGES,
        REQ_SCATTER_GATHER,
        REQ_MONITOR_RECEIVED_SCATTER_MESSAGES,
        REQ_RESPOND_TO_SCATTERED_MESSAGE,
        REQ_IGNORE_SCATTERED_MESSAGE,
        REQ_START_CUSTOM_COMMAND,
        REQ_TERMINATE_CUSTOM_COMMAND,
        REQ_WRITE_CUSTOM_COMMAND_STDIN
    };

    enum Status {
        RES_OK = 0,
        RES_INTERNAL_SERVER_ERROR,
        RES_INVALID_REQUEST,
        RES_API_ERROR,
        RES_ALREADY_MONITORING,
        RES_INVALID_TERMINAL_ID,
        RES_INVALID_BINDING_ID,
        RES_INVALID_OPERATION_ID,
        RES_INVALID_COMMAND_ID,
        RES_INVALID_TERMINAL_TYPE,

		ASY_DNS_LOOKUP,
        ASY_CUSTOM_COMMAND_STATE,

        MON_CONNECTION_CHANGED,
        MON_KNOWN_TERMINALS_CHANGED,
        MON_BINDING_STATE_CHANGED,
		MON_BUILTIN_BINDING_STATE_CHANGED,
        MON_SUBSCRIPTION_STATE_CHANGED,
        MON_PUBLISHED_MESSAGE_RECEIVED,
        MON_CACHED_PUBLISHED_MESSAGE_RECEIVED,
        MON_SCATTERED_MESSAGE_RECEIVED,
        MON_GATHERED_MESSAGE_RECEIVED
    };

    enum ConnectionFactoryType {
        CFT_TCP_CLIENT = 0,
        CFT_TCP_SERVER
    };

    struct TerminalInfo {
        int                                             id;
        std::unique_ptr<yogi::Terminal>             terminal;
        std::unique_ptr<yogi::BindingObserver>      bindingObserver;
        std::unique_ptr<yogi::SubscriptionObserver> subscriptionObserver;
        std::unique_ptr<yogi::Observer>             messageObserver;

        std::unordered_map<yogi::raw_operation_id, yogi::RawScatterGatherTerminal::Operation>        sgOperations;
        std::unordered_map<yogi::raw_operation_id, yogi::RawScatterGatherTerminal::ScatteredMessage> sgScatteredMessages;
        std::unordered_map<yogi::raw_operation_id, yogi::RawClientTerminal::Operation>               scOperations;
        std::unordered_map<yogi::raw_operation_id, yogi::RawServiceTerminal::Request>                scRequests;

        TerminalInfo(yogi::Leaf& leaf, yogi::terminal_type type, const char* name, yogi::Signature signature);
    };

    struct BindingInfo {
        int                                        id;
        yogi::Binding                          binding;
        std::unique_ptr<yogi::BindingObserver> observer;

        BindingInfo(yogi::PrimitiveTerminal& terminal, const char* targets);
    };

private:
    yogi::Logger                              m_logger;
    const QString                                 m_logPrefix;
    const QString                                 m_clientIdentification;
    QWebSocket*                                   m_socket;
    yogi::Node&                               m_node;
    yogi::Leaf                                m_leaf;
    yogi::LocalConnection                     m_connection;
    bool                                          m_monitoringConnections;
    bool                                          m_monitoringKnownTerminals;
    QByteArray                                    m_monitorKnownTerminalsBuffer;
    QVector<QMetaObject::Connection>              m_qtConnections;

    unsigned                                      m_lastTerminalId;
    QMap<unsigned, std::shared_ptr<TerminalInfo>> m_terminalLut;
    QMutex                                        m_terminalLutMutex;

    unsigned                                      m_lastBindingId;
    QMap<unsigned, std::shared_ptr<BindingInfo>>  m_bindingLut;
    QMutex                                        m_bindingLutMutex;

    unsigned                                                       m_lastCommandId;
    QMap<unsigned, std::shared_ptr<CustomCommandService::Command>> m_commandLut;
    QMutex                                                         m_commandLutMutex;

private:
    static QByteArray make_response(Status status = RES_OK);
    QByteArray to_byte_array(YogiTcpClient::ServerInformation info);
    QByteArray to_byte_array(YogiTcpServer::ClientInformation info);
    QByteArray make_connections_byte_array();
    char make_idx(YogiTcpClient* client);
    char make_idx(YogiTcpServer* server);
    template <typename Fn> QByteArray use_terminal(QByteArray request, Fn fn);
    template <typename Terminal> void create_message_observer_and_add_callback(TerminalInfo& info, void (YogiSession::*fn)(TerminalInfo&, const std::vector<char>&, yogi::cached_flag));
    template <typename Terminal, typename ScatteredMessage> void create_message_observer_and_add_callback(TerminalInfo& info, void (YogiSession::*fn)(TerminalInfo&, ScatteredMessage&&));
    template <typename Terminal> yogi::Optional<std::vector<char>> get_cached_message(TerminalInfo& info);
    template <typename MessageMap> QByteArray respond_to_scattered_message(MessageMap& messages, yogi::raw_operation_id opId, const QByteArray& request);
    template <typename MessageMap> QByteArray ignore_scattered_message(MessageMap& messages, yogi::raw_operation_id opId);

    QByteArray handle_version_request(const QByteArray& request);
    QByteArray handle_current_time_request(const QByteArray& request);
    QByteArray handle_test_command(const QByteArray& request);
    QByteArray handle_known_terminals_request(const QByteArray& request);
    QByteArray handle_known_terminals_subtree_request(const QByteArray& request);
	QByteArray handle_find_known_terminals(const QByteArray& request);
	QByteArray handle_monitor_known_terminals_request(const QByteArray& request);
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

private:
    void on_binding_state_changed(BindingInfo& info, yogi::binding_state state);
    void on_builtin_binding_state_changed(TerminalInfo& info, yogi::binding_state state);
    void on_subscription_state_changed(TerminalInfo& info, yogi::subscription_state state);
    void on_published_message_received(TerminalInfo& info, const std::vector<char>& payload, yogi::cached_flag cached);
    void on_cached_published_message_received(TerminalInfo& info, const std::vector<char>& payload, yogi::cached_flag cached);
    void on_scattered_message_received(TerminalInfo& info, yogi::RawScatterGatherTerminal::ScatteredMessage&& msg);
    yogi::control_flow on_gathered_message_received(TerminalInfo& info, const yogi::Result& result, yogi::RawScatterGatherTerminal::GatheredMessage&& msg);
    void on_request_received(TerminalInfo& info, yogi::RawServiceTerminal::Request&& request);
    yogi::control_flow on_response_received(TerminalInfo& info, const yogi::Result& result, yogi::RawClientTerminal::Response&& response);

Q_SIGNALS:
    void received_sg_scatter_message(TerminalInfo*, std::shared_ptr<yogi::RawScatterGatherTerminal::ScatteredMessage>);
    void received_sg_gather_message(TerminalInfo*, std::shared_ptr<yogi::RawScatterGatherTerminal::GatheredMessage>);
    void received_sc_request(TerminalInfo*, std::shared_ptr<yogi::RawServiceTerminal::Request>);
    void received_sc_response(TerminalInfo*, std::shared_ptr<yogi::RawClientTerminal::Response>);
    void notify_client(QWebSocket*, QByteArray);

private Q_SLOTS:
	void on_dns_lookup_finished(QHostInfo info);
    void on_process_update(CustomCommandService::Command* command, QProcess::ProcessState state, QByteArray out, QByteArray err, int exitCode, QProcess::ProcessError error);
    void handle_received_sg_scatter_message(TerminalInfo* info, std::shared_ptr<yogi::RawScatterGatherTerminal::ScatteredMessage> msg);
    void handle_received_sg_gather_message( TerminalInfo* info, std::shared_ptr<yogi::RawScatterGatherTerminal::GatheredMessage> msg);
    void handle_received_sc_request(        TerminalInfo* info, std::shared_ptr<yogi::RawServiceTerminal::Request> request);
    void handle_received_sc_response(       TerminalInfo* info, std::shared_ptr<yogi::RawClientTerminal::Response> response);

public:
    YogiSession(QWebSocket* socket, yogi::Node& node, const QString& clientIdentification, QObject* parent = Q_NULLPTR);
    ~YogiSession();

    QByteArray handle_request(const QByteArray& request);
};

#endif // YOGISESSION_HPP
