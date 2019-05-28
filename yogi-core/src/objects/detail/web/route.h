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

#include "../../../config.h"
#include "../../logger.h"
#include "permissions.h"
#include "auth_provider.h"

#include <nlohmann/json.hpp>
#include <memory>
#include <vector>

namespace objects {
namespace detail {
namespace web {

class Route;
class ContentRoute;
class FilesystemRoute;
class CustomRoute;

typedef std::unique_ptr<Route> RoutePtr;
typedef std::vector<RoutePtr> RoutesVector;

class Route : public objects::LoggerUser {
 public:
  static RoutePtr Create(const AuthProvider& auth,
                         const nlohmann::json::const_iterator& cfg_it,
                         const std::string& logging_prefix);

  virtual ~Route() {}

  const std::string& GetBaseUri() const { return base_uri_; }
  bool IsEnabled() const { return enabled_; }
  const Permissions& GetPermissions() const { return permissions_; }
  const UserPtr& GetOwner() const { return owner_; }

 protected:
  Route(const std::string& base_uri, const nlohmann::json& permissions_cfg,
        const std::string& logging_prefix);
  Route(const AuthProvider& auth, const nlohmann::json::const_iterator& cfg_it,
        const std::string& logging_prefix);

 private:
  const std::string base_uri_;
  const bool enabled_;
  const Permissions permissions_;
  const UserPtr owner_;
};

class ContentRoute : public Route {
 public:
  ContentRoute(const AuthProvider& auth,
               const nlohmann::json::const_iterator& cfg_it,
               const std::string& logging_prefix);

 private:
  const std::string mime_type_;
};

class FileSystemRoute : public Route {
 public:
  FileSystemRoute(const AuthProvider& auth,
                  const nlohmann::json::const_iterator& cfg_it,
                  const std::string& logging_prefix);

 private:
  const std::string path_;
};

class CustomRoute : public Route {
 public:
  CustomRoute(const AuthProvider& auth,
              const nlohmann::json::const_iterator& cfg_it,
              const std::string& logging_prefix);
};

class ApiEndpoint : public Route {
 public:
  ApiEndpoint(const std::string& base_uri,
              const nlohmann::json& permissions_cfg,
              const std::string& logging_prefix);
};

}  // namespace web
}  // namespace detail
}  // namespace objects
