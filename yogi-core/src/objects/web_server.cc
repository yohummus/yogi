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

using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {

WebServer::WebServer(ContextPtr context, BranchPtr branch,
                     const nlohmann::json& cfg)
    : context_(context),
      branch_(branch),
      port_(
          cfg.value("port", static_cast<unsigned short>(api::kDefaultWebPort))),
      ifs_(utils::GetFilteredNetworkInterfaces(utils::ExtractArrayOfStrings(
          cfg, "interfaces", api::kDefaultWebInterfaces))),
      timeout_(utils::ExtractDuration(cfg, "timeout", api::kDefaultWebTimeout)),
      test_mode_(cfg.value("test_mode", false)),
      compress_assets_(cfg.value("compress_assets", true)),
      cache_size_(
          utils::ExtractSize(cfg, "cache_size", api::kDefaultWebCacheSize)),
      logging_prefix_("["s + std::to_string(port_) + ']'),
      auth_(detail::web::AuthProvider::Create(cfg["authentication"],
                                              logging_prefix_)),
      routes_(CreateAllRoutes(cfg)),
      ssl_(cfg["ssl"], logging_prefix_) {
  YOGI_ASSERT(context);
  SetLoggingPrefix(logging_prefix_);
}

void WebServer::Start() {}

detail::web::RoutesVector WebServer::CreateAllRoutes(
    const nlohmann::json& cfg) const {
  detail::web::RoutesVector routes;

  auto& api_perm_cfg = cfg["api_permissions"];
  if (!api_perm_cfg.is_object()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid API permissions section.";
  }

  for (auto it = api_perm_cfg.begin(); it != api_perm_cfg.end(); ++it) {
    // routes.push_back(std::make_unique<detail::web::ApiEndpoint>(
    //     it.key(), it.value(), logging_prefix_, auth_->GetGroups()));
  }

  auto& routes_cfg = cfg["routes"];
  if (!routes_cfg.is_object()) {
    throw api::DescriptiveError(YOGI_ERR_CONFIG_NOT_VALID)
        << "Missing or invalid routes section.";
  }

  for (auto it = routes_cfg.begin(); it != routes_cfg.end(); ++it) {
    routes.push_back(detail::web::Route::Create(*auth_, it, logging_prefix_,
                                                auth_->GetGroups()));
  }

  return routes;
}

}  // namespace objects
