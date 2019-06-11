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
#include "../../../api/errors.h"
#include "../../../utils/json_helpers.h"

#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>

using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("WebServer");

namespace objects {
namespace detail {
namespace web {

Listener::Listener(ContextPtr context, const nlohmann::json& cfg)
    : context_(context),
      acceptor_(boost::asio::make_strand(context->IoContext())) {
  timeout_ = utils::ExtractDuration(cfg, "timeout", api::kDefaultWebTimeout);
  port_ = cfg.value("port", static_cast<unsigned short>(api::kDefaultWebPort));
  ifs_ = utils::GetFilteredNetworkInterfaces(utils::ExtractArrayOfStrings(
      cfg, "interfaces", api::kDefaultWebInterfaces));

  SetLoggingPrefix("["s + std::to_string(port_) + ']');

  SetupAcceptor();
}

void Listener::SetupAcceptor() {
  //   boost::system::error_code ec;
  //   acceptor_.open(protocol, ec);
  //   if (ec) throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);

  //   acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true),
  //   ec); if (ec) throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);

  //   bool bound_at_least_once = false;
  //   for (auto& info : ifs_) {
  //     for (auto& addr : info.addresses) {
  //       acceptor_.bind(boost::asio::ip::tcp::endpoint(protocol, port_), ec);
  //       if (ec) {
  //         LOG_ERR("Could not bind to interface "
  //                 << addr
  //                 << " for client connections. This interface will be
  //                 ignored.");
  //         continue;
  //       }

  //       LOG_IFO("Using interface " << addr << " for branch connections.");

  //       bound_at_least_once = true;
  //     }
  //   }

  //   if (!bound_at_least_once) {
  //     acceptor_.listen(acceptor_.max_listen_connections, ec);
  //     if (ec) throw api::Error(YOGI_ERR_LISTEN_SOCKET_FAILED);
  //   } else {
  //     LOG_ERR("No network interfaces available for client connections.");
  //   }
}

}  // namespace web
}  // namespace detail
}  // namespace objects
