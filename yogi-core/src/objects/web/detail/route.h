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
#include "../../log/logger.h"
#include "auth/permissions.h"
#include "auth/auth_provider.h"

#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <vector>
#include <functional>

namespace objects {
namespace web {
namespace detail {

class Session;
typedef std::shared_ptr<Session> SessionPtr;

class Route;
typedef std::shared_ptr<Route> RoutePtr;
typedef std::vector<RoutePtr> RoutesVector;
typedef std::shared_ptr<RoutesVector> RoutesVectorPtr;

class Route : public objects::log::LoggerUser {
 public:
  typedef boost::beast::http::string_body MsgBody;
  typedef boost::beast::http::request<MsgBody> Request;
  typedef boost::beast::http::response<MsgBody> Response;
  typedef std::function<void()> SendResponseFn;

  static RoutesVectorPtr CreateAll(const nlohmann::json& cfg,
                                   const AuthProvider& auth,
                                   const std::string& logging_prefix);
  static RoutePtr FindRouteByUri(boost::beast::string_view uri,
                                 const RoutesVector& routes);

  virtual ~Route() {}

  virtual void HandleRequest(const Request& req, const std::string& uri,
                             Response* resp, SessionPtr session,
                             SendResponseFn send_fn) {
    // TODO: Function should be pure virtual and implemented in derived classes
    resp->result(boost::beast::http::status::ok);
    resp->set(boost::beast::http::field::content_type, "text/html");
    resp->body() = R"(<!DOCTYPE html>
<html>
<body>
  <h1>Welcome to the Yogi web server!</h1>
</body>
</html>
)";
    resp->prepare_payload();
    send_fn();
  };

  const std::string& GetBaseUri() const { return base_uri_; }
  bool IsEnabled() const { return enabled_; }
  const Permissions& GetPermissions() const { return permissions_; }
  const UserPtr& GetOwner() const { return owner_; }

 protected:
  virtual void ReadConfiguration(const nlohmann::json::const_iterator&){};

 private:
  static const nlohmann::json& GetDefaultRoutesSection();
  static const nlohmann::json& GetDefaultApiPermissionsSection();

  void InitMemberVariables(const nlohmann::json::const_iterator& it,
                           const nlohmann::json& permissions_cfg,
                           const AuthProvider& auth,
                           const std::string& logging_prefix);

  std::string base_uri_;
  bool enabled_;
  UserPtr owner_;
  Permissions permissions_;
};

class ContentRoute : public Route {
 public:
  const std::string& GetMimeType() { return mime_type_; }
  const std::string& GetContent() { return content_; }

 protected:
  void ReadConfiguration(
      const nlohmann::json::const_iterator& route_it) override;

 private:
  std::string mime_type_;
  std::string content_;
};

class FileSystemRoute : public Route {
 public:
  const std::string& GetPath() { return path_; }

 protected:
  void ReadConfiguration(
      const nlohmann::json::const_iterator& route_it) override;

 private:
  std::string path_;
};

class CustomRoute : public Route {
 protected:
  void ReadConfiguration(
      const nlohmann::json::const_iterator& route_it) override;
};

class ApiEndpoint : public Route {};

}  // namespace detail
}  // namespace web
}  // namespace objects
