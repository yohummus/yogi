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

#include "web_server.h"
#include "../../api/errors.h"
#include "../../api/constants.h"
#include "../../utils/algorithm.h"
#include "../../utils/bind.h"
#include "../../utils/json_helpers.h"
#include "../../schema/schema.h"

#include <algorithm>

#include <string>
using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("WebServer")

namespace objects {
namespace web {

WebServer::WebServer(ContextPtr context, branch::BranchPtr branch,
                     const nlohmann::json& cfg)
    : context_(context), branch_(branch), worker_pool_(context) {
  schema::ValidateJson(cfg, "web_server.schema.json");

  CreateListener(cfg);
  SetLoggingPrefix("Port "s + std::to_string(GetPort()));

  // clang-format off
  timeout_         = utils::ExtractDuration(cfg, "timeout", api::kDefaultWebTimeout);
  test_mode_       = cfg.value("test_mode", false);
  compress_assets_ = cfg.value("compress_assets", true);
  cache_size_      = utils::ExtractSize(cfg, "cache_size", api::kDefaultWebCacheSize);
  auth_            = detail::AuthProvider::Create(cfg, GetLoggingPrefix());
  routes_          = detail::Route::CreateAll(cfg, *auth_, GetLoggingPrefix());
  ssl_             = detail::SslContext::Create(cfg, GetLoggingPrefix());
  // clang-format on
}

WebServer::~WebServer() { CloseAllSessions(); }

void WebServer::AddWorker(ContextPtr worker_context) {
  worker_pool_.AddWorker(worker_context);
}

void WebServer::Start() {
  listener_->Start(utils::BindWeak(&WebServer::OnAccepted, this));
}

void WebServer::DestroySession(detail::SessionPtr session) {
  session->Close();

  std::lock_guard<std::mutex> lock(sessions_mutex_);
  YOGI_ASSERT(sessions_.count(session->SessionId()));
  sessions_.erase(session->SessionId());
}

void WebServer::ReplaceSession(detail::SessionPtr session) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  YOGI_ASSERT(sessions_.count(session->SessionId()));
  sessions_[session->SessionId()] = session;
}

void WebServer::CreateListener(const nlohmann::json& cfg) {
  auto interfaces = utils::ExtractArrayOfStrings(cfg, "interfaces",
                                                 api::kDefaultWebInterfaces);
  auto port =
      cfg.value("port", static_cast<unsigned short>(api::kDefaultWebPort));

  listener_ = std::make_shared<network::TcpListener>(
      context_, interfaces, utils::IpVersion::kAny, port, "web server");
}

void WebServer::OnAccepted(boost::asio::ip::tcp::socket socket) {
  try {
    auto session = detail::Session::Create(MakeWeakPtr(), worker_pool_, auth_,
                                           ssl_, routes_, timeout_, test_mode_,
                                           std::move(socket));

    YOGI_ASSERT(!sessions_.count(session->SessionId()));
    sessions_[session->SessionId()] = session;

    LOG_DBG("Session " << session->GetLoggingPrefix() << " created for client "
                       << session->GetRemoteEndpoint());

    session->Start();
  } catch (const std::exception& e) {
    LOG_ERR("Could not start session: " << e.what());
  }
}

void WebServer::CloseAllSessions() {
  for (auto& session : sessions_) {
    session.second->Close();
  }
}

}  // namespace web
}  // namespace objects
