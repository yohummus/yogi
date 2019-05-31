/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2019 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "auth_provider.h"
#include "../../../api/errors.h"
#include "../../../api/constants.h"
#include "../../../utils/crypto.h"

#include <boost/filesystem.hpp>
#include <fstream>

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {
namespace detail {
namespace web {

AuthProviderPtr AuthProvider::Create(const nlohmann::json& auth_cfg,
                                     const std::string& logging_prefix) {
  static const nlohmann::json dummy = nlohmann::json::object_t{};

  auto& cfg = auth_cfg.is_null() ? dummy : auth_cfg;
  if (!cfg.is_object()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Invalid authentication section.";
  }

  AuthProviderPtr auth;

  auto provider = cfg.value("provider", std::string{});
  if (!cfg.contains("provider") || provider == "config") {
    auth = std::make_unique<ConfigAuthProvider>();
  } else if (provider == "files") {
    auth = std::make_unique<FilesAuthProvider>();
  } else {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Invalid authentication provider. Valid types are \"config\" or "
           "\"files\".";
  }

  auth->SetLoggingPrefix(logging_prefix);
  auth->readonly_ = cfg.value("readonly", false);

  auto users_and_groups = auth->ReadConfiguration(cfg);
  auth->users_ = std::get<UsersMap>(users_and_groups);
  auth->groups_ = std::get<GroupsMap>(users_and_groups);

  return auth;
}

UserPtr AuthProvider::GetUserOptional(const std::string& user_name) const {
  auto it = users_.find(user_name);
  if (it == users_.end()) return {};
  return it->second;
}

GroupPtr AuthProvider::GetGroupOptional(const std::string& group_name) const {
  auto it = groups_.find(group_name);
  if (it == groups_.end()) return {};
  return it->second;
}

nlohmann::json AuthProvider::GetSection(const nlohmann::json& json,
                                        const char* key,
                                        const std::string& source) {
  auto it = json.find(key);
  if (it == json.end() || !it.value().is_object()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid \"" << key << "\" section in " << source << ".";
  }

  return it.value();
}

nlohmann::json AuthProvider::GetSectionFromFile(const std::string& file,
                                                const char* key) {
  std::ifstream ifs(file);
  if (!ifs.is_open() || ifs.fail()) {
    throw api::DescriptiveError(YOGI_ERR_READ_FILE_FAILED)
        << "The file " << file << " does not exist or is not readable.";
  }

  nlohmann::json json;
  try {
    ifs >> json;
  } catch (const std::exception& e) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_FILE_FAILED)
        << "Could not parse " << file << ": " << e.what();
  }

  return GetSection(json, key, file);
}

std::tuple<UsersMap, GroupsMap> ConfigAuthProvider::ReadConfiguration(
    const nlohmann::json& auth_cfg) {
  nlohmann::json groups_cfg;
  if (auth_cfg.contains("groups")) {
    groups_cfg = GetSection(auth_cfg, "groups", "authentication settings");
  } else {
    groups_cfg = MakeDefaultGroupsSection();
    LOG_IFO("Using default groups in configuration");
  }

  auto groups = Group::CreateAllFromJson(groups_cfg);
  LOG_IFO("Loaded " << groups.size() << " groups from configuration");

  nlohmann::json users_cfg;
  if (auth_cfg.contains("users")) {
    users_cfg = GetSection(auth_cfg, "users", "authentication settings");
  } else {
    users_cfg = MakeDefaultUsersSection();
    LOG_IFO("Using default users in configuration");
  }

  auto users = User::CreateAllFromJson(users_cfg, groups);
  LOG_IFO("Loaded " << users.size() << " users from configuration");

  return std::make_tuple(users, groups);
}

nlohmann::json ConfigAuthProvider::MakeDefaultGroupsSection() {
  static nlohmann::json section = {
      {"admins",
       {{"name", "Administrators"},
        {"description", "Users with unrestricted access to everything"},
        {"unrestricted", true}}},
      {"users",
       {
           {"name", "Users"},
           {"description", "All registered users"},
       }},
  };

  return section;
}

nlohmann::json ConfigAuthProvider::MakeDefaultUsersSection() {
  static nlohmann::json section = {
      {api::kDefaultAdminUser,
       {
           {"first_name", "Administrator"},
           {"password", utils::MakeSha256String(api::kDefaultAdminPassword)},
           {"groups", nlohmann::json::array({"admins", "users"})},
       }}};

  return section;
}

std::tuple<UsersMap, GroupsMap> FilesAuthProvider::ReadConfiguration(
    const nlohmann::json& auth_cfg) {
  auto groups_file = auth_cfg.value("groups_file", "");
  if (groups_file.empty()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid groups_file in authentication section.";
  }

  groups_file = boost::filesystem::absolute(groups_file).string();
  auto groups_section = GetSectionFromFile(groups_file, "groups");
  auto groups = Group::CreateAllFromJson(groups_section);
  LOG_IFO("Loaded " << groups.size() << " groups from " << groups_file);

  auto users_file = auth_cfg.value("users_file", "");
  if (users_file.empty()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid users_file in authentication section.";
  }

  users_file = boost::filesystem::absolute(users_file).string();
  auto users_section = GetSectionFromFile(users_file, "users");
  auto users = User::CreateAllFromJson(users_section, groups);
  LOG_IFO("Loaded " << users.size() << " users from " << users_file);

  return std::make_tuple(users, groups);
}

}  // namespace web
}  // namespace detail
}  // namespace objects
