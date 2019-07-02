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

#include "../../common.h"
#include "../../../src/objects/web/detail/session/url_encoding.h"
using namespace objects::web::detail;

TEST(UrlEncodingTest, DecodeUrl) {
  auto dec = DecodeUrl("");
  ASSERT_TRUE(dec);
  EXPECT_EQ(*dec, "");

  dec = DecodeUrl("/test.html?x=4&y=2");
  ASSERT_TRUE(dec);
  EXPECT_EQ(*dec, "/test.html?x=4&y=2");

  dec = DecodeUrl("/abc%25%456");
  ASSERT_TRUE(dec);
  EXPECT_EQ(*dec, "/abc%E6");

  dec = DecodeUrl("/ab%%44");
  ASSERT_FALSE(dec);

  dec = DecodeUrl("/ab%3");
  ASSERT_FALSE(dec);

  dec = DecodeUrl("/ab%ga");
  ASSERT_FALSE(dec);
}
