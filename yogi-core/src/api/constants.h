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

#pragma once

#include "../config.h"

namespace api {

// clang-format off
#define SCC static constexpr const
SCC char*     kVersionNumber                 = YOGI_HDR_VERSION;
SCC int       kVersionMajor                  = YOGI_HDR_VERSION_MAJOR;
SCC int       kVersionMinor                  = YOGI_HDR_VERSION_MINOR;
SCC int       kVersionPatch                  = YOGI_HDR_VERSION_PATCH;
SCC char*     kDefaultAdvInterfaces          = "[\"localhost\"]";
SCC char*     kDefaultAdvAddress             = "ff02::8000:2439";
SCC int       kDefaultAdvPort                = 13531;
SCC long long kDefaultAdvInterval            = 1'000'000'000;
SCC long long kDefaultConnectionTimeout      = 3'000'000'000;
SCC int       kDefaultLoggerVerbosity        = YOGI_VB_INFO;
SCC char*     kDefaultLogTimeFormat          = "%F %T.%3";
SCC char*     kDefaultLogFormat              = "$t [T$T] $<$s $c: $m$>";
SCC int       kMaxMessagePayloadSize         = 32'768;
SCC char*     kDefaultTimeFormat             = "%FT%T.%3Z";
SCC char*     kDefaultInfiniteDurationString = "%-inf";
SCC char*     kDefaultDurationFormat         = "%-%dd %T.%3%6%9";
SCC char*     kDefaultInvalidHandleString    = "INVALID HANDLE";
SCC char*     kDefaultObjectFormat           = "$T [$x]";
SCC int       kMinTxQueueSize                = 35'000;
SCC int       kMaxTxQueueSize                = 10'000'000;
SCC int       kDefaultTxQueueSize            = kMinTxQueueSize;
SCC int       kMinRxQueueSize                = 35'000;
SCC int       kMaxRxQueueSize                = 10'000'000;
SCC int       kDefaultRxQueueSize            = kMinRxQueueSize;
#undef SCC
// clang-format on

void GetConstant(void* dest, int constant);

}  // namespace api
