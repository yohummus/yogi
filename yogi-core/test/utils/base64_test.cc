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
#include "../../src/utils/base64.h"

#include <vector>

class Base64Test : public TestFixture {
 protected:
  std::string plain_ = "hello";
  std::string encoded_ = "aGVsbG8=";
};

TEST_F(Base64Test, Encode) {
  auto encoded = utils::EncodeBase64(plain_);
  EXPECT_EQ(encoded, encoded_);
}

TEST_F(Base64Test, Decode) {
  auto plain = utils::DecodeBase64(encoded_);
  EXPECT_EQ(plain, plain_);
}
