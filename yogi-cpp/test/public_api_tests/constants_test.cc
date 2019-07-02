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

using namespace std::chrono_literals;

TEST(ConstantsTest, kVersion) {
  EXPECT_EQ(yogi::constants::kVersion, YOGI_HDR_VERSION);
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

TEST(ConstantsTest, kVersionSuffix) {
  EXPECT_EQ(yogi::constants::kVersionSuffix, YOGI_HDR_VERSION_SUFFIX);
}

TEST(ConstantsTest, kDefaultAdvInterfaces) {
  EXPECT_NE(yogi::constants::kDefaultAdvInterfaces.find("localhost"),
            std::string::npos);
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

TEST(ConstantsTest, kDefaultWebPort) {
  EXPECT_GT(yogi::constants::kDefaultWebPort, 1024);
  EXPECT_LT(yogi::constants::kDefaultWebPort, 65535);
}

TEST(ConstantsTest, kDefaultWebInterfaces) {
  EXPECT_NE(yogi::constants::kDefaultWebInterfaces.find("localhost"),
            std::string::npos);
}

TEST(ConstantsTest, kDefaultWebTimeout) {
  EXPECT_GT(yogi::constants::kDefaultWebTimeout, 500ms);
}

TEST(ConstantsTest, kDefaultHttpHeaderLimit) {
  EXPECT_GT(yogi::constants::kDefaultHttpHeaderLimit, 100);
  EXPECT_LT(yogi::constants::kDefaultHttpHeaderLimit, 1000000);
}

TEST(ConstantsTest, kDefaultHttpBodyLimit) {
  EXPECT_GT(yogi::constants::kDefaultHttpBodyLimit, 100);
  EXPECT_GE(yogi::constants::kDefaultHttpBodyLimit,
            yogi::constants::kDefaultHttpHeaderLimit);
}

TEST(ConstantsTest, kDefaultWebCacheSize) {
  EXPECT_GE(yogi::constants::kDefaultWebCacheSize, 1'000'000);
  EXPECT_LT(yogi::constants::kDefaultWebCacheSize,
            yogi::constants::kMaxWebCacheSize);
}

TEST(ConstantsTest, kMaxWebCacheSize) {
  EXPECT_GE(yogi::constants::kMaxWebCacheSize, 100'000'000);
  EXPECT_LE(yogi::constants::kMaxWebCacheSize, 1'000'000'000);
}

TEST(ConstantsTest, kDefaultAdminUser) {
  EXPECT_GE(yogi::constants::kDefaultAdminUser.size(), 3u);
}

TEST(ConstantsTest, kDefaultAdminPassword) {
  EXPECT_GE(yogi::constants::kDefaultAdminPassword.size(), 3u);
}

TEST(ConstantsTest, kDefaultSslPrivateKey) {
  EXPECT_NE(yogi::constants::kDefaultSslPrivateKey.find(
                "-----BEGIN PRIVATE KEY-----"),
            std::string::npos);
}

TEST(ConstantsTest, kDefaultSslCertificateChain) {
  EXPECT_NE(yogi::constants::kDefaultSslCertificateChain.find(
                "-----BEGIN CERTIFICATE-----"),
            std::string::npos);
}

TEST(ConstantsTest, kDefaultSslDhParams) {
  EXPECT_NE(yogi::constants::kDefaultSslDhParams.find(
                "-----BEGIN DH PARAMETERS-----"),
            std::string::npos);
}
