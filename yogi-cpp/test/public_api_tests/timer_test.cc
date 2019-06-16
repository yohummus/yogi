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

#include <yogi_core.h>

class TimerTest : public testing::Test {
 protected:
  yogi::ContextPtr context_ = yogi::Context::Create();
  yogi::TimerPtr timer_ = yogi::Timer::Create(context_);
};

TEST_F(TimerTest, Start) {
  bool called = false;
  timer_->Start(yogi::Duration::FromMilliseconds(1), [&](auto& res) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kOk);
    called = true;
  });

  while (!called) {
    context_->RunOne();
  }

  EXPECT_TRUE(called);
}

TEST_F(TimerTest, Cancel) {
  EXPECT_FALSE(timer_->Cancel());

  bool called = false;
  timer_->Start(yogi::Duration::kInfinity, [&](auto& res) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Failure&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kCanceled);
    called = true;
  });

  EXPECT_TRUE(timer_->Cancel());

  while (!called) {
    context_->RunOne();
  }

  EXPECT_TRUE(called);
}
