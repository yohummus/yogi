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
#include "../../logger.h"
#include "group.h"
#include "user.h"

#include <nlohmann/json.hpp>
#include <memory>
#include <tuple>

namespace objects {
namespace detail {
namespace web {

class AuthProvider;
class ConfigAuthProvider;
class FilesAuthProvider;

typedef std::unique_ptr<AuthProvider> AuthProviderPtr;

class AuthProvider : public objects::LoggerUser {
 public:
  static AuthProviderPtr Create(const nlohmann::json& auth_cfg,
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

  nlohmann::json GetSection(const nlohmann::json& json, const char* key,
                            const std::string& source);
  nlohmann::json GetSectionFromFile(const std::string& file, const char* key);

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
  static nlohmann::json MakeDefaultGroupsSection();
  static nlohmann::json MakeDefaultUsersSection();
};

class FilesAuthProvider : public AuthProvider {
 protected:
  virtual std::tuple<UsersMap, GroupsMap> ReadConfiguration(
      const nlohmann::json& auth_cfg) override;
};

}  // namespace web
}  // namespace detail
}  // namespace objects
