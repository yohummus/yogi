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

#include "ip.h"

namespace network {

std::string MakeIpAddressString(const boost::asio::ip::address& addr) {
  auto s = addr.to_string();

  auto pos = s.find('%');
  if (pos != std::string::npos) {
    s.erase(pos);
  }

  return s;
}

std::string MakeIpAddressString(const boost::asio::ip::tcp::endpoint& ep) {
  return MakeIpAddressString(ep.address());
}

std::string MakeIpAddressString(const boost::asio::ip::udp::endpoint& ep) {
  return MakeIpAddressString(ep.address());
}

}  // namespace network
