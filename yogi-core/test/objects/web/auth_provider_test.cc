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
#include "../../../src/api/constants.h"
#include "../../../src/utils/crypto.h"
#include "../../../src/objects/web/detail/auth_provider.h"
using objects::web::detail::AuthProvider;

class AuthProviderTest : public TestFixture {};

TEST_F(AuthProviderTest, Defaults) {
  auto auth = AuthProvider::Create(nlohmann::json::object_t{}, "");
  EXPECT_FALSE(auth->IsReadonly());

  auto user = auth->GetUserOptional(api::kDefaultAdminUser);
  EXPECT_TRUE(!!user);
  EXPECT_EQ(user->GetPassword(),
            utils::MakeSha256String(api::kDefaultAdminPassword));
  EXPECT_EQ(user->GetGroups().size(), 2);
}

TEST_F(AuthProviderTest, IsReadonly) {
  auto cfg = nlohmann::json::parse(R"({"authentication": {"readonly":true}})");
  auto auth = AuthProvider::Create(cfg, "");
  EXPECT_TRUE(auth->IsReadonly());

  cfg = nlohmann::json::parse(R"({"authentication": {"readonly":false}})");
  auth = AuthProvider::Create(cfg, "");
  EXPECT_FALSE(auth->IsReadonly());
}

TEST_F(AuthProviderTest, GetUserOptional) {
  auto auth = AuthProvider::Create(nlohmann::json::object_t{}, "");
  EXPECT_TRUE(!!auth->GetUserOptional(api::kDefaultAdminUser));
  EXPECT_FALSE(!!auth->GetUserOptional("larry"));
}

TEST_F(AuthProviderTest, GetGroupOptional) {
  auto auth = AuthProvider::Create(nlohmann::json::object_t{}, "");
  EXPECT_TRUE(!!auth->GetGroupOptional("users"));
  EXPECT_FALSE(!!auth->GetGroupOptional("aliens"));
}

TEST_F(AuthProviderTest, LoadFromConfig) {
  auto auth_cfg = nlohmann::json::parse(R"(
    {
      "authentication": {
        "provider": "config",
        "readonly": true,
        "users": {
          "larry": {
            "first_name": "Larry",
            "last_name": "Green",
            "email": "larry@green.com",
            "phone": "123-456",
            "password": "secret",
            "groups": ["people"],
            "enabled": false
          }
        },
        "groups": {
          "people": {
            "name": "People",
            "description": "Blabla"
          }
        }
      }
    }
  )");

  auto auth = AuthProvider::Create(auth_cfg, "");
  EXPECT_TRUE(auth->IsReadonly());
  EXPECT_FALSE(auth->GetUserOptional(api::kDefaultAdminUser));

  auto group = auth->GetGroupOptional("people");
  ASSERT_TRUE(!!group);
  EXPECT_EQ(group->ToJson().value("name", "NOT FOUND"), "People");
  EXPECT_EQ(group->ToJson().value("description", "NOT FOUND"), "Blabla");
  EXPECT_TRUE(group->ToJson().contains("unrestricted"));
  EXPECT_FALSE(group->ToJson()["unrestricted"].get<bool>());

  auto user = auth->GetUserOptional("larry");
  ASSERT_TRUE(!!user);
  EXPECT_FALSE(user->IsEnabled());
  EXPECT_EQ(user->GetPassword(), "secret");
  ASSERT_EQ(user->GetGroups().size(), 1);
  EXPECT_EQ(*user->GetGroups().begin(), group);
  EXPECT_EQ(user->ToJson().value("first_name", "NOT FOUND"), "Larry");
  EXPECT_EQ(user->ToJson().value("last_name", "NOT FOUND"), "Green");
  EXPECT_EQ(user->ToJson().value("email", "NOT FOUND"), "larry@green.com");
  EXPECT_EQ(user->ToJson().value("phone", "NOT FOUND"), "123-456");
  ASSERT_TRUE(user->ToJson().contains("groups"));
  EXPECT_EQ(user->ToJson()["groups"][0].get<std::string>(), "people");
  EXPECT_TRUE(user->ToJson().contains("enabled"));
  EXPECT_FALSE(user->ToJson()["enabled"].get<bool>());
}

TEST_F(AuthProviderTest, LoadFromFiles) {
  nlohmann::json auth_cfg = {
      {"authentication",
       {
           {"provider", "files"},
           {"users_file", MakeTestDataPath("users.json")},
           {"groups_file", MakeTestDataPath("groups.json")},
       }}};

  auto auth = AuthProvider::Create(auth_cfg, "");
  EXPECT_FALSE(auth->IsReadonly());
  EXPECT_FALSE(auth->GetUserOptional(api::kDefaultAdminUser));

  auto group = auth->GetGroupOptional("staff");
  ASSERT_TRUE(!!group);
  EXPECT_EQ(group->ToJson().value("name", "NOT FOUND"), "Staff");
  EXPECT_EQ(group->ToJson().value("description", "NOT FOUND"), "Employees");
  EXPECT_TRUE(group->ToJson().contains("unrestricted"));
  EXPECT_TRUE(group->ToJson()["unrestricted"].get<bool>());

  auto user = auth->GetUserOptional("katy");
  ASSERT_TRUE(!!user);
  EXPECT_TRUE(user->IsEnabled());
  EXPECT_EQ(user->GetPassword(), "gum");
  ASSERT_EQ(user->GetGroups().size(), 1);
  EXPECT_EQ(*user->GetGroups().begin(), group);
  EXPECT_EQ(user->ToJson().value("first_name", "NOT FOUND"), "Katy");
  EXPECT_EQ(user->ToJson().value("last_name", "NOT FOUND"), "Hill");
  EXPECT_EQ(user->ToJson().value("email", "NOT FOUND"), "katy@hill.com");
  EXPECT_EQ(user->ToJson().value("phone", "NOT FOUND"), "555-222");
  ASSERT_TRUE(user->ToJson().contains("groups"));
  EXPECT_EQ(user->ToJson()["groups"][0].get<std::string>(), "staff");
  EXPECT_TRUE(user->ToJson().contains("enabled"));
  EXPECT_TRUE(user->ToJson()["enabled"].get<bool>());
}
