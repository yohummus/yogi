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

#include <yogi_core.h>

class BranchTest : public testing::Test {
 protected:
  BranchTest()
      : context_(yogi::Context::Create()),
        json_view_(json_data_),
        big_json_data_(MakeBigJsonData()),
        big_json_view_(big_json_data_),
        msgpack_view_(msgpack_data_, sizeof(msgpack_data_)) {}

  static std::vector<char> MakeBigJsonData(std::size_t size = 10000) {
    std::vector<char> data{'[', '"', '"', ']', '\0'};
    data.insert(data.begin() + 2, size - data.size(), '.');
    return data;
  }

  yogi::ContextPtr context_;
  const char json_data_[8] = "[1,2,3]";
  const yogi::JsonView json_view_;
  const std::vector<char> big_json_data_;
  const yogi::JsonView big_json_view_;
  const char msgpack_data_[4] = {-109, 1, 2, 3};
  const yogi::MsgpackView msgpack_view_;
};

TEST_F(BranchTest, CreateWithSubSection) {
  auto branch = yogi::Branch::Create(
      context_, "{\"branch\":{\"name\":\"Cow\"}}", "/branch");
  EXPECT_EQ(branch->GetName(), "Cow");
}

TEST_F(BranchTest, BranchEventsEnum) {
  // clang-format off
  CHECK_ENUM_ELEMENT(BranchEvents, kNone,             YOGI_BEV_NONE);
  CHECK_ENUM_ELEMENT(BranchEvents, kBranchDiscovered, YOGI_BEV_BRANCH_DISCOVERED);
  CHECK_ENUM_ELEMENT(BranchEvents, kBranchQueried,    YOGI_BEV_BRANCH_QUERIED);
  CHECK_ENUM_ELEMENT(BranchEvents, kConnectFinished,  YOGI_BEV_CONNECT_FINISHED);
  CHECK_ENUM_ELEMENT(BranchEvents, kConnectionLost,   YOGI_BEV_CONNECTION_LOST);
  CHECK_ENUM_ELEMENT(BranchEvents, kAll,              YOGI_BEV_ALL);
  // clang-format on

  auto events = yogi::BranchEvents::kNone;
  EXPECT_EQ(yogi::ToString(events), "kNone");
  events = events | yogi::BranchEvents::kBranchDiscovered;
  EXPECT_EQ(yogi::ToString(events), "kBranchDiscovered");
  events |= yogi::BranchEvents::kConnectFinished;
  EXPECT_EQ(yogi::ToString(events), "kBranchDiscovered | kConnectFinished");
  events |= yogi::BranchEvents::kAll;
  EXPECT_EQ(yogi::ToString(events), "kAll");
}

TEST_F(BranchTest, Info) {
  auto branch = yogi::Branch::Create(context_, R"raw({
    "name": "My Branch",
    "description": "Stuff",
    "network_name": "My Network",
    "network_password": "Password",
    "path": "/some/path",
    "advertising_address": "239.255.0.1",
    "advertising_port": 12345,
    "advertising_interval": 7,
    "timeout": -1
    })raw");

  auto info = branch->GetInfo();
  EXPECT_EQ(info.ToJson(), yogi::Json::parse(info.ToString()));

  EXPECT_NE(info.GetUuid(), yogi::Uuid{});
  EXPECT_EQ(info.GetName(), "My Branch");
  EXPECT_EQ(info.GetDescription(), "Stuff");
  EXPECT_EQ(info.GetNetworkName(), "My Network");
  EXPECT_EQ(info.GetPath(), "/some/path");
  EXPECT_FALSE(info.GetHostname().empty());
  EXPECT_GT(info.GetPid(), 0);
  EXPECT_EQ(info.GetAdvertisingAddress(), "239.255.0.1");
  EXPECT_EQ(info.GetAdvertisingPort(), 12345);
  EXPECT_EQ(info.GetAdvertisingInterval(), yogi::Duration::FromSeconds(7));
  EXPECT_GT(info.GetTcpServerPort(), 0);
  EXPECT_LT(info.GetStartTime(), yogi::GetCurrentTime());
  EXPECT_EQ(info.GetTimeout(), yogi::Duration::kInfinity);
  EXPECT_FALSE(info.GetGhostMode());
  EXPECT_GT(info.GetTxQueueSize(), 1000);
  EXPECT_GT(info.GetRxQueueSize(), 1000);

  EXPECT_EQ(branch->GetUuid(), info.GetUuid());
  EXPECT_EQ(branch->GetName(), info.GetName());
  EXPECT_EQ(branch->GetDescription(), info.GetDescription());
  EXPECT_EQ(branch->GetNetworkName(), info.GetNetworkName());
  EXPECT_EQ(branch->GetPath(), info.GetPath());
  EXPECT_EQ(branch->GetHostname(), info.GetHostname());
  EXPECT_EQ(branch->GetPid(), info.GetPid());
  EXPECT_EQ(branch->GetAdvertisingAddress(), info.GetAdvertisingAddress());
  EXPECT_EQ(branch->GetAdvertisingPort(), info.GetAdvertisingPort());
  EXPECT_EQ(branch->GetAdvertisingInterval(), info.GetAdvertisingInterval());
  EXPECT_EQ(branch->GetTcpServerPort(), info.GetTcpServerPort());
  EXPECT_EQ(branch->GetStartTime(), info.GetStartTime());
  EXPECT_EQ(branch->GetTimeout(), info.GetTimeout());
  EXPECT_EQ(branch->GetGhostMode(), info.GetGhostMode());
  EXPECT_EQ(branch->GetTxQueueSize(), info.GetTxQueueSize());
  EXPECT_EQ(branch->GetRxQueueSize(), info.GetRxQueueSize());
}

