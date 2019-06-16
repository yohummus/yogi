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
#include "../api/errors.h"
#include "../api/enums.h"
#include "../utils/types.h"

#include <msgpack.hpp>
#include <boost/asio/buffer.hpp>
#include <fstream>
#include <array>

namespace network {
namespace internal {

class MsgPackBufferStream {
 public:
  MsgPackBufferStream(utils::SmallByteVector* buffer) : buffer_(*buffer) {}

  void write(const char* data, std::size_t size) {
    buffer_.insert(buffer_.end(), data, data + size);
  }

 private:
  utils::SmallByteVector& buffer_;
};

}  // namespace internal

enum MessageType : utils::Byte {
  kHeartbeat,
  kAcknowledge,
  kBroadcast,
};

class Message {
 public:
  virtual ~Message() {}

  virtual MessageType GetType() const = 0;
  virtual std::string ToString() const = 0;
};

class IncomingMessage : virtual public Message {
 public:
  typedef std::function<void(const IncomingMessage&)> MessageHandler;

  static void Deserialize(const utils::ByteVector& serialized_msg,
                          const MessageHandler& fn);

 protected:
  IncomingMessage() = default;
};

class Payload {
 public:
  Payload(boost::asio::const_buffer data, api::Encoding enc)
      : data_(data), enc_(enc) {}

  void SerializeTo(utils::SmallByteVector* buffer) const;
  api::Result SerializeToUserBuffer(boost::asio::mutable_buffer buffer,
                                    api::Encoding enc,
                                    std::size_t* bytes_written) const;

 private:
  boost::asio::const_buffer data_;
  api::Encoding enc_;
};

template <MessageType MsgType>
class MessageT : virtual public Message {
 public:
  static constexpr MessageType kMessageType = MsgType;

  virtual MessageType GetType() const override final { return kMessageType; }

 protected:
  static utils::SmallByteVector MakeMsgBytes() {
    return utils::SmallByteVector{kMessageType};
  }

  static utils::SmallByteVector MakeMsgBytes(const Payload& payload) {
    utils::SmallByteVector bytes{kMessageType};
    payload.SerializeTo(&bytes);
    return bytes;
  }

  template <typename... Fields>
  static utils::SmallByteVector MakeMsgBytes(
      const std::tuple<Fields...>& fields) {
    utils::SmallByteVector bytes{kMessageType};
    internal::MsgPackBufferStream stream(&bytes);
    msgpack::pack(stream, fields);
    return bytes;
  }

  template <typename... Fields>
  static utils::SmallByteVector MakeMsgBytes(
      const std::tuple<Fields...>& fields, const Payload& payload) {
    utils::SmallByteVector bytes{kMessageType};
    internal::MsgPackBufferStream stream(&bytes);
    msgpack::pack(stream, fields);
    payload.SerializeTo(&bytes);
    return bytes;
  }
};

class OutgoingMessage : virtual public Message {
 public:
  std::size_t GetSize() const;
  const utils::SmallByteVector& Serialize() const;
  utils::SharedSmallByteVector SerializeShared();

 protected:
  OutgoingMessage(utils::SmallByteVector serialized_msg);

 private:
  utils::SmallByteVector serialized_msg_;
  utils::SharedSmallByteVector shared_serialized_msg_;
};

namespace messages {

// This is a message whose length is zero (msg type is omitted)
class Heartbeat : public MessageT<MessageType::kHeartbeat> {
  virtual std::string ToString() const override final { return "Heartbeat"; }
};

class HeartbeatIncoming : public IncomingMessage, public Heartbeat {};

class HeartbeatOutgoing : public OutgoingMessage, public Heartbeat {
 public:
  HeartbeatOutgoing() : OutgoingMessage({}) {}
};

class Acknowledge : public MessageT<MessageType::kAcknowledge> {
  virtual std::string ToString() const override final { return "Acknowledge"; }
};

class AcknowledgeIncoming : public IncomingMessage, public Acknowledge {};

class AcknowledgeOutgoing : public OutgoingMessage, public Acknowledge {
 public:
  AcknowledgeOutgoing() : OutgoingMessage(MakeMsgBytes()) {}
};

class Broadcast : public MessageT<MessageType::kBroadcast> {};

class BroadcastIncoming : public IncomingMessage, public Broadcast {
 public:
  BroadcastIncoming(const utils::ByteVector& serialized_msg);

  virtual std::string ToString() const override final;

  const Payload& GetPayload() const { return payload_; }

 private:
  const Payload payload_;
};

class BroadcastOutgoing : public OutgoingMessage, public Broadcast {
 public:
  BroadcastOutgoing(const Payload& payload);

  virtual std::string ToString() const override final;
};

}  // namespace messages
}  // namespace network

std::ostream& operator<<(std::ostream& os, const network::Message& msg);
