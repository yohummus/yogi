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

#include "../../common.h"
#include "../../../src/objects/detail/web/permissions.h"
using namespace objects::detail::web;

#include <boost/algorithm/string.hpp>

class PermissionsTest : public TestFixture {
 protected:
  GroupsMap groups_ = Group::CreateAllFromJson(nlohmann::json::parse(R"(
        {
          "people": {},
          "staff": {},
          "admins": { "unrestricted": true }
        }
    )"));
};

TEST_F(PermissionsTest, Construct) {
  auto cfg = nlohmann::json::parse(R"(
        {
          "*": ["GET"],
          "people": ["POST"],
          "owner": ["DELETE"]
        }
    )");

  EXPECT_NO_THROW(Permissions("", cfg, groups_));
}

TEST_F(PermissionsTest, UnknownGroup) {
  auto cfg = nlohmann::json::parse(R"({"staff": ["GET"]})");
  EXPECT_NO_THROW(Permissions("", cfg, groups_));

  cfg = nlohmann::json::parse(R"({"dogs": ["GET"]})");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("", cfg, groups_),
                                 YOGI_ERR_CONFIG_NOT_VALID);
}

TEST_F(PermissionsTest, SectionInvalid) {
  auto cfg = nlohmann::json::parse(R"([])");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("", cfg, groups_),
                                 YOGI_ERR_CONFIG_NOT_VALID);
}

TEST_F(PermissionsTest, InvalidPermissionsArray) {
  auto cfg = nlohmann::json::parse(R"({"staff": {}})");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("", cfg, groups_),
                                 YOGI_ERR_CONFIG_NOT_VALID);
}

TEST_F(PermissionsTest, InvalidRequestMethod) {
  auto cfg = nlohmann::json::parse(R"({"staff": ["MOVE"]})");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("", cfg, groups_),
                                 YOGI_ERR_CONFIG_NOT_VALID);

  cfg = nlohmann::json::parse(R"({"staff": [3]})");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("", cfg, groups_),
                                 YOGI_ERR_CONFIG_NOT_VALID);
}

TEST_F(PermissionsTest, DISABLED_MayUserAccess) {}
