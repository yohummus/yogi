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
#include "../../src/network/msg_transport.h"
using namespace network;

#include <random>
#include <atomic>
#include <algorithm>

class FakeOutgoingMessage : public OutgoingMessage,
                            public MessageT<MessageType::kBroadcast> {
 public:
  using MessageT::MakeMsgBytes;

  FakeOutgoingMessage(utils::SmallByteVector serialized_msg)
      : OutgoingMessage(serialized_msg) {}

  virtual std::string ToString() const override { return {}; }
};

class FakeTransport : public Transport {
 public:
  FakeTransport(objects::ContextPtr context)
      : Transport(context, std::chrono::nanoseconds::max(), true, "",
                  std::numeric_limits<std::size_t>::max()) {}

  utils::ByteVector tx_data;
  utils::ByteVector rx_data;
  std::size_t tx_send_limit = std::numeric_limits<std::size_t>::max();
  bool dead = false;

 protected:
  virtual void WriteSomeAsync(boost::asio::const_buffer data,
                              TransferSomeHandler handler) override {
    static std::default_random_engine gen;
    std::uniform_int_distribution<std::size_t> dist(1, data.size());
    auto n = std::min(dist(gen), tx_send_limit);
    auto p = static_cast<const char*>(data.data());

    tx_data.insert(tx_data.end(), p, p + n);

    PostHandler(handler, n);
  }

  virtual void ReadSomeAsync(boost::asio::mutable_buffer data,
                             TransferSomeHandler handler) override {
    if (rx_data.empty()) return;

    static std::default_random_engine gen;
    std::uniform_int_distribution<std::size_t> dist(
        1, std::min(data.size(), rx_data.size()));
    auto n = dist(gen);
    std::copy_n(rx_data.begin(), n, static_cast<char*>(data.data()));
    rx_data.erase(
        rx_data.begin(),
        rx_data.begin() + static_cast<utils::ByteVector::difference_type>(n));

    PostHandler(handler, n);
  }

  virtual void Shutdown() override { dead = true; }

 private:
  void PostHandler(TransferSomeHandler handler, std::size_t bytes_transferred) {
    if (dead) {
      GetContext()->Post([=] {
        handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED), bytes_transferred);
      });
    } else {
      GetContext()->Post([=] { handler(api::kSuccess, bytes_transferred); });
    }
  }
};

class MessageTransportTest : public TestFixture {
 protected:
  virtual void SetUp() override {
    context_ = std::make_shared<objects::Context>();
    transport_ = std::make_shared<FakeTransport>(context_);
    uut_ = std::make_shared<MessageTransport>(transport_, 8, 8);
  }

  static FakeOutgoingMessage MakeMessage(std::size_t msg_size) {
    if (msg_size == 0) {
      return FakeOutgoingMessage({});
    }

    static std::default_random_engine gen;
    std::uniform_int_distribution<int> dist(1, 100);

    utils::SmallByteVector data(msg_size - 1);
    for (auto& byte : data) {
      byte = static_cast<utils::Byte>(dist(gen));
    }
    data.insert(data.begin(), FakeOutgoingMessage::kMessageType);

    auto msg = FakeOutgoingMessage(data);
    msg.SerializeShared();  // To trigger copying the data vector
    return msg;
  }

  template <typename... Parts>
  static utils::ByteVector MakeTransportBytes(Parts... parts) {
    utils::ByteVector v;
    MakeTransportBytesImpl(&v, parts...);
    return v;
  }

  template <typename Next, typename... Rest>
  static void MakeTransportBytesImpl(utils::ByteVector* v, Next next,
                                     Rest&&... rest) {
    AppendToByteVector(v, next);
    MakeTransportBytesImpl(v, rest...);
  }

  template <typename Last>
  static void MakeTransportBytesImpl(utils::ByteVector* v, Last last) {
    AppendToByteVector(v, last);
  }

  static void AppendToByteVector(utils::ByteVector* v, int byte) {
    v->push_back(static_cast<utils::Byte>(byte));
  }

  static void AppendToByteVector(utils::ByteVector* v,
                                 const utils::SmallByteVector& bytes) {
    v->insert(v->end(), bytes.begin(), bytes.end());
  }

  static void AppendToByteVector(utils::ByteVector* v,
                                 const OutgoingMessage& msg) {
    AppendToByteVector(v, msg.Serialize());
  }

  objects::ContextPtr context_;
  std::shared_ptr<FakeTransport> transport_;
  MessageTransportPtr uut_;
};

