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

#include <vector>
#include <type_traits>

class OperationIdTest : public testing::Test {
 protected:
  const yogi::OperationId oid_ = yogi::internal::MakeOperationId(123);
};

TEST_F(OperationIdTest, DefaultConstructor) {
  yogi::OperationId oid;
  EXPECT_FALSE(oid.IsValid());
  EXPECT_EQ(oid.Value(), 0);
}

TEST_F(OperationIdTest, Value) { EXPECT_EQ(oid_.Value(), 123); }

TEST_F(OperationIdTest, IsValid) {
  EXPECT_TRUE(oid_.IsValid());

  auto oid = yogi::internal::MakeOperationId(0);
  EXPECT_FALSE(oid.IsValid());

  oid = yogi::internal::MakeOperationId(-1);
  EXPECT_FALSE(oid.IsValid());
}

TEST_F(OperationIdTest, Copy) {
  yogi::OperationId oid;
  oid = oid_;
  EXPECT_EQ(oid.Value(), 123);
  EXPECT_TRUE(oid.IsValid());
}

TEST_F(OperationIdTest, Operators) {
  auto fn = [](int val) { return yogi::internal::MakeOperationId(val); };

  EXPECT_TRUE(fn(5) == fn(5));
  EXPECT_FALSE(fn(5) == fn(10));

  EXPECT_TRUE(fn(5) != fn(10));
  EXPECT_FALSE(fn(5) != fn(5));

  EXPECT_TRUE(fn(5) < fn(10));
  EXPECT_FALSE(fn(5) < fn(5));
  EXPECT_FALSE(fn(5) < fn(1));

  EXPECT_TRUE(fn(5) <= fn(10));
  EXPECT_TRUE(fn(5) <= fn(5));
  EXPECT_FALSE(fn(5) <= fn(1));

  EXPECT_FALSE(fn(5) > fn(10));
  EXPECT_FALSE(fn(5) > fn(5));
  EXPECT_TRUE(fn(5) > fn(1));

  EXPECT_FALSE(fn(5) >= fn(10));
  EXPECT_TRUE(fn(5) >= fn(5));
  EXPECT_TRUE(fn(5) >= fn(1));
}