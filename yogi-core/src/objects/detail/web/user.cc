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

#include "user.h"
#include "../../../api/errors.h"
#include "../../../utils/json_helpers.h"

namespace objects {
namespace detail {
namespace web {

UsersMap User::CreateAllFromJson(const nlohmann::json& json,
                                 const GroupsMap& groups) {
  UsersMap users;
  for (auto it : json.items()) {
    auto user = std::make_shared<User>();
    utils::CopyJsonProperty(it.value(), "first_name", "", &user->props_);
    utils::CopyJsonProperty(it.value(), "last_name", "", &user->props_);
    utils::CopyJsonProperty(it.value(), "email", "", &user->props_);
    utils::CopyJsonProperty(it.value(), "phone", "", &user->props_);
    utils::CopyJsonProperty(it.value(), "enabled", true, &user->props_);

    user->enabled_ = user->props_["enabled"].get<bool>();
    user->password_ = it.value().value("password", "");

    if (!it.value().contains("groups")) {
      throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
          << "Section for user \"" << it.key()
          << "\" does not contain a groups array.";
    }

    nlohmann::json groups_arr;
    for (auto group : it.value()["groups"]) {
      if (!group.is_string()) {
        throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
            << "Groups array for user \"" << it.key()
            << "\" contains values other than strings.";
      }

      groups_arr.push_back(group);

      auto group_name = group.get<std::string>();
      auto group_it = groups.find(group_name);
      if (group_it == groups.end()) {
        throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
            << "Group \"" << group_name << "\" for user \"" << it.key()
            << "\" does not exist.";
      }

      user->groups_.insert(group_it->second);
    }
    user->props_["groups"] = groups_arr;

    users[it.key()] = user;
  }

  return users;
}

}  // namespace web
}  // namespace detail
}  // namespace objects
