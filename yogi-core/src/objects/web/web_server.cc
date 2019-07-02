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

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
namespace asio = boost::asio;
using asio::ip::tcp;

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
  header_limit_    = utils::ExtractNumberWithInfSupport<std::uint32_t>(cfg, "http_header_limit", api::kDefaultHttpHeaderLimit);
  body_limit_      = utils::ExtractNumberWithInfSupport<std::uint32_t>(cfg, "http_body_limit", api::kDefaultHttpBodyLimit);
  test_mode_       = cfg.value("test_mode", false);
  compress_assets_ = cfg.value("compress_assets", true);
  cache_size_      = utils::ExtractSize(cfg, "cache_size", api::kDefaultWebCacheSize);
  auth_            = detail::AuthProvider::Create(cfg, GetLoggingPrefix());
  routes_          = detail::Route::CreateAll(cfg, *auth_, GetLoggingPrefix());
  ssl_             = detail::SslContext::Create(cfg, GetLoggingPrefix());
  sessions_        = std::make_shared<detail::SessionManager>();
  // clang-format on
}

WebServer::~WebServer() { sessions_->CloseAll(); }

void WebServer::AddWorker(ContextPtr worker_context) {
  worker_pool_.AddWorker(worker_context);
}

void WebServer::Start() {
  listener_->Start(utils::BindWeak(&WebServer::OnAccepted, this));
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
    auto worker = worker_pool_.AcquireWorker();
    auto ass_socket = MakeSocketAssignedToWorker(std::move(socket), worker);
    auto session = sessions_->CreateSession(
        auth_, ssl_, routes_, std::move(worker), timeout_, header_limit_,
        body_limit_, test_mode_, std::move(ass_socket));

    LOG_DBG("Session " << session->GetLoggingPrefix() << " created for client "
                       << session->RemoteEndpoint());

    session->Start();
  } catch (const std::exception& e) {
    LOG_ERR("Could not start session: " << e.what());
  }
}

tcp::socket WebServer::MakeSocketAssignedToWorker(
    tcp::socket&& socket, const detail::Worker& worker) {
  if (worker.IsFallback()) {
    return std::move(socket);
  } else {
    auto& ioc = worker.Context()->IoContext();
    auto prot = socket.local_endpoint().protocol();
    return tcp::socket(asio::make_strand(ioc), prot, socket.release());
  }
}

}  // namespace web
}  // namespace objects
