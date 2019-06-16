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

#define MSGPACK_VERSION_MAJOR 3

namespace msgpack {

using object = int;

}  // namespace msgpack

#include <yogi/msgpack.h>
#include <gtest/gtest.h>
#include <type_traits>

TEST(MsgpackTest, CustomMsgpackLibrary) {
  EXPECT_TRUE((std::is_same<yogi::msgpack::object, int>::value));
}