TEST_F(BranchTest, GetConnectedBranches) {
  auto branch = yogi::Branch::Create(context_, "{\"name\":\"My Branch\"}");
  auto branch_a = yogi::Branch::Create(context_, "{\"name\":\"A\"}");
  auto branch_b = yogi::Branch::Create(context_, "{\"name\":\"B\"}");

  while (!branch->GetConnectedBranches().count(branch_a->GetUuid()) ||
         !branch->GetConnectedBranches().count(branch_b->GetUuid())) {
    context_->RunOne();
  }

  auto branches = branch->GetConnectedBranches();

  ASSERT_EQ(branches.count(branch_a->GetUuid()), 1u);
  EXPECT_EQ(branches.at(branch_a->GetUuid()).GetName(), branch_a->GetName());
  EXPECT_FALSE(branches.at(branch_a->GetUuid()).GetTcpServerAddress().empty());

  ASSERT_EQ(branches.count(branch_b->GetUuid()), 1u);
  EXPECT_EQ(branches.at(branch_b->GetUuid()).GetName(), branch_b->GetName());
  EXPECT_FALSE(branches.at(branch_b->GetUuid()).GetTcpServerAddress().empty());
}

TEST_F(BranchTest, AwaitEventAsync) {
  auto branch = yogi::Branch::Create(context_, "{\"name\":\"My Branch\"}");
  auto branch_a = yogi::Branch::Create(context_, "{\"name\":\"A\"}");

  auto events =
      yogi::BranchEvents::kBranchQueried | yogi::BranchEvents::kConnectionLost;
  bool called = false;
  branch->AwaitEventAsync(events, [&](auto& res, auto event, auto& evres,
                                      auto& info) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kOk);
    EXPECT_EQ(event, yogi::BranchEvents::kBranchQueried);
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(evres));
    EXPECT_EQ(evres.GetErrorCode(), yogi::ErrorCode::kOk);
    EXPECT_NO_THROW(dynamic_cast<const yogi::BranchQueriedEventInfo&>(info));
    EXPECT_EQ(
        static_cast<const yogi::BranchQueriedEventInfo&>(info).GetStartTime(),
        branch_a->GetStartTime());
    EXPECT_EQ(
        static_cast<const yogi::BranchQueriedEventInfo&>(info).GetTimeout(),
        branch_a->GetTimeout());
    EXPECT_EQ(info.ToJson(), yogi::Json::parse(info.ToString()));
    called = true;
  });

  while (!called) context_->RunOne();

  EXPECT_TRUE(called);
}

