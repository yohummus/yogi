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

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {

WebServer::WebServer(ContextPtr context, BranchPtr branch,
                     const nlohmann::json& cfg)
    : context_(context), branch_(branch) {
  utils::ValidateJson(cfg, "web_server.schema.json");

  listener_ = std::make_shared<detail::web::Listener>(context, cfg);
  SetLoggingPrefix(listener_->GetLoggingPrefix());

  // clang-format off
  test_mode_       = cfg.value("test_mode", false);
  compress_assets_ = cfg.value("compress_assets", true);
  cache_size_      = utils::ExtractSize(cfg, "cache_size", api::kDefaultWebCacheSize);
  auth_            = detail::web::AuthProvider::Create(cfg, GetLoggingPrefix());
  routes_          = detail::web::Route::CreateAll(cfg, *auth_, GetLoggingPrefix());
  ssl_             = std::make_unique<detail::web::SslParameters>(cfg, GetLoggingPrefix());
  // clang-format on
}

void WebServer::Start() {}

}  // namespace objects
