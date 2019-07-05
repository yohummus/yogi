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

#include "api_endpoint.h"

YOGI_DEFINE_INTERNAL_LOGGER("WebServer.Session.HTTPS")

namespace objects {
namespace web {
namespace detail {

ApiEndpointPtr ApiEndpoint::Create(const std::string& base_uri) {
#define TRY_CREATE(uri, cls) \
  if (base_uri == uri) return std::make_shared<cls>();

  TRY_CREATE("/api/auth/session", AuthSessionApiEndpoint);
  TRY_CREATE("/api/auth/groups", AuthGroupsApiEndpoint);
  TRY_CREATE("/api/auth/users", AuthUsersApiEndpoint);
  TRY_CREATE("/api/branch/info", BranchInfoApiEndpoint);
  TRY_CREATE("/api/branch/connections", BranchConnectionsApiEndpoint);
  TRY_CREATE("/api/branch/broadcasts", BranchBroadcastsApiEndpoint);

#undef TRY_CREATE

  throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
      << "Uknown API endpoint '" << base_uri << "'";
}

void ApiEndpoint::LogCreation() {
  LOG_IFO("Added API endpoint " << GetBaseUri());
}

void AuthSessionApiEndpoint::HandleRequest(const Request& req,
                                           const std::string& uri,
                                           Response* resp, SessionPtr session,
                                           SendResponseFn send_fn) {}

void AuthGroupsApiEndpoint::HandleRequest(const Request& req,
                                          const std::string& uri,
                                          Response* resp, SessionPtr session,
                                          SendResponseFn send_fn) {}

void AuthUsersApiEndpoint::HandleRequest(const Request& req,
                                         const std::string& uri, Response* resp,
                                         SessionPtr session,
                                         SendResponseFn send_fn) {}

void BranchInfoApiEndpoint::HandleRequest(const Request& req,
                                          const std::string& uri,
                                          Response* resp, SessionPtr session,
                                          SendResponseFn send_fn) {}

void BranchConnectionsApiEndpoint::HandleRequest(const Request& req,
                                                 const std::string& uri,
                                                 Response* resp,
                                                 SessionPtr session,
                                                 SendResponseFn send_fn) {}

void BranchBroadcastsApiEndpoint::HandleRequest(const Request& req,
                                                const std::string& uri,
                                                Response* resp,
                                                SessionPtr session,
                                                SendResponseFn send_fn) {}

}  // namespace detail
}  // namespace web
}  // namespace objects