TEST_F(BranchTest, CancelAwaitEvent) {
  auto branch = yogi::Branch::Create(context_, "{\"name\":\"My Branch\"}");

  EXPECT_FALSE(branch->CancelAwaitEvent());

  bool called = false;
  branch->AwaitEventAsync(
      yogi::BranchEvents::kAll, [&](auto& res, auto event, auto& evres, auto&) {
        EXPECT_NO_THROW(dynamic_cast<const yogi::Failure&>(res));
        EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kCanceled);
        EXPECT_EQ(event, yogi::BranchEvents::kNone);
        EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(evres));
        EXPECT_EQ(evres.GetErrorCode(), yogi::ErrorCode::kOk);
        called = true;
      });

  EXPECT_TRUE(branch->CancelAwaitEvent());
  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(BranchTest, SendBroadcast) {
  auto branch_a = yogi::Branch::Create(
      context_, "{\"name\":\"a\", \"_transceive_byte_limit\": 5}");
  auto branch_b = yogi::Branch::Create(context_, "{\"name\":\"b\"}");
  RunContextUntilBranchesAreConnected(context_, {branch_a, branch_b});
  context_->RunInBackground();

  // Receive a broadcast to verify that it has actually been sent
  bool broadcast_received = false;
  branch_b->ReceiveBroadcastAsync(yogi::EncodingType::kJson,
                                  [&](auto& res, auto&, auto& payload) {
                                    EXPECT_EQ(res, yogi::Success());
                                    EXPECT_EQ(payload, big_json_view_);
                                    broadcast_received = true;
                                  });

  // Blocking
  for (int i = 0; i < 3; ++i) {
    EXPECT_TRUE(branch_a->SendBroadcast(big_json_view_, true));
  }

  // Non-blocking
  while (branch_a->SendBroadcast(big_json_view_, false))
    ;

  context_->Stop();
  context_->WaitForStopped();

  // Verify that a broadcast has actually been sent
  while (!broadcast_received) context_->RunOne();
}

TEST_F(BranchTest, SendBroadcastAsync) {
  auto branch_a = yogi::Branch::Create(
      context_, "{\"name\":\"a\", \"_transceive_byte_limit\": 5}");
  auto branch_b = yogi::Branch::Create(context_, "{\"name\":\"b\"}");
  RunContextUntilBranchesAreConnected(context_, {branch_a, branch_b});

  // Receive a broadcast to verify that it has actually been sent
  bool broadcast_received = false;
  branch_b->ReceiveBroadcastAsync(yogi::EncodingType::kJson,
                                  [&](auto& res, auto&, auto& payload) {
                                    EXPECT_EQ(res, yogi::Success());
                                    EXPECT_EQ(payload, big_json_view_);
                                    broadcast_received = true;
                                  });

  // Send with retry = true
  const int n = 3;
  std::vector<yogi::Result> results;
  for (int i = 0; i < n; ++i) {
    auto oid = branch_a->SendBroadcastAsync(big_json_view_, true,
                                            [&](auto& res, auto oid) {
                                              EXPECT_EQ(res, yogi::Success());
                                              EXPECT_TRUE(oid.IsValid());
                                              results.push_back(res);
                                            });
    EXPECT_TRUE(oid.IsValid());
  }

  while (results.size() != n) context_->Poll();

  // Send with retry = false
  do {
    branch_a->SendBroadcastAsync(big_json_view_, false, [&](auto& res, auto) {
      results.push_back(res);
    });

    context_->PollOne();
  } while (results.back() == yogi::Success());

  EXPECT_EQ(results.back(), yogi::Failure(yogi::ErrorCode::kTxQueueFull));

  // Verify that a broadcast has actually been sent
  while (!broadcast_received) context_->RunOne();
}

TEST_F(BranchTest, CancelSendBroadcast) {
  auto branch_a = yogi::Branch::Create(
      context_, "{\"name\":\"a\", \"_transceive_byte_limit\": 5}");
  auto branch_b = yogi::Branch::Create(context_, "{\"name\":\"b\"}");
  RunContextUntilBranchesAreConnected(context_, {branch_a, branch_b});

  std::map<yogi::OperationId, yogi::ErrorCode> oid_to_ec;
  while (true) {
    auto oid = branch_a->SendBroadcastAsync(
        big_json_view_,
        [&](auto& res, auto oid) { oid_to_ec[oid] = res.GetErrorCode(); });
    EXPECT_TRUE(oid.IsValid());

    oid_to_ec[oid] = yogi::ErrorCode::kUnknown;

    if (branch_a->CancelSendBroadcast(oid)) {
      context_->Poll();
      EXPECT_EQ(oid_to_ec[oid], yogi::ErrorCode::kCanceled);
      break;
    }
  }
}

