#include "Account.hh"


namespace accounts {

Account::Account(boost::asio::io_service& ios, storage::StorageProvider& storageProvider,
    std::string username, std::string password, std::string firstName, std::string lastName,
    std::unordered_set<std::string> groups, bool enabled)
: m_ios(ios)
, m_logger("Account")
, m_username(username)
, m_path(yogi::Path("Accounts") / username)
, m_storage(ios, storageProvider, m_logger, m_path, username)
, m_firstNameTerminal(m_path / "First Name")
, m_lastNameTerminal(m_path / "Last Name")
, m_enabledTerminal(m_path / "Enabled")
, m_groupsTerminal(m_path / "Groups")
{
    set_password(password);
    set_first_name(firstName);
    set_last_name(lastName);
    set_groups(groups);
    set_enabled(enabled);
}

const std::string& Account::username() const
{
    return m_username;
}

const std::string& Account::password() const
{
    return m_password;
}

const std::string& Account::first_name() const
{
    return m_firstName;
}

const std::string& Account::last_name() const
{
    return m_lastName;
}

const std::unordered_set<std::string>& Account::groups() const
{
    return m_groups;
}

bool Account::enabled() const
{
    return m_enabled;
}

void Account::set_password(const std::string& password)
{
    m_password = password;
}

void Account::set_first_name(const std::string& firstName)
{
    m_firstName = firstName;
    publish_string(&m_firstNameTerminal, firstName);
}

void Account::set_last_name(const std::string& lastName)
{
    m_lastName = lastName;
    publish_string(&m_lastNameTerminal, lastName);
}

void Account::set_groups(const std::unordered_set<std::string>& groups)
{
    m_groups = groups;
    publish_set(&m_groupsTerminal, groups);
}

void Account::set_enabled(bool enabled)
{
    m_enabled = enabled;
    publish_bool(&m_enabledTerminal, enabled);
}

void Account::publish_string(string_terminal* tm, const std::string& str)
{
    auto msg = tm->make_message();
    msg.set_value(str);
    tm->try_publish(msg);
}

void Account::publish_set(list_terminal* tm, const std::unordered_set<std::string>& set)
{
    auto msg = tm->make_message();
    for (auto& str : set) {
        msg.add_value(str);
    }

    tm->try_publish(msg);
}

void Account::publish_bool(bool_terminal* tm, bool val)
{
    auto msg = tm->make_message();
    msg.set_value(val);
    tm->try_publish(msg);
}

} // namespace accounts
