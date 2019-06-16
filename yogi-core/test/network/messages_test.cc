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
#include "../../src/network/messages.h"
using namespace network;

#include <nlohmann/json.hpp>

class FakeOutgoingMessage : public OutgoingMessage,
                            public MessageT<MessageType::kBroadcast> {
 public:
  using MessageT::MakeMsgBytes;

  FakeOutgoingMessage(utils::SmallByteVector serialized_msg)
      : OutgoingMessage(serialized_msg) {}

  virtual std::string ToString() const override { return {}; }
};

TEST(MessagesTest, UserDataJson) {
  auto payload =
      Payload(boost::asio::buffer("{\"x\": 456}"), api::Encoding::kJson);

  utils::SmallByteVector buffer;
  EXPECT_NO_THROW(payload.SerializeTo(&buffer));

  auto json = nlohmann::json::from_msgpack(buffer);
  EXPECT_EQ(json.value("x", -1), 456);

  const utils::Byte invalid_data[] = {200};
  payload = Payload(boost::asio::buffer(invalid_data), api::Encoding::kJson);
  EXPECT_THROW_ERROR(payload.SerializeTo(&buffer),
                     YOGI_ERR_PARSING_JSON_FAILED);
}

TEST(MessageTest, UserDataMsgPack) {
  auto data = utils::SmallByteVector{0x93, 0x1, 0x2, 0x3};
  auto payload = Payload(boost::asio::buffer(data.data(), data.size()),
                         api::Encoding::kMsgPack);

  utils::SmallByteVector buffer;
  EXPECT_NO_THROW(payload.SerializeTo(&buffer));
  EXPECT_EQ(buffer, data);

  const utils::Byte invalid_data[] = {200};
  payload = Payload(boost::asio::buffer(invalid_data), api::Encoding::kMsgPack);
  EXPECT_THROW_ERROR(payload.SerializeTo(&buffer),
                     YOGI_ERR_INVALID_USER_MSGPACK);
}

TEST(MessagesTest, UserDataSerializeToUserBuffer) {
  auto json = utils::ByteVector{'[', '1', ',', '2', ',', '3', ']', '\0'};
  auto msgpack = utils::ByteVector{0x93, 0x1, 0x2, 0x3};

  auto fn = [&](const utils::ByteVector& bytes, api::Encoding enc) {
    Payload payload(boost::asio::buffer(bytes), enc);
    utils::ByteVector data;
    std::size_t n = 0;

    // To JSON: Buffer too small
    data.resize(3);
    auto res = payload.SerializeToUserBuffer(boost::asio::buffer(data),
                                             api::Encoding::kJson, &n);
    EXPECT_EQ(res, api::Error(YOGI_ERR_BUFFER_TOO_SMALL));
    EXPECT_EQ(n, data.size());
    EXPECT_EQ(data[0], json[0]);
    EXPECT_EQ(data[1], json[1]);
    EXPECT_EQ(data[2], '\0');

    // To JSON: Success
    data.resize(json.size());
    res = payload.SerializeToUserBuffer(boost::asio::buffer(data),
                                        api::Encoding::kJson, &n);
    EXPECT_EQ(res, api::kSuccess);
    EXPECT_EQ(n, json.size());
    EXPECT_EQ(data, json);

    // To MsgPack: Buffer too small
    data.resize(3);
    res = payload.SerializeToUserBuffer(boost::asio::buffer(data),
                                        api::Encoding::kMsgPack, &n);
    EXPECT_EQ(res, api::Error(YOGI_ERR_BUFFER_TOO_SMALL));
    EXPECT_EQ(n, data.size());
    EXPECT_EQ(data[0], msgpack[0]);
    EXPECT_EQ(data[1], msgpack[1]);
    EXPECT_EQ(data[2], msgpack[2]);

    // To MsgPack: Success
    data.resize(msgpack.size());
    res = payload.SerializeToUserBuffer(boost::asio::buffer(data),
                                        api::Encoding::kMsgPack, &n);
    EXPECT_EQ(res, api::kSuccess);
    EXPECT_EQ(n, msgpack.size());
    EXPECT_EQ(data, msgpack);
  };

  fn(json, api::Encoding::kJson);
  fn(msgpack, api::Encoding::kMsgPack);
}

TEST(MessagesTest, GetType) {
  auto fakeType = FakeOutgoingMessage::kMessageType;
  EXPECT_EQ(fakeType, MessageType::kBroadcast);
  EXPECT_EQ(fakeType, FakeOutgoingMessage({}).GetType());
}

TEST(MessagesTest, MakeMsgBytes) {
  auto type = FakeOutgoingMessage::kMessageType;
  auto payload = Payload(boost::asio::buffer("x", 1), api::Encoding::kMsgPack);

  auto bytes = FakeOutgoingMessage::MakeMsgBytes();
  EXPECT_EQ(bytes, utils::SmallByteVector{type});

  bytes = FakeOutgoingMessage::MakeMsgBytes(payload);
  EXPECT_EQ(bytes, (utils::SmallByteVector{type, 'x'}));

  bytes = FakeOutgoingMessage::MakeMsgBytes(std::make_tuple(true, false, 123));
  EXPECT_EQ(bytes, (utils::SmallByteVector{type, 0x93, 0xc3, 0xc2, 0x7b}));

  bytes = FakeOutgoingMessage::MakeMsgBytes(std::make_tuple(true, false, 123),
                                            payload);
  EXPECT_EQ(bytes, (utils::SmallByteVector{type, 0x93, 0xc3, 0xc2, 0x7b, 'x'}));
}

TEST(MessagesTest, GetSize) {
  auto msg = FakeOutgoingMessage({1, 2, 3});
  EXPECT_EQ(msg.GetSize(), 3);
}

TEST(MessagesTest, Serialize) {
  auto msg = FakeOutgoingMessage({1, 2, 3});
  auto bytes = msg.Serialize();
  EXPECT_EQ(bytes, (utils::SmallByteVector{1, 2, 3}));

  msg.SerializeShared();
  EXPECT_EQ(msg.Serialize(), bytes);
}

TEST(MessagesTest, SerializeShared) {
  auto msg = FakeOutgoingMessage({1, 2, 3});
  auto bytes = msg.SerializeShared();
  EXPECT_EQ(*bytes, (utils::SmallByteVector{1, 2, 3}));
}

TEST(MessagesTest, Deserialize) {
  utils::ByteVector bytes = {MessageType::kBroadcast, 0x93, 0x01, 0x02, 0x03};

  bool called = false;
  IncomingMessage::Deserialize(bytes, [&](const IncomingMessage& msg) {
    EXPECT_EQ(msg.GetType(), MessageType::kBroadcast);

    auto bcm = dynamic_cast<const messages::BroadcastIncoming*>(&msg);
    ASSERT_NE(bcm, nullptr);

    utils::SmallByteVector data;
    bcm->GetPayload().SerializeTo(&data);
    EXPECT_EQ(data, utils::SmallByteVector(bytes.begin() + 1, bytes.end()));

    called = true;
  });

  EXPECT_TRUE(called);
}
