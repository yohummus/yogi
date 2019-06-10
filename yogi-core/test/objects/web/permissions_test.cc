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
      "groups": {
        "people": {},
        "staff":  {},
        "admins": { "unrestricted": true }
      }
    }
  )"));

  UsersMap users_ = User::CreateAllFromJson(nlohmann::json::parse(R"(
    {
      "users": {
        "bob":   { "groups": ["people"] },
        "larry": { "groups": ["staff"]  },
        "admin": { "groups": ["admins"] }
      }
    }
  )"),
                                            groups_);
};

TEST_F(PermissionsTest, Construct) {
  auto cfg = nlohmann::json::parse(R"(
    {
      "*":      ["GET"],
      "people": ["POST"],
      "owner":  ["DELETE"]
    }
  )");

  EXPECT_NO_THROW(Permissions("/", cfg, groups_));
}

TEST_F(PermissionsTest, UnknownGroup) {
  auto cfg = nlohmann::json::parse(R"({"staff": ["GET"]})");
  EXPECT_NO_THROW(Permissions("/", cfg, groups_));

  cfg = nlohmann::json::parse(R"({"dogs": ["GET"]})");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("/", cfg, groups_),
                                 YOGI_ERR_CONFIG_NOT_VALID);
}

TEST_F(PermissionsTest, SectionInvalid) {
  auto cfg = nlohmann::json::parse(R"([])");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("/", cfg, groups_),
                                 YOGI_ERR_CONFIGURATION_VALIDATION_FAILED);
}

TEST_F(PermissionsTest, InvalidPermissionsArray) {
  auto cfg = nlohmann::json::parse(R"({"staff": {}})");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("/", cfg, groups_),
                                 YOGI_ERR_CONFIGURATION_VALIDATION_FAILED);
}

TEST_F(PermissionsTest, InvalidRequestMethod) {
  auto cfg = nlohmann::json::parse(R"({"staff": ["MOVE"]})");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("/", cfg, groups_),
                                 YOGI_ERR_CONFIGURATION_VALIDATION_FAILED);

  cfg = nlohmann::json::parse(R"({"staff": [3]})");
  EXPECT_THROW_DESCRIPTIVE_ERROR(Permissions("/", cfg, groups_),
                                 YOGI_ERR_CONFIGURATION_VALIDATION_FAILED);
}

TEST_F(PermissionsTest, MayUserAccess) {
  auto cfg = nlohmann::json::parse(R"(
    {
      "*":      ["PUT"],
      "people": ["POST"],
      "owner":  ["DELETE"]
    }
  )");

  Permissions perms("", cfg, groups_);

  // Everyone ("*")
  EXPECT_FALSE(perms.MayUserAccess({}, api::kGet, {}));
  EXPECT_TRUE(perms.MayUserAccess({}, api::kPut, {}));
  EXPECT_FALSE(perms.MayUserAccess({}, api::kPost, {}));

  // Restricted users
  auto bob = users_["bob"];
  EXPECT_FALSE(perms.MayUserAccess(bob, api::kGet, {}));
  EXPECT_TRUE(perms.MayUserAccess(bob, api::kPut, {}));
  EXPECT_TRUE(perms.MayUserAccess(bob, api::kPost, {}));
  EXPECT_FALSE(perms.MayUserAccess(bob, api::kDelete, {}));

  auto larry = users_["larry"];
  EXPECT_TRUE(perms.MayUserAccess(larry, api::kPut, {}));
  EXPECT_FALSE(perms.MayUserAccess(larry, api::kPost, {}));
  EXPECT_FALSE(perms.MayUserAccess(larry, api::kDelete, {}));

  // Unrestricted users
  auto admin = users_["admin"];
  EXPECT_TRUE(perms.MayUserAccess(admin, api::kGet, {}));
  EXPECT_TRUE(perms.MayUserAccess(admin, api::kHead, {}));
  EXPECT_TRUE(perms.MayUserAccess(admin, api::kPut, {}));
  EXPECT_TRUE(perms.MayUserAccess(admin, api::kPost, {}));
  EXPECT_TRUE(perms.MayUserAccess(admin, api::kDelete, {}));
  EXPECT_TRUE(perms.MayUserAccess(admin, api::kPatch, {}));

  // Owners
  EXPECT_FALSE(perms.MayUserAccess(bob, api::kDelete, {}));
  EXPECT_TRUE(perms.MayUserAccess(bob, api::kDelete, bob));
  EXPECT_FALSE(perms.MayUserAccess(larry, api::kDelete, bob));
}

TEST_F(PermissionsTest, GetImpliesHead) {
  // GET permissions imply HEAD permissions
  auto cfg = nlohmann::json::parse(R"({ "*": ["GET"] })");
  auto perms = Permissions("/", cfg, groups_);
  EXPECT_TRUE(perms.MayUserAccess({}, api::kGet, {}));
  EXPECT_TRUE(perms.MayUserAccess({}, api::kHead, {}));

  // But not the other way around!
  cfg = nlohmann::json::parse(R"({ "*": ["HEAD"] })");
  perms = Permissions("/", cfg, groups_);
  EXPECT_FALSE(perms.MayUserAccess({}, api::kGet, {}));
  EXPECT_TRUE(perms.MayUserAccess({}, api::kHead, {}));
}