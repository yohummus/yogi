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
#include "../../src/network/tcp_transport.h"

using namespace std::chrono_literals;

class TcpTransportTest : public TestFixture {
 protected:
  virtual void SetUp() override {
    acceptor_.open(kTcpProtocol);
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(boost::asio::ip::tcp::endpoint(kTcpProtocol, 0));
    acceptor_.listen();

    acceptor_ep_ = boost::asio::ip::tcp::endpoint(
        kLoopbackAddress, acceptor_.local_endpoint().port());
  }

  network::TcpTransportPtr Connect() {
    network::TcpTransportPtr transport;
    auto guard = network::TcpTransport::ConnectAsync(
        context_, acceptor_ep_, 10s, std::numeric_limits<std::size_t>::max(),
        [&](auto& res, auto tp, auto) {
          ASSERT_EQ(res, api::kSuccess);
          transport = tp;
        });

    bool accepted = false;
    acceptor_.async_accept(socket_, [&](auto& ec) {
      EXPECT_TRUE(!ec) << ec.message();
      accepted = true;
    });

    while (!transport || !accepted) {
      context_->RunOne(100us);
    }

    return transport;
  };

  objects::ContextPtr context_ = objects::Context::Create();
  boost::asio::ip::tcp::acceptor acceptor_{context_->IoContext()};
  boost::asio::ip::tcp::endpoint acceptor_ep_;
  boost::asio::ip::tcp::socket socket_{context_->IoContext()};
  std::vector<char> data_ = {1, 2, 3, 4, 5, 6};
};

TEST_F(TcpTransportTest, Accept) {
  bool called = false;
  auto guard = network::TcpTransport::AcceptAsync(
      context_, &acceptor_, 10s, std::numeric_limits<std::size_t>::max(),
      [&](auto& res, auto transport, auto) {
        EXPECT_EQ(res, api::kSuccess);
        ASSERT_TRUE(!!transport);
        EXPECT_TRUE(transport->CreatedFromIncomingConnectionRequest());
        called = true;
      });

  socket_.async_connect(acceptor_ep_,
                        [](auto& ec) { EXPECT_TRUE(!ec) << ec.message(); });

  while (!called) {
    context_->RunOne(100us);
  }
}

TEST_F(TcpTransportTest, CancelAccept) {
  bool called = false;
  auto guard = network::TcpTransport::AcceptAsync(
      context_, &acceptor_, 10s, std::numeric_limits<std::size_t>::max(),
      [&](auto& res, auto transport, auto guard) {
        EXPECT_EQ(res, api::Error(YOGI_ERR_CANCELED));
        ASSERT_FALSE(!!transport);
        ASSERT_FALSE(!!guard);
        called = true;
      });

  guard.reset();

  while (!called) {
    context_->RunOne(100us);
  }
}

TEST_F(TcpTransportTest, Connect) {
  auto transport = Connect();
  EXPECT_FALSE(transport->CreatedFromIncomingConnectionRequest());
}

TEST_F(TcpTransportTest, CancelConnect) {
  acceptor_.close();

  bool called = false;
  auto guard = network::TcpTransport::ConnectAsync(
      context_, acceptor_ep_, 10s, std::numeric_limits<std::size_t>::max(),
      [&](auto& res, auto transport, auto guard) {
        EXPECT_EQ(res, api::Error(YOGI_ERR_CANCELED));
        ASSERT_FALSE(!!transport);
        ASSERT_FALSE(!!guard);
        called = true;
      });

  guard.reset();

  while (!called) {
    context_->RunOne(100us);
  }
}

TEST_F(TcpTransportTest, ConnectTimeout) {
  acceptor_.close();

  auto start_time = std::chrono::steady_clock::now();

  bool called = false;
  auto guard = network::TcpTransport::ConnectAsync(
      context_, acceptor_ep_, 1ms, std::numeric_limits<std::size_t>::max(),
      [&](auto& res, auto transport, auto) {
        EXPECT_TRUE(res == api::Error(YOGI_ERR_TIMEOUT) ||
                    res == api::Error(YOGI_ERR_CONNECT_SOCKET_FAILED));
        EXPECT_FALSE(!!transport);
        called = true;
      });

  while (!called) {
    context_->RunOne(100us);
  }

  EXPECT_LT(std::chrono::steady_clock::now(), start_time + 1ms + 1s);
}

TEST_F(TcpTransportTest, PeerDescription) {
  auto transport = Connect();
  EXPECT_GT(transport->GetPeerDescription().size(), 3);
  EXPECT_NE(transport->GetPeerDescription().find(':'), std::string::npos);
}

TEST_F(TcpTransportTest, Send) {
  auto transport = Connect();

  bool received = false;
  std::vector<char> buffer(data_.size());
  boost::asio::async_read(socket_, boost::asio::buffer(buffer),
                          [&](auto& ec, auto bytes_read) {
                            EXPECT_TRUE(!ec) << ec.message();
                            EXPECT_EQ(bytes_read, buffer.size());
                            received = true;
                          });

  bool sent = false;
  transport->SendAllAsync(boost::asio::buffer(data_), [&](auto& res) {
    EXPECT_EQ(res, api::kSuccess);
    sent = true;
  });

  while (!sent || !received) {
    context_->RunOne(100us);
  }

  EXPECT_EQ(data_, buffer);
}

TEST_F(TcpTransportTest, SendFailure) {
  auto transport = Connect();
  transport->Close();

  bool called = false;
  transport->SendSomeAsync(boost::asio::buffer(data_), [&](auto& res, auto) {
    EXPECT_TRUE(res.IsError());
    called = true;
  });

  while (!called) {
    context_->RunOne(100us);
  }
}

TEST_F(TcpTransportTest, Receive) {
  auto transport = Connect();

  bool received = false;
  std::vector<char> buffer(data_.size());
  transport->ReceiveAllAsync(boost::asio::buffer(buffer), [&](auto& res) {
    EXPECT_EQ(res, api::kSuccess);
    received = true;
  });

  bool sent = false;
  boost::asio::async_write(socket_, boost::asio::buffer(data_),
                           [&](auto& ec, auto bytes_written) {
                             EXPECT_TRUE(!ec) << ec.message();
                             EXPECT_EQ(bytes_written, this->data_.size());
                             sent = true;
                           });

  while (!sent || !received) {
    context_->RunOne(100us);
  }

  EXPECT_EQ(data_, buffer);
}

TEST_F(TcpTransportTest, ReceiveFailure) {
  auto transport = Connect();

  bool called = false;
  transport->ReceiveSomeAsync(boost::asio::buffer(data_), [&](auto& res, auto) {
    EXPECT_TRUE(res.IsError());
    called = true;
  });

  transport->Close();

  while (!called) {
    context_->RunOne(100us);
  }
}
