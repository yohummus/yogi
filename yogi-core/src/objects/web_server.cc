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

namespace objects {

WebServer::WebServer(ContextPtr context, BranchPtr branch,
                     const nlohmann::json& cfg) {
  YOGI_ASSERT(context);
}

void WebServer::Start() {}

const LoggerPtr WebServer::logger_ =
    Logger::CreateStaticInternalLogger("WebServer");

}  // namespace objects
