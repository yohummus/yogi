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

#include "session.h"
#include "ssl_detector_session.h"
#include "../../web_server.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <string>
using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("WebServer.Session");

namespace objects {
namespace web {
namespace detail {

SessionPtr Session::Create(WebServerWeakPtr server, WorkerPool& worker_pool,
                           AuthProviderPtr auth, SslContextPtr ssl,
                           RoutesVectorPtr routes,
                           std::chrono::nanoseconds timeout, bool test_mode,
                           boost::asio::ip::tcp::socket socket) {
  auto worker = worker_pool.AcquireWorker();

  SessionPtr session;
  if (socket.get_executor() == worker.Context()->IoContext().get_executor()) {
    session = std::make_shared<SslDetectorSession>(std::move(socket));
  } else {
    session = std::make_shared<SslDetectorSession>(boost::asio::ip::tcp::socket(
        boost::asio::make_strand(worker.Context()->IoContext()),
        socket.local_endpoint().protocol(), socket.release()));
  }

  session->server_ = server;
  session->worker_ = std::move(worker);
  session->auth_ = auth;
  session->ssl_ = ssl;
  session->routes_ = routes;
  session->timeout_ = timeout;
  session->test_mode_ = test_mode;
  session->session_id_ = boost::uuids::random_generator()();

  session->SetLoggingPrefix(
      "["s + boost::uuids::to_string(session->session_id_) + ']');

  return session;
}

Session::~Session() {
  if (!replaced_) {
    LOG_DBG("Session terminated");
  }
}

boost::asio::ip::tcp::endpoint Session::GetRemoteEndpoint() {
  return Stream().socket().remote_endpoint();
}

void Session::Close() {
  boost::system::error_code ec;
  Stream().socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  Stream().socket().cancel(ec);
  Stream().close();
}

void Session::Destroy() {
  Close();

  auto server = server_.lock();
  if (server) {
    server->DestroySession(shared_from_this());
  }
}

void Session::StartTimeout() { Stream().expires_after(timeout_); }

void Session::CancelTimeout() { Stream().expires_never(); }

void Session::ChangeSessionTypeImpl(SessionPtr new_session) {
  auto server = server_.lock();
  if (!server) return;

  new_session->server_ = std::move(server_);
  new_session->worker_ = std::move(worker_);
  new_session->auth_ = std::move(auth_);
  new_session->ssl_ = std::move(ssl_);
  new_session->routes_ = std::move(routes_);
  new_session->timeout_ = std::move(timeout_);
  new_session->test_mode_ = std::move(test_mode_);
  new_session->session_id_ = std::move(session_id_);
  new_session->buffer_ = std::move(buffer_);

  new_session->SetLoggingPrefix(GetLoggingPrefix());

  server->ReplaceSession(new_session);
  replaced_ = true;

  new_session->Start();
}

}  // namespace detail
}  // namespace web
}  // namespace objects
