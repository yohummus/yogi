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

#ifndef YOGI_CONSTANTS_H
#define YOGI_CONSTANTS_H

//! \file
//!
//! Constants.

#include "logging.h"
#include "duration.h"
#include "internal/error_code_helpers.h"
#include "internal/library.h"

#include <string>
#include <chrono>

namespace yogi {

_YOGI_DEFINE_API_FN(int, YOGI_GetConstant, (void* dest, int constant))

namespace internal {

template <typename T>
inline T GetConstant(int constant) {
  T val;
  int res = YOGI_GetConstant(&val, constant);
  CheckErrorCode(res);
  return val;
}

}  // namespace internal

////////////////////////////////////////////////////////////////////////////////
/// Constants built into the Yogi Core library.
////////////////////////////////////////////////////////////////////////////////
struct constants {
  /// Complete Yogi Core version string.
  static const std::string kVersion;

  /// Yogi Core major version number.
  static const int kVersionMajor;

  /// Yogi Core minor version number.
  static const int kVersionMinor;

  /// Yogi Core patch version number.
  static const int kVersionPatch;

  /// Yogi Core patch version number.
  static const std::string kVersionSuffix;

  /// Default network interfaces to use for advertising.
  static const std::string kDefaultAdvInterfaces;

  /// Default IP address for advertising.
  static const std::string kDefaultAdvAddress;

  /// Default UDP port for advertising.
  static const int kDefaultAdvPort;

  /// Default time between two advertising messages.
  static const Duration kDefaultAdvInterval;

  /// Default timeout for connections between two branches.
  static const Duration kDefaultConnectionTimeout;

  /// Default verbosity for newly created loggers.
  static const Verbosity kDefaultLoggerVerbosity;

  /// Default format of the time string in log entries.
  static const std::string kDefaultLogTimeFormat;

  /// Default format of a log entry.
  static const std::string kDefaultLogFormat;

  /// Maximum size of the payload of a message between two branches.
  static const int kMaxMessagePayloadSize;

  /// Default textual format for timestamps.
  static const std::string kDefaultTimeFormat;

  /// Default string to denote an infinite duration.
  static const std::string kDefaultInfiniteDurationString;

  /// Default textual format for duration strings.
  static const std::string kDefaultDurationFormat;

  /// Default string to denote an invalid object handle.
  static const std::string kDefaultInvalidHandleString;

  /// Default textual format for strings describing an object.
  static const std::string kDefaultObjectFormat;

  /// Minimum size of a send queue for a remote branch (int).
  static const int kMinTxQueueSize;

  /// Maximum size of a send queue for a remote branch (int).
  static const int kMaxTxQueueSize;

  /// Default size of a send queue for a remote branch (int).
  static const int kDefaultTxQueueSize;

  /// Minimum size of a receive queue for a remote branch (int).
  static const int kMinRxQueueSize;

  /// Maximum size of a receive queue for a remote branch (int).
  static const int kMaxRxQueueSize;

  /// Default size of a receive queue for a remote branch (int).
  static const int kDefaultRxQueueSize;
};

_YOGI_WEAK_SYMBOL const std::string constants::kVersion =
    internal::GetConstant<char*>(1);

_YOGI_WEAK_SYMBOL const int constants::kVersionMajor =
    internal::GetConstant<int>(2);

_YOGI_WEAK_SYMBOL const int constants::kVersionMinor =
    internal::GetConstant<int>(3);

_YOGI_WEAK_SYMBOL const int constants::kVersionPatch =
    internal::GetConstant<int>(4);

_YOGI_WEAK_SYMBOL const std::string constants::kVersionSuffix =
    internal::GetConstant<char*>(5);

_YOGI_WEAK_SYMBOL const std::string constants::kDefaultAdvInterfaces =
    internal::GetConstant<char*>(6);

_YOGI_WEAK_SYMBOL const std::string constants::kDefaultAdvAddress =
    internal::GetConstant<char*>(7);

_YOGI_WEAK_SYMBOL const int constants::kDefaultAdvPort =
    internal::GetConstant<int>(8);

_YOGI_WEAK_SYMBOL const Duration constants::kDefaultAdvInterval =
    Duration::FromNanoseconds(internal::GetConstant<long long>(9));

_YOGI_WEAK_SYMBOL const Duration constants::kDefaultConnectionTimeout =
    Duration::FromNanoseconds(internal::GetConstant<long long>(10));

_YOGI_WEAK_SYMBOL const Verbosity constants::kDefaultLoggerVerbosity =
    static_cast<Verbosity>(internal::GetConstant<int>(11));

_YOGI_WEAK_SYMBOL const std::string constants::kDefaultLogTimeFormat =
    internal::GetConstant<char*>(12);

_YOGI_WEAK_SYMBOL const std::string constants::kDefaultLogFormat =
    internal::GetConstant<char*>(13);

_YOGI_WEAK_SYMBOL const int constants::kMaxMessagePayloadSize =
    internal::GetConstant<int>(14);

_YOGI_WEAK_SYMBOL const std::string constants::kDefaultTimeFormat =
    internal::GetConstant<char*>(15);

_YOGI_WEAK_SYMBOL const std::string constants::kDefaultInfiniteDurationString =
    internal::GetConstant<char*>(16);

_YOGI_WEAK_SYMBOL const std::string constants::kDefaultDurationFormat =
    internal::GetConstant<char*>(17);

_YOGI_WEAK_SYMBOL const std::string constants::kDefaultInvalidHandleString =
    internal::GetConstant<char*>(18);

_YOGI_WEAK_SYMBOL const std::string constants::kDefaultObjectFormat =
    internal::GetConstant<char*>(19);

_YOGI_WEAK_SYMBOL const int constants::kMinTxQueueSize =
    internal::GetConstant<int>(20);

_YOGI_WEAK_SYMBOL const int constants::kMaxTxQueueSize =
    internal::GetConstant<int>(21);

_YOGI_WEAK_SYMBOL const int constants::kDefaultTxQueueSize =
    internal::GetConstant<int>(22);

_YOGI_WEAK_SYMBOL const int constants::kMinRxQueueSize =
    internal::GetConstant<int>(23);

_YOGI_WEAK_SYMBOL const int constants::kMaxRxQueueSize =
    internal::GetConstant<int>(24);

_YOGI_WEAK_SYMBOL const int constants::kDefaultRxQueueSize =
    internal::GetConstant<int>(25);

}  // namespace yogi

#endif  // YOGI_CONSTANTS_H
