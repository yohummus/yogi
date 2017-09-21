#include "AccountProvider.hh"
#include "FileAccountProvider.hh"

#include <algorithm>
using namespace std::string_literals;


namespace accounts {

account_provider_ptr AccountProvider::create_provider(boost::asio::io_service& ios,
    storage::StorageProvider& storageProvider, const std::string& configPath)
{
    auto type = yogi::ProcessInterface::config().get<std::string>(configPath + ".type");
    if (type == "file") {
        return std::make_unique<FileAccountProvider>(ios, storageProvider,
            yogi::ProcessInterface::config().get<std::string>(configPath + ".file"));
    }
    else {
        throw std::runtime_error("Unknown account provider type '"s + type + "'");
    }
}

AccountProvider::AccountProvider(boost::asio::io_service& ios,
    storage::StorageProvider& storageProvider)
: m_ios(ios)
, m_storageProvider(storageProvider)
, m_logger("AccountProvider")
{
}

AccountProvider::~AccountProvider()
{
}

boost::asio::io_service& AccountProvider::io_service()
{
    return m_ios;
}

storage::StorageProvider& AccountProvider::storage_provider()
{
    return m_storageProvider;
}

const AccountProvider::accounts_map& AccountProvider::accounts() const
{
    return m_accounts;
}

void AccountProvider::add_account(const account_ptr& account)
{
    m_accounts[account->username()] = account;
}

void AccountProvider::remove_account(const account_ptr& account)
{
    m_accounts.erase(account->username());
}

yogi::Logger& AccountProvider::logger()
{
    return m_logger;
}

} // namespace accounts
