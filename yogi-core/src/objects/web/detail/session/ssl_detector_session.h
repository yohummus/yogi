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
#include "session.h"

namespace objects {
namespace web {
namespace detail {

class SslDetectorSession : public Session {
 public:
  SslDetectorSession(boost::asio::ip::tcp::socket&& socket);

  virtual void Start() override;

 protected:
  virtual boost::beast::tcp_stream& Stream() override;

 private:
  void OnDetectSslFinished(boost::beast::error_code ec, boost::tribool is_ssl);

  boost::beast::tcp_stream stream_;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
