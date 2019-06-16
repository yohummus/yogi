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

template <typename T>
void Check(const T& val) {
  std::ostringstream oss;
  oss << val;
  EXPECT_EQ(oss.str(), yogi::ToString(val));
}

TEST(StreamIoTest, ErrorCode) {
  Check(yogi::ErrorCode::kBusy);
}

TEST(StreamIoTest, Result) {
  Check(yogi::Failure(yogi::ErrorCode::kBadAlloc));
}

TEST(StreamIoTest, Object) {
  Check(*yogi::Context::Create());
}

TEST(StreamIoTest, Verbosity) {
  Check(yogi::Verbosity::kWarning);
}

TEST(StreamIoTest, Stream) {
  Check(yogi::Stream::kStderr);
}

TEST(StreamIoTest, BranchEvents) {
  Check(yogi::BranchEvents::kBranchDiscovered |
        yogi::BranchEvents::kBranchQueried);
}

TEST(StreamIoTest, Duration) {
  Check(yogi::Duration());
}

TEST(StreamIoTest, Timestamp) {
  Check(yogi::Timestamp::Now());
}

TEST(StreamIoTest, BranchInfo) {
  auto context = yogi::Context::Create();
  auto branch = yogi::Branch::Create(context);
  Check(branch->GetInfo());
}

TEST(StreamIoTest, Signals) {
  Check(yogi::Signals::kInt | yogi::Signals::kUsr3);
}

TEST(StreamIoTest, ConfigurationFlags) {
  Check(yogi::ConfigurationFlags::kDisableVariables);
}

TEST(StreamIoTest, CommandLineOptions) {
  Check(yogi::CommandLineOptions::kBranchAll |
        yogi::CommandLineOptions::kLogging);
}
