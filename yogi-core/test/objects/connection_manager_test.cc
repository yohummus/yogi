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
#include "../../src/api/constants.h"

#include <boost/asio.hpp>

class ConnectionManagerTest : public TestFixture {
 protected:
  virtual void SetUp() override {
    context_ = CreateContext();
    branch_ = CreateBranch(context_);
  }

  void ReCreateBranch(const char* adv_addr) {
    auto res = YOGI_Destroy(branch_);
    ASSERT_OK(res);
    branch_ =
        CreateBranch(context_, nullptr, nullptr, nullptr, nullptr, adv_addr);
  }

  void* context_;
  void* branch_;

  void TestAdvertising(const char* adv_addr) {
    MulticastSocket multicast(boost::asio::ip::udp::endpoint(
        boost::asio::ip::make_address(adv_addr), kAdvPort));

    ReCreateBranch(adv_addr);
    RunContextInBackground(context_);

    auto msg = multicast.Receive();
    ASSERT_EQ(msg.size(), 25) << "Unexpected advertising message size";

    boost::uuids::uuid uuid;
    YOGI_BranchGetInfo(branch_, &uuid, nullptr, 0);

    EXPECT_EQ(msg[0], 'Y');
    EXPECT_EQ(msg[1], 'O');
    EXPECT_EQ(msg[2], 'G');
    EXPECT_EQ(msg[3], 'I');
    EXPECT_EQ(msg[4], '\0');
    EXPECT_EQ(msg[5], api::kVersionMajor);
    EXPECT_EQ(msg[6], api::kVersionMinor);
    EXPECT_EQ(std::memcmp(&uuid, msg.data() + 7, sizeof(uuid)), 0);
    EXPECT_TRUE(msg[23] != 0 || msg[24] != 0);
  }

  void TestConnectNormally(const char* adv_addr) {
    ReCreateBranch(adv_addr);

    void* branch_a =
        CreateBranch(context_, "a", nullptr, nullptr, nullptr, adv_addr);
    void* branch_b =
        CreateBranch(context_, "b", nullptr, nullptr, nullptr, adv_addr);

    BranchEventRecorder rec(context_, branch_);
    rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_a, YOGI_OK);
    rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_b, YOGI_OK);
  }
};

TEST_F(ConnectionManagerTest, AdvertisingIPv4) {
  TestAdvertising("239.255.0.1");
}

TEST_F(ConnectionManagerTest, AdvertisingIPv6) {
  TestAdvertising("ff02::8000:2439");
}

TEST_F(ConnectionManagerTest, ConnectNormallyIPv4) {
  TestConnectNormally("239.255.0.1");
}

TEST_F(ConnectionManagerTest, ConnectNormallyIPv6) {
  TestConnectNormally("ff02::8000:2439");
}

TEST_F(ConnectionManagerTest, DuplicateBranchName) {
  void* branch_a = CreateBranch(context_);  // Same name as branch_
  BranchEventRecorder rec(context_, branch_);
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_a,
                      YOGI_ERR_DUPLICATE_BRANCH_NAME);
  YOGI_Destroy(branch_a);

  void* branch_b = CreateBranch(context_, "Same name");
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_b, YOGI_OK);

  void* branch_c = CreateBranch(context_, "Same name");
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_c,
                      YOGI_ERR_DUPLICATE_BRANCH_NAME);
}

TEST_F(ConnectionManagerTest, DuplicateBranchPath) {
  auto branch_path = GetBranchInfo(branch_)["path"].get<std::string>();
  void* branch_a =
      CreateBranch(context_, "a", nullptr, nullptr, branch_path.c_str());
  BranchEventRecorder rec(context_, branch_);
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_a,
                      YOGI_ERR_DUPLICATE_BRANCH_PATH);
  YOGI_Destroy(branch_a);

  void* branch_b = CreateBranch(context_, "b", nullptr, nullptr, "/tmp");
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_b, YOGI_OK);

  void* branch_c = CreateBranch(context_, "c", nullptr, nullptr, "/tmp");
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_c,
                      YOGI_ERR_DUPLICATE_BRANCH_PATH);
}

TEST_F(ConnectionManagerTest, NetNameMismatch) {
  void* branch_a = CreateBranch(context_, "a", "other net");
  BranchEventRecorder rec(context_, branch_);
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_a,
                      YOGI_ERR_NET_NAME_MISMATCH);
}

TEST_F(ConnectionManagerTest, PasswordMismatch) {
  void* branch_a = CreateBranch(context_, "a", nullptr, "diferent password");
  BranchEventRecorder rec(context_, branch_);
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_a,
                      YOGI_ERR_PASSWORD_MISMATCH);
}

