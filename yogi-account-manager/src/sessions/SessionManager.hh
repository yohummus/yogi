#ifndef SESSIONS_SESSIONMANAGER_HH
#define SESSIONS_SESSIONMANAGER_HH

#include "Session.hh"
#include "../accounts/AccountManager.hh"
#include "../storage/StorageProvider.hh"
#include "../proto/yogi_0000040d.h"
#include "../proto/yogi_001ad001.h"
#include "../proto/yogi_001ad1a1.h"

#include <boost/optional.hpp>


namespace sessions {

class SessionManager
{
public:
    SessionManager(accounts::AccountManager& accountManager);

    boost::asio::io_service& io_service();
    storage::StorageProvider& storage_provider();

    void terminate_session(session_ptr session);

private:
    typedef std::unordered_map<std::string, session_ptr>    sessions_map;
    typedef yogi::ServiceTerminal<yogi_001ad1a1>            create_terminal;
    typedef yogi::MessageObserver<create_terminal>          create_observer;
    typedef std::shared_ptr<create_terminal::Request>       create_request;
    typedef yogi::ServiceTerminal<yogi_0000040d>            get_sessions_terminal;
    typedef yogi::MessageObserver<get_sessions_terminal>    get_sessions_observer;
    typedef std::shared_ptr<get_sessions_terminal::Request> get_sessions_request;

    accounts::AccountManager&        m_accountManager;
    storage::storage_provider_ptr    m_storageProvider;
    yogi::Logger                     m_logger;
    sessions_map                     m_sessions;
    yogi::ManualOperationalCondition m_readyOc;
    create_terminal                  m_createTerminal;
    create_observer                  m_createObserver;
    create_terminal                  m_webCreateTerminal;
    create_observer                  m_webCreateObserver;
    get_sessions_terminal            m_getSessionsTerminal;
    get_sessions_observer            m_getSessionsObserver;

    void setup_storage_provider();
    void setup_observers();

    void on_clear_storage_finished(bool success);
    void on_create_request_received(create_request req);
    void on_web_create_request_received(create_request req);
    void on_get_sessions_request_received(get_sessions_request req);

    void respond_to_create_request(create_request req, session_ptr session);
};

} // namespace sessions

#endif // SESSIONS_SESSIONMANAGER_HH
