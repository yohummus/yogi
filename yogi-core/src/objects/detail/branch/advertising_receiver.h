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

#include <boost/uuid/uuid.hpp>
#include <boost/asio/ip/udp.hpp>
#include <vector>
#include <memory>
#include <functional>

namespace objects {
namespace detail {

class AdvertisingReceiver
    : public std::enable_shared_from_this<AdvertisingReceiver> {
 public:
  typedef std::function<void(const boost::uuids::uuid& uuid,
                             const boost::asio::ip::tcp::endpoint& ep)>
      ObserverFn;

  AdvertisingReceiver(ContextPtr context,
                      const boost::asio::ip::udp::endpoint& adv_ep,
                      ObserverFn observer_fn);

  void Start(LocalBranchInfoPtr info);

 private:
  void SetupSocket();
  bool JoinMulticastGroups();
  void StartReceiveAdvertisement();
  void OnReceivedAdvertisementFinished(const boost::system::error_code& ec,
                                       std::size_t bytes_received);

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const boost::asio::ip::udp::endpoint adv_ep_;
  const ObserverFn observer_fn_;
  const utils::SharedByteVector buffer_;
  LocalBranchInfoPtr info_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint sender_ep_;
};

typedef std::shared_ptr<AdvertisingReceiver> AdvertisingReceiverPtr;

}  // namespace detail
}  // namespace objects
