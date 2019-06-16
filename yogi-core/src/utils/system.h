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

#include <vector>
#include <string>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/udp.hpp>

namespace utils {

enum class IpVersion { kAny, k4, k6 };

struct NetworkInterfaceInfo {
  std::string name;
  std::string identifier;  // The part after the % sign (e.g. ::1%en5 => "en5")
  std::string mac;
  std::vector<boost::asio::ip::address> addresses;
  bool is_loopback = false;
};

typedef std::vector<NetworkInterfaceInfo> NetworkInterfaceInfosVector;

std::string GetHostname();
int GetProcessId();
int GetCurrentThreadId();
NetworkInterfaceInfosVector GetNetworkInterfaces();
NetworkInterfaceInfosVector GetFilteredNetworkInterfaces(
    const std::vector<std::string>& adv_if_strings,
    IpVersion ip_version = IpVersion::kAny);
NetworkInterfaceInfosVector GetFilteredNetworkInterfaces(
    const std::vector<std::string>& adv_if_strings,
    const boost::asio::ip::udp& protocol);

}  // namespace utils
