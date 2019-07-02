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
#include "../../../log/logger.h"
#include "../auth/auth_provider.h"
#include "../worker_pool.h"
#include "../ssl_context.h"
#include "../route.h"

#include <boost/uuid/uuid.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/functional/hash.hpp>

#include <memory>
#include <unordered_map>
#include <utility>
#include <mutex>

namespace objects {
namespace web {
namespace detail {

class Session;
typedef std::shared_ptr<Session> SessionPtr;

class SessionManager;
typedef std::shared_ptr<SessionManager> SessionManagerPtr;

class Session : public log::LoggerUser,
                public std::enable_shared_from_this<Session> {
 public:
  static SessionPtr Create(SessionManagerPtr manager, AuthProviderPtr auth,
                           SslContextPtr ssl, RoutesVectorPtr routes,
                           Worker&& worker, std::chrono::nanoseconds timeout,
                           std::uint32_t header_limit, std::uint32_t body_limit,
                           bool test_mode,
                           boost::asio::ip::tcp::socket&& socket);

  virtual ~Session();

  virtual void Start() = 0;
  void Close();

  const boost::uuids::uuid& SessionId() const { return session_id_; }
  const boost::asio::ip::tcp::endpoint& RemoteEndpoint() const { return rep_; }

 protected:
  virtual boost::beast::tcp_stream& Stream() = 0;

  std::chrono::nanoseconds Timeout() const { return timeout_; }
  std::uint32_t HeaderLimit() const { return header_limit_; }
  std::uint32_t BodyLimit() const { return body_limit_; }
  bool TestModeEnabled() const { return test_mode_; }
  const detail::AuthProviderPtr& AuthProvider() const { return auth_; }
  const detail::SslContextPtr& SslContext() const { return ssl_; }
  const detail::RoutesVectorPtr& Routes() const { return routes_; }
  boost::beast::flat_buffer& Buffer() { return buffer_; }
  const ContextPtr& Context() const { return worker_.Context(); }

  template <typename SessionType, typename... Args>
  void ChangeSessionType(Args&&... args) {
    CancelTimeout();
    ChangeSessionTypeImpl(
        std::make_shared<SessionType>(std::forward<Args>(args)...));
  }

  void StartTimeout();
  void CancelTimeout();

 private:
  void PopulateMembers(SessionManagerPtr manager, Worker&& worker,
                       AuthProviderPtr auth, SslContextPtr ssl,
                       RoutesVectorPtr routes, std::chrono::nanoseconds timeout,
                       std::uint32_t header_limit, std::uint32_t body_limit,
                       bool test_mode, boost::uuids::uuid session_id,
                       boost::asio::ip::tcp::endpoint remote_ep,
                       boost::beast::flat_buffer buffer,
                       std::string logging_prefix);
  void ChangeSessionTypeImpl(SessionPtr new_session);
  void DoClose();

  SessionManagerPtr manager_;
  Worker worker_;
  AuthProviderPtr auth_;
  SslContextPtr ssl_;
  RoutesVectorPtr routes_;
  std::chrono::nanoseconds timeout_;
  std::uint32_t header_limit_;
  std::uint32_t body_limit_;
  bool test_mode_;
  boost::uuids::uuid session_id_;
  boost::asio::ip::tcp::endpoint rep_;
  boost::beast::flat_buffer buffer_;
  bool replaced_ = false;
};

template <typename SessionType>
class SessionT : public Session {
 public:
  std::shared_ptr<SessionType> MakeSharedPtr() {
    return std::static_pointer_cast<SessionType>(shared_from_this());
  }

  std::weak_ptr<SessionType> MakeWeakPtr() { return {MakeSharedPtr()}; }
};

class SessionManager : public std::enable_shared_from_this<SessionManager> {
 public:
  template <typename... Args>
  SessionPtr CreateSession(Args&&... args) {
    auto session =
        Session::Create(shared_from_this(), std::forward<Args>(args)...);

    std::lock_guard<std::mutex> lock(mutex_);
    YOGI_ASSERT(!sessions_.count(session->SessionId()));
    sessions_[session->SessionId()] = session;

    return session;
  }

  void Remove(const Session& session);
  void Replace(SessionPtr new_session);
  void CloseAll();

 private:
  typedef std::unordered_map<boost::uuids::uuid, SessionPtr,
                             boost::hash<boost::uuids::uuid>>
      SessionsMap;

  SessionsMap sessions_;
  std::mutex mutex_;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
