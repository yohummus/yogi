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
#include "../../../api/errors.h"

#include <map>

namespace objects {
namespace detail {
namespace web {

Permissions::Permissions(const std::string& base_uri, const nlohmann::json& cfg,
                         const GroupsMap& groups) {
  if (!cfg.is_object()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Invalid permissions section in route " << base_uri << ".";
  }

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

bool Permissions::MayUserAccess(const UserPtr& user, api::RequestMethods method,
                                const UserPtr& route_owner) const {
  if (user) {
    if (route_owner && route_owner == user && (allowed_for_owner_ & method)) {
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
  if (!it->is_array()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Invalid permissions array for group \"" << it.key()
        << "\" in route " << base_uri << ".";
  }

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

    if (!entry.is_string()) {
      throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
          << "Invalid entry in permissions array for group \"" << it.key()
          << "\" in route " << base_uri << ".";
    }

    auto method_str = entry.get<std::string>();
    auto map_it = map.find(method_str);
    if (map_it == map.end()) {
      throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
          << "Invalid request method \"" << method_str
          << "\" in permissions for group \"" << it.key() << "\" in route "
          << base_uri << ".";
    }

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

}  // namespace web
}  // namespace detail
}  // namespace objects
