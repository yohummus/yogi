#ifndef ACCOUNTS_ACCOUNTMANAGER_HH
#define ACCOUNTS_ACCOUNTMANAGER_HH

#include "AccountProvider.hh"
#include "../storage/StorageProvider.hh"
#include "../proto/yogi_001ad001.h"
#include "../proto/yogi_0000040d.h"


namespace accounts {

class AccountManager
{
public:
    AccountManager(boost::asio::io_service& ios);

    boost::asio::io_service& io_service();
    storage::StorageProvider& storage_provider();

    account_ptr get_account(const std::string& username) const;

private:
    typedef std::unordered_map<std::string, account_ptr>    accounts_map;
    typedef yogi::ServiceTerminal<yogi_001ad001>            auth_terminal;
    typedef yogi::MessageObserver<auth_terminal>            auth_observer;
    typedef std::shared_ptr<auth_terminal::Request>         auth_request;
    typedef yogi::ServiceTerminal<yogi_0000040d>            get_accounts_terminal;
    typedef yogi::MessageObserver<get_accounts_terminal>    get_accounts_observer;
    typedef std::shared_ptr<get_accounts_terminal::Request> get_accounts_request;

    boost::asio::io_service&         m_ios;
    storage::storage_provider_ptr    m_storageProvider;
    account_provider_ptr             m_accountProvider;
    yogi::Logger                     m_logger;
    yogi::Error                      m_loadAccountsError;
    yogi::Error                      m_saveAccountsError;
    yogi::ManualOperationalCondition m_readyOc;
    auth_terminal                    m_authTerminal;
    auth_observer                    m_authObserver;
    get_accounts_terminal            m_getAccountsTerminal;
    get_accounts_observer            m_getAccountsObserver;

    void setup_providers();
    void setup_observers();

    void load_accounts();

    void on_load_accounts_finished(bool success);
    void on_auth_request_received(auth_request req);
    void on_get_accounts_request_received(get_accounts_request req);
};

} // namespace accounts

#endif // ACCOUNTS_ACCOUNTMANAGER_HH
