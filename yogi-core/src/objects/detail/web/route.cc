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

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {
namespace detail {
namespace web {
namespace {

nlohmann::json ExtractPermissionsSection(
    const nlohmann::json::const_iterator& cfg_it) {
  auto perm_it = cfg_it->find("permissions");
  if (perm_it == cfg_it->end()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing permissions section in route " << cfg_it.key() << ".";
  }

  return *perm_it;
}

UserPtr ExtractOwner(const AuthProvider& auth,
                     const nlohmann::json::const_iterator& cfg_it) {
  auto elem = cfg_it.value()["owner"];
  if (elem.is_null()) return {};

  UserPtr user;
  if (elem.is_string()) {
    user = auth.GetUserOptional(elem.get<std::string>());
  }

  if (!user) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid owner in route " << cfg_it.key()
        << ". The value must be a registered user.";
  }

  return user;
}

}  // namespace

RoutePtr Route::Create(const AuthProvider& auth,
                       const nlohmann::json::const_iterator& route_it,
                       const std::string& logging_prefix,
                       const GroupsMap& groups) {
  RoutePtr route;

  auto type = route_it->value("type", std::string{});
  if (type == "content") {
    route = std::make_unique<ContentRoute>();
  } else if (type == "filesystem") {
    route = std::make_unique<FileSystemRoute>();
  } else if (type == "custom") {
    route = std::make_unique<CustomRoute>();
  } else {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid route type in route " << route_it.key()
        << ". Valid types are \"content\", \"filesystem\" or \"custom\".";
  }

  route->SetLoggingPrefix(logging_prefix);
  route->base_uri_ = route_it.key();
  route->enabled_ = route_it->value("enabled", true);
  route->owner_ = ExtractOwner(auth, route_it);
  // route->permissions_ = Permissions(
  //     route->base_uri_, ExtractPermissionsSection(route_it),
  //     auth.GetGroups());

  route->ReadConfiguration(route_it);

  return route;
}

void ContentRoute::ReadConfiguration(
    const nlohmann::json::const_iterator& route_it) {
  mime_type_ = route_it->value("mime", "");
  LOG_DBG("Added content route " << route_it.key() << " serving " << mime_type_
                                 << (IsEnabled() ? "" : " (disabled)"));
}

void FileSystemRoute::ReadConfiguration(
    const nlohmann::json::const_iterator& route_it) {
  path_ = route_it->value("path", "");
  if (path_.empty()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing path property for route " << route_it.key() << ".";
  }

  LOG_DBG("Added filesystem route " << route_it.key() << " serving " << path_
                                    << (IsEnabled() ? "" : " (disabled)"));
}

void CustomRoute::ReadConfiguration(
    const nlohmann::json::const_iterator& route_it) {
  LOG_DBG("Configured custom route " << route_it.key()
                                     << (IsEnabled() ? "" : " (disabled)"));
}

void ApiEndpoint::ReadConfiguration(
    const nlohmann::json::const_iterator& route_it) {
  // LOG_DBG("Configured API endpoint " << base_uri
  //                                    << (IsEnabled() ? "" : " (disabled)"));
}

}  // namespace web
}  // namespace detail
}  // namespace objects
