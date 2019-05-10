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

TEST(ObjectFormatTest, FormatObject) {
  void* context;
  int res = YOGI_ContextCreate(&context);
  EXPECT_OK(res);

  char str[32];
  res = YOGI_FormatObject(context, str, sizeof(str), nullptr, nullptr);
  EXPECT_OK(res);
  std::string s1 = str;
  EXPECT_NE(s1.find("Context"), std::string::npos);
  EXPECT_NE(s1.find("["), std::string::npos);
  EXPECT_NE(s1.find("]"), std::string::npos);
  EXPECT_EQ(s1.find("[]"), std::string::npos);

  res = YOGI_FormatObject(context, str, sizeof(str), "$T$x$X", nullptr);
  EXPECT_OK(res);
  std::string s2 = str;
  EXPECT_EQ(s2.find("Context"), 0);
  EXPECT_GT(s2.size(), sizeof("Context") + 2);

  res = YOGI_FormatObject(context, str, sizeof(str), "$T$X$x", nullptr);
  EXPECT_OK(res);
  std::string s3 = str;

  std::string tmp = s3.substr(sizeof("Context"));
  if (tmp.find_first_of("abcdef") == std::string::npos) {
    EXPECT_EQ(s3, s2);
  } else {
    EXPECT_NE(s3, s2);
  }

  res = YOGI_FormatObject(nullptr, str, sizeof(str), nullptr, "abc");
  EXPECT_OK(res);
  std::string s4 = str;
  EXPECT_EQ(s4, "abc");

  res = YOGI_FormatObject(nullptr, str, sizeof(str), "$T $x", "abc");
  EXPECT_OK(res);
  std::string s5 = str;
  EXPECT_EQ(s5, "abc");
}
