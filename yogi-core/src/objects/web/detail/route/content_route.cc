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

#include "content_route.h"
#include "../session/https_session.h"
#include "../session/methods.h"

#include <sstream>
namespace http = boost::beast::http;

YOGI_DEFINE_INTERNAL_LOGGER("WebServer.Session.HTTPS")

namespace objects {
namespace web {
namespace detail {

void ContentRoute::HandleRequest(const Request& req, const std::string& uri,
                                 const HttpsSessionPtr& session, UserPtr user) {
  auto method = VerbToMethod(req.method());

  if (method != api::kGet && method != api::kHead) {
    session->SendResponse(http::status::method_not_allowed);
    return;
  }

  if (!GetPermissions().MayUserAccess(user, method)) {
    session->SendResponse(http::status::forbidden);
    return;
  }

  LOG_IFO(session->GetLoggingPrefix()
          << ": Serving static content from " << GetBaseUri());

  HttpsSession::GenericResponse resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, GetMimeType());
  if (method == api::kHead) {
    resp.content_length(GetContent().size());
  } else {
    resp.body() = GetContent();
    resp.prepare_payload();
  }

  session->SendResponse(std::move(resp));
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
}

void ContentRoute::LogCreation() {
  LOG_DBG("Added content route " << GetBaseUri() << " serving " << mime_type_
                                 << (IsEnabled() ? "" : " (disabled)"));
}

}  // namespace detail
}  // namespace web
}  // namespace objects
