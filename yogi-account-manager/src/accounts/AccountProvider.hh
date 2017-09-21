#ifndef ACCOUNTS_ACCOUNTPROVIDER_HH
#define ACCOUNTS_ACCOUNTPROVIDER_HH

#include "Account.hh"


namespace accounts {

class AccountProvider
{
public:
    typedef std::unordered_map<std::string, account_ptr> accounts_map;
    typedef std::function<void (bool success)>           completion_handler;

    static std::unique_ptr<AccountProvider> create_provider(boost::asio::io_service& ios,
        storage::StorageProvider& storageProvider, const std::string& configPath);

    AccountProvider(boost::asio::io_service& ios, storage::StorageProvider& storageProvider);
    virtual ~AccountProvider();

    boost::asio::io_service& io_service();
    storage::StorageProvider& storage_provider();

    virtual void async_load_accounts(completion_handler fn) =0;
    virtual void async_save_accounts(completion_handler fn) =0;

    const accounts_map& accounts() const;
    void add_account(const account_ptr& account);
    void remove_account(const account_ptr& account);

protected:
    yogi::Logger& logger();

private:
    boost::asio::io_service&  m_ios;
    storage::StorageProvider& m_storageProvider;
    yogi::Logger              m_logger;
    accounts_map              m_accounts;
};

typedef std::unique_ptr<AccountProvider> account_provider_ptr;

} // namespace accounts

#endif // ACCOUNTS_ACCOUNTPROVIDER_HH
