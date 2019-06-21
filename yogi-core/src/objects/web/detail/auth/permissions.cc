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

#include "permissions.h"
#include "../../../../api/errors.h"
#include "../../../../schema/schema.h"

#include <map>
using namespace std::string_literals;

namespace objects {
namespace web {
namespace detail {

Permissions::Permissions(const std::string& base_uri, const nlohmann::json& cfg,
                         UserPtr default_owner, const GroupsMap& groups)
    : default_owner_(default_owner),
      allowed_for_everyone_(api::kNoMethod),
      allowed_for_owner_(api::kNoMethod) {
  schema::ValidateJson(cfg, "web_permissions.schema.json",
                       "In route "s + base_uri);

  for (auto it = cfg.begin(); it != cfg.end(); ++it) {
    auto methods = ExtractMethods(base_uri, it);

    if (it.key() == "*") {
      allowed_for_everyone_ = methods;
    } else if (it.key() == "owner") {
      allowed_for_owner_ = methods;
    } else {
      auto group = GetGroup(base_uri, it, groups);
      allowed_for_group_[group] = methods;
    }
  }
}

bool Permissions::MayUserAccess(const UserPtr& user,
                                api::RequestMethods method) const {
  return MayUserAccess(user, method, default_owner_);
}

bool Permissions::MayUserAccess(const UserPtr& user, api::RequestMethods method,
                                const UserPtr& owner) const {
  if (user) {
    if (owner && owner == user && (allowed_for_owner_ & method)) {
      return true;
    }

    for (auto& group : user->GetGroups()) {
      if (group->IsUnrestricted()) {
        return true;
      }

      auto it = allowed_for_group_.find(group);
      if (it != allowed_for_group_.end() && (it->second & method)) {
        return true;
      }
    }
  }

  return !!(allowed_for_everyone_ & method);
}

api::RequestMethods Permissions::ExtractMethods(
    const std::string& base_uri, const nlohmann::json::const_iterator& it) {
  YOGI_ASSERT(it->is_array());

  auto methods = api::kNoMethod;
  for (auto entry : *it) {
    static const std::map<std::string, api::RequestMethods> map = {
        {"GET", api::kGet | api::RequestMethods::kHead},
        {"HEAD", api::kHead},
        {"POST", api::kPost},
        {"PUT", api::kPut},
        {"DELETE", api::kDelete},
        {"PATCH", api::kPatch},
    };

    YOGI_ASSERT(entry.is_string());

    auto method_str = entry.get<std::string>();
    auto map_it = map.find(entry.get<std::string>());
    YOGI_ASSERT(map_it != map.end());

    methods |= map_it->second;
  }

  return methods;
}

GroupPtr Permissions::GetGroup(const std::string& base_uri,
                               const nlohmann::json::const_iterator& it,
                               const GroupsMap& groups) {
  auto group = groups.find(it.key());
  if (group == groups.end()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Unknown group \"" << it.key() << "\" in permissions for route "
        << base_uri << ".";
  }

  return group->second;
}

}  // namespace detail
}  // namespace web
}  // namespace objects
