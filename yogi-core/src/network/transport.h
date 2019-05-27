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

#include "../config.h"
#include "../objects/context.h"
#include "../objects/logger.h"
#include "../utils/types.h"

#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <sstream>
#include <chrono>
#include <string>

namespace network {

class Transport;
typedef std::shared_ptr<Transport> TransportPtr;
typedef std::weak_ptr<Transport> TransportWeakPtr;

class Transport : public std::enable_shared_from_this<Transport>,
                  public objects::LoggerUser {
 public:
  typedef std::function<void(const api::Result&,
                             const std::size_t bytes_transferred)>
      TransferSomeHandler;
  typedef std::function<void(const api::Result&)> TransferAllHandler;

  Transport(objects::ContextPtr context, std::chrono::nanoseconds timeout,
            bool created_from_incoming_conn_req, std::string peer_description,
            std::size_t transceive_byte_limit);
  virtual ~Transport();

  objects::ContextPtr GetContext() const { return context_; }
  const std::string& GetPeerDescription() const { return peer_description_; }

  bool CreatedFromIncomingConnectionRequest() const {
    return created_from_incoming_conn_req_;
  };

  void SendSomeAsync(boost::asio::const_buffer data,
                     TransferSomeHandler handler);
  void SendAllAsync(boost::asio::const_buffer data, TransferAllHandler handler);
  void SendAllAsync(utils::SharedByteVector data, TransferAllHandler handler);
  void SendAllAsync(utils::SharedSmallByteVector data,
                    TransferAllHandler handler);
  void ReceiveSomeAsync(boost::asio::mutable_buffer data,
                        TransferSomeHandler handler);
  void ReceiveAllAsync(boost::asio::mutable_buffer data,
                       TransferAllHandler handler);
  void ReceiveAllAsync(utils::SharedByteVector data,
                       TransferAllHandler handler);
  void Close();

 protected:
  virtual void WriteSomeAsync(boost::asio::const_buffer data,
                              TransferSomeHandler handler) = 0;
  virtual void ReadSomeAsync(boost::asio::mutable_buffer data,
                             TransferSomeHandler handler) = 0;
  virtual void Shutdown() = 0;

 private:
  TransportWeakPtr MakeWeakPtr() { return shared_from_this(); }
  void SendAllAsyncImpl(boost::asio::const_buffer data, const api::Result& res,
                        std::size_t bytes_written, TransferAllHandler handler);
  void ReceiveAllAsyncImpl(boost::asio::mutable_buffer data,
                           const api::Result& res, std::size_t bytes_read,
                           TransferAllHandler handler);
  void StartTimeout(boost::asio::steady_timer* timer,
                    TransportWeakPtr weak_self);
  void OnTimeout();

  const objects::ContextPtr context_;
  const std::chrono::nanoseconds timeout_;
  const bool created_from_incoming_conn_req_;
  const std::string peer_description_;
  const std::size_t transceive_byte_limit_;
  boost::asio::steady_timer tx_timer_;
  boost::asio::steady_timer rx_timer_;
  bool timed_out_;
  YOGI_DEBUG_ONLY(bool close_called_ = false;)
};

}  // namespace network

std::ostream& operator<<(std::ostream& os, const network::Transport& transport);
