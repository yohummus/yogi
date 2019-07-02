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
#include "../../../../utils/bind.h"

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

  resp_.version(req.version());
  resp_.set(http::field::server, "Yogi " YOGI_HDR_VERSION " Web Server");
  resp_.keep_alive(req.keep_alive());

  auto self = MakeSharedPtr();
  auto send_fn = [self, this] {
    self->StartSendResponse();
    LOG_IFO("Response " << resp_.result_int() << " sent");
  };

  auto uri = DecodeUrl(req.target());
  if (!uri) {
    LOG_ERR("Received " << req.method() << " with invalid URI encoding");

    resp_.result(http::status::bad_request);
    resp_.set(http::field::content_type, "text/html");
    resp_.body() = "Invalid URI encoding in header.";
    resp_.prepare_payload();
    send_fn();

    return;
  }

  LOG_IFO("Received " << req.method() << ' ' << *uri);

  auto route = Route::FindRouteByUri(*uri, *Routes());
  if (route) {
    route->HandleRequest(req, *uri, &resp_, self, send_fn);
  } else {
    LOG_ERR("Route " << uri->substr(0, uri->find('?')) << " not found");

    resp_.result(http::status::not_found);
    resp_.set(http::field::content_type, "text/html");
    resp_.body() = "The resource '" + *uri + "' was not found.";
    resp_.prepare_payload();
    send_fn();
  }
}

void HttpsSession::StartSendResponse() {
  http::async_write(
      stream_, resp_,
      utils::BindStrong(&HttpsSession::OnSendResponseFinished, this));
}

void HttpsSession::OnSendResponseFinished(boost::beast::error_code ec,
                                          std::size_t) {
  if (ec) {
    LOG_ERR("Sending HTTP request failed: " << ec.message());
    Close();
    return;
  }

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

}  // namespace detail
}  // namespace web
}  // namespace objects
