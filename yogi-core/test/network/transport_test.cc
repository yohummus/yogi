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
#include "../../src/network/transport.h"

#include <gmock/gmock.h>
using namespace testing;
using namespace std::chrono_literals;

MATCHER_P(BufferEq, other, std::string(negation ? "isn't" : "is")) {
  *result_listener << "";
  return arg.data() == other.data() && arg.size() == other.size();
}

class MockTransport : public network::Transport {
 public:
  MockTransport(objects::ContextPtr context, std::chrono::nanoseconds timeout,
                bool created_from_incoming_conn_req)
      : network::Transport(context, timeout, created_from_incoming_conn_req,
                           "Broccoli",
                           std::numeric_limits<std::size_t>::max()) {}

  MOCK_METHOD2(WriteSomeAsync, void(boost::asio::const_buffer data,
                                    TransferSomeHandler handler));
  MOCK_METHOD2(ReadSomeAsync, void(boost::asio::mutable_buffer data,
                                   TransferSomeHandler handler));
  MOCK_METHOD0(Shutdown, void());
};

class TransportTest : public TestFixture {
 protected:
  objects::ContextPtr context_ = objects::Context::Create();
  std::shared_ptr<MockTransport> transport_ =
      std::make_shared<MockTransport>(context_, 10s, false);
  utils::ByteVector data_ = {1, 2, 3, 4, 5, 6};
};

TEST_F(TransportTest, GetContext) {
  EXPECT_EQ(transport_->GetContext(), context_);
}

TEST_F(TransportTest, CreatedFromIncomingConnectionRequest) {
  auto a = std::make_shared<MockTransport>(context_, 1s, true);
  EXPECT_TRUE(a->CreatedFromIncomingConnectionRequest());

  auto b = std::make_shared<MockTransport>(context_, 1s, false);
  EXPECT_FALSE(b->CreatedFromIncomingConnectionRequest());
}

TEST_F(TransportTest, GetPeerDescription) {
  EXPECT_EQ(transport_->GetPeerDescription(), "Broccoli");
}

TEST_F(TransportTest, SendSomeSuccess) {
  // clang-format off
  EXPECT_CALL(*transport_, Shutdown())
    .Times(0);

  EXPECT_CALL(*transport_, WriteSomeAsync(BufferEq(boost::asio::buffer(data_)), _))
      .WillOnce(InvokeArgument<1>(api::kSuccess, 2));
  // clang-format on

  bool called = false;
  transport_->SendSomeAsync(boost::asio::buffer(data_),
                            [&](auto& res, auto bytes_sent) {
                              EXPECT_EQ(res, api::kSuccess);
                              EXPECT_EQ(bytes_sent, 2);
                              called = true;
                            });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, SendSomeFailure) {
  // clang-format off
  EXPECT_CALL(*transport_, Shutdown());

  EXPECT_CALL(*transport_, WriteSomeAsync(_, _))
    .WillOnce(InvokeArgument<1>(api::Error(YOGI_ERR_RW_SOCKET_FAILED), 2));
  // clang-format on

  bool called = false;
  transport_->SendSomeAsync(
      boost::asio::buffer(data_), [&](auto& res, auto bytes_sent) {
        EXPECT_EQ(res, api::Error(YOGI_ERR_RW_SOCKET_FAILED));
        EXPECT_EQ(bytes_sent, 2);
        called = true;
      });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, SendSomeTimeout) {
  // clang-format off
  transport_ = std::make_shared<MockTransport>(context_, 1ms, false);

  network::Transport::TransferSomeHandler handler;
  EXPECT_CALL(*transport_, WriteSomeAsync(_, _))
    .WillOnce(SaveArg<1>(&handler));

  EXPECT_CALL(*transport_, Shutdown())
    .WillOnce(Invoke([&] { handler(api::Error(YOGI_ERR_CANCELED), 2); }));
  // clang-format on

  auto start_time = std::chrono::steady_clock::now();

  bool called = false;
  transport_->SendSomeAsync(boost::asio::buffer(data_),
                            [&](auto& res, auto bytes_sent) {
                              EXPECT_EQ(res, api::Error(YOGI_ERR_TIMEOUT));
                              EXPECT_EQ(bytes_sent, 2);
                              called = true;
                            });

  while (!called) {
    context_->RunOne(100us);
  }

  EXPECT_GT(std::chrono::steady_clock::now(), start_time + 1ms);
  EXPECT_LT(std::chrono::steady_clock::now(), start_time + 1ms + kTimingMargin);
}

