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
#include "route.h"

namespace objects {
namespace web {
namespace detail {

class ApiEndpoint;
typedef std::shared_ptr<ApiEndpoint> ApiEndpointPtr;

class ApiEndpoint : public Route {
 public:
  static ApiEndpointPtr Create(const std::string& base_uri);

 protected:
  virtual void LogCreation() override;
};

class AuthSessionApiEndpoint : public ApiEndpoint {
 public:
  virtual void HandleRequest(const Request& req, const std::string& uri,
                             Response* resp, SessionPtr session, UserPtr user,
                             SendResponseFn send_fn) override;
};

class AuthGroupsApiEndpoint : public ApiEndpoint {
 public:
  virtual void HandleRequest(const Request& req, const std::string& uri,
                             Response* resp, SessionPtr session, UserPtr user,
                             SendResponseFn send_fn) override;
};

class AuthUsersApiEndpoint : public ApiEndpoint {
 public:
  virtual void HandleRequest(const Request& req, const std::string& uri,
                             Response* resp, SessionPtr session, UserPtr user,
                             SendResponseFn send_fn) override;
};

class BranchInfoApiEndpoint : public ApiEndpoint {
 public:
  virtual void HandleRequest(const Request& req, const std::string& uri,
                             Response* resp, SessionPtr session, UserPtr user,
                             SendResponseFn send_fn) override;
};

class BranchConnectionsApiEndpoint : public ApiEndpoint {
 public:
  virtual void HandleRequest(const Request& req, const std::string& uri,
                             Response* resp, SessionPtr session, UserPtr user,
                             SendResponseFn send_fn) override;
};

class BranchBroadcastsApiEndpoint : public ApiEndpoint {
 public:
  virtual void HandleRequest(const Request& req, const std::string& uri,
                             Response* resp, SessionPtr session, UserPtr user,
                             SendResponseFn send_fn) override;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
