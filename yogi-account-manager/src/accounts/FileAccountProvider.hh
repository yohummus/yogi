#ifndef ACCOUNTS_FILEACCOUNTPROVIDER_HH
#define ACCOUNTS_FILEACCOUNTPROVIDER_HH

#include "AccountProvider.hh"


namespace accounts {

class FileAccountProvider : public AccountProvider
{
public:
    FileAccountProvider(boost::asio::io_service& ios, storage::StorageProvider& storageProvider,
        const std::string& filename);

    virtual void async_load_accounts(completion_handler fn) override;
    virtual void async_save_accounts(completion_handler fn) override;

private:
    const std::string m_filename;

    std::vector<std::string> get_sorted_usernames();
    std::vector<std::string> get_sorted_groups(const account_ptr& account);
};

} // namespace accounts

#endif // ACCOUNTS_FILEACCOUNTPROVIDER_HH
