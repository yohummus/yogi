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
#include "../../src/api/constants.h"

void check(int constant, int value) {
  int x;
  YOGI_GetConstant(&x, constant);
  EXPECT_EQ(x, value) << "Constant (int): ";
}

void check(int constant, long long value) {
  long long x;
  YOGI_GetConstant(&x, constant);
  EXPECT_EQ(x, value) << "Constant (long long): ";
}

void check(int constant, const char* value) {
  const char* x;
  YOGI_GetConstant(&x, constant);
  EXPECT_STREQ(x, value) << "Constant (const char*): ";
}

TEST(ConstantsTest, GetConstant) {
  int num = -1;
  YOGI_GetConstant(&num, YOGI_CONST_VERSION_MAJOR);
  EXPECT_NE(num, -1);

  int res = YOGI_GetConstant(nullptr, YOGI_CONST_VERSION_MAJOR);
  EXPECT_ERR(res, YOGI_ERR_INVALID_PARAM);

  EXPECT_EQ(YOGI_GetConstant(&num, -9999), YOGI_ERR_INVALID_PARAM);
}

TEST(ConstantsTest, GetEachConstant) {
  using namespace api;

  // clang-format off
  check(YOGI_CONST_VERSION_NUMBER,                kVersionNumber);
  check(YOGI_CONST_VERSION_MAJOR,                 kVersionMajor);
  check(YOGI_CONST_VERSION_MINOR,                 kVersionMinor);
  check(YOGI_CONST_VERSION_PATCH,                 kVersionPatch);
  check(YOGI_CONST_DEFAULT_ADV_INTERFACES,        kDefaultAdvInterfaces);
  check(YOGI_CONST_DEFAULT_ADV_ADDRESS,           kDefaultAdvAddress);
  check(YOGI_CONST_DEFAULT_ADV_PORT,              kDefaultAdvPort);
  check(YOGI_CONST_DEFAULT_ADV_INTERVAL,          kDefaultAdvInterval);
  check(YOGI_CONST_DEFAULT_CONNECTION_TIMEOUT,    kDefaultConnectionTimeout);
  check(YOGI_CONST_DEFAULT_LOGGER_VERBOSITY,      kDefaultLoggerVerbosity);
  check(YOGI_CONST_DEFAULT_LOG_TIME_FORMAT,       kDefaultLogTimeFormat);
  check(YOGI_CONST_DEFAULT_LOG_FORMAT,            kDefaultLogFormat);
  check(YOGI_CONST_MAX_MESSAGE_PAYLOAD_SIZE,      kMaxMessagePayloadSize);
  check(YOGI_CONST_DEFAULT_TIME_FORMAT,           kDefaultTimeFormat);
  check(YOGI_CONST_DEFAULT_INF_DURATION_STRING,   kDefaultInfiniteDurationString);
  check(YOGI_CONST_DEFAULT_DURATION_FORMAT,       kDefaultDurationFormat);
  check(YOGI_CONST_DEFAULT_INVALID_HANDLE_STRING, kDefaultInvalidHandleString);
  check(YOGI_CONST_DEFAULT_OBJECT_FORMAT,         kDefaultObjectFormat);
  check(YOGI_CONST_MIN_TX_QUEUE_SIZE,             kMinTxQueueSize);
  check(YOGI_CONST_MAX_TX_QUEUE_SIZE,             kMaxTxQueueSize);
  check(YOGI_CONST_DEFAULT_TX_QUEUE_SIZE,         kDefaultTxQueueSize);
  check(YOGI_CONST_MIN_RX_QUEUE_SIZE,             kMinRxQueueSize);
  check(YOGI_CONST_MAX_RX_QUEUE_SIZE,             kMaxRxQueueSize);
  check(YOGI_CONST_DEFAULT_RX_QUEUE_SIZE,         kDefaultRxQueueSize);
  // clang-format on
}
