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
#include "../../src/network/serialize.h"

#include <boost/uuid/uuid_generators.hpp>

class SerializeTest : public TestFixture {
 protected:
  template <typename T>
  void Check(T val) {
    utils::ByteVector buffer;
    network::Serialize(&buffer, val);

    T des_val;
    auto it = buffer.cbegin();
    EXPECT_TRUE(network::Deserialize(&des_val, buffer, &it));
    EXPECT_EQ(des_val, val);
    EXPECT_EQ(it, buffer.end());

    EXPECT_FALSE(network::Deserialize(&des_val, buffer, &it));
  }
};

TEST_F(SerializeTest, bool) {
  Check<bool>(true);
  Check<bool>(false);
}

TEST_F(SerializeTest, unsigned_short) {
  Check<unsigned short>(12345);
}

TEST_F(SerializeTest, int) {
  Check<int>(-1234567890);
}

TEST_F(SerializeTest, size_t) {
  Check<size_t>(1234567890);
}

TEST_F(SerializeTest, nanoseconds) {
  Check(std::chrono::nanoseconds(1234567890));
}

TEST_F(SerializeTest, Timestamp) {
  Check(utils::Timestamp::Now());
}

TEST_F(SerializeTest, string) {
  Check<std::string>("Hello");
}

TEST_F(SerializeTest, uuid) {
  Check(boost::uuids::random_generator()());
}
