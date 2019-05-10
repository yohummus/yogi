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

#include <yogi/payload_view.h>

class PayloadViewTest : public testing::Test {
 protected:
  PayloadViewTest()
      : data_("Hello"), view_(data_, 123, yogi::EncodingType::kMsgpack) {}

  const char* data_;
  const yogi::PayloadView view_;
};

TEST_F(PayloadViewTest, Getters) {
  EXPECT_EQ(view_.Data(), data_);
  EXPECT_EQ(view_.Size(), 123);
  EXPECT_EQ(view_.Encoding(), yogi::EncodingType::kMsgpack);
}

TEST_F(PayloadViewTest, DefaultConstruct) {
  yogi::PayloadView view;
  EXPECT_EQ(view.Data(), nullptr);
  EXPECT_EQ(view.Size(), 0);
  EXPECT_EQ(view.Encoding(), yogi::EncodingType::kJson);
}

TEST_F(PayloadViewTest, ConstructFromBuffer) {
  const char* data = "hello";
  int size = static_cast<int>(std::strlen(data));
  auto view = yogi::PayloadView(data, size, yogi::EncodingType::kMsgpack);
  EXPECT_EQ(view.Data(), data);
  EXPECT_EQ(view.Size(), size);
  EXPECT_EQ(view.Encoding(), yogi::EncodingType::kMsgpack);
}

TEST_F(PayloadViewTest, ConstructFromVector) {
  std::vector<char> data = {'h', 'i'};
  int size = static_cast<int>(data.size());
  auto view = yogi::PayloadView(data, yogi::EncodingType::kMsgpack);
  EXPECT_EQ(view.Data(), data.data());
  EXPECT_EQ(view.Size(), size);
  EXPECT_EQ(view.Encoding(), yogi::EncodingType::kMsgpack);
}

TEST_F(PayloadViewTest, ConstructFromJsonView) {
  yogi::Json json = {12345};
  auto json_view = yogi::JsonView(json);
  auto view = yogi::PayloadView(json_view);

  auto s = json.dump();
  EXPECT_EQ(s, view.Data());
  EXPECT_EQ(view.Encoding(), yogi::EncodingType::kJson);
}

TEST_F(PayloadViewTest, ConstructFromMsgpackView) {
  std::string msgpack = "Hello";
  auto msgpack_view = yogi::MsgpackView(msgpack);
  auto view = yogi::PayloadView(msgpack_view);

  EXPECT_EQ(msgpack,
            std::string(view.Data(), static_cast<std::size_t>(view.Size())));
  EXPECT_EQ(view.Encoding(), yogi::EncodingType::kMsgpack);
}

TEST_F(PayloadViewTest, ClassComparisonOperators) {
  const char* ab = "Hello";
  yogi::PayloadView a_view(ab, std::strlen(ab), yogi::EncodingType::kJson);
  yogi::PayloadView b_view(ab, std::strlen(ab), yogi::EncodingType::kMsgpack);

  std::string c = ab;
  yogi::PayloadView c_view(c.c_str(), c.size(), yogi::EncodingType::kJson);

  std::string d = c + " you";
  yogi::PayloadView d_view(d.c_str(), d.size(), yogi::EncodingType::kJson);

  std::string e = "olleH";
  yogi::PayloadView e_view(e.c_str(), e.size(), yogi::EncodingType::kJson);

  EXPECT_TRUE(a_view == a_view);
  EXPECT_FALSE(a_view == b_view);
  EXPECT_TRUE(a_view == c_view);
  EXPECT_FALSE(a_view == d_view);
  EXPECT_FALSE(a_view == e_view);

  EXPECT_FALSE(a_view != a_view);
  EXPECT_TRUE(a_view != b_view);
  EXPECT_FALSE(a_view != c_view);
  EXPECT_TRUE(a_view != d_view);
  EXPECT_TRUE(a_view != e_view);
}

TEST_F(PayloadViewTest, FreeComparisonOperators) {
  const char* json = "json";
  yogi::JsonView json_view(json);
  yogi::PayloadView json_payload(json_view);

  EXPECT_TRUE(json_payload == json_view);
  EXPECT_TRUE(json_view == json_payload);
  EXPECT_FALSE(json_payload != json_view);
  EXPECT_FALSE(json_view != json_payload);

  const char* msgpack = "msgpack";
  yogi::MsgpackView msgpack_view(msgpack, std::strlen(msgpack));
  yogi::PayloadView msgpack_payload(msgpack_view);

  EXPECT_TRUE(msgpack_payload == msgpack_view);
  EXPECT_TRUE(msgpack_view == msgpack_payload);
  EXPECT_FALSE(msgpack_payload != msgpack_view);
  EXPECT_FALSE(msgpack_view != msgpack_payload);
}