# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2018 Johannes Bergmann.
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this library. If not, see <http://www.gnu.org/licenses/>.

from .library import yogi

from ctypes import c_char_p, c_int, create_string_buffer
from enum import IntEnum
from typing import Callable, Union


class ErrorCode(IntEnum):
    """Error codes returned by functions from the Yogi Core library."""
    OK = 0
    UNKNOWN = -1
    OBJECT_STILL_USED = -2
    BAD_ALLOC = -3
    INVALID_PARAM = -4
    INVALID_HANDLE = -5
    WRONG_OBJECT_TYPE = -6
    CANCELED = -7
    BUSY = -8
    TIMEOUT = -9
    TIMER_EXPIRED = -10
    BUFFER_TOO_SMALL = -11
    OPEN_SOCKET_FAILED = -12
    BIND_SOCKET_FAILED = -13
    LISTEN_SOCKET_FAILED = -14
    SET_SOCKET_OPTION_FAILED = -15
    INVALID_REGEX = -16
    OPEN_FILE_FAILED = -17
    RW_SOCKET_FAILED = -18
    CONNECT_SOCKET_FAILED = -19
    INVALID_MAGIC_PREFIX = -20
    INCOMPATIBLE_VERSION = -21
    DESERIALIZE_MSG_FAILED = -22
    ACCEPT_SOCKET_FAILED = -23
    LOOPBACK_CONNECTION = -24
    PASSWORD_MISMATCH = -25
    NET_NAME_MISMATCH = -26
    DUPLICATE_BRANCH_NAME = -27
    DUPLICATE_BRANCH_PATH = -28
    PAYLOAD_TOO_LARGE = -29
    PARSING_CMDLINE_FAILED = -30
    PARSING_JSON_FAILED = -31
    PARSING_FILE_FAILED = -32
    CONFIG_NOT_VALID = -33
    HELP_REQUESTED = -34
    WRITE_TO_FILE_FAILED = -35
    UNDEFINED_VARIABLES = -36
    NO_VARIABLE_SUPPORT = -37
    VARIABLE_USED_IN_KEY = -38
    INVALID_TIME_FORMAT = -39
    PARSING_TIME_FAILED = -40
    TX_QUEUE_FULL = -41
    INVALID_OPERATION_ID = -42
    OPERATION_NOT_RUNNING = -43
    INVALID_USER_MSGPACK = -44
    JOIN_MULTICAST_GROUP_FAILED = -45
    ENUMERATE_NETWORK_INTERFACES_FAILED = -46
    CONFIGURATION_SECTION_NOT_FOUND = -47


yogi.YOGI_GetErrorString.restype = c_char_p
yogi.YOGI_GetErrorString.argtypes = [c_int]


class Result:
    """Represents a result of an operation.

    This is a wrapper around the result code returned by the functions from
    the Yogi Core library. A result is represented by a number which is >= 0
    in case of success and < 0 in case of a failure.
    """

    def __init__(self, value_or_ec: Union[int, ErrorCode]):
        """Constructs the result.

        Args:
            value: Number as returned by the Yogi Core library functions.
        """
        if isinstance(value_or_ec, int):
            self._value = value_or_ec
        else:
            self._value = value_or_ec.value

    @property
    def value(self) -> int:
        """The number as returned by the Yogi Core library functions."""
        return self._value

    @property
    def error_code(self) -> ErrorCode:
        """Error code associated with this result."""
        return ErrorCode(self._value) if self._value < 0 else ErrorCode.OK

    def __bool__(self) -> bool:
        return self._value >= 0

    def __eq__(self, other) -> bool:
        return isinstance(other, Result) and self._value == other.value

    def __ne__(self, other) -> bool:
        return not (self == other)

    def __str__(self) -> str:
        return yogi.YOGI_GetErrorString(self._value).decode("utf-8")

    def __hash__(self) -> int:
        return hash(self._value)


class Success(Result):
    """Represents the success of an operation."""

    def __init__(self, value: int = 0):
        assert value >= 0
        Result.__init__(self, value)


class Failure(Result):
    """Represents the failure of an operation."""

    def __init__(self, ec: ErrorCode):
        """Constructs the Failure.

        Args:
            ec: Error code.
        """
        Result.__init__(self, ec.value)

    def __str__(self) -> str:
        return Result.__str__(self)


class DescriptiveFailure(Failure):
    """A failure of an operation that includes a description."""

    def __init__(self, ec: ErrorCode, description: str):
        """Constructs the DescriptiveFailure.

        Args:
            ec: Error code.
            description: Additional information about the failure.
        """
        Failure.__init__(self, ec)
        self._description = description

    @property
    def description(self) -> str:
        """More detailed information about the error."""
        return self._description

    def __str__(self) -> str:
        return Failure.__str__(self) + ". Description: " + self._description


class Exception(Exception):
    """Base class for all Yogi exceptions."""
    @property
    def failure(self) -> Failure:
        """Wrapped Failure or DescriptiveFailure object."""
        raise NotImplementedError()


class FailureException(Exception):
    """Exception wrapping a Failure object.

    This exception type is used for failures without a detailed description.
    """

    def __init__(self, ec: ErrorCode):
        """Constructs the exception.

        Args:
            ec: Error code.
        """
        self._failure = Failure(ec)

    @property
    def failure(self) -> Failure:
        """Wrapped Failure object."""
        return self._failure


class DescriptiveFailureException(FailureException):
    """Exception wrapping a DescriptiveFailure object.

    This exception type is used for failures that have detailed information
    available
    """

    def __init__(self, ec: ErrorCode, description: str):
        """Constructs the exception.

        Args:
            ec: Error code.
            description: Additional information about the failure.
        """
        self._failure = DescriptiveFailure(ec, description)

    @property
    def failure(self) -> DescriptiveFailure:
        """Wrapped DescriptiveFailure object."""
        return self._failure


def api_result_handler(res: int) -> Success:
    if res < 0:
        raise FailureException(ErrorCode(res))
    else:
        return Success(res)


def error_code_to_result(res: int) -> Result:
    if res < 0:
        return Failure(ErrorCode(res))
    else:
        return Success(res)


def false_if_specific_ec_else_raise(res: int, ec: ErrorCode) -> Result:
    if res == ec:
        return False
    elif res < 0:
        raise FailureException(ErrorCode(res))
    else:
        return True


def run_with_discriptive_failure_awareness(fn: Callable[[any], int]) -> None:
    err = create_string_buffer(256)
    res = fn(err)
    if res < 0:
        description = err.value.decode()
        if len(description):
            raise DescriptiveFailureException(ErrorCode(res), description)
        else:
            raise FailureException(ErrorCode(res))
