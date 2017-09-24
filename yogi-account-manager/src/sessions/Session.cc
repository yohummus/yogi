#include "Session.hh"
#include "SessionManager.hh"

#include <mutex>


namespace sessions {

Session::Session(SessionManager& manager, accounts::account_ptr account,
    const std::string& tag, const std::string& description)
: m_manager(manager)
, m_logger("Session")
, m_name(account->username() + "-" + make_name_suffix(account, tag))
, m_path(yogi::Path("Sessions") / m_name)
, m_storage(manager.io_service(), manager.storage_provider(), m_logger, m_path, m_name)
, m_accountTerminal(m_path / "Account")
, m_createdTimeTerminal(m_path / "Created Time")
, m_descriptionTerminal(m_path / "Description")
, m_terminateTerminal(m_path / "Terminate")
, m_terminateObserver(m_terminateTerminal)
{
    publish_time(&m_createdTimeTerminal, yogi::Timestamp::now());
    publish_string(&m_accountTerminal, account->username());
    publish_string(&m_descriptionTerminal, description);

    clear_storage();
    setup_observers();
}

Session::~Session()
{
    clear_storage();
}

const std::string& Session::name() const
{
    return m_name;
}

std::string Session::make_name_suffix(accounts::account_ptr account, const std::string& tag)
{
    static std::unordered_map<std::string, unsigned> counters;
    static std::mutex mutex;

    std::lock_guard<std::mutex> lock(mutex);
    auto cnt = ++counters[account->username()];

    if (tag.empty()) {
        return std::to_string(cnt);
    }
    else {
        return tag + "-" + std::to_string(cnt);
    }
}

void Session::setup_observers()
{
    m_terminateObserver.set([=](auto&& req) {
        auto reqPtr = std::make_shared<terminate_terminal::Request>(std::move(req));
        this->m_manager.io_service().post([=] {
            this->on_terminate_request_received(reqPtr);
        });
    });
    m_terminateObserver.start();
}

void Session::on_terminate_request_received(terminate_request req)
{
    m_manager.terminate_session(shared_from_this());
}

void Session::publish_time(time_terminal* tm, const yogi::Timestamp& time)
{
    auto msg = tm->make_message();
    msg.set_value(time.time_since_epoch().count());
    tm->try_publish(msg);
}

void Session::publish_string(string_terminal* tm, const std::string& str)
{
    auto msg = tm->make_message();
    msg.set_value(str);
    tm->try_publish(msg);
}

void Session::clear_storage()
{
    YOGI_LOG_DEBUG(m_logger, "Clearing session storage for " << m_name);
    m_manager.storage_provider().async_clear_section(m_name, [=](bool) {});
}

} // namespace sessions
