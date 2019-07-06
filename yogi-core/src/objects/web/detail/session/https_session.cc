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

#include "https_session.h"
#include "url_encoding.h"
#include "methods.h"
#include "../../../../utils/bind.h"
#include "../../../../utils/base64.h"

YOGI_DEFINE_INTERNAL_LOGGER("WebServer.Session.HTTPS")

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;

namespace objects {
namespace web {
namespace detail {

HttpsSession::HttpsSession(beast::tcp_stream&& stream, const SslContextPtr& ssl)
    : stream_(std::move(stream), ssl->AsioSslContext()) {}

void HttpsSession::Start() {
  StartTimeout();

  stream_.async_handshake(
      stream_.server, Buffer().data(),
      utils::BindStrong(&HttpsSession::OnHandshakeFinished, this));
}

boost::beast::tcp_stream& HttpsSession::Stream() {
  return boost::beast::get_lowest_layer(stream_);
}

void HttpsSession::OnHandshakeFinished(boost::beast::error_code ec,
                                       std::size_t bytes_used) {
  if (ec) {
    LOG_ERR("SSL handshake failed: " << ec.message());
    Close();
    return;
  }

  Buffer().consume(bytes_used);

  StartReceiveRequest();
}

void HttpsSession::StartReceiveRequest() {
  parser_.emplace();
  parser_->header_limit(HeaderLimit());
  parser_->body_limit(BodyLimit());

  StartTimeout();

  http::async_read(
      stream_, Buffer(), *parser_,
      utils::BindStrong(&HttpsSession::OnReceiveRequestFinished, this));
}

void HttpsSession::OnReceiveRequestFinished(boost::beast::error_code ec,
                                            std::size_t) {
  if (ec) {
    if (ec == http::error::end_of_stream) {
      StartShutdown();
    } else {
      LOG_ERR("Receiving HTTP request failed: " << ec.message());
      Close();
    }

    return;
  }

  HandleRequest();
}

void HttpsSession::HandleRequest() {
  auto& req = parser_->get();
  InitResponse();

  if (!CheckRequestMethod()) return;

  auto uri = DecodeTargetUri();
  if (uri.empty()) return;
  LOG_IFO("Received " << req.method() << ' ' << uri);

  auto route = FindRoute(uri);
  if (!route) return;

  UserPtr user;
  if (!AuthenticateUser(&user)) return;
  if (user) {
    LOG_DBG("Request is from by user: " << user->GetUsername());
  } else {
    LOG_DBG("Request is from anonymous user");
  }

  auto self = MakeSharedPtr();
  route->HandleRequest(req, uri, &resp_, self, user,
                       [self] { self->StartSendResponse(); });
}

void HttpsSession::StartSendResponse() {
  http::async_write(
      stream_, resp_,
      utils::BindStrong(&HttpsSession::OnSendResponseFinished, this));
}

void HttpsSession::OnSendResponseFinished(boost::beast::error_code ec,
                                          std::size_t) {
  if (ec) {
    LOG_ERR("Sending response failed: " << ec.message());
    Close();
    return;
  }

  LOG_IFO("Response " << resp_.result_int() << " sent");

  if (resp_.need_eof()) {
    StartShutdown();
  } else {
    StartReceiveRequest();
  }
}

void HttpsSession::StartShutdown() {
  stream_.async_shutdown(
      utils::BindStrong(&HttpsSession::OnShutdownFinished, this));
}

void HttpsSession::OnShutdownFinished(boost::beast::error_code ec) {
  if (ec) {
    LOG_ERR("Shutdown failed: " << ec.message());
  }

  Close();
}

void HttpsSession::InitResponse() {
  auto& req = parser_->get();
  resp_.version(req.version());
  resp_.set(http::field::server, "Yogi " YOGI_HDR_VERSION " Web Server");
  resp_.keep_alive(req.keep_alive());
  resp_.result(http::status::ok);
}

bool HttpsSession::CheckRequestMethod() {
  auto verb = parser_->get().method();
  auto method = VerbToMethod(verb);
  if (method == api::kNoMethod) {
    LOG_ERR("Received invalid method " << verb);

    resp_.result(http::status::method_not_allowed);
    StartSendResponse();

    return false;
  }

  return true;
}

std::string HttpsSession::DecodeTargetUri() {
  auto& req = parser_->get();
  auto uri = DecodeUrl(req.target());
  if (!uri) {
    LOG_ERR("Received " << req.method() << " with invalid URI encoding");

    resp_.result(http::status::bad_request);
    resp_.set(http::field::content_type, "text/html");
    resp_.body() = "Invalid URI encoding in header.";
    resp_.prepare_payload();
    StartSendResponse();
  }

  if (uri) {
    return *uri;
  } else {
    return {};
  }
}

RoutePtr HttpsSession::FindRoute(const std::string& uri) {
  auto route = Route::FindRouteByUri(uri, *Routes());
  if (!route) {
    LOG_ERR("Route " << uri.substr(0, uri.find('?')) << " not found");

    resp_.result(http::status::not_found);
    resp_.set(http::field::content_type, "text/html");
    resp_.body() = "The resource '" + uri + "' was not found.";
    resp_.prepare_payload();
    StartSendResponse();
  }

  return route;
}

bool HttpsSession::AuthenticateUser(UserPtr* user) {
  auto& req = parser_->get();

  if (!req.count(http::field::authorization)) {
    return true;
  }

  const char* auth_error = nullptr;
  auto& field_val = req[http::field::authorization];
  if (field_val.starts_with("Basic ")) {
    auto auth_token =
        utils::DecodeBase64(field_val.substr(sizeof("Basic")).to_string());

    auto split_pos = auth_token.find(':');
    if (split_pos != std::string::npos) {
      auto username = auth_token.substr(0, split_pos);
      auto password = auth_token.substr(split_pos + 1);
      *user = AuthProvider()->Authenticate(username, password);
      if (!*user) {
        auth_error = "Invalid user and/or password.";
      }
    } else {
      auth_error = "Invalid authentication format. Must be user:password.";
    }
  } else {
    auth_error = "Invalid authentication method.";
  }

  if (auth_error) {
    resp_.result(http::status::unauthorized);
    resp_.set(http::field::www_authenticate, "Basic");
    resp_.set(http::field::content_type, "text/html");
    resp_.body() = auth_error;
    resp_.prepare_payload();
    StartSendResponse();
  }

  return !auth_error;
}

}  // namespace detail
}  // namespace web
}  // namespace objects
