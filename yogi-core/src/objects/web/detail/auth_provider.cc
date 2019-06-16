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
#include "../../../utils/json_helpers.h"
#include "../../../schema/schema.h"

#include <boost/filesystem.hpp>
#include <fstream>

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {
namespace web {
namespace detail {

AuthProviderPtr AuthProvider::Create(const nlohmann::json& cfg,
                                     const std::string& logging_prefix) {
  schema::ValidateJson(cfg, "web_authentication.schema.json");
  static const nlohmann::json dummy = nlohmann::json::object_t{};
  const auto& auth_cfg = cfg.value("authentication", dummy);

  AuthProviderPtr auth;

  auto provider = auth_cfg.value("provider", "config");
  if (provider == "config") {
    auth = std::make_unique<ConfigAuthProvider>();
  } else if (provider == "files") {
    auth = std::make_unique<FilesAuthProvider>();
  } else {
    YOGI_NEVER_REACHED;
  }

  auth->SetLoggingPrefix(logging_prefix);
  auth->readonly_ = auth_cfg.value("readonly", false);

  auto users_and_groups = auth->ReadConfiguration(auth_cfg);
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

std::tuple<UsersMap, GroupsMap> ConfigAuthProvider::ReadConfiguration(
    const nlohmann::json& auth_cfg) {
  const nlohmann::json* groups_cfg;
  if (auth_cfg.contains("groups")) {
    groups_cfg = &auth_cfg;
  } else {
    groups_cfg = &GetDefaultGroupsSection();
    LOG_IFO("Using default groups in configuration");
  }

  auto groups = Group::CreateAll(*groups_cfg);
  LOG_IFO("Loaded " << groups.size() << " groups from configuration");

  const nlohmann::json* users_cfg;
  if (auth_cfg.contains("users")) {
    users_cfg = &auth_cfg;
  } else {
    users_cfg = &GetDefaultUsersSection();
    LOG_IFO("Using default users in configuration");
  }

  auto users = User::CreateAll(*users_cfg, groups);
  LOG_IFO("Loaded " << users.size() << " users from configuration");

  return std::make_tuple(users, groups);
}

const nlohmann::json& ConfigAuthProvider::GetDefaultGroupsSection() {
  static nlohmann::json section = nlohmann::json::parse(R"(
    {
      "groups": {
        "admins": {
          "name":         "Administrators",
          "description":  "Users with unrestricted access to everything",
          "unrestricted": true
        },
        "users": {
          "name":         "Users",
          "description":  "All registered users"
        }
      }
    }
  )");

  return section;
}

const nlohmann::json& ConfigAuthProvider::GetDefaultUsersSection() {
  static nlohmann::json section = {
      // clang-format off
    {"users", {
      {api::kDefaultAdminUser, {
        {"first_name", "Administrator"},
        {"password",   utils::MakeSha256String(api::kDefaultAdminPassword)},
        {"groups",     nlohmann::json::array({"admins", "users"})},
      }}
    }}
      // clang-format on
  };

  return section;
}

std::tuple<UsersMap, GroupsMap> FilesAuthProvider::ReadConfiguration(
    const nlohmann::json& auth_cfg) {
  auto groups_file = auth_cfg["groups_file"].get<std::string>();
  YOGI_ASSERT(!groups_file.empty());
  groups_file = boost::filesystem::absolute(groups_file).string();

  auto groups_file_cfg = utils::ReadJsonFile(groups_file);
  auto groups = Group::CreateAll(groups_file_cfg, groups_file);
  LOG_IFO("Loaded " << groups.size() << " groups from " << groups_file);

  auto users_file = auth_cfg["users_file"].get<std::string>();
  YOGI_ASSERT(!users_file.empty());
  users_file = boost::filesystem::absolute(users_file).string();

  auto users_file_cfg = utils::ReadJsonFile(users_file);
  auto users = User::CreateAll(users_file_cfg, groups, users_file);
  LOG_IFO("Loaded " << users.size() << " users from " << users_file);

  return std::make_tuple(users, groups);
}

}  // namespace detail
}  // namespace web
}  // namespace objects