TEST_F(MessageTransportTest, MsgSizeFieldSerialization) {
  using namespace internal;

  struct Entry {
    std::size_t ser_length;
    std::size_t value;
  };

  // those are all the corner cases
  const Entry entries[] = {
      // clang-format off
      {1, 0ul},
      {1, 127ul},
      {2, 128ul},
      {2, 16383ul},
      {3, 16384ul},
      {3, 2097151ul},
      {4, 2097152ul},
      {4, 268435455ul},
      {5, 268435456ul},
      {5, 4294967295ul},
      // clang-format on
  };

  for (auto entry : entries) {
    std::array<utils::Byte, 5> buffer = {0};

    auto n = SerializeMsgSizeField(entry.value, &buffer);
    EXPECT_EQ(n, entry.ser_length);

    std::size_t des_value;
    for (std::size_t i = 1; i < entry.ser_length; ++i) {
      EXPECT_FALSE(DeserializeMsgSizeField(buffer, i, &des_value));
    }

    EXPECT_TRUE(DeserializeMsgSizeField(buffer, entry.ser_length, &des_value));
    EXPECT_EQ(des_value, entry.value);
  }
}

TEST_F(MessageTransportTest, TrySend) {
  uut_->Start();

  auto msg = MakeMessage(5);
  EXPECT_TRUE(uut_->TrySend(msg));
  EXPECT_FALSE(uut_->TrySend(msg));
  context_->Poll();
  EXPECT_TRUE(uut_->TrySend(msg));
  context_->Poll();

  EXPECT_EQ(transport_->tx_data, MakeTransportBytes(5, msg, 5, msg));
}

TEST_F(MessageTransportTest, TrySendTransportFailure) {
  uut_->Start();

  transport_->Close();
  uut_->TrySend(MakeMessage(5));
  context_->Poll();

  EXPECT_THROW_ERROR(uut_->TrySend(MakeMessage(5)), YOGI_ERR_RW_SOCKET_FAILED);
}

TEST_F(MessageTransportTest, SendAsync) {
  transport_->tx_send_limit = 1;
  uut_->Start();

  auto msg = MakeMessage(6);
  bool called = false;
  uut_->SendAsync(&msg, [&](auto& res) {
    EXPECT_EQ(res, api::kSuccess);
    called = true;
  });
  context_->PollOne();
  context_->PollOne();
  EXPECT_TRUE(called);

  called = false;
  uut_->SendAsync(&msg, [&](auto& res) {
    EXPECT_EQ(res, api::kSuccess);
    called = true;
  });
  context_->PollOne();
  context_->PollOne();
  EXPECT_FALSE(called);
  context_->Poll();
  EXPECT_TRUE(called);

  EXPECT_EQ(transport_->tx_data, MakeTransportBytes(6, msg, 6, msg));
}