TEST_F(BranchTest, ReceiveBroadcast) {
  auto branch_a = yogi::Branch::Create(context_, "{\"name\":\"a\"}");
  auto branch_b = yogi::Branch::Create(context_, "{\"name\":\"b\"}");
  RunContextUntilBranchesAreConnected(context_, {branch_a, branch_b});

  // Simplest form
  auto uuid_b = branch_b->GetUuid();
  bool called = false;
  branch_a->ReceiveBroadcastAsync([&](auto& res, auto& source, auto& payload) {
    EXPECT_EQ(res, yogi::Success());
    EXPECT_EQ(source, uuid_b);
    EXPECT_EQ(payload, msgpack_view_);
    called = true;
  });

  branch_b->SendBroadcastAsync(msgpack_view_, {});
  while (!called) context_->RunOne();

  // With buffer in handler function
  called = false;
  branch_a->ReceiveBroadcastAsync(
      [&](auto& res, auto&, auto& payload, auto&& buffer) {
        EXPECT_EQ(res, yogi::Success());
        EXPECT_EQ(payload, msgpack_view_);
        EXPECT_GE(buffer->size(), static_cast<std::size_t>(
                                      yogi::constants::kMaxMessagePayloadSize));
        called = true;
      });

  branch_b->SendBroadcastAsync(msgpack_view_, {});
  while (!called) context_->RunOne();

  // With encoding
  called = false;
  branch_a->ReceiveBroadcastAsync(yogi::EncodingType::kJson,
                                  [&](auto& res, auto&, auto& payload) {
                                    EXPECT_EQ(res, yogi::Success());
                                    EXPECT_EQ(payload, json_view_);
                                    called = true;
                                  });

  branch_b->SendBroadcastAsync(msgpack_view_, {});
  while (!called) context_->RunOne();

  // With encoding and with buffer in handler function
  called = false;
  branch_a->ReceiveBroadcastAsync(
      yogi::EncodingType::kJson,
      [&](auto& res, auto&, auto& payload, auto&& buffer) {
        EXPECT_EQ(res, yogi::Success());
        EXPECT_EQ(payload, json_view_);
        EXPECT_GE(buffer->size(), static_cast<std::size_t>(
                                      yogi::constants::kMaxMessagePayloadSize));
        called = true;
      });

  branch_b->SendBroadcastAsync(msgpack_view_, {});
  while (!called) context_->RunOne();

  // With custom buffer
  auto buffer = std::make_unique<yogi::Buffer>(123);
  called = false;
  branch_a->ReceiveBroadcastAsync(std::move(buffer),
                                  [&](auto& res, auto&, auto& payload) {
                                    EXPECT_EQ(res, yogi::Success());
                                    EXPECT_EQ(payload, msgpack_view_);
                                    called = true;
                                  });

  branch_b->SendBroadcastAsync(msgpack_view_, {});
  while (!called) context_->RunOne();

  // With custom buffer and with buffer in handler function
  buffer = std::make_unique<yogi::Buffer>(123);
  called = false;
  branch_a->ReceiveBroadcastAsync(
      std::move(buffer), [&](auto& res, auto&, auto& payload, auto&& buffer) {
        EXPECT_EQ(res, yogi::Success());
        EXPECT_EQ(payload, msgpack_view_);
        EXPECT_EQ(buffer->size(), 123u);
        called = true;
      });

  branch_b->SendBroadcastAsync(msgpack_view_, {});
  while (!called) context_->RunOne();

  // With custom buffer and encoding
  buffer = std::make_unique<yogi::Buffer>(123);
  called = false;
  branch_a->ReceiveBroadcastAsync(yogi::EncodingType::kJson, std::move(buffer),
                                  [&](auto& res, auto&, auto& payload) {
                                    EXPECT_EQ(res, yogi::Success());
                                    EXPECT_EQ(payload, json_view_);
                                    called = true;
                                  });

  branch_b->SendBroadcastAsync(msgpack_view_, {});
  while (!called) context_->RunOne();

  // With custom buffer and encoding and buffer in handler function
  buffer = std::make_unique<yogi::Buffer>(123);
  called = false;
  branch_a->ReceiveBroadcastAsync(
      yogi::EncodingType::kJson, std::move(buffer),
      [&](auto& res, auto&, auto& payload, auto&& buffer) {
        EXPECT_EQ(res, yogi::Success());
        EXPECT_EQ(payload, json_view_);
        EXPECT_GE(buffer->size(), 123u);
        called = true;
      });

  branch_b->SendBroadcastAsync(msgpack_view_, {});
  while (!called) context_->RunOne();
}

TEST_F(BranchTest, CancelReceiveBroadcast) {
  auto branch_a = yogi::Branch::Create(context_, "{\"name\":\"a\"}");

  EXPECT_FALSE(branch_a->CancelReceiveBroadcast());

  bool called = false;
  branch_a->ReceiveBroadcastAsync([&](auto& res, auto&, auto&, auto&&) {
    EXPECT_EQ(res, yogi::Failure(yogi::ErrorCode::kCanceled));
    called = true;
  });

  EXPECT_TRUE(branch_a->CancelReceiveBroadcast());
  context_->Poll();
  EXPECT_TRUE(called);
}
