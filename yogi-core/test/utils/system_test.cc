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

#include "../common.h"
#include "../../src/utils/system.h"
#include "../../src/utils/algorithm.h"

#include <regex>

TEST(SystemTest, GetHostname) { EXPECT_FALSE(utils::GetHostname().empty()); }

TEST(SystemTest, GetProcessId) { EXPECT_GT(utils::GetProcessId(), 0); }

TEST(SystemTest, GetCurrentThreadId) {
  EXPECT_GT(utils::GetCurrentThreadId(), 0);
}

TEST(SystemTest, GetNetworkInterfaces) {
  auto ifs = utils::GetNetworkInterfaces();
  ASSERT_FALSE(ifs.empty());

  auto localhost_found = false;

  for (auto& info : ifs) {
    EXPECT_FALSE(info.name.empty());
    EXPECT_FALSE(info.identifier.empty());
    EXPECT_FALSE(info.mac.empty() && info.addresses.empty());

    if (!info.mac.empty()) {
      std::regex re("^([0-9a-f][0-9a-f]:){5}[0-9a-f][0-9a-f]$");
      std::smatch m;
      EXPECT_TRUE(std::regex_match(info.mac, m, re)) << "String: " << info.mac;
    }

    bool is_loopback = utils::contains_if(
        info.addresses, [](auto& addr) { return addr.is_loopback(); });
    EXPECT_EQ(is_loopback, info.is_loopback);
    localhost_found |= is_loopback;
  }

  EXPECT_TRUE(localhost_found);
}

TEST(SystemTest, GetFilteredNetworkInterfaces) {
  std::vector<boost::asio::ip::udp> protocols = {boost::asio::ip::udp::v4(),
                                                 boost::asio::ip::udp::v6()};
  for (auto protocol : protocols) {
    auto ifs = utils::GetFilteredNetworkInterfaces({"localhost"}, protocol);
    ASSERT_EQ(ifs.size(), 1);
    EXPECT_TRUE(ifs[0].is_loopback);

    ifs = utils::GetFilteredNetworkInterfaces({"all"}, protocol);
    ASSERT_GT(ifs.size(), 1)
        << "Make sure you have an active LAN or Wi-Fi connection, otherwise "
           "the test fails because it cannot find any network interfaces other "
           "than the loopback interface.";

    auto if_it = utils::find_if(
        ifs, [](auto& info) { return !info.is_loopback && !info.mac.empty(); });
    ASSERT_NE(if_it, ifs.end()) << "No network interface found that has a MAC "
                                   "and that is not a loopback interface.";
    auto ifc = *if_it;

    ifs = utils::GetFilteredNetworkInterfaces({ifc.name}, protocol);
    ASSERT_EQ(ifs.size(), 1);
    EXPECT_EQ(ifs[0].name, ifc.name);

    ifs = utils::GetFilteredNetworkInterfaces({ifc.mac}, protocol);
    ASSERT_EQ(ifs.size(), 1);
    EXPECT_EQ(ifs[0].mac, ifc.mac);
  }
}
