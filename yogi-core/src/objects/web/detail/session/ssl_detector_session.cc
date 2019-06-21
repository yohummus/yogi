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

#include "ssl_detector_session.h"
#include "http_session.h"
#include "https_session.h"

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;

namespace objects {
namespace web {
namespace detail {

SslDetectorSession::SslDetectorSession(tcp::socket&& socket)
    : stream_(std::move(socket)) {}

void SslDetectorSession::Start() {
  StartTimeout();
  beast::async_detect_ssl(
      stream_, Buffer(),
      beast::bind_front_handler(
          &SslDetectorSession::OnDetectSslFinished,
          std::static_pointer_cast<SslDetectorSession>(shared_from_this())));
}

beast::tcp_stream& SslDetectorSession::Stream() {
  return beast::get_lowest_layer(stream_);
}

void SslDetectorSession::OnDetectSslFinished(beast::error_code ec,
                                             boost::tribool is_ssl) {
  if (LogAndDestroyIfFailed(ec, "Detecting SSL failed")) return;
  if (is_ssl) {
    ChangeSessionType<HttpsSession>(std::move(stream_), SslContext());
  } else {
    ChangeSessionType<HttpSession>(std::move(stream_));
  }
}

}  // namespace detail
}  // namespace web
}  // namespace objects
