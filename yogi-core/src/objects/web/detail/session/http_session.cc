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

#include "http_session.h"
#include "../../../../utils/bind.h"

#include <sstream>

YOGI_DEFINE_INTERNAL_LOGGER("WebServer.Session.HTTP")

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;

namespace objects {
namespace web {
namespace detail {

HttpSession::HttpSession(beast::tcp_stream&& stream)
    : stream_(std::move(stream)) {}

void HttpSession::Start() { StartReceiveRequest(); }

boost::beast::tcp_stream& HttpSession::Stream() {
  return boost::beast::get_lowest_layer(stream_);
}

std::string HttpSession::MakeHttpsLocation() const {
  std::stringstream ss;
  ss << "https://" << stream_.socket().local_endpoint();
  return ss.str();
}

void HttpSession::StartReceiveRequest() {
  req_ = {};
  StartTimeout();

  http::async_read(
      stream_, Buffer(), req_,
      utils::BindStrong(&HttpSession::OnReceiveRequestFinished, this));
}

void HttpSession::OnReceiveRequestFinished(boost::beast::error_code ec,
                                           std::size_t) {
  if (ec) {
    if (ec != http::error::end_of_stream) {
      LOG_ERR("Receiving HTTP request failed: " << ec.message());
    }

    Close();
    return;
  }

  PopulateResponse();
  StartSendResponse();
}

void HttpSession::PopulateResponse() {
  resp_ = {http::status::moved_permanently, req_.version()};
  resp_.set(http::field::server, "Yogi " YOGI_HDR_VERSION " Web Server");
  resp_.set(http::field::location, MakeHttpsLocation());
  resp_.set(http::field::connection, "close");
  resp_.keep_alive(false);
  resp_.prepare_payload();
}

void HttpSession::StartSendResponse() {
  http::async_write(
      stream_, resp_,
      utils::BindStrong(&HttpSession::OnSendResponseFinished, this));
}

void HttpSession::OnSendResponseFinished(boost::beast::error_code ec,
                                         std::size_t) {
  if (ec) {
    LOG_ERR("Sending HTTP request failed: " << ec.message());
  } else {
    LOG_IFO("Redirected client to use HTTPS");
  }

  Close();
}

}  // namespace detail
}  // namespace web
}  // namespace objects
