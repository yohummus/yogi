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
#include "../../src/utils/algorithm.h"

#include <vector>

class AlgorithmTest : public TestFixture {
 protected:
  std::vector<int> vec_ = {1, 2, 3, 3, 4, 3, 5};
};

TEST_F(AlgorithmTest, Find) {
  EXPECT_EQ(utils::find(vec_, 3), vec_.begin() + 2);
  EXPECT_EQ(utils::find(vec_, 0), vec_.end());
}

TEST_F(AlgorithmTest, FindIf) {
  auto it = utils::find_if(vec_, [](int val) { return val == 3; });
  EXPECT_EQ(it, vec_.begin() + 2);

  it = utils::find_if(vec_, [](int) { return false; });
  EXPECT_EQ(it, vec_.end());
}

TEST_F(AlgorithmTest, Contains) {
  EXPECT_TRUE(utils::contains(vec_, 3));
  EXPECT_FALSE(utils::contains(vec_, 7));
}

TEST_F(AlgorithmTest, ContainsIf) {
  bool contains = utils::contains_if(vec_, [](int val) { return val == 3; });
  EXPECT_TRUE(contains);

  contains = utils::contains_if(vec_, [](int) { return false; });
  EXPECT_FALSE(contains);
}

TEST_F(AlgorithmTest, RemoveErase) {
  auto it = utils::remove_erase(vec_, 3);
  EXPECT_EQ(vec_, (std::vector<int>{1, 2, 4, 5}));
  EXPECT_EQ(it, vec_.end());
}

TEST_F(AlgorithmTest, RemoveEraseIf) {
  auto it = utils::remove_erase_if(vec_, [](int val) { return val > 3; });
  EXPECT_EQ(vec_, (std::vector<int>{1, 2, 3, 3, 3}));
  EXPECT_EQ(it, vec_.end());
}