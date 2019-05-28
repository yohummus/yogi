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

#include "../config.h"
#include "../utils/system.h"
#include "context.h"
#include "branch.h"
#include "logger.h"
#include "detail/web/auth_provider.h"
#include "detail/web/ssl_parameters.h"
#include "detail/web/route.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace objects {

class WebServer
    : public api::ExposedObjectT<WebServer, api::ObjectType::kWebServer>,
      public LoggerUser {
 public:
  WebServer(ContextPtr context, BranchPtr branch, const nlohmann::json& cfg);

  void Start();

 private:
  detail::web::RoutesVector CreateAllRoutes(const nlohmann::json& cfg) const;

  const ContextPtr context_;
  const BranchPtr branch_;
  const unsigned short port_;
  const std::vector<utils::NetworkInterfaceInfo> ifs_;
  const std::chrono::nanoseconds timeout_;
  const bool test_mode_;
  const bool compress_assets_;
  const std::size_t cache_size_;
  const std::string logging_prefix_;
  const detail::web::AuthProviderPtr auth_;
  const detail::web::RoutesVector routes_;
  const detail::web::SslParameters ssl_;
};

typedef std::shared_ptr<WebServer> WebServerPtr;

}  // namespace objects
