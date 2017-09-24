#include "AccountManager.hh"
using namespace std::string_literals;


namespace accounts {

AccountManager::AccountManager(boost::asio::io_service& ios)
: m_ios(ios)
, m_logger("AccountManager")
, m_loadAccountsError("Loading accounts failed")
, m_saveAccountsError("Saving accounts failed")
, m_readyOc("Accounts retrieved successfully")
, m_authTerminal("Authenticate"s)
, m_authObserver(m_authTerminal)
, m_getAccountsTerminal("Get Accounts"s)
, m_getAccountsObserver(m_getAccountsTerminal)
{
    setup_providers();
    setup_observers();

    load_accounts();
}

boost::asio::io_service& AccountManager::io_service()
{
    return m_ios;
}

storage::StorageProvider& AccountManager::storage_provider()
{
    return *m_storageProvider;
}

account_ptr AccountManager::get_account(const std::string& username) const
{
    auto account = try_get_account(username);
    if (!account) {
        throw std::runtime_error("Unknown username "s + username);
    }

    return account;
}

account_ptr AccountManager::try_get_account(const std::string& username) const
{
    auto it = m_accountProvider->accounts().find(username);
    return it != m_accountProvider->accounts().end() ? it->second : account_ptr();
}

void AccountManager::setup_providers()
{
    m_storageProvider = storage::StorageProvider::create_provider(m_ios, "account-storage-provider");
    m_accountProvider = AccountProvider::create_provider(m_ios, *m_storageProvider, "account-provider");
}

void AccountManager::setup_observers()
{
    m_authObserver.set([=](auto&& req) {
        auto reqPtr = std::make_shared<auth_terminal::Request>(std::move(req));
        this->m_ios.post([=] {
            this->on_auth_request_received(reqPtr);
        });
    });
    m_authObserver.start();

    m_getAccountsObserver.set([=](auto&& req) {
        auto reqPtr = std::make_shared<get_accounts_terminal::Request>(std::move(req));
        this->m_ios.post([=] {
            this->on_get_accounts_request_received(reqPtr);
        });
    });
    m_getAccountsObserver.start();
}

void AccountManager::load_accounts()
{
    m_accountProvider->async_load_accounts([=](bool success) {
        this->on_load_accounts_finished(success);
    });
}

void AccountManager::on_load_accounts_finished(bool success)
{
    if (success) {
        YOGI_LOG_INFO(m_logger, m_readyOc.name());
        m_readyOc.set();
    }
    else {
        m_loadAccountsError.set();
    }
}

void AccountManager::on_auth_request_received(auth_request req)
{
    auto username = req->message().value().first();
    auto password = req->message().value().second();

    auto msg = auth_terminal::make_response_message();

    auto accIt = m_accountProvider->accounts().find(username);
    if (accIt == m_accountProvider->accounts().end()) {
        YOGI_LOG_WARNING(m_logger, "Authentication for " << username << " failed: Unknown username");
        msg.set_value(false);
    }
    else {
        if (password == accIt->second->password()) {
            YOGI_LOG_DEBUG(m_logger, "Authentication for " << username << " succeeded");
            msg.set_value(true);
        }
        else {
            YOGI_LOG_WARNING(m_logger, "Authentication for " << username << " failed: Wrong password");
            msg.set_value(false);
        }
    }

    req->try_respond(msg);
}

void AccountManager::on_get_accounts_request_received(get_accounts_request req)
{
    auto msg = get_accounts_terminal::make_response_message();
    for (auto& account : m_accountProvider->accounts()) {
        msg.add_value(account.first);
    }

    req->try_respond(msg);
}

} // namespace accounts