TEST_F(ConnectionManagerTest, AdvAndInfoMessageUuidMismatch) {
  RunContextInBackground(context_);
  FakeBranch fake;

  auto fn = [](auto msg) { ++msg->at(8); };
  fake.Advertise(fn);

  EXPECT_THROW(fake.Accept(), boost::system::system_error);
}

TEST_F(ConnectionManagerTest, Reconnect) {
  RunContextInBackground(context_);
  FakeBranch fake;

  fake.Connect(branch_);
  while (!fake.IsConnectedTo(branch_))
    ;

  fake.Disconnect();
  while (fake.IsConnectedTo(branch_))
    ;

  fake.Advertise();
  fake.Accept();
  while (!fake.IsConnectedTo(branch_))
    ;
}

TEST_F(ConnectionManagerTest, InvalidMagicPrefix) {
  RunContextInBackground(context_);
  FakeBranch fake;

  auto fn = [](auto msg) { msg->at(1) = 'X'; };

  EXPECT_THROW(fake.Connect(branch_, fn), boost::system::system_error);
  fake.Advertise();
  EXPECT_THROW(fake.Accept(fn), boost::system::system_error);
}

TEST_F(ConnectionManagerTest, IncompatibleVersion) {
  RunContextInBackground(context_);
  FakeBranch fake;

  auto fn = [](auto msg) { ++msg->at(5); };

  EXPECT_THROW(fake.Connect(branch_, fn), boost::system::system_error);
  fake.Advertise();
  EXPECT_THROW(fake.Accept(fn), boost::system::system_error);
}

TEST_F(ConnectionManagerTest, LoopbackConnection) {
  RunContextInBackground(context_);
  FakeBranch fake;

  auto fn = [&](auto msg) {
    auto uuid = GetBranchUuid(branch_);
    std::copy(uuid.begin(), uuid.end(), msg->begin() + 7);
  };

  EXPECT_THROW(fake.Connect(branch_, fn), boost::system::system_error);
  fake.Advertise();
  EXPECT_THROW(fake.Accept(fn), boost::system::system_error);
}

TEST_F(ConnectionManagerTest, BrokenAdvertisementMessage) {
  RunContextInBackground(context_);
  FakeBranch fake;

  fake.Advertise([](auto msg) { msg->push_back('x'); });
  std::this_thread::sleep_for(1ms);
  // just checking that nothing crashes
}

TEST_F(ConnectionManagerTest, BrokenInfoMessage) {
  RunContextInBackground(context_);
  FakeBranch fake;

  auto fn = [](auto msg) {
    ++msg->at(objects::detail::BranchInfo::kAdvertisingMessageSize) = 0xFF;
  };

  EXPECT_THROW(fake.Connect(branch_, fn), boost::system::system_error);
  fake.Advertise();
  EXPECT_THROW(fake.Accept(fn), boost::system::system_error);
}

TEST_F(ConnectionManagerTest, BranchEvents) {
  void* branch_a = CreateBranch(context_, "a");
  auto uuid = GetBranchUuid(branch_a);
  auto info = GetBranchInfo(branch_a);

  BranchEventRecorder rec(context_, branch_);
  auto json = rec.RunContextUntil(YOGI_BEV_BRANCH_DISCOVERED, uuid, YOGI_OK);
  CheckJsonElementsAreEqual(json, info, "uuid");
  EXPECT_FALSE(json.value("tcp_server_address", "").empty());
  CheckJsonElementsAreEqual(json, info, "tcp_server_port");

  json = rec.RunContextUntil(YOGI_BEV_BRANCH_QUERIED, uuid, YOGI_OK);
  CheckJsonElementsAreEqual(json, info, "uuid");
  CheckJsonElementsAreEqual(json, info, "name");
  CheckJsonElementsAreEqual(json, info, "description");
  CheckJsonElementsAreEqual(json, info, "network_name");
  CheckJsonElementsAreEqual(json, info, "path");
  CheckJsonElementsAreEqual(json, info, "hostname");
  CheckJsonElementsAreEqual(json, info, "pid");
  EXPECT_FALSE(json.value("tcp_server_address", "").empty());
  CheckJsonElementsAreEqual(json, info, "tcp_server_port");
  CheckJsonElementsAreEqual(json, info, "start_time");
  CheckJsonElementsAreEqual(json, info, "timeout");
  CheckJsonElementsAreEqual(json, info, "advertising_interval");
  CheckJsonElementsAreEqual(json, info, "ghost_mode");

  json = rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, uuid, YOGI_OK);
  CheckJsonElementsAreEqual(json, info, "uuid");

  YOGI_Destroy(branch_a);
  json = rec.RunContextUntil(YOGI_BEV_CONNECTION_LOST, uuid,
                             YOGI_ERR_RW_SOCKET_FAILED);
  CheckJsonElementsAreEqual(json, info, "uuid");
}

