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
#include <type_traits>

TEST(ErrorsTest, ErrorCodeEnum) {
  // clang-format off
  CHECK_ENUM_ELEMENT(ErrorCode, kOk,                               YOGI_OK);
  CHECK_ENUM_ELEMENT(ErrorCode, kUnknown,                          YOGI_ERR_UNKNOWN);
  CHECK_ENUM_ELEMENT(ErrorCode, kObjectStillUsed,                  YOGI_ERR_OBJECT_STILL_USED);
  CHECK_ENUM_ELEMENT(ErrorCode, kBadAlloc,                         YOGI_ERR_BAD_ALLOC);
  CHECK_ENUM_ELEMENT(ErrorCode, kInvalidParam,                     YOGI_ERR_INVALID_PARAM);
  CHECK_ENUM_ELEMENT(ErrorCode, kInvalidHandle,                    YOGI_ERR_INVALID_HANDLE);
  CHECK_ENUM_ELEMENT(ErrorCode, kWrongObjectType,                  YOGI_ERR_WRONG_OBJECT_TYPE);
  CHECK_ENUM_ELEMENT(ErrorCode, kCanceled,                         YOGI_ERR_CANCELED);
  CHECK_ENUM_ELEMENT(ErrorCode, kBusy,                             YOGI_ERR_BUSY);
  CHECK_ENUM_ELEMENT(ErrorCode, kTimeout,                          YOGI_ERR_TIMEOUT);
  CHECK_ENUM_ELEMENT(ErrorCode, kTimerExpired,                     YOGI_ERR_TIMER_EXPIRED);
  CHECK_ENUM_ELEMENT(ErrorCode, kBufferTooSmall,                   YOGI_ERR_BUFFER_TOO_SMALL);
  CHECK_ENUM_ELEMENT(ErrorCode, kOpenSocketFailed,                 YOGI_ERR_OPEN_SOCKET_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kBindSocketFailed,                 YOGI_ERR_BIND_SOCKET_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kListenSocketFailed,               YOGI_ERR_LISTEN_SOCKET_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kSetSocketOptionFailed,            YOGI_ERR_SET_SOCKET_OPTION_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kInvalidRegex,                     YOGI_ERR_INVALID_REGEX);
  CHECK_ENUM_ELEMENT(ErrorCode, kReadFileFailed,                   YOGI_ERR_READ_FILE_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kRwSocketFailed,                   YOGI_ERR_RW_SOCKET_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kConnectSocketFailed,              YOGI_ERR_CONNECT_SOCKET_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kInvalidMagicPrefix,               YOGI_ERR_INVALID_MAGIC_PREFIX);
  CHECK_ENUM_ELEMENT(ErrorCode, kIncompatibleVersion,              YOGI_ERR_INCOMPATIBLE_VERSION);
  CHECK_ENUM_ELEMENT(ErrorCode, kDeserializeMsgFailed,             YOGI_ERR_DESERIALIZE_MSG_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kAcceptSocketFailed,               YOGI_ERR_ACCEPT_SOCKET_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kLoopbackConnection,               YOGI_ERR_LOOPBACK_CONNECTION);
  CHECK_ENUM_ELEMENT(ErrorCode, kPasswordMismatch,                 YOGI_ERR_PASSWORD_MISMATCH);
  CHECK_ENUM_ELEMENT(ErrorCode, kNetNameMismatch,                  YOGI_ERR_NET_NAME_MISMATCH);
  CHECK_ENUM_ELEMENT(ErrorCode, kDuplicateBranchName,              YOGI_ERR_DUPLICATE_BRANCH_NAME);
  CHECK_ENUM_ELEMENT(ErrorCode, kDuplicateBranchPath,              YOGI_ERR_DUPLICATE_BRANCH_PATH);
  CHECK_ENUM_ELEMENT(ErrorCode, kPayloadTooLarge,                  YOGI_ERR_PAYLOAD_TOO_LARGE);
  CHECK_ENUM_ELEMENT(ErrorCode, kParsingCmdlineFailed,             YOGI_ERR_PARSING_CMDLINE_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kParsingJsonFailed,                YOGI_ERR_PARSING_JSON_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kParsingFileFailed,                YOGI_ERR_PARSING_FILE_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kConfigNotValid,                   YOGI_ERR_CONFIG_NOT_VALID);
  CHECK_ENUM_ELEMENT(ErrorCode, kHelpRequested,                    YOGI_ERR_HELP_REQUESTED);
  CHECK_ENUM_ELEMENT(ErrorCode, kWriteFileFailed,                  YOGI_ERR_WRITE_FILE_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kUndefinedVariables,               YOGI_ERR_UNDEFINED_VARIABLES);
  CHECK_ENUM_ELEMENT(ErrorCode, kNoVariableSupport,                YOGI_ERR_NO_VARIABLE_SUPPORT);
  CHECK_ENUM_ELEMENT(ErrorCode, kVariableUsedInKey,                YOGI_ERR_VARIABLE_USED_IN_KEY);
  CHECK_ENUM_ELEMENT(ErrorCode, kInvalidTimeFormat,                YOGI_ERR_INVALID_TIME_FORMAT);
  CHECK_ENUM_ELEMENT(ErrorCode, kParsingTimeFailed,                YOGI_ERR_PARSING_TIME_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kTxQueueFull,                      YOGI_ERR_TX_QUEUE_FULL);
  CHECK_ENUM_ELEMENT(ErrorCode, kInvalidOperationId,               YOGI_ERR_INVALID_OPERATION_ID);
  CHECK_ENUM_ELEMENT(ErrorCode, kOperationNotRunning,              YOGI_ERR_OPERATION_NOT_RUNNING);
  CHECK_ENUM_ELEMENT(ErrorCode, kInvalidUserMsgPack,               YOGI_ERR_INVALID_USER_MSGPACK);
  CHECK_ENUM_ELEMENT(ErrorCode, kJoinMulticastGroupFailed,         YOGI_ERR_JOIN_MULTICAST_GROUP_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kEnumerateNetworkInterfacesFailed, YOGI_ERR_ENUMERATE_NETWORK_INTERFACES_FAILED);
  CHECK_ENUM_ELEMENT(ErrorCode, kConfigurationSectionNotFound,     YOGI_ERR_CONFIGURATION_SECTION_NOT_FOUND);
  CHECK_ENUM_ELEMENT(ErrorCode, kConfigurationValidationFailed,    YOGI_ERR_CONFIGURATION_VALIDATION_FAILED);
  // clang-format on
}

