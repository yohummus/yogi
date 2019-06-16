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

TEST(StringViewTest, Default) {
  EXPECT_EQ(static_cast<const char*>(yogi::StringView()), nullptr);
}

TEST(StringViewTest, ConstCharString) {
  const char* s = "Hello";
  EXPECT_EQ(static_cast<const char*>(yogi::StringView(s)), s);
}

TEST(StringViewTest, StdString) {
  std::string s = "Hello";
  EXPECT_EQ(static_cast<const char*>(yogi::StringView(s)), s.c_str());
}

TEST(StringViewTest, ComparisonOperators) {
  const char* a = "Hello";
  std::string b = a;
  std::string c = b + " you";
  std::string d = "olleH";

  EXPECT_TRUE(yogi::StringView(a) == yogi::StringView(a));
  EXPECT_TRUE(yogi::StringView(a) == yogi::StringView(b));
  EXPECT_FALSE(yogi::StringView(a) == yogi::StringView(c));
  EXPECT_FALSE(yogi::StringView(a) == yogi::StringView(d));

  EXPECT_FALSE(yogi::StringView(a) != yogi::StringView(a));
  EXPECT_FALSE(yogi::StringView(a) != yogi::StringView(b));
  EXPECT_TRUE(yogi::StringView(a) != yogi::StringView(c));
  EXPECT_TRUE(yogi::StringView(a) != yogi::StringView(d));
}