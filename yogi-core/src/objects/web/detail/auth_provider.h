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

#pragma once

#include "../../../config.h"
#include "../../log/logger.h"
#include "group.h"
#include "user.h"

#include <nlohmann/json.hpp>
#include <memory>
#include <tuple>

namespace objects {
namespace web {
namespace detail {

class AuthProvider;
class ConfigAuthProvider;
class FilesAuthProvider;

typedef std::shared_ptr<AuthProvider> AuthProviderPtr;

class AuthProvider : public objects::log::LoggerUser {
 public:
  static AuthProviderPtr Create(const nlohmann::json& cfg,
                                const std::string& logging_prefix);

  virtual ~AuthProvider() {}

  bool IsReadonly() const { return readonly_; }
  const UsersMap& GetUsers() const { return users_; }
  const GroupsMap& GetGroups() const { return groups_; }
  UserPtr GetUserOptional(const std::string& user_name) const;
  GroupPtr GetGroupOptional(const std::string& group_name) const;

 protected:
  virtual std::tuple<UsersMap, GroupsMap> ReadConfiguration(
      const nlohmann::json& auth_cfg) = 0;

 private:
  bool readonly_;
  UsersMap users_;
  GroupsMap groups_;
};

class ConfigAuthProvider : public AuthProvider {
 protected:
  virtual std::tuple<UsersMap, GroupsMap> ReadConfiguration(
      const nlohmann::json& auth_cfg) override;

 private:
  static const nlohmann::json& GetDefaultGroupsSection();
  static const nlohmann::json& GetDefaultUsersSection();
};

class FilesAuthProvider : public AuthProvider {
 protected:
  virtual std::tuple<UsersMap, GroupsMap> ReadConfiguration(
      const nlohmann::json& auth_cfg) override;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
