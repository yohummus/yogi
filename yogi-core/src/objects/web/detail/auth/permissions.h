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

#include "../../../../config.h"
#include "../../../../api/enums.h"
#include "group.h"
#include "user.h"

#include <nlohmann/json.hpp>

#include <unordered_map>

namespace objects {
namespace web {
namespace detail {

class Permissions {
 public:
  Permissions() {}
  Permissions(const std::string& base_uri, const nlohmann::json& cfg,
              UserPtr default_owner, const GroupsMap& groups);

  const UserPtr& GetDefaultOwner() const { return default_owner_; }

  bool MayUserAccess(const UserPtr& user, api::RequestMethods method) const;

  bool MayUserAccess(const UserPtr& user, api::RequestMethods method,
                     const UserPtr& owner) const;

 private:
  typedef std::unordered_map<GroupPtr, api::RequestMethods> AllowedMethodsMap;

  static api::RequestMethods ExtractMethods(
      const nlohmann::json::const_iterator& it);
  static GroupPtr GetGroup(const std::string& base_uri,
                           const nlohmann::json::const_iterator& it,
                           const GroupsMap& groups);

  UserPtr default_owner_;
  api::RequestMethods allowed_for_everyone_;
  api::RequestMethods allowed_for_owner_;
  AllowedMethodsMap allowed_for_group_;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
