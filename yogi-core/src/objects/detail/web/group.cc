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

#include "group.h"
#include "../../../api/errors.h"
#include "../../../utils/json_helpers.h"
#include "../../../utils/schema.h"

namespace objects {
namespace detail {
namespace web {

GroupsMap Group::CreateAll(const nlohmann::json& json,
                           const std::string& source) {
  utils::ValidateJson(json, "web_groups.schema.json", source);

  GroupsMap groups;
  for (auto it : json["groups"].items()) {
    auto group = std::make_shared<Group>();
    utils::CopyJsonProperty(it.value(), "name", "", &group->props_);
    utils::CopyJsonProperty(it.value(), "description", "", &group->props_);

    group->unrestricted_ = it.value().value("unrestricted", false);
    group->props_["unrestricted"] = group->unrestricted_;

    groups[it.key()] = group;
  }

  return groups;
}

}  // namespace web
}  // namespace detail
}  // namespace objects