TEST_F(TransportTest, SendAllSuccess) {
  // clang-format off
  EXPECT_CALL(*transport_, Shutdown())
    .Times(0);

  InSequence dummy;
  EXPECT_CALL(*transport_, WriteSomeAsync(BufferEq(boost::asio::buffer(data_)), _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 2));

  EXPECT_CALL(*transport_, WriteSomeAsync(BufferEq(boost::asio::buffer(data_) + 2), _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 1));

  EXPECT_CALL(*transport_, WriteSomeAsync(BufferEq(boost::asio::buffer(data_) + 3), _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 3));
  // clang-format on

  bool called = false;
  transport_->SendAllAsync(boost::asio::buffer(data_), [&](auto& res) {
    EXPECT_EQ(res, api::kSuccess);
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, SendAllFailure) {
  // clang-format off
  EXPECT_CALL(*transport_, Shutdown());

  InSequence dummy;
  EXPECT_CALL(*transport_, WriteSomeAsync(BufferEq(boost::asio::buffer(data_)), _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 2));

  EXPECT_CALL(*transport_, WriteSomeAsync(BufferEq(boost::asio::buffer(data_) + 2), _))
    .WillOnce(InvokeArgument<1>(api::Error(YOGI_ERR_RW_SOCKET_FAILED), 1));
  // clang-format on

  bool called = false;
  transport_->SendAllAsync(boost::asio::buffer(data_), [&](auto& res) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_RW_SOCKET_FAILED));
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, SendAllTimeout) {
  // clang-format off
  transport_ = std::make_shared<MockTransport>(context_, 1ms, false);

  network::Transport::TransferSomeHandler handler;
  EXPECT_CALL(*transport_, WriteSomeAsync(_, _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 1))
    .WillOnce(SaveArg<1>(&handler));

  EXPECT_CALL(*transport_, Shutdown())
    .WillOnce(Invoke([&] { handler(api::Error(YOGI_ERR_CANCELED), 2); }));
  // clang-format on

  auto start_time = std::chrono::steady_clock::now();

  bool called = false;
  transport_->SendAllAsync(boost::asio::buffer(data_), [&](auto& res) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_TIMEOUT));
    called = true;
  });

  while (!called) {
    context_->RunOne(100us);
  }

  EXPECT_GT(std::chrono::steady_clock::now(), start_time + 1ms);
  EXPECT_LT(std::chrono::steady_clock::now(), start_time + 1ms + kTimingMargin);
}

