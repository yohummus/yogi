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

#include "constants.h"
#include "errors.h"

namespace api {

void GetConstant(void* dest, int constant) {
  switch (constant) {
    case YOGI_CONST_VERSION:
      *static_cast<const char**>(dest) = kVersion;
      break;

    case YOGI_CONST_VERSION_MAJOR:
      *static_cast<int*>(dest) = kVersionMajor;
      break;

    case YOGI_CONST_VERSION_MINOR:
      *static_cast<int*>(dest) = kVersionMinor;
      break;

    case YOGI_CONST_VERSION_PATCH:
      *static_cast<int*>(dest) = kVersionPatch;
      break;

    case YOGI_CONST_VERSION_SUFFIX:
      *static_cast<const char**>(dest) = kVersionSuffix;
      break;

    case YOGI_CONST_DEFAULT_ADV_INTERFACES:
      *static_cast<const char**>(dest) = kDefaultAdvInterfaces;
      break;

    case YOGI_CONST_DEFAULT_ADV_ADDRESS:
      *static_cast<const char**>(dest) = kDefaultAdvAddress;
      break;

    case YOGI_CONST_DEFAULT_ADV_PORT:
      *static_cast<int*>(dest) = kDefaultAdvPort;
      break;

    case YOGI_CONST_DEFAULT_ADV_INTERVAL:
      // NOLINTNEXTLINE(google-runtime-int)
      *static_cast<long long*>(dest) = kDefaultAdvInterval;
      break;

    case YOGI_CONST_DEFAULT_CONNECTION_TIMEOUT:
      // NOLINTNEXTLINE(google-runtime-int)
      *static_cast<long long*>(dest) = kDefaultConnectionTimeout;
      break;

    case YOGI_CONST_DEFAULT_LOGGER_VERBOSITY:
      *static_cast<int*>(dest) = kDefaultLoggerVerbosity;
      break;

    case YOGI_CONST_DEFAULT_LOG_TIME_FORMAT:
      *static_cast<const char**>(dest) = kDefaultLogTimeFormat;
      break;

    case YOGI_CONST_DEFAULT_LOG_FORMAT:
      *static_cast<const char**>(dest) = kDefaultLogFormat;
      break;

    case YOGI_CONST_MAX_MESSAGE_PAYLOAD_SIZE:
      *static_cast<int*>(dest) = kMaxMessagePayloadSize;
      break;

    case YOGI_CONST_DEFAULT_TIME_FORMAT:
      *static_cast<const char**>(dest) = kDefaultTimeFormat;
      break;

    case YOGI_CONST_DEFAULT_INF_DURATION_STRING:
      *static_cast<const char**>(dest) = kDefaultInfiniteDurationString;
      break;

    case YOGI_CONST_DEFAULT_DURATION_FORMAT:
      *static_cast<const char**>(dest) = kDefaultDurationFormat;
      break;

    case YOGI_CONST_DEFAULT_INVALID_HANDLE_STRING:
      *static_cast<const char**>(dest) = kDefaultInvalidHandleString;
      break;

    case YOGI_CONST_DEFAULT_OBJECT_FORMAT:
      *static_cast<const char**>(dest) = kDefaultObjectFormat;
      break;

    case YOGI_CONST_MIN_TX_QUEUE_SIZE:
      *static_cast<int*>(dest) = kMinTxQueueSize;
      break;

    case YOGI_CONST_MAX_TX_QUEUE_SIZE:
      *static_cast<int*>(dest) = kMaxTxQueueSize;
      break;

    case YOGI_CONST_DEFAULT_TX_QUEUE_SIZE:
      *static_cast<int*>(dest) = kDefaultTxQueueSize;
      break;

    case YOGI_CONST_MIN_RX_QUEUE_SIZE:
      *static_cast<int*>(dest) = kMinRxQueueSize;
      break;

    case YOGI_CONST_MAX_RX_QUEUE_SIZE:
      *static_cast<int*>(dest) = kMaxRxQueueSize;
      break;

    case YOGI_CONST_DEFAULT_RX_QUEUE_SIZE:
      *static_cast<int*>(dest) = kDefaultRxQueueSize;
      break;

    case YOGI_CONST_DEFAULT_WEB_PORT:
      *static_cast<int*>(dest) = kDefaultWebPort;
      break;

    case YOGI_CONST_DEFAULT_WEB_INTERFACES:
      *static_cast<const char**>(dest) = kDefaultWebInterfaces;
      break;

    case YOGI_CONST_DEFAULT_WEB_TIMEOUT:
      // NOLINTNEXTLINE(google-runtime-int)
      *static_cast<long long*>(dest) = kDefaultWebTimeout;
      break;

    case YOGI_CONST_DEFAULT_WEB_CACHE_SIZE:
      *static_cast<int*>(dest) = kDefaultWebCacheSize;
      break;

    case YOGI_CONST_MAX_WEB_CACHE_SIZE:
      *static_cast<int*>(dest) = kMaxWebCacheSize;
      break;

    case YOGI_CONST_DEFAULT_ADMIN_USER:
      *static_cast<const char**>(dest) = kDefaultAdminUser;
      break;

    case YOGI_CONST_DEFAULT_ADMIN_PASSWORD:
      *static_cast<const char**>(dest) = kDefaultAdminPassword;
      break;

    case YOGI_CONST_DEFAULT_SSL_PRIVATE_KEY:
      *static_cast<const char**>(dest) = kDefaultSslPrivateKey;
      break;

    case YOGI_CONST_DEFAULT_SSL_CERTIFICATE_CHAIN:
      *static_cast<const char**>(dest) = kDefaultSslCertificateChain;
      break;

    case YOGI_CONST_DEFAULT_SSL_DH_PARAMS:
      *static_cast<const char**>(dest) = kDefaultSslDhParams;
      break;

    default:
      throw Error(YOGI_ERR_INVALID_PARAM);
  }
}

}  // namespace api
