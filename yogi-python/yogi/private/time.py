# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2019 Johannes Bergmann.
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
from .timestamp import Timestamp

import datetime
from ctypes import c_longlong, c_char_p, c_int, byref, POINTER, \
    create_string_buffer, sizeof


def get_current_time() -> Timestamp:
    """Returns the current time.

    Returns:
        Aware datetime object representing the current time.
    """
    return Timestamp.now()