TEST_F(TransportTest, SendAllSharedByteVector) {
  // clang-format off
  EXPECT_CALL(*transport_, WriteSomeAsync(_, _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, data_.size()));
  // clang-format on

  bool called = false;
  transport_->SendAllAsync(utils::MakeSharedByteVector(data_), [&](auto& res) {
    EXPECT_EQ(res, api::kSuccess);
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, SendAllSharedSmallByteVector) {
  // clang-format off
  EXPECT_CALL(*transport_, WriteSomeAsync(_, _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, data_.size()));
  // clang-format on

  bool called = false;
  transport_->SendAllAsync(
      utils::MakeSharedSmallByteVector(data_.begin(), data_.end()),
      [&](auto& res) {
        EXPECT_EQ(res, api::kSuccess);
        called = true;
      });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, ReceiveSomeSuccess) {
  // clang-format off
  EXPECT_CALL(*transport_, Shutdown())
    .Times(0);

  EXPECT_CALL(*transport_, ReadSomeAsync(BufferEq(boost::asio::buffer(data_)), _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 2));
  // clang-format on

  bool called = false;
  transport_->ReceiveSomeAsync(boost::asio::buffer(data_),
                               [&](auto& res, auto bytes_received) {
                                 EXPECT_EQ(res, api::kSuccess);
                                 EXPECT_EQ(bytes_received, 2);
                                 called = true;
                               });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, ReceiveSomeFailure) {
  // clang-format off
  EXPECT_CALL(*transport_, Shutdown());

  EXPECT_CALL(*transport_, ReadSomeAsync(_, _))
    .WillOnce(InvokeArgument<1>(api::Error(YOGI_ERR_RW_SOCKET_FAILED), 2));
  // clang-format on

  bool called = false;
  transport_->ReceiveSomeAsync(
      boost::asio::buffer(data_), [&](auto& res, auto bytes_received) {
        EXPECT_EQ(res, api::Error(YOGI_ERR_RW_SOCKET_FAILED));
        EXPECT_EQ(bytes_received, 2);
        called = true;
      });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, ReceiveSomeTimeout) {
  // clang-format off
  transport_ = std::make_shared<MockTransport>(context_, 1ms, false);

  network::Transport::TransferSomeHandler handler;
  EXPECT_CALL(*transport_, ReadSomeAsync(_, _))
    .WillOnce(SaveArg<1>(&handler));

  EXPECT_CALL(*transport_, Shutdown())
    .WillOnce(Invoke([&] { handler(api::Error(YOGI_ERR_CANCELED), 2); }));
  // clang-format on

  auto start_time = std::chrono::steady_clock::now();

  bool called = false;
  transport_->ReceiveSomeAsync(boost::asio::buffer(data_),
                               [&](auto& res, auto bytes_sent) {
                                 EXPECT_EQ(res, api::Error(YOGI_ERR_TIMEOUT));
                                 EXPECT_EQ(bytes_sent, 2);
                                 called = true;
                               });

  while (!called) {
    context_->RunOne(100us);
  }

  EXPECT_GT(std::chrono::steady_clock::now(), start_time + 1ms);
  EXPECT_LT(std::chrono::steady_clock::now(), start_time + 1ms + kTimingMargin);
}

TEST_F(TransportTest, ReceiveAllSuccess) {
  // clang-format off
  EXPECT_CALL(*transport_, Shutdown())
    .Times(0);

  InSequence dummy;
  EXPECT_CALL(*transport_, ReadSomeAsync(BufferEq(boost::asio::buffer(data_)), _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 2));

  EXPECT_CALL(*transport_, ReadSomeAsync(BufferEq(boost::asio::buffer(data_) + 2), _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 1));

  EXPECT_CALL(*transport_, ReadSomeAsync(BufferEq(boost::asio::buffer(data_) + 3), _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 3));
  // clang-format on

  bool called = false;
  transport_->ReceiveAllAsync(boost::asio::buffer(data_), [&](auto& res) {
    EXPECT_EQ(res, api::kSuccess);
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, ReceiveAllFailure) {
  // clang-format off
  EXPECT_CALL(*transport_, Shutdown());

  InSequence dummy;
  EXPECT_CALL(*transport_, ReadSomeAsync(BufferEq(boost::asio::buffer(data_)), _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 2));

  EXPECT_CALL(*transport_, ReadSomeAsync(BufferEq(boost::asio::buffer(data_) + 2), _))
    .WillOnce(InvokeArgument<1>(api::Error(YOGI_ERR_RW_SOCKET_FAILED), 1));
  // clang-format on

  bool called = false;
  transport_->ReceiveAllAsync(boost::asio::buffer(data_), [&](auto& res) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_RW_SOCKET_FAILED));
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, ReceiveAllTimeout) {
  // clang-format off
  transport_ = std::make_shared<MockTransport>(context_, 1ms, false);

  network::Transport::TransferSomeHandler handler;
  EXPECT_CALL(*transport_, ReadSomeAsync(_, _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, 1))
    .WillOnce(SaveArg<1>(&handler));

  EXPECT_CALL(*transport_, Shutdown())
    .WillOnce(Invoke([&] { handler(api::Error(YOGI_ERR_CANCELED), 2); }));
  // clang-format on

  auto start_time = std::chrono::steady_clock::now();

  bool called = false;
  transport_->ReceiveAllAsync(boost::asio::buffer(data_), [&](auto& res) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_TIMEOUT));
    called = true;
  });

  while (!called) {
    context_->RunOne(100us);
  }

  EXPECT_GT(std::chrono::steady_clock::now(), start_time + 1ms);
  EXPECT_LT(std::chrono::steady_clock::now(), start_time + 1ms + kTimingMargin);
}

TEST_F(TransportTest, ReceiveAllSharedByteVector) {
  // clang-format off
  EXPECT_CALL(*transport_, ReadSomeAsync(_, _))
    .WillOnce(InvokeArgument<1>(api::kSuccess, data_.size()));
  // clang-format on

  bool called = false;
  transport_->ReceiveAllAsync(utils::MakeSharedByteVector(data_),
                              [&](auto& res) {
                                EXPECT_EQ(res, api::kSuccess);
                                called = true;
                              });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, CallingSendHandlerOnDestruction) {
  // clang-format off
  network::Transport::TransferSomeHandler handler;
  EXPECT_CALL(*transport_, WriteSomeAsync(_, _))
    .WillOnce(SaveArg<1>(&handler));
  // clang-format on

  bool called = false;
  transport_->SendSomeAsync(boost::asio::buffer(data_), [&](auto& res, auto) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_CANCELED));
    called = true;
  });

  transport_.reset();
  EXPECT_FALSE(called);

  handler(api::Error(YOGI_ERR_CANCELED), 0);
  EXPECT_TRUE(called);
}

TEST_F(TransportTest, CallingReceiveHandlerOnDestruction) {
  // clang-format off
  network::Transport::TransferSomeHandler handler;
  EXPECT_CALL(*transport_, ReadSomeAsync(_, _))
    .WillOnce(SaveArg<1>(&handler));
  // clang-format on

  bool called = false;
  transport_->ReceiveSomeAsync(boost::asio::buffer(data_),
                               [&](auto& res, auto) {
                                 EXPECT_EQ(res, api::Error(YOGI_ERR_CANCELED));
                                 called = true;
                               });

  transport_.reset();
  EXPECT_FALSE(called);

  handler(api::Error(YOGI_ERR_CANCELED), 0);
  EXPECT_TRUE(called);
}
