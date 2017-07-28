#ifndef SESSION_SERVICES_SERVICE_HH
#define SESSION_SERVICES_SERVICE_HH

#include <QObject>
#include <QByteArray>

#include <memory>
#include <functional>
#include <map>


namespace session_services {

class Service : public QObject
{
    Q_OBJECT

public:
    enum request_type : unsigned {
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

    enum response_type : unsigned {
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

    typedef std::pair<response_type, QByteArray>                     response_pair;
    typedef std::function<response_pair (const QByteArray& request)> request_handler;
    typedef std::map<request_type, request_handler>                  request_handlers_map;

    virtual ~Service();

    virtual request_handlers_map make_request_handlers() =0;
};

typedef std::shared_ptr<Service> service_ptr;

} // namespace session_services

#endif // SESSION_SERVICES_SERVICE_HH
