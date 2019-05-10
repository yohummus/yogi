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

from .errors import api_result_handler
from .library import yogi

import datetime
import math
from ctypes import c_longlong, c_char_p, c_int, byref, POINTER, \
    create_string_buffer, sizeof
from typing import Union, Optional


def add_safely(a: int, b: int) -> int:
    if a > 0 and b > 9223372036854775807 - a:
        raise OverflowError("Duration value overflow")
    elif a < 0 and b < -9223372036854775808 - a:
        raise OverflowError("Duration value underflow")

    return a + b


def multiply_safely(val: int, multiplicator: float) -> int:
    if math.isnan(multiplicator):
        raise ArithmeticError("Trying to multiply duration value and NaN")

    if multiplicator == 0:
        return 0

    if abs(multiplicator) > 1:
        max_val = int(9223372036854775807 / multiplicator)
        if abs(val) > max_val:
            raise OverflowError("Duration value overflow")

    return int(val * multiplicator)


def check_divisor(divisor: float) -> None:
    if math.isnan(divisor):
        raise ArithmeticError("Trying to divide duration value by NaN")

    if divisor == 0:
        raise ZeroDivisionError("Trying to divide duration value by zero")


def divide_safetly(val: int, divisor: float) -> int:
    check_divisor(divisor)

    if math.isinf(divisor):
        return 0

    if abs(divisor) < 1:
        max_val = int(9223372036854775807 * divisor)
        if abs(val) > max_val:
            raise OverflowError("Duration value overflow")

    return int(val / divisor)


yogi.YOGI_FormatDuration.restype = api_result_handler
yogi.YOGI_FormatDuration.argtypes = [c_longlong, c_int, c_char_p, c_int,
                                     c_char_p, c_char_p]


class MetaDuration(type):
    @property
    def zero(self) -> 'Duration':
        """Zero duration."""
        return Duration()

    @property
    def infinity(self) -> 'Duration':
        """Infinite duration."""
        return Duration.from_nanoseconds(math.inf)

    @property
    def negative_infinity(self) -> 'Duration':
        """Negative infinite duration."""
        return Duration.from_nanoseconds(-math.inf)


