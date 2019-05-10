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
from .duration import Duration

import datetime
from ctypes import c_longlong, c_char_p, c_int, byref, POINTER, \
    create_string_buffer, sizeof
from typing import Union


yogi.YOGI_GetCurrentTime.restype = api_result_handler
yogi.YOGI_GetCurrentTime.argtypes = [POINTER(c_longlong)]

yogi.YOGI_FormatTime.restype = api_result_handler
yogi.YOGI_FormatTime.argtypes = [c_longlong, c_char_p, c_int, c_char_p]

yogi.YOGI_ParseTime.restype = api_result_handler
yogi.YOGI_ParseTime.argtypes = [POINTER(c_longlong), c_char_p, c_char_p]


class Timestamp:
    """Represents a UTC timestamp.

    Timestamps are expressed in nanoseconds since 01/01/1970 UTC.
    """
    @classmethod
    def from_duration_since_epoch(cls, duration: Duration) -> 'Timestamp':
        """Creates a timestamp from a duration since the epoch 01/01/1970 UTC.

        Args:
            duration: Duration since the epoch.

        Returns:
            Timestamp object.
        """
        if not duration.is_finite or duration < Duration.zero:
            raise ArithmeticError(
                "Invalid duration value for use as a timestamp")

        ts = cls()
        ts._dur_since_epoch = duration
        return ts

    @classmethod
    def now(cls) -> 'Timestamp':
        """Creates a timestamp from the current time.

        Returns:
            Timestamp corresponding to the current time.
        """
        timestamp = c_longlong()
        yogi.YOGI_GetCurrentTime(byref(timestamp))
        dur = Duration.from_nanoseconds(timestamp.value)
        return cls.from_duration_since_epoch(dur)

    @classmethod
    def parse(cls, s: str, fmt: str = None) -> 'Timestamp':
        """Converts a string into a timestamp.

        The fmt parameter describes the textual format of the conversion. The
        following placeholders are supported:
            %Y: Four digit year.
            %m: Month name as a decimal 01 to 12.
            %d: Day of the month as decimal 01 to 31.
            %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
            %H: The hour as a decimal number using a 24-hour clock (00 to 23).
            %M: The minute as a decimal 00 to 59.
            %S: Seconds as a decimal 00 to 59.
            %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
            %3: Milliseconds as decimal number 000 to 999.
            %6: Microseconds as decimal number 000 to 999.
            %9: Nanoseconds as decimal number 000 to 999.

        By default, the timestamp will be parsed in the format
        "2009-02-11T12:53:09.123Z".

        Args:
            s:   String to parse.
            fmt: Format of the time string.

        Returns:
            The parsed timestamp.
        """
        timestamp = c_longlong()
        s = s.encode()
        fmt = None if fmt is None else fmt.encode()
        yogi.YOGI_ParseTime(byref(timestamp), s, fmt)
        dur = Duration.from_nanoseconds(timestamp.value)
        return cls.from_duration_since_epoch(dur)

    def __init__(self):
        """Constructs the timestamp using the epoch 01/01/1970 UTC."""
        self._dur_since_epoch = Duration()

    @property
    def duration_since_epoch(self) -> Duration:
        """The duration since 01/01/1970 UTC."""
        return self._dur_since_epoch

    @property
    def nanoseconds(self) -> int:
        """The nanoseconds fraction of the timestamp (0-999)."""
        return self._dur_since_epoch.nanoseconds

    @property
    def microseconds(self) -> int:
        """The microseconds fraction of the timestamp (0-999)."""
        return self._dur_since_epoch.microseconds

    @property
    def milliseconds(self) -> int:
        """The milliseconds fraction of the timestamp (0-999)."""
        return self._dur_since_epoch.milliseconds

    def to_datetime(self) -> datetime.datetime:
        """Converts the timestamp to a datetime object.

        Returns:
            Timestamp as a datetime object.
        """
        epoch = datetime.datetime(1970, 1, 1, tzinfo=datetime.timezone.utc)
        delta = self._dur_since_epoch.to_timedelta()
        return epoch + delta

    def format(self, fmt: str = None) -> str:
        """Converts the timestamp to a string.

        The fmt parameter describes the textual format of the conversion. The
        following placeholders are supported:
            %Y: Four digit year.
            %m: Month name as a decimal 01 to 12.
            %d: Day of the month as decimal 01 to 31.
            %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
            %H: The hour as a decimal number using a 24-hour clock (00 to 23).
            %M: The minute as a decimal 00 to 59.
            %S: Seconds as a decimal 00 to 59.
            %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
            %3: Milliseconds as decimal number 000 to 999.
            %6: Microseconds as decimal number 000 to 999.
            %9: Nanoseconds as decimal number 000 to 999.

        By default, the timestamp will be formatted as an ISO-8601 string with
        up to millisecond resolution, e.g. "2018-04-23T18:25:43.511Z".

        Args:
            fmt: Format of the time string.

        Returns:
            The timestamp as a string.
        """
        timestamp = c_longlong(self._dur_since_epoch.nanoseconds_count)
        fmt = None if fmt is None else fmt.encode()
        s = create_string_buffer(128)
        yogi.YOGI_FormatTime(timestamp, s, sizeof(s), fmt)
        return s.value.decode("utf-8")

    def __str__(self) -> str:
        """Returns the time in ISO-8601 format up to milliseonds.

        Example: "2018-04-23T18:25:43.511Z".

        Returns:
            The time in ISO-8601 format.
        """
        return self.format()

    def __add__(self, rhs: Duration) -> 'Timestamp':
        return self.from_duration_since_epoch(self._dur_since_epoch + rhs)

    def __sub__(self, rhs: Union[Duration, 'Timestamp']) \
            -> Union['Timestamp', Duration]:
        if isinstance(rhs, Duration):
            return self.from_duration_since_epoch(self._dur_since_epoch - rhs)
        else:
            return self._dur_since_epoch - rhs._dur_since_epoch

    def __eq__(self, rhs: 'Timestamp') -> bool:
        return self._dur_since_epoch == rhs._dur_since_epoch

    def __ne__(self, rhs: 'Timestamp') -> bool:
        return self._dur_since_epoch != rhs._dur_since_epoch

    def __lt__(self, rhs: 'Timestamp') -> bool:
        return self._dur_since_epoch < rhs._dur_since_epoch

    def __gt__(self, rhs: 'Timestamp') -> bool:
        return self._dur_since_epoch > rhs._dur_since_epoch

    def __le__(self, rhs: 'Timestamp') -> bool:
        return self._dur_since_epoch <= rhs._dur_since_epoch

    def __ge__(self, rhs: 'Timestamp') -> bool:
        return self._dur_since_epoch >= rhs._dur_since_epoch

    def __hash__(self) -> int:
        return hash(self._dur_since_epoch)


def api_timestamp_to_timestamp(timestamp: int) -> Timestamp:
    dur = Duration.from_nanoseconds(timestamp)
    return Timestamp.from_duration_since_epoch(dur)
