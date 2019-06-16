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

#include "messages.h"
#include "../api/errors.h"

#include <nlohmann/json.hpp>

namespace network {
namespace internal {

struct MsgPackCheckVisitor : public msgpack::null_visitor {
  void parse_error(std::size_t parsed_offset, std::size_t error_offset) {
    throw api::DescriptiveError(YOGI_ERR_INVALID_USER_MSGPACK)
        << "Parse errror. Parsed offset: " << parsed_offset
        << "; Error offset: " << error_offset;
  }

  void insufficient_bytes(std::size_t parsed_offset, std::size_t error_offset) {
    throw api::DescriptiveError(YOGI_ERR_INVALID_USER_MSGPACK)
        << "Insufficient bytes. Parsed offset: " << parsed_offset
        << "; Error offset: " << error_offset;
  }
};

namespace {

void CheckPayloadIsValidMsgPack(const char* data, std::size_t size) {
  MsgPackCheckVisitor visitor;
  bool ok = msgpack::parse(data, size, visitor);
  if (!ok) {
    throw api::DescriptiveError(YOGI_ERR_INVALID_USER_MSGPACK)
        << "msgpack::parse() returned false";
  }
}

utils::ByteVector CheckAndConvertPayloadFromJsonToMsgPack(const char* data,
                                                          std::size_t size) {
  YOGI_ASSERT(size > 0);
  if (data[size - 1] != '\0') {
    throw api::DescriptiveError(YOGI_ERR_PARSING_JSON_FAILED)
        << "Unterminated string";
  }

  try {
    auto json = nlohmann::json::parse(data);
    return nlohmann::json::to_msgpack(json);
  } catch (const std::exception& e) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_JSON_FAILED) << e.what();
  }
}

}  // anonymous namespace
}  // namespace internal

void IncomingMessage::Deserialize(const utils::ByteVector& serialized_msg,
                                  const MessageHandler& fn) {
  if (serialized_msg.empty()) {
    fn(messages::HeartbeatIncoming());
    return;
  }

  switch (serialized_msg[0]) {
    case MessageType::kAcknowledge:
      fn(messages::AcknowledgeIncoming());
      break;

    case MessageType::kBroadcast:
      fn(messages::BroadcastIncoming(serialized_msg));
      break;

    default:
      throw api::DescriptiveError(YOGI_ERR_DESERIALIZE_MSG_FAILED)
          << "Unknown message type " << serialized_msg[0];
  }
}

void Payload::SerializeTo(utils::SmallByteVector* buffer) const {
  if (data_.size() == 0) return;

  auto raw = static_cast<const char*>(data_.data());

  switch (enc_) {
    case api::Encoding::kJson: {
      auto data =
          internal::CheckAndConvertPayloadFromJsonToMsgPack(raw, data_.size());
      buffer->insert(buffer->end(), data.begin(), data.end());
      break;
    }

    case api::Encoding::kMsgPack: {
      internal::CheckPayloadIsValidMsgPack(raw, data_.size());
      buffer->insert(buffer->end(), raw, raw + data_.size());

      break;
    }

    default:
      YOGI_NEVER_REACHED;
  }
}

api::Result Payload::SerializeToUserBuffer(boost::asio::mutable_buffer buffer,
                                           api::Encoding enc,
                                           std::size_t* bytes_written) const {
  utils::ByteVector tmp_buf;
  std::string tmp_str;
  boost::asio::const_buffer src;

  if (enc == enc_) {
    src = data_;
  } else {
    switch (enc) {
      case api::Encoding::kJson: {
        auto raw = static_cast<const unsigned char*>(data_.data());
        auto json = nlohmann::json::from_msgpack(raw, data_.size());
        tmp_str = json.dump();
        src = boost::asio::buffer(tmp_str.data(), tmp_str.size() + 1);
        break;
      }

      case api::Encoding::kMsgPack: {
        auto raw = static_cast<const char*>(data_.data());
        auto json = nlohmann::json::parse(raw);
        tmp_buf = nlohmann::json::to_msgpack(json);
        src = boost::asio::buffer(tmp_buf);
        break;
      }

      default:
        YOGI_NEVER_REACHED;
    }
  }

  auto n = boost::asio::buffer_copy(buffer, src);
  YOGI_ASSERT(bytes_written != nullptr);
  *bytes_written = n;

  if (n < src.size()) {
    if (enc == api::Encoding::kJson) {
      static_cast<char*>(buffer.data())[buffer.size() - 1] = '\0';
    }

    return api::Error(YOGI_ERR_BUFFER_TOO_SMALL);
  }

  return api::kSuccess;
}

std::size_t OutgoingMessage::GetSize() const { return Serialize().size(); }

const utils::SmallByteVector& OutgoingMessage::Serialize() const {
  if (shared_serialized_msg_) {
    return *shared_serialized_msg_;
  } else {
    return serialized_msg_;
  }
}

utils::SharedSmallByteVector OutgoingMessage::SerializeShared() {
  if (!shared_serialized_msg_) {
    shared_serialized_msg_ =
        utils::MakeSharedSmallByteVector(std::move(serialized_msg_));
  }

  return shared_serialized_msg_;
}

OutgoingMessage::OutgoingMessage(utils::SmallByteVector serialized_msg)
    : serialized_msg_(serialized_msg) {}

namespace messages {

BroadcastIncoming::BroadcastIncoming(const utils::ByteVector& serialized_msg)
    : payload_(boost::asio::buffer(serialized_msg) + 1,
               api::Encoding::kMsgPack) {}

std::string BroadcastIncoming::ToString() const {
  std::stringstream ss;
  ss << "Broadcast, " << BroadcastOutgoing(payload_).Serialize().size() - 1
     << " bytes user data";
  return ss.str();
}

BroadcastOutgoing::BroadcastOutgoing(const Payload& payload)
    : OutgoingMessage(MakeMsgBytes(payload)) {}

std::string BroadcastOutgoing::ToString() const {
  std::stringstream ss;
  ss << "Broadcast, " << GetSize() - 1 << " bytes user data";
  return ss.str();
}

}  // namespace messages
}  // namespace network

std::ostream& operator<<(std::ostream& os, const network::Message& msg) {
  os << msg.ToString();
  return os;
}