class Duration(metaclass=MetaDuration):
    """Represents a time duration.

    The resolution of a time duration is nanoseconds. Durations can be
    positive or negative. Exceptions are thrown in case of arithmetic errors.
    """
    @property
    def zero(self) -> 'Duration':
        """Zero duration."""
        return type(self).zero

    @property
    def infinity(self) -> 'Duration':
        """Infinite duration."""
        return type(self).infinity

    @property
    def negative_infinity(self) -> 'Duration':
        """Negative infinite duration."""
        return type(self).negative_infinity

    @classmethod
    def from_nanoseconds(cls, ns: Union[int, float]) -> 'Duration':
        """Constructs a duration from a number of nanoseconds.

        Args:
            ns: Number of nanoseconds.

        Returns:
            Duration instance.
        """
        return cls.__from_time_unit(1, ns)

    @classmethod
    def from_microseconds(cls, us: Union[int, float]) -> 'Duration':
        """Constructs a duration from a number of microseconds.

        Args:
            us: Number of microseconds.

        Returns:
            Duration instance.
        """
        return cls.__from_time_unit(1000, us)

    @classmethod
    def from_milliseconds(cls, ms: Union[int, float]) -> 'Duration':
        """Constructs a duration from a number of milliseconds.

        Args:
            ms: Number of milliseconds.

        Returns:
            Duration instance.
        """
        return cls.__from_time_unit(1000000, ms)

    @classmethod
    def from_seconds(cls, seconds: Union[int, float]) -> 'Duration':
        """Constructs a duration from a number of seconds.

        Args:
            seconds: Number of seconds.

        Returns:
            Duration instance.
        """
        return cls.__from_time_unit(1000000000, seconds)

    @classmethod
    def from_minutes(cls, minutes: Union[int, float]) -> 'Duration':
        """Constructs a duration from a number of minutes.

        Args:
            minutes: Number of minutes.

        Returns:
            Duration instance.
        """
        return cls.__from_time_unit(60000000000, minutes)

    @classmethod
    def from_hours(cls, hours: Union[int, float]) -> 'Duration':
        """Constructs a duration from a number of hours.

        Args:
            hours: Number of hours.

        Returns:
            Duration instance.
        """
        return cls.__from_time_unit(3600000000000, hours)

    @classmethod
    def from_days(cls, days: Union[int, float]) -> 'Duration':
        """Constructs a duration from a number of days.

        Args:
            days: Number of days.

        Returns:
            Duration instance.
        """
        return cls.__from_time_unit(86400000000000, days)

    def __init__(self, delta: datetime.timedelta = None):
        """Constructs a duration.

        Args:
            delta: Time delta to use or None for zero duration.
        """
        if delta is None:
            self._ns_count = 0
        else:
            self._ns_count = int(delta.total_seconds() * 1e9)

        self._inf_type = 0  # -1, 0 or +1

    @property
    def nanoseconds(self) -> int:
        """Nanoseconds fraction (0-999) of the duration.

        If the duration is infinite, this value will be undefined.
        """
        return self.nanoseconds_count % 1000

    @property
    def microseconds(self) -> int:
        """Microseconds fraction (0-999) of the duration.

        If the duration is infinite, this value will be undefined.
        """
        return (self.nanoseconds_count // 1000) % 1000

    @property
    def milliseconds(self) -> int:
        """Milliseconds fraction (0-999) of the duration.

        If the duration is infinite, this value will be undefined.
        """
        return (self.nanoseconds_count // 1000000) % 1000

    @property
    def seconds(self) -> int:
        """Seconds fraction (0-59) of the duration.

        If the duration is infinite, this value will be undefined.
        """
        return (self.nanoseconds_count // 1000000000) % 60

    @property
    def minutes(self) -> int:
        """Minutes fraction (0-59) of the duration.

        If the duration is infinite, this value will be undefined.
        """
        return (self.nanoseconds_count // 60000000000) % 60

    @property
    def hours(self) -> int:
        """Hours fraction (0-24) of the duration.

        If the duration is infinite, this value will be undefined.
        """
        return (self.nanoseconds_count // 3600000000000) % 24

    @property
    def days(self) -> int:
        """Hours fraction (0-24) of the duration.

        If the duration is infinite, this value will be undefined.
        """
        return self.nanoseconds_count // 86400000000000

    @property
    def nanoseconds_count(self) -> int:
        """Total number of nanoseconds.

        If the duration is infinite, this value will be undefined.
        """
        return self._ns_count

    @property
    def total_nanoseconds(self) -> float:
        """Total number of nanoseconds.

        If the duration is infinite, this value will be the appropriate
        infinite value.
        """
        return self.__to_total_time_unit(1)

    @property
    def total_microseconds(self) -> float:
        """Total number of microseconds.

        If the duration is infinite, this value will be the appropriate
        infinite value.
        """
        return self.__to_total_time_unit(1000)

    @property
    def total_milliseconds(self) -> float:
        """Total number of milliseconds.

        If the duration is infinite, this value will be the appropriate
        infinite value.
        """
        return self.__to_total_time_unit(1000000)

    @property
    def total_seconds(self) -> float:
        """Total number of seconds.

        If the duration is infinite, this value will be the appropriate
        infinite value.
        """
        return self.__to_total_time_unit(1000000000)

    @property
    def total_minutes(self) -> float:
        """Total number of minutes.

        If the duration is infinite, this value will be the appropriate
        infinite value.
        """
        return self.__to_total_time_unit(60000000000)

    @property
    def total_hours(self) -> float:
        """Total number of hours.

        If the duration is infinite, this value will be the appropriate
        infinite value.
        """
        return self.__to_total_time_unit(3600000000000)

    @property
    def total_days(self) -> float:
        """Total number of days.

        If the duration is infinite, this value will be the appropriate
        infinite value.
        """
        return self.__to_total_time_unit(86400000000000)

    @property
    def negated(self) -> 'Duration':
        """The negated duration, i.e. multiplied by -1."""
        if self._inf_type < 0:
            return self.infinity
        elif self._inf_type > 0:
            return self.negative_infinity
        else:
            return self.from_nanoseconds(-self._ns_count)

    def to_timedelta(self) -> datetime.timedelta:
        """Converts the duration to a datetime.timedelta object.

        In case that the duration is infinite, the respective min or max value
        of the timedelta will be returned.
        """
        if self._inf_type < 0:
            return datetime.timedelta.min
        elif self._inf_type > 0:
            return datetime.timedelta.max
        else:
            ms = int(self.total_microseconds)
            return datetime.timedelta(microseconds=int(ms))

    @property
    def is_finite(self) -> bool:
        """True if the duration is finite and false otherwise."""
        return self._inf_type == 0

    def format(self, dur_fmt: str = None, inf_fmt: str = None) -> str:
        """Converts the duration to a string.

        The dur_fmt parameter describes the format of the conversion. The
        following placeholders are supported:
            *%+*: Plus sign if duration is positive; minus sign if negative
            *%-*: Minus sign (only) if duration is negative
            *%d*: Total number of days
            *%D*: Total number of days if days > 0
            *%H*: Fractional hours (range 00 to 23)
            *%M*: Fractional minutes (range 00 to 59)
            *%S*: Fractional seconds (range 00 to 59)
            *%T*: Equivalent to %H:%M:%S
            *%3*: Fractional milliseconds (range 000 to 999)
            *%6*: Fractional microseconds (range 000 to 999)
            *%9*: Fractional nanoseconds (range 000 to 999)

        The inf_fmt parameter describes the format to use for infinite
        durations. The following placeholders are supported:
            *%+*: Plus sign if duration is positive; minus sign if negative
            *%-*: Minus sign (only) if duration is negative

        By default, the duration will be formatted in the format
        "-23d 04:19:33.123456789". If the duration is infinite, then the
        string constructed using inf_fmt will be returned ("inf" and "-inf"
        respectively by default).

        Args:
            dur_fmt: Format of the duration string.
            inf_fmt: Format to use for infinity.

        Returns:
            The formatted duration string.
        """
        dur = c_longlong(self._ns_count) if self.is_finite else -1
        is_neg = 1 if (dur == -1 and self._inf_type < 0) else 0
        dur_fmt = None if dur_fmt is None else dur_fmt.encode()
        inf_fmt = None if inf_fmt is None else inf_fmt.encode()
        s = create_string_buffer(128)
        yogi.YOGI_FormatDuration(dur, is_neg, s, sizeof(s), dur_fmt, inf_fmt)
        return s.value.decode("utf-8")

    def __str__(self) -> str:
        """Converts the duration to a string in the format
        "-23d 04:19:33.123456789"
        """
        return self.format()

    def __add__(self, rhs: 'Duration') -> 'Duration':
        if self._inf_type == 0 and rhs._inf_type == 0:
            return self.from_nanoseconds(add_safely(self._ns_count,
                                                    rhs._ns_count))

        inf_type_sum = self._inf_type + rhs._inf_type
        if inf_type_sum == 0:
            raise ArithmeticError("Trying to add positive and negative"
                                  " infinite duration values")

        return self.infinity if inf_type_sum > 0 else self.negative_infinity

    def __sub__(self, rhs: 'Duration') -> 'Duration':
        return self + rhs.negated

    def __mul__(self, rhs: Union[int, float]) -> 'Duration':
        if self._inf_type != 0 and rhs == 0:
            raise ArithmeticError("Trying to multiply infinite duration value"
                                  " and zero")

        if self._inf_type == 0 and math.isfinite(rhs):
            return self.from_nanoseconds(multiply_safely(self._ns_count, rhs))

        rhs_inf_type = 1 if rhs > 0 else -1
        if self._inf_type == rhs_inf_type:
            return self.infinity
        else:
            return self.negative_infinity

    def __truediv__(self, rhs: Union[int, float]) -> 'Duration':
        if self._inf_type == 0:
            return self.from_nanoseconds(divide_safetly(self._ns_count, rhs))

        check_divisor(rhs)
        rhs_inf_type = 1 if rhs > 0 else -1
        if self._inf_type == rhs_inf_type:
            return self.infinity
        else:
            return self.negative_infinity

    def __eq__(self, rhs: 'Duration') -> bool:
        if not isinstance(rhs, Duration):
            return False

        if self._inf_type == rhs._inf_type and self._inf_type != 0:
            return True

        return self._ns_count == rhs._ns_count

    def __ne__(self, rhs: 'Duration') -> bool:
        return not (self == rhs)

    def __lt__(self, rhs: 'Duration') -> bool:
        if self._inf_type == rhs._inf_type and self._inf_type == 0:
            return self._ns_count < rhs._ns_count

        return self._inf_type < rhs._inf_type

    def __gt__(self, rhs: 'Duration') -> bool:
        if self._inf_type == rhs._inf_type and self._inf_type == 0:
            return self._ns_count > rhs._ns_count

        return self._inf_type > rhs._inf_type

    def __le__(self, rhs: 'Duration') -> bool:
        return not (self > rhs)

    def __ge__(self, rhs: 'Duration') -> bool:
        return not (self < rhs)

    def __hash__(self) -> int:
        return hash(self._inf_type) * 17 + hash(self._ns_count)

    @classmethod
    def __from_time_unit(cls, multiplicator: int, val: Union[int, float]):
        if math.isnan(val):
            raise ArithmeticError("Cannot construct duration from NaN")

        dur = cls()
        if math.isfinite(val):
            max_val = 9223372036854775807 // multiplicator
            if abs(val) > max_val:
                raise OverflowError("Duration value overflow")

            dur._ns_count = int(val * multiplicator)
        else:
            dur._inf_type = -1 if val < 0 else +1

        return dur

    def __to_total_time_unit(self, divisor):
        if self._inf_type < 0:
            return -math.inf
        elif self._inf_type > 0:
            return math.inf
        else:
            return self._ns_count / divisor


def duration_to_api_duration(duration: Optional[Duration],
                             default: Duration = None) -> int:
    if duration is None:
        duration = default

    if duration.is_finite:
        return duration.nanoseconds_count
    else:
        return -1
