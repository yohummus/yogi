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

#include "listener.h"
#include "../../../api/constants.h"
#include "../../../utils/json_helpers.h"

using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {
namespace detail {
namespace web {

Listener::Listener(ContextPtr context, const nlohmann::json& cfg)
    : context_(context) {
  // clang-format off
  port_    = cfg.value("port", static_cast<unsigned short>(api::kDefaultWebPort));
  ifs_     = utils::GetFilteredNetworkInterfaces(utils::ExtractArrayOfStrings(cfg, "interfaces", api::kDefaultWebInterfaces));
  timeout_ = utils::ExtractDuration(cfg, "timeout", api::kDefaultWebTimeout);
  // clang-format on

  SetLoggingPrefix("["s + std::to_string(port_) + ']');
}

}  // namespace web
}  // namespace detail
}  // namespace objects
