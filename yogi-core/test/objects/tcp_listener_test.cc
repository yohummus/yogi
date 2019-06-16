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

#include "../common.h"
#include "../../src/api/constants.h"
#include "../../src/objects/detail/tcp_listener.h"
#include "../../src/utils/algorithm.h"
#include "../../src/utils/system.h"
using namespace objects;
using namespace objects::detail;

#include <boost/asio/ip/tcp.hpp>
using namespace boost::asio;

#include <vector>
#include <chrono>
using namespace std::chrono_literals;

class TcpListenerTest : public TestFixture {
 protected:
  enum InterfacesCount { kOne, kTwo, kAll };

  struct TestInterface {
    std::string name;
    ip::address_v4 addr_v4;
    ip::address_v6 addr_v6;
  };

  virtual void SetUp() override {
    // Find network interfaces that support both IPv4 and IPv6
    for (auto info : utils::GetNetworkInterfaces()) {
      auto v4_it = utils::find_if(info.addresses,
                                  [](auto& addr) { return addr.is_v4(); });
      auto v6_it = utils::find_if(info.addresses,
                                  [](auto& addr) { return addr.is_v6(); });
      if (v4_it != info.addresses.end() && v6_it != info.addresses.end()) {
        ifs_.push_back({info.name, v4_it->to_v4(), v6_it->to_v6()});
      }
    }

    ASSERT_GE(ifs_.size(), 2)
        << "Not enough configured network interfaces "
           "available that support both IPv4 and IPv6. Usually this can be "
           "fixed by making sure that you have an active LAN or Wi-Fi "
           "connection.";
  }

  ContextPtr context_ = std::make_shared<Context>();
  TcpListenerPtr listener_;
  std::vector<TestInterface> ifs_;
  const int kFixedPort = api::kDefaultWebPort;
  int accepted_cnt_ = 0;

  void CreateListener(InterfacesCount if_cnt, utils::IpVersion ip_version,
                      int port = 0) {
    std::vector<std::string> interfaces;

    switch (if_cnt) {
      case kOne:
        interfaces.push_back(ifs_[0].name);
        break;

      case kTwo:
        interfaces.push_back(ifs_[0].name);
        interfaces.push_back(ifs_[1].name);
        break;

      case kAll:
        interfaces.push_back("all");
        break;
    }

    listener_ =
        std::make_shared<TcpListener>(context_, interfaces, ip_version, "");

    listener_->Start([this](auto socket) { ++this->accepted_cnt_; });
  }

  bool CanConnect(const ip::address& addr) {
    ip::tcp::socket s(context_->IoContext());

    int port = listener_ ? listener_->GetPort() : kFixedPort;
    ip::tcp::endpoint ep(addr, static_cast<unsigned short>(port));

    int prev_accepted_cnt = accepted_cnt_;

    bool called = false;
    bool connected = false;
    s.async_connect(ep, [&](auto ec) {
      connected = !ec;
      called = true;
    });

    while (!called && context_->RunOne(1s))
      ;

    EXPECT_TRUE(called);

    if (connected) {
      while (accepted_cnt_ == prev_accepted_cnt && context_->RunOne(1s))
        ;
    }

    EXPECT_EQ(accepted_cnt_, prev_accepted_cnt + (connected ? 1 : 0));

    return connected;
  }
};

TEST_F(TcpListenerTest, GetPortForAllInterfacesWithAnyPort) {
  CreateListener(kAll, utils::IpVersion::kAny);
  EXPECT_GT(listener_->GetPort(), 0);
}

TEST_F(TcpListenerTest, GetPortForOneInterfaceWithAnyPort) {
  CreateListener(kTwo, utils::IpVersion::kAny);
  EXPECT_GT(listener_->GetPort(), 0);
}

TEST_F(TcpListenerTest, GetPortForAllInterfacesWithFixedPort) {
  CreateListener(kAll, utils::IpVersion::kAny, kFixedPort);
  EXPECT_GT(listener_->GetPort(), kFixedPort);
}

TEST_F(TcpListenerTest, GetPortForOneInterfaceWithFixedPort) {
  CreateListener(kTwo, utils::IpVersion::kAny, kFixedPort);
  EXPECT_GT(listener_->GetPort(), kFixedPort);
}

TEST_F(TcpListenerTest, AllInterfacesAnyPort) {
  CreateListener(kAll, utils::IpVersion::kAny);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v6));
}

TEST_F(TcpListenerTest, AllInterfacesIpv4AnyPort) {
  CreateListener(kAll, utils::IpVersion::k4);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[0].addr_v6));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v6));
}

TEST_F(TcpListenerTest, AllInterfacesIpv6AnyPort) {
  CreateListener(kAll, utils::IpVersion::k6);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v6));

  // Connecting using IPv4 may still work
}

TEST_F(TcpListenerTest, AllInterfacesFixedPort) {
  EXPECT_FALSE(CanConnect(ifs_[0].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[0].addr_v6));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v6));

  CreateListener(kAll, utils::IpVersion::kAny, kFixedPort);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v6));
}

TEST_F(TcpListenerTest, AllInterfacesIpv4FixedPort) {
  CreateListener(kAll, utils::IpVersion::k4, kFixedPort);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[0].addr_v6));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v6));
}

TEST_F(TcpListenerTest, AllInterfacesIpv6FixedPort) {
  CreateListener(kAll, utils::IpVersion::k6, kFixedPort);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v6));

  // Connecting using IPv4 may still work
}

TEST_F(TcpListenerTest, OneInterfaceAnyPort) {
  CreateListener(kOne, utils::IpVersion::kAny);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v6));
}

TEST_F(TcpListenerTest, OneInterfaceIpv4AnyPort) {
  CreateListener(kOne, utils::IpVersion::k4);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[0].addr_v6));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v6));
}

TEST_F(TcpListenerTest, OneInterfaceIpv6AnyPort) {
  CreateListener(kOne, utils::IpVersion::k6);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v6));

  // Connecting using IPv4 may still work
}

TEST_F(TcpListenerTest, OneInterfaceFixedPort) {
  CreateListener(kOne, utils::IpVersion::kAny, kFixedPort);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v6));
}

TEST_F(TcpListenerTest, OneInterfaceIpv4FixedPort) {
  CreateListener(kOne, utils::IpVersion::k4, kFixedPort);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[0].addr_v6));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v4));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v6));
}

TEST_F(TcpListenerTest, OneInterfaceIpv6FixedPort) {
  CreateListener(kOne, utils::IpVersion::k6, kFixedPort);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_FALSE(CanConnect(ifs_[1].addr_v6));

  // Connecting using IPv4 may still work
}

TEST_F(TcpListenerTest, TwoInterfacesAnyPort) {
  CreateListener(kTwo, utils::IpVersion::kAny);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v6));
}

TEST_F(TcpListenerTest, TwoInterfacesFixedPort) {
  CreateListener(kTwo, utils::IpVersion::kAny, kFixedPort);

  EXPECT_TRUE(CanConnect(ifs_[0].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[0].addr_v6));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v4));
  EXPECT_TRUE(CanConnect(ifs_[1].addr_v6));
}
