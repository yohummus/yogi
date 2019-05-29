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
#include "../../../api/enums.h"
#include "group.h"
#include "user.h"

#include <nlohmann/json.hpp>

#include <unordered_map>

namespace objects {
namespace detail {
namespace web {

class Permissions {
 public:
  Permissions(const std::string& base_uri, const nlohmann::json& cfg,
              const GroupsMap& groups);

  bool MayUserAccess(const UserPtr& user, api::RequestMethods method,
                     const UserPtr& route_owner) const;

 private:
  typedef std::unordered_map<GroupPtr, api::RequestMethods> AllowedMethodsMap;

  static api::RequestMethods ExtractMethods(
      const std::string& base_uri, const nlohmann::json::const_iterator& it);
  static GroupPtr GetGroup(const std::string& base_uri,
                           const nlohmann::json::const_iterator& it,
                           const GroupsMap& groups);

  api::RequestMethods allowed_for_everyone_;
  api::RequestMethods allowed_for_owner_;
  AllowedMethodsMap allowed_for_group_;
};

}  // namespace web
}  // namespace detail
}  // namespace objects
