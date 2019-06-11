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

#include "../../../config.h"
#include "../../../utils/system.h"
#include "../../context.h"
#include "../../logger.h"

#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <chrono>

namespace objects {
namespace detail {
namespace web {

class Listener : public LoggerUser {
 public:
  Listener(ContextPtr context, const nlohmann::json& cfg);

 private:
  void SetupAcceptor();
  
  const ContextPtr context_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::chrono::nanoseconds timeout_;
  unsigned short port_;
  std::vector<utils::NetworkInterfaceInfo> ifs_;
};

typedef std::shared_ptr<Listener> ListenerPtr;

}  // namespace web
}  // namespace detail
}  // namespace objects
