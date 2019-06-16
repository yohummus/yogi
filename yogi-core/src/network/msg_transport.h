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
#include "../utils/ringbuffer.h"
#include "../objects/log/logger.h"
#include "transport.h"
#include "messages.h"

#include <boost/asio/buffer.hpp>
#include <functional>
#include <memory>
#include <array>
#include <vector>

namespace network {
namespace internal {

std::size_t SerializeMsgSizeField(std::size_t msg_size,
                                  std::array<utils::Byte, 5>* buffer);
bool DeserializeMsgSizeField(const std::array<utils::Byte, 5>& buffer,
                             std::size_t size, std::size_t* msg_size);

}  // namespace internal

class MessageTransport;
typedef std::shared_ptr<MessageTransport> MessageTransportPtr;
typedef std::weak_ptr<MessageTransport> MessageTransportWeakPtr;

class MessageTransport : public std::enable_shared_from_this<MessageTransport>,
                         public objects::log::LoggerUser {
 public:
  typedef int OperationTag;
  typedef std::function<void(const api::Result&)> SendHandler;
  typedef std::function<void(const api::Result&, std::size_t msg_size)>
      ReceiveHandler;
  typedef ReceiveHandler SizeFieldReceiveHandler;

  MessageTransport(TransportPtr transport, std::size_t tx_queue_size,
                   std::size_t rx_queue_size);

  objects::ContextPtr GetContext() const { return context_; }

  void Start();

  bool TrySend(const OutgoingMessage& msg);
  void SendAsync(OutgoingMessage* msg, OperationTag tag, SendHandler handler);
  void SendAsync(OutgoingMessage* msg, SendHandler handler);
  bool CancelSend(OperationTag tag);
  void ReceiveAsync(boost::asio::mutable_buffer msg, ReceiveHandler handler);

  void CancelReceive();
  void Close() { transport_->Close(); }

 private:
  typedef std::array<utils::Byte, 5> SizeFieldBuffer;

  struct PendingSend {
    OperationTag tag;  // 0 => operation cannot be canceled
    utils::SharedSmallByteVector msg_bytes;
    SendHandler handler;
  };

  MessageTransportWeakPtr MakeWeakPtr() { return shared_from_this(); }
  bool TrySendImpl(const utils::SmallByteVector& msg_bytes);
  bool CanSend(std::size_t msg_size) const;
  void SendAsyncImpl(OutgoingMessage* msg, OperationTag tag,
                     SendHandler handler);
  void SendSomeBytesToTransport();
  void RetrySendingPendingSends();
  bool TryGetReceivedSizeField(std::size_t* msg_size);
  void ResetReceivedSizeField();
  void ReceiveSomeBytesFromTransport();
  void TryDeliveringPendingReceive();
  void HandleSendError(const api::Error& err);
  void HandleReceiveError(const api::Error& err);
  void CheckOperationTagIsNotUsed(OperationTag tag);

  const objects::ContextPtr context_;
  const TransportPtr transport_;
  utils::LockFreeRingBuffer tx_rb_;
  utils::LockFreeRingBuffer rx_rb_;
  std::mutex tx_mutex_;
  api::Result last_tx_error_;
  bool send_to_transport_running_;
  std::vector<PendingSend> pending_sends_;
  SizeFieldBuffer size_field_buffer_;
  std::size_t size_field_buffer_size_;
  std::size_t size_field_;
  bool size_field_valid_;
  boost::asio::mutable_buffer pending_receive_buffer_;
  ReceiveHandler pending_receive_handler_;
  bool receive_from_transport_running_;
  api::Result last_rx_error_;
};

}  // namespace network
