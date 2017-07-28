#ifndef SESSION_SERVICES_TERMINALSSERVICE_HH
#define SESSION_SERVICES_TERMINALSSERVICE_HH

#include "Service.hh"
#include "../yogi_network/YogiSession.hh"
#include "../helpers/LookupTable.hh"

#include <mutex>


namespace session_services {

class TerminalsService : public Service
{
    Q_OBJECT

public:
    TerminalsService(yogi_network::YogiSession& session);
    ~TerminalsService();

    virtual request_handlers_map make_request_handlers() override;

private:
    typedef std::vector<char> byte_array;

    template <typename T> using op_id_map = std::unordered_map<yogi::raw_operation_id, T>;
    typedef op_id_map<yogi::RawScatterGatherTerminal::Operation>        sg_operations_map;
    typedef op_id_map<yogi::RawScatterGatherTerminal::ScatteredMessage> sg_scattered_messages_map;
    typedef op_id_map<yogi::RawClientTerminal::Operation>               sc_operations_map;
    typedef op_id_map<yogi::RawServiceTerminal::Request>                sc_requests_map;

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

        TerminalInfo(yogi::Leaf& leaf, yogi::terminal_type type, const char* name,
            yogi::Signature signature);
    };

    struct BindingInfo {
        int                                    id;
        yogi::Binding                          binding;
        std::unique_ptr<yogi::BindingObserver> observer;

        BindingInfo(yogi::PrimitiveTerminal& terminal, const char* targets);
    };

    yogi_network::YogiSession&         m_session;
    yogi::Logger                       m_logger;
    std::mutex                         m_mapsMutex;
    helpers::LookupTable<TerminalInfo> m_terminals;
    helpers::LookupTable<BindingInfo>  m_bindings;

    response_pair handle_create_terminal_request(const QByteArray& request);
    response_pair handle_destroy_terminal_request(const QByteArray& request);
    response_pair handle_create_binding_request(const QByteArray& request);
    response_pair handle_destroy_binding_request(const QByteArray& request);
    response_pair handle_monitor_binding_state_request(const QByteArray& request);
    response_pair handle_monitor_builtin_binding_state_request(const QByteArray& request);
    response_pair handle_monitor_subscription_state_request(const QByteArray& request);
    response_pair handle_publish_message_request(const QByteArray& request);
    response_pair handle_monitor_received_publish_messages_request(const QByteArray& request);
    response_pair handle_scatter_gather_request(const QByteArray& request);
    response_pair handle_monitor_received_scatter_messages_request(const QByteArray& request);
    response_pair handle_respond_to_scattered_message_request(const QByteArray& request);
    response_pair handle_ignore_scattered_message_request(const QByteArray& request);

    void on_binding_state_changed(BindingInfo& info, yogi::binding_state state);
    void on_builtin_binding_state_changed(TerminalInfo& info, yogi::binding_state state);
    void on_subscription_state_changed(TerminalInfo& info, yogi::subscription_state state);
    void on_published_message_received(TerminalInfo& info, const byte_array& payload,
        yogi::cached_flag cached);
    void on_cached_published_message_received(TerminalInfo& info, const byte_array& payload,
        yogi::cached_flag cached);
    void on_scattered_message_received(TerminalInfo& info,
        yogi::RawScatterGatherTerminal::ScatteredMessage&& msg);
    yogi::control_flow on_gathered_message_received(TerminalInfo& info, const yogi::Result& result,
        yogi::RawScatterGatherTerminal::GatheredMessage&& msg);
    void on_request_received(TerminalInfo& info, yogi::RawServiceTerminal::Request&& request);
    yogi::control_flow on_response_received(TerminalInfo& info, const yogi::Result& result,
        yogi::RawClientTerminal::Response&& response);

    template <typename Fn>
    response_pair use_terminal(const QByteArray& request, Fn fn);

    template <typename Terminal>
    void create_message_observer_and_add_callback(TerminalInfo& info,
        void (TerminalsService::*fn)(TerminalInfo&, const byte_array&, yogi::cached_flag));

    template <typename Terminal, typename ScatteredMessage>
    void create_message_observer_and_add_callback(TerminalInfo& info,
        void (TerminalsService::*fn)(TerminalInfo&, ScatteredMessage&&));

    template <typename Terminal>
    yogi::Optional<byte_array> get_cached_message(TerminalInfo& info);

    template <typename MessageMap>
    response_pair respond_to_scattered_message(MessageMap& messages, yogi::raw_operation_id opId,
        const QByteArray& request);

    template <typename MessageMap>
    response_pair ignore_scattered_message(MessageMap& messages, yogi::raw_operation_id opId);
};

} // namespace session_services

#endif // SESSION_SERVICES_TERMINALSSERVICE_HH
