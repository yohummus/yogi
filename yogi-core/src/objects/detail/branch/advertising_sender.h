/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
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
#include "../../context.h"
#include "../../logger.h"
#include "branch_info.h"

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <memory>

namespace objects {
namespace detail {

class AdvertisingSender
    : public std::enable_shared_from_this<AdvertisingSender> {
 public:
  AdvertisingSender(ContextPtr context,
                    const boost::asio::ip::udp::endpoint& adv_ep);
  void Start(LocalBranchInfoPtr info);

  const boost::asio::ip::udp::endpoint& GetEndpoint() const { return adv_ep_; }

 private:
  struct SocketEntry {
    std::string interface_name;
    boost::asio::ip::address address;
    boost::asio::ip::udp::socket socket;

    SocketEntry(boost::asio::io_context& ioc) : socket(ioc) {}
  };

  void SetupSockets();
  bool ConfigureSocket(std::shared_ptr<SocketEntry> entry);
  void SendAdvertisements();
  void StartTimer();

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const boost::asio::ip::udp::endpoint adv_ep_;
  LocalBranchInfoPtr info_;
  boost::asio::steady_timer timer_;
  std::vector<std::shared_ptr<SocketEntry>> sockets_;
  int active_send_ops_;
};

typedef std::shared_ptr<AdvertisingSender> AdvertisingSenderPtr;

}  // namespace detail
}  // namespace objects
