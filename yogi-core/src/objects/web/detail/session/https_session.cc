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

YOGI_DEFINE_INTERNAL_LOGGER("WebServer.Session.HTTPS");

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;

namespace objects {
namespace web {
namespace detail {

HttpsSession::HttpsSession(beast::tcp_stream&& stream, const SslContextPtr& ssl)
    : stream_(std::move(stream), ssl->AsioSslContext()) {}

void HttpsSession::Start() {
  StartTimeout();
  stream_.async_handshake(
      stream_.server, Buffer().data(),
      beast::bind_front_handler(&HttpsSession::OnHandshakeFinished,
                                MakeSharedPtr()));
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
}

}  // namespace detail
}  // namespace web
}  // namespace objects
