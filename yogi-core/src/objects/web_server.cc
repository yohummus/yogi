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
#include "../api/errors.h"
#include "../api/constants.h"
#include "../utils/json_helpers.h"
#include "../utils/schema.h"

using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {

WebServer::WebServer(ContextPtr context, BranchPtr branch,
                     const nlohmann::json& cfg)
    : context_(context), branch_(branch) {
  utils::ValidateJson(cfg, "web_server.schema.json");

  // clang-format off
  port_            = cfg.value("port", static_cast<unsigned short>(api::kDefaultWebPort));
  ifs_             = utils::GetFilteredNetworkInterfaces(utils::ExtractArrayOfStrings(cfg, "interfaces", api::kDefaultWebInterfaces));
  timeout_         = utils::ExtractDuration(cfg, "timeout", api::kDefaultWebTimeout);
  test_mode_       = cfg.value("test_mode", false);
  compress_assets_ = cfg.value("compress_assets", true);
  cache_size_      = utils::ExtractSize(cfg, "cache_size", api::kDefaultWebCacheSize);
  logging_prefix_  = "["s + std::to_string(port_) + ']';
  auth_            = detail::web::AuthProvider::Create(cfg["authentication"], logging_prefix_);
  routes_          = detail::web::Route::CreateAll(cfg, *auth_, logging_prefix_);
  ssl_             = std::make_unique<detail::web::SslParameters>(cfg["ssl"], logging_prefix_);
  // clang-format on

  SetLoggingPrefix(logging_prefix_);
}

void WebServer::Start() {}

}  // namespace objects
