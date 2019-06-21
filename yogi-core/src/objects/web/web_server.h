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

#include "../../config.h"
#include "../../utils/system.h"
#include "../../network/tcp_listener.h"
#include "../context.h"
#include "../branch/branch.h"
#include "../log/logger.h"
#include "detail/auth/auth_provider.h"
#include "detail/session/session.h"
#include "detail/ssl_context.h"
#include "detail/route.h"
#include "detail/worker_pool.h"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>

#include <vector>
#include <chrono>
#include <memory>
#include <unordered_set>
#include <mutex>

namespace objects {
namespace web {

class WebServer;
typedef std::shared_ptr<WebServer> WebServerPtr;
typedef std::weak_ptr<WebServer> WebServerWeakPtr;

class WebServer
    : public api::ExposedObjectT<WebServer, api::ObjectType::kWebServer>,
      public log::LoggerUser {
 public:
  WebServer(ContextPtr context, branch::BranchPtr branch,
            const nlohmann::json& cfg);
  virtual ~WebServer();

  int GetPort() { return listener_->GetPort(); }
  void AddWorker(ContextPtr worker);
  void Start();

  bool CompressAssetsEnabled() const { return compress_assets_; }
  std::size_t CacheSize() const { return cache_size_; }

 protected:
  friend class detail::Session;
  void DestroySession(detail::SessionPtr session);
  void ReplaceSession(detail::SessionPtr session);

 private:
  void CreateListener(const nlohmann::json& cfg);
  void OnAccepted(boost::asio::ip::tcp::socket socket);
  void CloseAllSessions();

  const ContextPtr context_;
  const branch::BranchPtr branch_;
  std::chrono::nanoseconds timeout_;
  bool test_mode_;
  bool compress_assets_;
  std::size_t cache_size_;
  network::TcpListenerPtr listener_;
  detail::AuthProviderPtr auth_;
  detail::RoutesVectorPtr routes_;
  detail::SslContextPtr ssl_;
  detail::WorkerPool worker_pool_;
  detail::SessionsMap sessions_;
  std::mutex sessions_mutex_;
};

}  // namespace web
}  // namespace objects
