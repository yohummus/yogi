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
#include "../../../src/objects/web/detail/route/api_endpoint.h"
#include "../../../src/objects/web/detail/route/content_route.h"
#include "../../../src/objects/web/detail/route/custom_route.h"
#include "../../../src/objects/web/detail/route/file_system_route.h"
#include "../../../src/objects/web/detail/route/route.h"
using namespace objects::web::detail;

class RoutesTest : public TestFixture {
 protected:
  template <typename T = Route>
  T& GetRoute(const std::string& base_uri) {
    for (auto& route : *routes_) {
      if (route->GetBaseUri() == base_uri) {
        auto p = dynamic_cast<T*>(route.get());
        EXPECT_NE(p, nullptr) << "Route " << base_uri << " has the wrong type.";
        return *p;
      }
    }

    EXPECT_FALSE(true) << "Route " << base_uri << " not found.";
    throw "Route not found";  // To make compiler happy
  }

  AuthProviderPtr auth_ = AuthProvider::Create(nlohmann::json::object_t{}, "");

  nlohmann::json cfg_ = nlohmann::json::parse(R"(
    {
      "routes": {
        "/": {
          "type":        "filesystem",
          "path":        "www",
          "permissions": { "*": ["GET"] },
          "enabled":     true
        },
        "/somewhere": {
          "type":        "filesystem",
          "path":        "/tmp",
          "permissions": { "*": ["GET"] },
          "enabled":     true
        },
        "/somewhere/far-away": {
          "type":        "filesystem",
          "path":        "/home",
          "permissions": { "*": ["GET"] },
          "enabled":     true
        },
        "/secret": {
          "type":        "content",
          "mime":        "text/plain",
          "owner":       "admin",
          "permissions": { "users": ["GET"], "owner": ["PUT"] },
          "content":     "I am your father",
          "enabled":     false
        },
        "/colors": {
          "type":        "content",
          "mime":        "text/plain",
          "content": [
            "red",
            "green"
          ],
          "permissions": {}
        },
        "/messages": {
          "type":        "custom",
          "permissions": { "*": ["GET"], "users": ["POST"] }
        }
      },
      "api_permissions": {
        "/auth/session": { "*": ["POST", "DELETE"] },
        "/auth/groups":  { "*": ["GET"] }
      }
    }
  )");

  RoutesVectorPtr routes_ = Route::CreateAll(cfg_, *auth_, "bla");
};

TEST_F(RoutesTest, NumberOfRoutes) {
  EXPECT_EQ(routes_->size(), 8);
  for (auto& route : *routes_) {
    EXPECT_TRUE(!!route);
  }
}

TEST_F(RoutesTest, BasicRouteProperties) {
  EXPECT_EQ(GetRoute("/secret").GetBaseUri(), "/secret");
  EXPECT_FALSE(GetRoute("/").GetOwner());
  EXPECT_EQ(GetRoute("/secret").GetOwner(), auth_->GetUserOptional("admin"));
  EXPECT_TRUE(GetRoute("/").IsEnabled());
  EXPECT_TRUE(GetRoute("/messages").IsEnabled());
  EXPECT_FALSE(GetRoute("/secret").IsEnabled());
  EXPECT_TRUE(GetRoute("/").GetPermissions().MayUserAccess({}, api::kGet, {}));
  EXPECT_FALSE(GetRoute("/").GetPermissions().MayUserAccess({}, api::kPut, {}));
  EXPECT_EQ(GetRoute("/secret").GetPermissions().GetDefaultOwner(),
            GetRoute("/secret").GetOwner());
}

TEST_F(RoutesTest, FileSystemRoute) {
  auto& route = GetRoute<FileSystemRoute>("/");
  EXPECT_EQ(route.GetPath(), "www");
}

TEST_F(RoutesTest, ContentRoute) {
  auto& route = GetRoute<ContentRoute>("/secret");
  EXPECT_EQ(route.GetContent(), "I am your father");
  EXPECT_EQ(route.GetMimeType(), "text/plain");
}

TEST_F(RoutesTest, ContentRouteMultiLineContent) {
  auto& route = GetRoute<ContentRoute>("/colors");
  EXPECT_EQ(route.GetContent(), "red\ngreen\n");
}

TEST_F(RoutesTest, CustomRoute) {
  EXPECT_NO_THROW(GetRoute<CustomRoute>("/messages"));
}

TEST_F(RoutesTest, ApiEndpoint) {
  auto& route = GetRoute<ApiEndpoint>("/auth/groups");
  EXPECT_FALSE(route.GetOwner());
  EXPECT_TRUE(route.GetPermissions().MayUserAccess({}, api::kGet, {}));
  EXPECT_FALSE(route.GetPermissions().MayUserAccess({}, api::kPut, {}));
  EXPECT_TRUE(route.IsEnabled());
}

TEST_F(RoutesTest, FindRouteByUri) {
  auto check_route = [&](std::string uri) {
    auto route = Route::FindRouteByUri(uri, *routes_);
    if (!route) std::cout << "ERROR: Route not found" << std::endl;
    return route ? route->GetBaseUri() : ""s;
  };

  EXPECT_EQ(check_route("/"), "/");
  EXPECT_EQ(check_route("/somewher"), "/");
  EXPECT_EQ(check_route("/somewher?a=b"), "/");
  EXPECT_EQ(check_route("/somewhere"), "/somewhere");
  EXPECT_EQ(check_route("/somewhere?a=b"), "/somewhere");
  EXPECT_EQ(check_route("/somewhere/"), "/somewhere");
  EXPECT_EQ(check_route("/somewhere/x?a=b"), "/somewhere");
  EXPECT_EQ(check_route("/somewhere/here"), "/somewhere");
  EXPECT_EQ(check_route("/somewhere/far-aways"), "/somewhere");
  EXPECT_EQ(check_route("/somewhere/far-away"), "/somewhere/far-away");
  EXPECT_EQ(check_route("/somewhere/far-away/"), "/somewhere/far-away");

  EXPECT_FALSE(Route::FindRouteByUri("", *routes_));
}