TEST_F(MessageTransportTest, AsyncSendTransportFailure) {
  uut_->Start();
  transport_->Close();

  auto msg = MakeMessage(6);
  uut_->TrySend(msg);

  bool called = false;
  uut_->SendAsync(&msg, [&](auto& res) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_RW_SOCKET_FAILED));
    called = true;
  });
  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(MessageTransportTest, CancelSend) {
  uut_->Start();

  auto msg = MakeMessage(6);
  EXPECT_TRUE(uut_->TrySend(msg));

  bool called = false;
  MessageTransport::OperationTag tag = 123;
  uut_->SendAsync(&msg, tag, [&](auto& res) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_CANCELED));
    called = true;
  });
  EXPECT_TRUE(uut_->CancelSend(tag));
  EXPECT_FALSE(uut_->CancelSend(tag));

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(MessageTransportTest, ReceiveAsync) {
  transport_->rx_data = utils::ByteVector{5, 1, 2, 3, 4, 5, 4, 1, 2, 3, 4};
  uut_->Start();
  context_->Poll();

  utils::ByteVector data(5);
  bool called = false;
  uut_->ReceiveAsync(boost::asio::buffer(data), [&](auto& res, auto size) {
    EXPECT_EQ(res, api::kSuccess);
    EXPECT_EQ(size, 5);
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
  EXPECT_EQ(data, (utils::ByteVector{1, 2, 3, 4, 5}));

  data = utils::ByteVector(4);
  called = false;
  uut_->ReceiveAsync(boost::asio::buffer(data), [&](auto& res, auto size) {
    EXPECT_EQ(res, api::kSuccess);
    EXPECT_EQ(size, 4);
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
  EXPECT_EQ(data, (utils::ByteVector{1, 2, 3, 4}));
}

TEST_F(MessageTransportTest, ReceiveAsyncTransportFailure) {
  transport_->rx_data = utils::ByteVector{5, 1, 2, 3, 4, 5, 4, 1, 2, 3, 4};
  transport_->Close();
  uut_->Start();

  utils::ByteVector data(5);
  bool called = false;
  uut_->ReceiveAsync(boost::asio::buffer(data), [&](auto& res, auto) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_RW_SOCKET_FAILED));
    called = true;
  });

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(MessageTransportTest, CancelReceive) {
  uut_->Start();

  utils::ByteVector data(10);
  bool called = false;
  uut_->ReceiveAsync(boost::asio::buffer(data), [&](auto& res, auto) {
    EXPECT_EQ(res, api::Error(YOGI_ERR_CANCELED));
    called = true;
  });
  uut_->CancelReceive();

  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(MessageTransportTest, Close) {
  uut_->Start();
  EXPECT_FALSE(transport_->dead);
  uut_->Close();
  EXPECT_TRUE(transport_->dead);
}

TEST_F(MessageTransportTest, MessageOrderPreservation) {
  transport_->tx_send_limit = 0;  // Make sure buffer is not emptied
  uut_->Start();

  auto msg1 = MakeMessage(4);
  EXPECT_TRUE(uut_->TrySend(msg1));

  // The msg2 and msg3 operations are to check that async writes are in order
  auto msg2 = MakeMessage(3);
  uut_->SendAsync(&msg2, [&](auto& res) { EXPECT_EQ(res, api::kSuccess); });

  auto msg3 = MakeMessage(1);
  uut_->SendAsync(&msg3, [&](auto& res) { EXPECT_EQ(res, api::kSuccess); });

  // This is to check that sync writes do not come before delayed async writes
  auto msg4 = MakeMessage(1);
  EXPECT_FALSE(uut_->TrySend(msg4));

  transport_->tx_send_limit = 100;  // Allow emptying the buffer

  auto msg5 = MakeMessage(1);
  uut_->SendAsync(&msg5, [&](auto& res) { EXPECT_EQ(res, api::kSuccess); });

  context_->Poll();

  EXPECT_EQ(transport_->tx_data,
            MakeTransportBytes(4, msg1, 3, msg2, 1, msg3, 1, msg5));
}

TEST_F(MessageTransportTest, Stress) {
  // Bigger queue size so that the message size field can go beyond one byte
  const std::size_t kQueueSize = 300;
  uut_ = std::make_shared<MessageTransport>(transport_, kQueueSize, kQueueSize);
  uut_->Start();

  // Create randomly sized messages
  const std::size_t kInputSize = 20'000;

  static std::default_random_engine gen;
  std::uniform_int_distribution<std::size_t> dist(1, kQueueSize - 5);

  std::vector<FakeOutgoingMessage> msgs;
  while (transport_->tx_data.size() < kInputSize) {
    auto msg = MakeMessage(dist(gen));
    msgs.push_back(msg);
    EXPECT_TRUE(uut_->TrySend(msg));
    context_->Poll();
  }

  auto old_tx_data = transport_->tx_data;

  // Re-create the transport and load the messages
  transport_ = std::make_shared<FakeTransport>(context_);
  uut_ = std::make_shared<MessageTransport>(transport_, kQueueSize, kQueueSize);
  transport_->rx_data = old_tx_data;
  uut_->Start();
  context_->Poll();

  // Create send and receive threads
  std::mutex mutex;
  std::thread tx_thread;
  std::thread rx_thread;

  std::vector<utils::ByteVector> sent_msgs_bytes;
  std::vector<utils::ByteVector> received_msgs_bytes;
  std::atomic<bool> send_done{false};

  // Start threads simultaneously'ish (that's what the mutex is for)
  {
    std::lock_guard<std::mutex> lock(mutex);

    // Send thread
    tx_thread = std::thread([&] {
      { std::lock_guard<std::mutex> lock(mutex); }

      for (auto& msg : msgs) {
        std::atomic<bool> called(false);
        uut_->SendAsync(&msg, [&](auto& res) {
          EXPECT_EQ(res, api::kSuccess);
          auto bytes = msg.Serialize();
          sent_msgs_bytes.push_back(
              utils::ByteVector(bytes.begin(), bytes.end()));
          called = true;
        });

        while (!called) {
          std::this_thread::yield();
        }
      }

      send_done = true;
    });

    // Receive thread
    rx_thread = std::thread([&] {
      { std::lock_guard<std::mutex> lock(mutex); }

      utils::ByteVector msg_bytes(kQueueSize);

      while (received_msgs_bytes.size() < msgs.size()) {
        std::generate(msg_bytes.begin(), msg_bytes.end(),
                      [] { return static_cast<utils::Byte>(0); });

        std::atomic<bool> called(false);
        uut_->ReceiveAsync(boost::asio::buffer(msg_bytes), [&](auto& res,
                                                               auto msg_size) {
          EXPECT_EQ(res, api::kSuccess);
          received_msgs_bytes.push_back(utils::ByteVector(
              msg_bytes.begin(),
              msg_bytes.begin() +
                  static_cast<utils::ByteVector::difference_type>(msg_size)));
          called = true;
        });

        while (!called) {
          context_->Poll();
        }
      }

      while (!send_done) {
        context_->Poll();
      }
    });
  }

  // Wait for threads to finish
  tx_thread.join();
  rx_thread.join();

  EXPECT_EQ(received_msgs_bytes.size(), sent_msgs_bytes.size());
  EXPECT_EQ(received_msgs_bytes, sent_msgs_bytes);
}
