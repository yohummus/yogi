#ifndef SESSIONS_SESSION_HH
#define SESSIONS_SESSION_HH

#include <yogi.hpp>

#include "../accounts/Account.hh"
#include "../storage/StorageInterface.hh"
#include "../proto/yogi_00000000.h"
#include "../proto/yogi_0000000d.h"
#include "../proto/yogi_00000010.h"

#include <boost/asio/io_service.hpp>


namespace sessions {

class SessionManager;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(SessionManager& manager, accounts::account_ptr account,
        const std::string& description);
    virtual ~Session();

    const std::string& name() const;

private:
    typedef yogi::CachedProducerTerminal<yogi_00000010>  time_terminal;
    typedef yogi::CachedProducerTerminal<yogi_0000000d>  string_terminal;
    typedef yogi::ServiceTerminal<yogi_00000000>         terminate_terminal;
    typedef yogi::MessageObserver<terminate_terminal>    terminate_observer;
    typedef std::shared_ptr<terminate_terminal::Request> terminate_request;

    SessionManager&           m_manager;
    yogi::Logger              m_logger;
    const std::string         m_name;
    const yogi::Path          m_path;
    storage::StorageInterface m_storage;
    string_terminal           m_accountTerminal;
    time_terminal             m_createdTimeTerminal;
    string_terminal           m_descriptionTerminal;
    terminate_terminal        m_terminateTerminal;
    terminate_observer        m_terminateObserver;

    static std::string make_name_suffix(accounts::account_ptr account);

    void setup_observers();
    void on_terminate_request_received(terminate_request req);
    void publish_time(time_terminal* tm, const yogi::Timestamp& time);
    void publish_string(string_terminal* tm, const std::string& str);
    void clear_storage();
};

typedef std::shared_ptr<Session> session_ptr;

} // namespace sessions

#endif // SESSIONS_SESSION_HH
