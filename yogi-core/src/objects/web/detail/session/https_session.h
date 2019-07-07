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
#include "../route/route.h"
#include "session.h"

#include <boost/optional.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>

namespace objects {
namespace web {
namespace detail {

class HttpsSession : public SessionT<HttpsSession> {
 public:
  typedef boost::beast::http::response<boost::beast::http::empty_body>
      EmptyResponse;
  typedef boost::beast::http::response<boost::beast::http::string_body>
      GenericResponse;
  typedef boost::beast::http::response<boost::beast::http::file_body>
      FileResponse;

  HttpsSession(boost::beast::tcp_stream&& stream, const SslContextPtr& ssl);

  virtual void Start() override;

  void SendResponse(EmptyResponse&& resp);
  void SendResponse(GenericResponse&& resp);
  void SendResponse(FileResponse&& resp);
  void SendResponse(boost::beast::http::status status, std::string html = {});

 protected:
  virtual boost::beast::tcp_stream& Stream() override;

 private:
  void OnHandshakeFinished(boost::beast::error_code ec, std::size_t bytes_used);
  void StartReceiveRequest();
  void OnReceiveRequestFinished(boost::beast::error_code ec, std::size_t);
  void HandleRequest();
  void OnSendResponseFinished(boost::beast::error_code ec, std::size_t);
  void StartShutdown();
  void OnShutdownFinished(boost::beast::error_code ec);

  bool CheckRequestMethod();
  std::string DecodeTargetUri();
  RoutePtr FindRoute(const std::string& uri);
  bool AuthenticateUser(UserPtr* user);

  template <typename Response>
  void FinaliseResponse(Response* resp);

  typedef boost::beast::http::request_parser<boost::beast::http::string_body>
      RequestParser;

  boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
  boost::optional<RequestParser> parser_;
  EmptyResponse empty_response_;
  GenericResponse generic_response_;
  FileResponse file_response_;
  unsigned response_status_;
  bool response_needs_eof_;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