TEST_F(ConnectionManagerTest, CancelAwaitBranchEvent) {
  int res = YOGI_BranchCancelAwaitEvent(branch_);
  ASSERT_ERR(res, YOGI_ERR_OPERATION_NOT_RUNNING);

  bool called = false;
  res = YOGI_BranchAwaitEventAsync(
      branch_, 0, nullptr, nullptr, 0,
      [](int res, int event, int ev_res, void* userarg) {
        EXPECT_ERR(res, YOGI_ERR_CANCELED);
        EXPECT_EQ(event, YOGI_BEV_NONE);
        EXPECT_EQ(ev_res, YOGI_OK);
        *static_cast<bool*>(userarg) = true;
      },
      &called);
  ASSERT_OK(res);

  res = YOGI_BranchCancelAwaitEvent(branch_);
  ASSERT_OK(res);

  while (!called) {
    res = YOGI_ContextRunOne(context_, nullptr, -1);
    EXPECT_OK(res);
  }
}

TEST_F(ConnectionManagerTest, AwaitBranchEventOnDestruction) {
  bool called = false;
  int res = YOGI_BranchAwaitEventAsync(
      branch_, 0, nullptr, nullptr, 0,
      [](int res, int event, int ev_res, void* userarg) {
        EXPECT_ERR(res, YOGI_ERR_CANCELED);
        EXPECT_EQ(event, YOGI_BEV_NONE);
        EXPECT_EQ(ev_res, YOGI_OK);
        *static_cast<bool*>(userarg) = true;
      },
      &called);
  ASSERT_OK(res);

  YOGI_Destroy(branch_);

  while (!called) {
    res = YOGI_ContextRunOne(context_, nullptr, -1);
    EXPECT_OK(res);
  }
}

TEST_F(ConnectionManagerTest, GetConnectedBranches) {
  void* branch_a = CreateBranch(context_, "a");
  void* branch_b = CreateBranch(context_, "b");

  BranchEventRecorder rec(context_, branch_);
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_a, YOGI_OK);
  rec.RunContextUntil(YOGI_BEV_CONNECT_FINISHED, branch_b, YOGI_OK);

  auto branches = GetConnectedBranches(branch_);
  EXPECT_EQ(branches.size(), 2);

  auto fn = [&](void* branch) {
    auto info = GetBranchInfo(branch);
    auto uuid = GetBranchUuid(branch);
    ASSERT_TRUE(branches.count(uuid));

    auto json = branches[uuid];
    CheckJsonElementsAreEqual(json, info, "uuid");
    CheckJsonElementsAreEqual(json, info, "name");
    CheckJsonElementsAreEqual(json, info, "description");
    CheckJsonElementsAreEqual(json, info, "network_name");
    CheckJsonElementsAreEqual(json, info, "path");
    CheckJsonElementsAreEqual(json, info, "hostname");
    CheckJsonElementsAreEqual(json, info, "pid");
    EXPECT_FALSE(json.value("tcp_server_address", "").empty());
    CheckJsonElementsAreEqual(json, info, "tcp_server_port");
    CheckJsonElementsAreEqual(json, info, "start_time");
    CheckJsonElementsAreEqual(json, info, "timeout");
    CheckJsonElementsAreEqual(json, info, "advertising_interval");
    CheckJsonElementsAreEqual(json, info, "ghost_mode");
  };

  fn(branch_a);
  fn(branch_b);
}

TEST_F(ConnectionManagerTest, GhostMode) {
  auto props = kBranchProps;
  props["name"] = "B";
  props["ghost_mode"] = true;

  void* ghost_branch;
  int res = YOGI_BranchCreate(&ghost_branch, context_, props.dump().c_str(),
                              nullptr, nullptr, 0);
  EXPECT_OK(res);

  BranchEventRecorder rec(context_, ghost_branch);
  rec.RunContextUntil(YOGI_BEV_BRANCH_QUERIED, branch_, YOGI_OK);
  YOGI_ContextRun(context_, nullptr, 10'000'000);

  auto branches = GetConnectedBranches(ghost_branch);
  EXPECT_TRUE(branches.empty());
}