TEST(ErrorsTest, Result) {
  EXPECT_FALSE(yogi::Result(-1));
  EXPECT_TRUE(yogi::Result(0));
  EXPECT_TRUE(yogi::Result(1));
  EXPECT_FALSE(!yogi::Result(1));
  EXPECT_EQ(yogi::Result(10).GetValue(), 10);
  EXPECT_EQ(yogi::Result(yogi::ErrorCode::kBusy).GetErrorCode(),
            yogi::ErrorCode::kBusy);
  EXPECT_EQ(yogi::Result(10).GetErrorCode(), yogi::ErrorCode::kOk);
  EXPECT_EQ(yogi::Result(3), yogi::Result(3));
  EXPECT_NE(yogi::Result(2), yogi::Result(3));
  EXPECT_GT(yogi::Result(-3).ToString().size(), 5);
  EXPECT_EQ(yogi::Result(0).GetErrorCode(), yogi::ErrorCode::kOk);
  EXPECT_EQ(yogi::Result(0).GetErrorCode(), yogi::Result(1).GetErrorCode());
}

TEST(ErrorsTest, Failure) {
  EXPECT_FALSE(yogi::Failure(yogi::ErrorCode::kBadAlloc));
  EXPECT_EQ(yogi::Failure(yogi::ErrorCode::kBadAlloc).ToString(),
            yogi::Result(yogi::ErrorCode::kBadAlloc).ToString());
  EXPECT_EQ(yogi::Failure(yogi::ErrorCode::kBadAlloc),
            yogi::Result(yogi::ErrorCode::kBadAlloc));
  EXPECT_TRUE((std::is_base_of<yogi::Result, yogi::Failure>::value));
}

TEST(ErrorsTest, DescriptiveFailure) {
  EXPECT_EQ(
      yogi::DescriptiveFailure(yogi::ErrorCode::kBusy, "ab").GetDescription(),
      "ab");
  EXPECT_TRUE(
      (std::is_base_of<yogi::Failure, yogi::DescriptiveFailure>::value));
  EXPECT_NE(yogi::DescriptiveFailure(yogi::ErrorCode::kBusy, "xyz")
                .ToString()
                .find(yogi::Failure(yogi::ErrorCode::kBusy).ToString()),
            std::string::npos);
  EXPECT_NE(yogi::DescriptiveFailure(yogi::ErrorCode::kBusy, "xyz")
                .ToString()
                .find("xyz"),
            std::string::npos);
}

TEST(ErrorsTest, Exception) {
  EXPECT_TRUE((std::is_base_of<std::exception, yogi::Exception>::value));
}

TEST(ErrorsTest, FailureException) {
  EXPECT_TRUE(
      (std::is_base_of<yogi::Exception, yogi::FailureException>::value));
  EXPECT_EQ(yogi::FailureException(yogi::ErrorCode::kBusy).what(),
            yogi::Failure(yogi::ErrorCode::kBusy).ToString());
  EXPECT_EQ(yogi::FailureException(yogi::ErrorCode::kBusy).GetFailure(),
            yogi::Failure(yogi::ErrorCode::kBusy));
}

TEST(ErrorsTest, DescriptiveFailureException) {
  EXPECT_TRUE((std::is_base_of<yogi::FailureException,
                               yogi::DescriptiveFailureException>::value));
  EXPECT_EQ(
      yogi::DescriptiveFailureException(yogi::ErrorCode::kBusy, "xyz").what(),
      yogi::DescriptiveFailure(yogi::ErrorCode::kBusy, "xyz").ToString());
  EXPECT_EQ(yogi::DescriptiveFailureException(yogi::ErrorCode::kBusy, "xyz")
                .GetFailure(),
            yogi::DescriptiveFailure(yogi::ErrorCode::kBusy, "xyz"));
}

TEST(ErrorsTest, Success) {
  EXPECT_TRUE(yogi::Success(0));
  EXPECT_TRUE(yogi::Success(1));
  EXPECT_EQ(yogi::Success(1).ToString(), yogi::Result(1).ToString());
  EXPECT_EQ(yogi::Success(1), yogi::Result(1));
  EXPECT_TRUE((std::is_base_of<yogi::Result, yogi::Success>::value));
}

TEST(ErrorsTest, ArithmeticException) {
  EXPECT_TRUE(
      (std::is_base_of<yogi::Exception, yogi::ArithmeticException>::value));
  EXPECT_STREQ(yogi::ArithmeticException("abc").what(), "abc");
}
