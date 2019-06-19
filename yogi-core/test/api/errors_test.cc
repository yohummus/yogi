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

static constexpr int kLastError = YOGI_ERR_WORKER_ALREADY_ADDED;

TEST(ErrorsTest, DefaultResultConstructor) {
  api::Result res;
  EXPECT_EQ(res.GetValue(), YOGI_ERR_UNKNOWN);
}

TEST(ErrorsTest, GetErrorString) {
  EXPECT_STRNE(YOGI_GetErrorString(-2), YOGI_GetErrorString(-1));
  EXPECT_STREQ(YOGI_GetErrorString(0), YOGI_GetErrorString(1));
}

TEST(ErrorsTest, DescriptionForEachError) {
  auto inv_err_str = YOGI_GetErrorString(kLastError - 1);

  for (int i = 0; i >= kLastError; --i) {
    EXPECT_STRNE(YOGI_GetErrorString(i), inv_err_str) << "Error code: " << i;
  }

  for (int i = kLastError - 1; i < kLastError - 10; --i) {
    EXPECT_STREQ(YOGI_GetErrorString(i), inv_err_str) << "Error code: " << i;
  }
}

TEST(ErrorsTest, NoDuplicates) {
  for (int i = 0; i >= kLastError; --i) {
    for (int j = 0; j > i; --j) {
      EXPECT_STRNE(YOGI_GetErrorString(i), YOGI_GetErrorString(j));
    }
  }
}
