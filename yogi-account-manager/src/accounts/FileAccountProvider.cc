#include "FileAccountProvider.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>

using namespace std::string_literals;


namespace accounts {

FileAccountProvider::FileAccountProvider(boost::asio::io_service& ios,
    storage::StorageProvider& storageProvider, const std::string& filename)
: AccountProvider(ios, storageProvider)
, m_filename(filename)
{
}

void FileAccountProvider::async_load_accounts(completion_handler fn)
{
    YOGI_LOG_INFO("Loading accounts from " << m_filename << "...");

    boost::property_tree::ptree ptree;

    try {
        boost::property_tree::read_json(m_filename, ptree);
    }
    catch (const std::exception& e) {
        YOGI_LOG_ERROR(logger(), "Could not read " << m_filename << ": " << e.what());
        io_service().post([=] { fn(false); });
        return;
    }

    while (!accounts().empty()) {
        remove_account(accounts().begin()->second);
    }

    for (auto accChild : ptree) {
        auto username  = accChild.first;
        auto password  = accChild.second.get<std::string>("password");
        auto firstName = accChild.second.get<std::string>("first-name");
        auto lastName  = accChild.second.get<std::string>("last-name");
        auto enabled   = accChild.second.get<bool>("enabled");

        std::unordered_set<std::string> groups;
        for (auto groupChild : accChild.second.get_child("groups")) {
            groups.insert(groupChild.second.get_value<std::string>());
        }

        auto account = std::make_shared<Account>(io_service(), storage_provider(), username,
            password, firstName, lastName, groups, enabled);
        add_account(account);

        YOGI_LOG_DEBUG(logger(), "Loaded account " << username << " ("
            << (enabled ? "enabled" : "disabled") << ")");
    }

    io_service().post([=] { fn(true); });
}

void FileAccountProvider::async_save_accounts(completion_handler fn)
{
    YOGI_LOG_INFO(logger(), "Saving accounts to " << m_filename << "...");

    std::vector<std::string> lines;
    lines.push_back("{");

    for (auto& username : get_sorted_usernames()) {
        auto account = accounts().at(username);

        lines.push_back("  \""s + username + "\": {");
        lines.push_back("    \"password\": \""s + account->password() + "\",");
        lines.push_back("    \"first-name\": \""s + account->first_name() + "\",");
        lines.push_back("    \"last-name\": \""s + account->last_name() + "\",");
        lines.push_back("    \"enabled\": "s + (account->enabled() ? "true" : "false") + ",");

        lines.push_back("    \"groups\": [");
        for (auto& group : get_sorted_groups(account)) {
            lines.push_back("      \""s + group + "\",");
        }

        if (!account->groups().empty()) {
            lines.back().pop_back();
        }

        lines.push_back("    ]");

        lines.push_back("  },");
    }

    if (!accounts().empty()) {
        lines.back().pop_back();
    }

    lines.push_back("}");

    try {
        std::ofstream file(m_filename, std::ios::out | std::ios::trunc);
        for (auto& line : lines) {
            file << line << std::endl;
        }
        file.close();

        io_service().post([=] { fn(true); });
    }
    catch (const std::exception& e) {
        YOGI_LOG_ERROR(logger(), "Could not write " << m_filename << ": " << e.what());
        io_service().post([=] { fn(false); });
    }
}

std::vector<std::string> FileAccountProvider::get_sorted_usernames()
{
    std::vector<std::string> usernames;
    for (auto& account : accounts()) {
        usernames.push_back(account.first);
    }

    std::sort(usernames.begin(), usernames.end());

    return usernames;
}

std::vector<std::string> FileAccountProvider::get_sorted_groups(const account_ptr& account)
{
    std::vector<std::string> groups;
    for (auto& group : account->groups()) {
        groups.push_back(group);
    }

    std::sort(groups.begin(), groups.end());

    return groups;
}

} // namespace accounts
