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

#include <boost/beast/http.hpp>

namespace objects {
namespace web {
namespace detail {

class HttpSession : public SessionT<HttpSession> {
 public:
  HttpSession(boost::beast::tcp_stream&& stream);

  virtual void Start() override;

 protected:
  virtual boost::beast::tcp_stream& Stream() override;

 private:
  std::string MakeHttpsLocation() const;
  void StartReceiveRequest();
  void OnReceiveRequestFinished(boost::beast::error_code ec, std::size_t);
  void StartSendResponse();
  void OnSendResponseFinished(boost::beast::error_code ec, std::size_t);

  boost::beast::tcp_stream stream_;
  boost::beast::http::request<boost::beast::http::string_body> req_;
  boost::beast::http::response<boost::beast::http::empty_body> resp_;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
