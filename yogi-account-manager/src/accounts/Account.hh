#ifndef ACCOUNTS_ACCOUNT_HH
#define ACCOUNTS_ACCOUNT_HH

#include <yogi.hpp>

#include "../storage/StorageInterface.hh"
#include "../proto/yogi_0000000d.h"
#include "../proto/yogi_0000040d.h"
#include "../proto/yogi_00000001.h"

#include <boost/asio/io_service.hpp>
#include <unordered_map>
#include <unordered_set>


namespace accounts {

class Account
{
public:
    Account(boost::asio::io_service& ios, storage::StorageProvider& storageProvider,
        std::string username, std::string password, std::string firstName,
        std::string lastName, std::unordered_set<std::string> groups, bool enabled);

    const std::string& username() const;
    const std::string& password() const;
    const std::string& first_name() const;
    const std::string& last_name() const;
    const std::unordered_set<std::string>& groups() const;
    bool enabled() const;

    void set_password(const std::string& password);
    void set_first_name(const std::string& firstName);
    void set_last_name(const std::string& lastName);
    void set_groups(const std::unordered_set<std::string>& groups);
    void set_enabled(bool enabled);

private:
    typedef yogi::CachedProducerTerminal<yogi_00000001> bool_terminal;
    typedef yogi::CachedProducerTerminal<yogi_0000000d> string_terminal;
    typedef yogi::CachedProducerTerminal<yogi_0000040d> list_terminal;

    boost::asio::io_service&        m_ios;
    yogi::Logger                    m_logger;
    const std::string               m_username;
    const yogi::Path                m_path;
    std::string                     m_password;
    std::string                     m_firstName;
    std::string                     m_lastName;
    std::unordered_set<std::string> m_groups;
    bool                            m_enabled;

    storage::StorageInterface       m_storage;
    string_terminal                 m_firstNameTerminal;
    string_terminal                 m_lastNameTerminal;
    bool_terminal                   m_enabledTerminal;
    list_terminal                   m_groupsTerminal;

    void publish_string(string_terminal* tm, const std::string& str);
    void publish_set(list_terminal* tm, const std::unordered_set<std::string>& set);
    void publish_bool(bool_terminal* tm, bool val);
};

typedef std::shared_ptr<Account> account_ptr;

} // namespace accounts

#endif // ACCOUNTS_ACCOUNT_HH
