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

using namespace std::string_literals;

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

void HttpsSession::SendResponse(EmptyResponse&& resp) {
  empty_response_ = std::move(resp);
  FinaliseResponse(&empty_response_);
  http::async_write(
      stream_, empty_response_,
      utils::BindStrong(&HttpsSession::OnSendResponseFinished, this));
}

void HttpsSession::SendResponse(GenericResponse&& resp) {
  generic_response_ = std::move(resp);
  FinaliseResponse(&generic_response_);
  http::async_write(
      stream_, generic_response_,
      utils::BindStrong(&HttpsSession::OnSendResponseFinished, this));
}

void HttpsSession::SendResponse(FileResponse&& resp) {
  file_response_ = std::move(resp);
  FinaliseResponse(&file_response_);
  http::async_write(
      stream_, file_response_,
      utils::BindStrong(&HttpsSession::OnSendResponseFinished, this));
}

void HttpsSession::SendResponse(boost::beast::http::status status,
                                std::string html) {
  GenericResponse resp;
  resp.result(status);
  if (!html.empty()) {
    resp.set(http::field::content_type, "text/html");
    resp.body() = html;
    resp.prepare_payload();
  }
  SendResponse(std::move(resp));
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
  if (!CheckRequestMethod()) return;

  auto uri = DecodeTargetUri();
  if (uri.empty()) return;

  auto& req = parser_->get();
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
  route->HandleRequest(req, uri, self, user);
}

void HttpsSession::OnSendResponseFinished(boost::beast::error_code ec,
                                          std::size_t) {
  empty_response_ = {};
  generic_response_ = {};
  file_response_ = {};

  if (ec) {
    LOG_ERR("Sending response failed: " << ec.message());
    Close();
    return;
  }

  LOG_IFO("Response " << response_status_ << " sent");

  if (response_needs_eof_) {
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

bool HttpsSession::CheckRequestMethod() {
  auto verb = parser_->get().method();
  auto method = VerbToMethod(verb);
  if (method == api::kNoMethod) {
    LOG_ERR("Received invalid method " << verb);
    SendResponse(http::status::method_not_allowed);
    return false;
  }

  return true;
}

std::string HttpsSession::DecodeTargetUri() {
  auto& req = parser_->get();
  auto uri = DecodeUrl(req.target());
  if (uri) {
    return *uri;
  }

  LOG_ERR("Received " << req.method() << " with invalid URI encoding");
  SendResponse(http::status::bad_request, "Invalid URI encoding in header.");

  return {};
}

RoutePtr HttpsSession::FindRoute(const std::string& uri) {
  auto route = Route::FindRouteByUri(uri, *Routes());
  if (route) {
    return route;
  }

  auto txt = "Route "s + uri.substr(0, uri.find('?')) + " not found";
  LOG_ERR(txt);
  SendResponse(http::status::not_found, txt + '.');

  return {};
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
    GenericResponse resp;
    resp.result(http::status::unauthorized);
    resp.set(http::field::www_authenticate, "Basic");
    resp.set(http::field::content_type, "text/html");
    resp.body() = auth_error;
    resp.prepare_payload();
    SendResponse(std::move(resp));
  }

  return !auth_error;
}

template <typename Response>
void HttpsSession::FinaliseResponse(Response* resp) {
  auto& req = parser_->get();
  resp->version(req.version());
  resp->set(http::field::server, "Yogi " YOGI_HDR_VERSION " Web Server");
  resp->keep_alive(!resp->need_eof() && req.keep_alive());

  response_status_ = resp->result_int();
  response_needs_eof_ = resp->need_eof();
}

}  // namespace detail
}  // namespace web
}  // namespace objects
