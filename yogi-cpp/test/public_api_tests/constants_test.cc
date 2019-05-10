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

#include <yogi_core.h>

using namespace std::chrono_literals;

TEST(ConstantsTest, kVersionNumber) {
  EXPECT_EQ(yogi::constants::kVersionNumber, YOGI_HDR_VERSION);
}

TEST(ConstantsTest, kVersionMajor) {
  EXPECT_EQ(yogi::constants::kVersionMajor, YOGI_HDR_VERSION_MAJOR);
}

TEST(ConstantsTest, kVersionMinor) {
  EXPECT_EQ(yogi::constants::kVersionMinor, YOGI_HDR_VERSION_MINOR);
}

TEST(ConstantsTest, kVersionPatch) {
  EXPECT_EQ(yogi::constants::kVersionPatch, YOGI_HDR_VERSION_PATCH);
}

TEST(ConstantsTest, kDefaultAdvInterfaces) {
  EXPECT_FALSE(yogi::constants::kDefaultAdvInterfaces.empty());
}

TEST(ConstantsTest, kDefaultAdvAddress) {
  EXPECT_FALSE(yogi::constants::kDefaultAdvAddress.empty());
}

TEST(ConstantsTest, kDefaultAdvPort) {
  EXPECT_GE(yogi::constants::kDefaultAdvPort, 1024);
}

TEST(ConstantsTest, kDefaultAdvInterval) {
  EXPECT_GT(yogi::constants::kDefaultAdvInterval, 500ms);
}

TEST(ConstantsTest, kDefaultConnectionTimeout) {
  EXPECT_GT(yogi::constants::kDefaultConnectionTimeout, 500ms);
}

TEST(ConstantsTest, kDefaultLoggerVerbosity) {
  EXPECT_EQ(yogi::Verbosity::kInfo, yogi::constants::kDefaultLoggerVerbosity);
}

TEST(ConstantsTest, kDefaultLogTimeFormat) {
  EXPECT_FALSE(yogi::constants::kDefaultLogTimeFormat.empty());
}

TEST(ConstantsTest, kDefaultLogFormat) {
  EXPECT_FALSE(yogi::constants::kDefaultLogFormat.empty());
}

TEST(ConstantsTest, kMaxMessagePayloadSize) {
  EXPECT_GE(yogi::constants::kMaxMessagePayloadSize, 1000);
}

TEST(ConstantsTest, kDefaultTimeFormat) {
  EXPECT_NE(yogi::constants::kDefaultTimeFormat.find(".%3"), std::string::npos);
}

TEST(ConstantsTest, kDefaultInfiniteDurationString) {
  EXPECT_NE(yogi::constants::kDefaultInfiniteDurationString.find("inf"),
            std::string::npos);
}

TEST(ConstantsTest, kDefaultDurationFormat) {
  EXPECT_NE(yogi::constants::kDefaultDurationFormat.find(".%3"),
            std::string::npos);
}

TEST(ConstantsTest, kDefaultInvalidHandleString) {
  EXPECT_NE(yogi::constants::kDefaultInvalidHandleString.find("INVALID"),
            std::string::npos);
}

TEST(ConstantsTest, kDefaultObjectFormat) {
  EXPECT_NE(yogi::constants::kDefaultObjectFormat.find("$T"),
            std::string::npos);
}

TEST(ConstantsTest, kMinTxQueueSize) {
  EXPECT_GE(yogi::constants::kMinTxQueueSize,
            yogi::constants::kMaxMessagePayloadSize);
}

TEST(ConstantsTest, kMaxTxQueueSize) {
  EXPECT_GE(yogi::constants::kMaxTxQueueSize, yogi::constants::kMinTxQueueSize);
}

TEST(ConstantsTest, kDefaultTxQueueSize) {
  EXPECT_LT(yogi::constants::kDefaultTxQueueSize,
            yogi::constants::kMaxTxQueueSize);
}

TEST(ConstantsTest, kMinRxQueueSize) {
  EXPECT_GE(yogi::constants::kMinRxQueueSize,
            yogi::constants::kMaxMessagePayloadSize);
}

TEST(ConstantsTest, kMaxRxQueueSize) {
  EXPECT_GE(yogi::constants::kMaxRxQueueSize, yogi::constants::kMinRxQueueSize);
}

TEST(ConstantsTest, kDefaultRxQueueSize) {
  EXPECT_LT(yogi::constants::kDefaultRxQueueSize,
            yogi::constants::kMaxRxQueueSize);
}
