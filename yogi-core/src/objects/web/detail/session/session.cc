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
#include "../../../../utils/bind.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

#include <string>
using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("WebServer.Session")

namespace objects {
namespace web {
namespace detail {

SessionPtr Session::Create(SessionManagerPtr manager, AuthProviderPtr auth,
                           SslContextPtr ssl, RoutesVectorPtr routes,
                           Worker&& worker, std::chrono::nanoseconds timeout,
                           bool test_mode, tcp::socket&& socket) {
  auto sid = boost::uuids::random_generator()();
  auto remote_ep = socket.remote_endpoint();

  auto session = std::make_shared<SslDetectorSession>(std::move(socket));
  session->PopulateMembers(manager, std::move(worker), auth, ssl, routes,
                           timeout, test_mode, sid, remote_ep, {},
                           "["s + boost::uuids::to_string(sid) + ']');

  return session;
}

Session::~Session() {
  if (!replaced_) {
    LOG_DBG("Session terminated");
  }
}

void Session::Close() {
  asio::post(Context()->IoContext(), utils::BindWeak(&Session::DoClose, this));
}

void Session::StartTimeout() { Stream().expires_after(timeout_); }

void Session::CancelTimeout() { Stream().expires_never(); }

void Session::PopulateMembers(SessionManagerPtr manager, Worker&& worker,
                              AuthProviderPtr auth, SslContextPtr ssl,
                              RoutesVectorPtr routes,
                              std::chrono::nanoseconds timeout, bool test_mode,
                              boost::uuids::uuid session_id,
                              boost::asio::ip::tcp::endpoint remote_ep,
                              boost::beast::flat_buffer buffer,
                              std::string logging_prefix) {
  manager_ = manager;
  worker_ = std::move(worker);
  auth_ = auth;
  ssl_ = ssl;
  routes_ = routes;
  timeout_ = timeout;
  test_mode_ = test_mode;
  session_id_ = session_id;
  rep_ = remote_ep;
  buffer_ = std::move(buffer);

  SetLoggingPrefix(logging_prefix);
}

void Session::ChangeSessionTypeImpl(SessionPtr new_session) {
  new_session->PopulateMembers(manager_, std::move(worker_), std::move(auth_),
                               std::move(ssl_), std::move(routes_), timeout_,
                               test_mode_, session_id_, rep_,
                               std::move(buffer_), GetLoggingPrefix());
  manager_->Replace(new_session);
  replaced_ = true;

  new_session->Start();
}

void Session::DoClose() {
  if (replaced_) return;

  auto& socket = Stream().socket();
  if (!socket.is_open()) return;

  boost::system::error_code ec;
  socket.shutdown(tcp::socket::shutdown_both, ec);
  socket.close(ec);

  manager_->Remove(*this);
}

void SessionManager::Remove(const Session& session) {
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_.erase(session.SessionId());
}

void SessionManager::Replace(SessionPtr new_session) {
  std::lock_guard<std::mutex> lock(mutex_);
  YOGI_ASSERT(sessions_.count(new_session->SessionId()));
  sessions_[new_session->SessionId()] = new_session;
}

void SessionManager::CloseAll() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& session : sessions_) {
    session.second->Close();
  }
}

}  // namespace detail
}  // namespace web
}  // namespace objects
