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

namespace objects {
namespace web {

class WebServer;
typedef std::weak_ptr<WebServer> WebServerWeakPtr;

namespace detail {

class Session;
typedef std::shared_ptr<Session> SessionPtr;
typedef std::unordered_map<boost::uuids::uuid, detail::SessionPtr,
                           boost::hash<boost::uuids::uuid>>
    SessionsMap;

class Session : public log::LoggerUser,
                public std::enable_shared_from_this<Session> {
 public:
  static SessionPtr Create(WebServerWeakPtr server, WorkerPool& worker_pool,
                           AuthProviderPtr auth, SslContextPtr ssl,
                           RoutesVectorPtr routes,
                           std::chrono::nanoseconds timeout, bool test_mode,
                           boost::asio::ip::tcp::socket socket);

  virtual ~Session();

  virtual void Start() = 0;

  std::chrono::nanoseconds Timeout() const { return timeout_; }
  bool TestModeEnabled() const { return test_mode_; }
  const detail::AuthProviderPtr& AuthProvider() const { return auth_; }
  const detail::SslContextPtr& SslContext() const { return ssl_; }
  const detail::RoutesVectorPtr& Routes() const { return routes_; }
  const boost::uuids::uuid& SessionId() const { return session_id_; }
  boost::asio::ip::tcp::endpoint GetRemoteEndpoint();

  void Close();
  void Destroy();

 protected:
  virtual boost::beast::tcp_stream& Stream() = 0;

  boost::beast::flat_buffer& Buffer() { return buffer_; }

  template <typename SessionType, typename... Args>
  void ChangeSessionType(Args&&... args) {
    auto new_session =
        std::make_shared<SessionType>(std::forward<Args>(args)...);
    ChangeSessionTypeImpl(new_session);
  }

  void StartTimeout();
  void CancelTimeout();

 private:
  void ChangeSessionTypeImpl(SessionPtr new_session);

  WebServerWeakPtr server_;
  Worker worker_;
  AuthProviderPtr auth_;
  SslContextPtr ssl_;
  RoutesVectorPtr routes_;
  std::chrono::nanoseconds timeout_;
  bool test_mode_;
  boost::uuids::uuid session_id_;
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

}  // namespace detail
}  // namespace web
}  // namespace objects
