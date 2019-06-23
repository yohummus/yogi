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
#include "../../../../utils/bind.h"

YOGI_DEFINE_INTERNAL_LOGGER("WebServer.Session.HTTPS");

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
      utils::BindWeak(&HttpsSession::OnHandshakeFinished, this));
}

boost::beast::tcp_stream& HttpsSession::Stream() {
  return boost::beast::get_lowest_layer(stream_);
}

void HttpsSession::OnHandshakeFinished(boost::beast::error_code ec,
                                       std::size_t bytes_used) {
  if (ec) {
    LOG_ERR("SSL handshake failed: " << ec.message());
    Destroy();
    return;
  }

  Buffer().consume(bytes_used);

  StartReceiveRequest();
}

void HttpsSession::StartReceiveRequest() {
  req_ = {};
  StartTimeout();

  http::async_read(
      stream_, Buffer(), req_,
      utils::BindWeak(&HttpsSession::OnReceiveRequestFinished, this));
}

void HttpsSession::OnReceiveRequestFinished(boost::beast::error_code ec,
                                            std::size_t) {
  if (ec) {
    if (ec == http::error::end_of_stream) {
      StartShutdown();
    } else {
      LOG_ERR("Receiving HTTP request failed: " << ec.message());
      Destroy();
    }

    return;
  }

  PopulateResponse();
  StartSendResponse();
}

void HttpsSession::PopulateResponse() {
  resp_ = {http::status::ok, req_.version()};
  resp_.set(http::field::server, "Yogi " YOGI_HDR_VERSION " Web Server");
  resp_.keep_alive(req_.keep_alive());
  resp_.set(http::field::content_type, "text/html");
  resp_.body() =
      R"(<!DOCTYPE html>
<html>
<body>
  <h1>Welcome to the Yogi web server!</h1>
</body>
</html>
)";
  resp_.prepare_payload();
}

void HttpsSession::StartSendResponse() {
  http::async_write(
      stream_, resp_,
      utils::BindWeak(&HttpsSession::OnSendResponseFinished, this));
}

void HttpsSession::OnSendResponseFinished(boost::beast::error_code ec,
                                          std::size_t) {
  if (ec) {
    LOG_ERR("Sending HTTP request failed: " << ec.message());
    Destroy();
    return;
  }

  if (resp_.need_eof()) {
    StartShutdown();
  } else {
    resp_ = {};
    StartReceiveRequest();
  }
}

void HttpsSession::StartShutdown() {
  stream_.async_shutdown(
      utils::BindWeak(&HttpsSession::OnShutdownFinished, this));
}

void HttpsSession::OnShutdownFinished(boost::beast::error_code ec) {
  if (ec) {
    LOG_ERR("Shutdown failed: " << ec.message());
  }

  Destroy();
}

}  // namespace detail
}  // namespace web
}  // namespace objects
