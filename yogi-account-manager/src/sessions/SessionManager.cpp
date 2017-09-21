#include "SessionManager.hh"
using namespace std::string_literals;


namespace sessions {

SessionManager::SessionManager(accounts::AccountManager& accountManager)
: m_accountManager(accountManager)
, m_logger("SessionManager")
, m_readyOc("Session storage cleared successfully")
, m_createTerminal("Create Session"s)
, m_createObserver(m_createTerminal)
, m_getSessionsTerminal("Get Sessions"s)
, m_getSessionsObserver(m_getSessionsTerminal)
{
    setup_storage_provider();
    setup_observers();
}

boost::asio::io_service& SessionManager::io_service()
{
    return m_accountManager.io_service();
}

storage::StorageProvider& SessionManager::storage_provider()
{
    return *m_storageProvider;
}

void SessionManager::terminate_session(session_ptr session)
{
    m_sessions.erase(session->name());
    YOGI_LOG_INFO(m_logger, "Terminated session " << session->name());
}

void SessionManager::setup_storage_provider()
{
    m_storageProvider = storage::StorageProvider::create_provider(this->io_service(),
        "session-storage-provider");
    m_storageProvider->async_clear([=](bool success) {
        this->io_service().post([=] {
            this->on_clear_storage_finished(success);
        });
    });
}

void SessionManager::setup_observers()
{
    m_createObserver.set([=](auto&& req) {
        auto reqPtr = std::make_shared<create_terminal::Request>(std::move(req));
        this->io_service().post([=] {
            this->on_create_request_received(reqPtr);
        });
    });
    m_createObserver.start();

    m_getSessionsObserver.set([=](auto&& req) {
        auto reqPtr = std::make_shared<get_sessions_terminal::Request>(std::move(req));
        this->io_service().post([=] {
            this->on_get_sessions_request_received(reqPtr);
        });
    });
    m_getSessionsObserver.start();
}

void SessionManager::on_clear_storage_finished(bool success)
{
    if (success) {
        YOGI_LOG_INFO(m_logger, m_readyOc.name());
        m_readyOc.set();
    }
    else {
        throw std::runtime_error("Could not clear session storage");
    }
}

void SessionManager::on_create_request_received(create_request req)
{
    auto username = req->message().value().first();
    auto description = req->message().value().second();

    auto msg = m_createTerminal.make_response_message();
    decltype(msg)::Pair pair;
    msg.set_allocated_value(&pair);

    try {
        auto account = m_accountManager.get_account(username);
        if (account) {
            auto session = std::make_shared<Session>(*this, account, description);
            m_sessions.insert(std::make_pair(session->name(), session));

            pair.set_first(true);
            pair.set_second(session->name());

            YOGI_LOG_INFO(m_logger, "Session " << session->name() << " created successfully");
        }
        else {
            YOGI_LOG_WARNING(m_logger, "Cannot create session for " << username
                << ": Unknown username");
            pair.set_first(false);
        }
    }
    catch (const std::exception& e) {
        YOGI_LOG_ERROR(m_logger, "Failed to create session for " << username << ": " << e.what());
        pair.set_first(false);
    }

    req->try_respond(msg);
    msg.release_value();
}

void SessionManager::on_get_sessions_request_received(get_sessions_request req)
{
    auto msg = get_sessions_terminal::make_response_message();
    for (auto& session : m_sessions) {
        msg.add_value(session.first);
    }

    req->try_respond(msg);
}

} // namespace sessions
