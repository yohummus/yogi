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

#include <nlohmann/json.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace objects {
namespace web {
namespace detail {

class Group;

typedef std::shared_ptr<Group> GroupPtr;
typedef std::unordered_map<std::string, GroupPtr> GroupsMap;
typedef std::unordered_set<GroupPtr> GroupsSet;

class Group {
 public:
  static GroupsMap CreateAll(const nlohmann::json& json,
                             const std::string& source = {});

  const nlohmann::json& ToJson() const { return props_; }
  bool IsUnrestricted() const { return unrestricted_; }

 private:
  nlohmann::json props_;
  bool unrestricted_;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
