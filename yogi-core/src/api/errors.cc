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

#include "errors.h"

namespace api {

Error Result::ToError() const {
  YOGI_ASSERT(IsError());
  return Error(ec_);
}

const char* Result::GetDescription() const {
  switch (GetErrorCode()) {
    case YOGI_OK:
      return "Success";

    case YOGI_ERR_UNKNOWN:
      return "Unknown internal error";

    case YOGI_ERR_OBJECT_STILL_USED:
      return "Object is still being used by another object";

    case YOGI_ERR_BAD_ALLOC:
      return "Memory allocation failed";

    case YOGI_ERR_INVALID_PARAM:
      return "Invalid parameter";

    case YOGI_ERR_INVALID_HANDLE:
      return "Invalid handle";

    case YOGI_ERR_WRONG_OBJECT_TYPE:
      return "Wrong object type";

    case YOGI_ERR_CANCELED:
      return "Operation has been canceled";

    case YOGI_ERR_BUSY:
      return "Object is busy";

    case YOGI_ERR_TIMEOUT:
      return "The operation timed out";

    case YOGI_ERR_TIMER_EXPIRED:
      return "The timer has not been started or already expired";

    case YOGI_ERR_BUFFER_TOO_SMALL:
      return "The supplied buffer is too small";

    case YOGI_ERR_OPEN_SOCKET_FAILED:
      return "Could not open a socket";

    case YOGI_ERR_BIND_SOCKET_FAILED:
      return "Could not bind a socket";

    case YOGI_ERR_LISTEN_SOCKET_FAILED:
      return "Could not listen on socket";

    case YOGI_ERR_SET_SOCKET_OPTION_FAILED:
      return "Could not set a socket option";

    case YOGI_ERR_INVALID_REGEX:
      return "Invalid regular expression";

    case YOGI_ERR_OPEN_FILE_FAILED:
      return "Could not open file";

    case YOGI_ERR_RW_SOCKET_FAILED:
      return "Could not read from or write to socket";

    case YOGI_ERR_CONNECT_SOCKET_FAILED:
      return "Could not connect a socket";

    case YOGI_ERR_INVALID_MAGIC_PREFIX:
      return "The magic prefix sent when establishing a connection is wrong";

    case YOGI_ERR_INCOMPATIBLE_VERSION:
      return "The local and remote branches use incompatible Yogi versions";

    case YOGI_ERR_DESERIALIZE_MSG_FAILED:
      return "Could not deserialize a message";

    case YOGI_ERR_ACCEPT_SOCKET_FAILED:
      return "Could not accept a socket";

    case YOGI_ERR_LOOPBACK_CONNECTION:
      return "Attempting to connect branch to itself";

    case YOGI_ERR_PASSWORD_MISMATCH:
      return "The passwords of the local and remote branch don't match";

    case YOGI_ERR_NET_NAME_MISMATCH:
      return "The net names of the local and remote branch don't match";

    case YOGI_ERR_DUPLICATE_BRANCH_NAME:
      return "A branch with the same name is already active";

    case YOGI_ERR_DUPLICATE_BRANCH_PATH:
      return "A branch with the same path is already active";

    case YOGI_ERR_PAYLOAD_TOO_LARGE:
      return "Message payload is too large";

    case YOGI_ERR_PARSING_CMDLINE_FAILED:
      return "Parsing the command line failed";

    case YOGI_ERR_PARSING_JSON_FAILED:
      return "Parsing a JSON string failed";

    case YOGI_ERR_PARSING_FILE_FAILED:
      return "Parsing a configuration file failed";

    case YOGI_ERR_CONFIG_NOT_VALID:
      return "The configuration is not valid";

    case YOGI_ERR_HELP_REQUESTED:
      return "Help/usage text requested";

    case YOGI_ERR_WRITE_TO_FILE_FAILED:
      return "Could not write to file";

    case YOGI_ERR_UNDEFINED_VARIABLES:
      return "One or more configuration variables are undefined or could not "
             "be resolved";

    case YOGI_ERR_NO_VARIABLE_SUPPORT:
      return "Support for configuration variables has been disabled";

    case YOGI_ERR_VARIABLE_USED_IN_KEY:
      return "A configuration variable has been used in a key";

    case YOGI_ERR_INVALID_TIME_FORMAT:
      return "Invalid time format";

    case YOGI_ERR_PARSING_TIME_FAILED:
      return "Could not parse time string";

    case YOGI_ERR_TX_QUEUE_FULL:
      return "A send queue for a remote branch is full";

    case YOGI_ERR_INVALID_OPERATION_ID:
      return "Invalid operation ID";

    case YOGI_ERR_OPERATION_NOT_RUNNING:
      return "Operation is not running";

    case YOGI_ERR_INVALID_USER_MSGPACK:
      return "User-supplied data is not valid MessagePack";

    case YOGI_ERR_JOIN_MULTICAST_GROUP_FAILED:
      return "Joining UDP multicast group failed";

    case YOGI_ERR_ENUMERATE_NETWORK_INTERFACES_FAILED:
      return "Enumerating network interfaces failed";
  }

  return "Invalid error code";
}

const char* Error::what() const noexcept { return GetDescription(); }

}  // namespace api

std::ostream& operator<<(std::ostream& os, const api::Result& res) {
  os << res.GetDescription();
  return os;
}

std::ostream& operator<<(std::ostream& os, const api::Error& err) {
  return os << static_cast<const api::Result&>(err);
}

std::ostream& operator<<(std::ostream& os, const api::DescriptiveError& err) {
  return os << static_cast<const api::Result&>(err);
}
