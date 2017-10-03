#ifndef SESSION_SERVICES_ACCOUNTSERVICE_HH
#define SESSION_SERVICES_ACCOUNTSERVICE_HH

#include "Service.hh"
#include "../yogi_network/YogiSession.hh"

#include "../proto/yogi_00000000.h"
#include "../proto/yogi_0000000d.h"
#include "../proto/yogi_0000040d.h"
#include "../proto/yogi_0000d1e1.h"
#include "../proto/yogi_001ad1a1.h"
#include "../proto/yogi_001ed001.h"


namespace session_services {

class AccountService : public Service
{
    Q_OBJECT

public:
    static void init();
    static void destroy();

    AccountService(yogi_network::YogiSession& session);

    virtual request_handlers_map make_request_handlers() override;

private:
    typedef yogi::ClientTerminal<yogi_001ad1a1>         create_session_terminal;
    typedef std::unique_ptr<create_session_terminal>    create_session_terminal_ptr;
    typedef yogi::ClientTerminal<yogi_00000000>         terminate_terminal;
    typedef std::unique_ptr<terminate_terminal>         terminate_terminal_ptr;
    typedef yogi::ClientTerminal<yogi_001ed001>         store_data_terminal;
    typedef std::unique_ptr<store_data_terminal>        store_data_terminal_ptr;
    typedef yogi::ClientTerminal<yogi_0000d1e1>         read_data_terminal;
    typedef std::unique_ptr<read_data_terminal>         read_data_terminal_ptr;
    typedef yogi::CachedConsumerTerminal<yogi_0000040d> groups_terminal;
    typedef std::unique_ptr<groups_terminal>            groups_terminal_ptr;
    typedef yogi::MessageObserver<groups_terminal>      groups_observer;
    typedef std::unique_ptr<groups_observer>            groups_observer_ptr;
    typedef yogi::CachedConsumerTerminal<yogi_0000000d> string_terminal;
    typedef std::unique_ptr<string_terminal>            string_terminal_ptr;
    typedef yogi::MessageObserver<string_terminal>      string_observer;
    typedef std::unique_ptr<string_observer>            string_observer_ptr;
    typedef std::unique_ptr<yogi::SubscriptionObserver> subscription_observer_ptr;

    static yogi::Path                               ms_accountManagerLocation;
    static create_session_terminal_ptr              ms_createSessionTerminal;
    static std::unique_ptr<yogi::ProcessDependency> ms_dependency;

    yogi_network::YogiSession&                      m_session;
    yogi::Logger                                    m_logger;
    bool                                            m_loggingIn;
    bool                                            m_loggedIn;
    unsigned                                        m_loginTaskId;
    std::string                                     m_username;
    std::string                                     m_webSessionName;
    yogi::Optional<std::string>                     m_firstName;
    yogi::Optional<std::string>                     m_lastName;
    yogi::Optional<std::vector<std::string>>        m_groups;

    std::unique_ptr<yogi::Leaf>                     m_serviceLeaf;
    std::unique_ptr<yogi::LocalConnection>          m_serviceConnection;
    string_terminal_ptr                             m_firstNameTerminal;
    string_observer_ptr                             m_firstNameObserver;
    string_terminal_ptr                             m_lastNameTerminal;
    string_observer_ptr                             m_lastNameObserver;
    groups_terminal_ptr                             m_groupsTerminal;
    groups_observer_ptr                             m_groupsObserver;
    store_data_terminal_ptr                         m_storeAccountDataTerminal;
    subscription_observer_ptr                       m_storeAccountDataSubObserver;
    read_data_terminal_ptr                          m_readAccountDataTerminal;
    subscription_observer_ptr                       m_readAccountDataSubObserver;
    store_data_terminal_ptr                         m_storeSessionDataTerminal;
    subscription_observer_ptr                       m_storeSessionDataSubObserver;
    read_data_terminal_ptr                          m_readSessionDataTerminal;
    subscription_observer_ptr                       m_readSessionDataSubObserver;
    terminate_terminal_ptr                          m_terminateTerminal;
    subscription_observer_ptr                       m_terminateSubObserver;

    // log in
    response_pair handle_start_login_task_request(const QByteArray& request);
    bool send_create_session_request(const std::string& password);
    yogi::control_flow on_login_request_answer_received(const yogi::Result& res,
        create_session_terminal::Response&& response);
    Q_SLOT void handle_login_request_answer_in_qt_thread(int res, bool success,
        const std::string& webSessionName);
    Q_SLOT void try_completing_login_request_in_qt_thread();

    // store data
    response_pair handle_start_store_data_task_request(const QByteArray& request);
    bool send_store_data_request(unsigned taskId, bool isAccountData,
        const std::string& variable, const QByteArray& data);
    yogi::control_flow on_store_data_request_answer_received(const yogi::Result& res,
        store_data_terminal::Response&& response, unsigned taskId);
    Q_SLOT void handle_store_data_request_answer_in_qt_thread(int res, bool success,
        unsigned taskId);

    // read data
    response_pair handle_start_read_data_task_request(const QByteArray& request);
    bool send_read_data_request(unsigned taskId, bool isAccountData,
        const std::string& variable);
    yogi::control_flow on_read_data_request_answer_received(const yogi::Result& res,
        read_data_terminal::Response&& response, unsigned taskId);
    Q_SLOT void handle_read_data_request_answer_in_qt_thread(int res, bool success,
        const QByteArray& readData, unsigned taskId);

    // helpers
    static unsigned get_task_id(const QByteArray& request);
    static int count_strings(const QByteArray& request, int offset);
    void setup_service_leaf();
    void setup_account_terminals();
    void setup_session_terminals();

    template <typename Terminal, typename Fn>
    void setup_consumer(std::unique_ptr<Terminal>* tm,
        std::unique_ptr<yogi::MessageObserver<Terminal>>* ob, const char* tmName, Fn fn);

    template <typename Terminal>
    void setup_client(std::unique_ptr<Terminal>* tm,
        std::unique_ptr<yogi::SubscriptionObserver>* ob, const char* tmName,
        bool sessionSpecific);
};

} // namespace session_services

#endif // SESSION_SERVICES_ACCOUNTSERVICE_HH
