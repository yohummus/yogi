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

#include "route.h"
#include "../../../api/errors.h"
#include "../../../schema/schema.h"

#include <sstream>

YOGI_DEFINE_INTERNAL_LOGGER("WebServer")

namespace objects {
namespace web {
namespace detail {
namespace {

UserPtr ExtractOwner(const AuthProvider& auth,
                     const nlohmann::json::const_iterator& cfg_it) {
  auto user_str = cfg_it->value("owner", "");
  if (user_str.empty()) return {};

  auto user = auth.GetUserOptional(user_str);

  if (!user) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Invalid owner in route " << cfg_it.key()
        << ". The value must be a registered user.";
  }

  return user;
}

bool DoesRouteSatisfyUri(const std::string& base_uri,
                         boost::beast::string_view uri) {
  if (!uri.starts_with(base_uri)) return false;
  if (uri.size() == base_uri.size()) return true;
  if (!base_uri.empty() && base_uri.back() == '/') return true;
  if (uri[base_uri.size()] == '/') return true;
  return false;
}

}  // namespace

RoutesVectorPtr Route::CreateAll(const nlohmann::json& cfg,
                                 const AuthProvider& auth,
                                 const std::string& logging_prefix) {
  schema::ValidateJson(cfg, "web_routes.schema.json");

  RoutesVectorPtr routes = std::make_shared<RoutesVector>();
  const auto& routes_cfg = cfg.value("routes", GetDefaultRoutesSection());
  for (auto it = routes_cfg.begin(); it != routes_cfg.end(); ++it) {
    RoutePtr route;
    auto type = (*it)["type"].get<std::string>();
    if (type == "content") {
      route = std::make_unique<ContentRoute>();
    } else if (type == "filesystem") {
      route = std::make_unique<FileSystemRoute>();
    } else if (type == "custom") {
      route = std::make_unique<CustomRoute>();
    } else {
      YOGI_NEVER_REACHED;
    }

    route->owner_ = ExtractOwner(auth, it);
    route->InitMemberVariables(it, (*it)["permissions"], auth, logging_prefix);
    routes->push_back(std::move(route));
  }

  const auto& api_cfg =
      cfg.value("api_permissions", GetDefaultApiPermissionsSection());
  for (auto it = api_cfg.begin(); it != api_cfg.end(); ++it) {
    auto route = std::make_unique<ApiEndpoint>();
    route->InitMemberVariables(it, it.value(), auth, logging_prefix);
    routes->push_back(std::move(route));
  }

  return routes;
}

RoutePtr Route::FindRouteByUri(boost::beast::string_view uri,
                               const RoutesVector& routes) {
  uri = uri.substr(0, uri.find('?'));

  std::size_t match_length = 0;
  RoutesVector::const_iterator match = routes.end();

  for (auto it = routes.begin(); it != routes.end(); ++it) {
    auto& base_uri = (*it)->GetBaseUri();
    if (base_uri.size() <= match_length) continue;

    if (DoesRouteSatisfyUri(base_uri, uri)) {
      match_length = base_uri.size();
      match = it;
    }
  }

  return match != routes.end() ? *match : RoutePtr{};
}

const nlohmann::json& Route::GetDefaultRoutesSection() {
  static const nlohmann::json json = nlohmann::json::parse(R"(
    {
      "/": {
        "type":        "content",
        "mime":        "text/html",
        "permissions": { "*": ["GET"] },
        "enabled":     true,
        "content": [
          "<!DOCTYPE html>",
          "<html>",
          "<body>",
          "  <h1>Welcome to the Yogi web server!</h1>",
          "</body>",
          "</html>"
        ]
      }
    }
  )");
  return json;
}

const nlohmann::json& Route::GetDefaultApiPermissionsSection() {
  static const nlohmann::json json = nlohmann::json::parse(R"(
    {
    }
  )");
  return json;
}

void Route::InitMemberVariables(const nlohmann::json::const_iterator& it,
                                const nlohmann::json& permissions_cfg,
                                const AuthProvider& auth,
                                const std::string& logging_prefix) {
  SetLoggingPrefix(logging_prefix);
  base_uri_ = it.key();
  enabled_ = it->value("enabled", true);
  permissions_ =
      Permissions(it.key(), permissions_cfg, owner_, auth.GetGroups());
  ReadConfiguration(it);
}

void ContentRoute::ReadConfiguration(
    const nlohmann::json::const_iterator& route_it) {
  mime_type_ = (*route_it)["mime"].get<std::string>();

  auto& content_cfg = (*route_it)["content"];
  if (content_cfg.is_string()) {
    content_ = content_cfg.get<std::string>();
  } else {
    std::stringstream ss;
    for (auto& line : content_cfg) {
      ss << line.get<std::string>() << '\n';
    }

    content_ = ss.str();
  }

  LOG_DBG("Added content route " << route_it.key() << " serving " << mime_type_
                                 << (IsEnabled() ? "" : " (disabled)"));
}

void FileSystemRoute::ReadConfiguration(
    const nlohmann::json::const_iterator& route_it) {
  path_ = (*route_it)["path"].get<std::string>();

  LOG_DBG("Added filesystem route " << route_it.key() << " serving " << path_
                                    << (IsEnabled() ? "" : " (disabled)"));
}

void CustomRoute::ReadConfiguration(
    const nlohmann::json::const_iterator& route_it) {
  LOG_DBG("Configured custom route " << route_it.key()
                                     << (IsEnabled() ? "" : " (disabled)"));
}

}  // namespace detail
}  // namespace web
}  // namespace objects
