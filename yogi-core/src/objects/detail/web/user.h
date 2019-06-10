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
#include "group.h"

#include <nlohmann/json.hpp>

#include <memory>
#include <unordered_map>

namespace objects {
namespace detail {
namespace web {

class User;

typedef std::shared_ptr<User> UserPtr;
typedef std::unordered_map<std::string, UserPtr> UsersMap;

class User {
 public:
  static UsersMap CreateAllFromJson(const nlohmann::json& json,
                                    const GroupsMap& groups,
                                    const std::string& source = {});

  const nlohmann::json& ToJson() const { return props_; }
  bool IsEnabled() const { return enabled_; }
  const std::string& GetPassword() const { return password_; }
  const GroupsSet& GetGroups() const { return groups_; }

 private:
  nlohmann::json props_;
  bool enabled_;
  std::string password_;
  GroupsSet groups_;
};

}  // namespace web
}  // namespace detail
}  // namespace objects
