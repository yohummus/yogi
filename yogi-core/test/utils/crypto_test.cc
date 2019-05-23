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
#include "../../src/utils/crypto.h"

#include <string>
using namespace std::string_literals;

TEST(CryptoTest, MakeSha256) {
  auto bytes = utils::MakeSha256(utils::ByteVector{'h', 'e', 'l', 'l', 'o'});
  auto hash = utils::ByteVector{
      0x2c, 0xf2, 0x4d, 0xba, 0x5f, 0xb0, 0xa3, 0x0e, 0x26, 0xe8, 0x3b,
      0x2a, 0xc5, 0xb9, 0xe2, 0x9e, 0x1b, 0x16, 0x1e, 0x5c, 0x1f, 0xa7,
      0x42, 0x5e, 0x73, 0x04, 0x33, 0x62, 0x93, 0x8b, 0x98, 0x24,
  };
  EXPECT_EQ(bytes, hash);

  EXPECT_EQ(utils::MakeSha256("hello"s), bytes);
}

TEST(CryptoTest, GenerateRandomBytes) {
  auto bytes = utils::GenerateRandomBytes(5);
  EXPECT_EQ(bytes.size(), 5);

  while (bytes == utils::GenerateRandomBytes(bytes.size()))
    ;
}