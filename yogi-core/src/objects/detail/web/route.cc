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
                       const nlohmann::json::const_iterator& cfg_it,
                       const std::string& logging_prefix) {
  RoutePtr route;

  auto type = cfg_it->value("type", std::string{});
  if (type == "content") {
    route = std::make_unique<ContentRoute>(auth, cfg_it, logging_prefix);
  } else if (type == "filesystem") {
    route = std::make_unique<FileSystemRoute>(auth, cfg_it, logging_prefix);
  } else if (type == "custom") {
    route = std::make_unique<CustomRoute>(auth, cfg_it, logging_prefix);
  } else {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid route type in route " << cfg_it.key()
        << ". Valid types are \"content\", \"filesystem\" or \"custom\".";
  }

  return route;
}

Route::Route(const std::string& base_uri, const nlohmann::json& permissions_cfg,
             const std::string& logging_prefix)
    : base_uri_(base_uri), enabled_(true), permissions_(permissions_cfg) {
  SetLoggingPrefix(logging_prefix);
}

Route::Route(const AuthProvider& auth,
             const nlohmann::json::const_iterator& cfg_it,
             const std::string& logging_prefix)
    : base_uri_(cfg_it.key()),
      enabled_(cfg_it->value("enabled", true)),
      permissions_(cfg_it.value()["permissions"]),
      owner_(ExtractOwner(auth, cfg_it)) {
  SetLoggingPrefix(logging_prefix);
}

ContentRoute::ContentRoute(const AuthProvider& auth,
                           const nlohmann::json::const_iterator& cfg_it,
                           const std::string& logging_prefix)
    : Route(auth, cfg_it, logging_prefix),
      mime_type_(cfg_it->value("mime", "")) {
  if (mime_type_.empty()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing mime property for route " << cfg_it.key() << ".";
  }

  LOG_DBG("Added content route " << cfg_it.key() << " serving " << mime_type_
                                 << (IsEnabled() ? "" : " (disabled)"));
}

FileSystemRoute::FileSystemRoute(const AuthProvider& auth,
                                 const nlohmann::json::const_iterator& cfg_it,
                                 const std::string& logging_prefix)
    : Route(auth, cfg_it, logging_prefix), path_(cfg_it->value("path", "")) {
  if (path_.empty()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing path property for route " << cfg_it.key() << ".";
  }

  LOG_DBG("Added filesystem route " << cfg_it.key() << " serving " << path_
                                    << (IsEnabled() ? "" : " (disabled)"));
}

CustomRoute::CustomRoute(const AuthProvider& auth,
                         const nlohmann::json::const_iterator& cfg_it,
                         const std::string& logging_prefix)
    : Route(auth, cfg_it, logging_prefix) {
  LOG_DBG("Configured custom route " << cfg_it.key()
                                     << (IsEnabled() ? "" : " (disabled)"));
}

ApiEndpoint::ApiEndpoint(const std::string& base_uri,
                         const nlohmann::json& permissions_cfg,
                         const std::string& logging_prefix)
    : Route(base_uri, permissions_cfg, logging_prefix) {
  LOG_DBG("Configured API endpoint " << base_uri
                                     << (IsEnabled() ? "" : " (disabled)"));
}

}  // namespace web
}  // namespace detail
}  // namespace objects
