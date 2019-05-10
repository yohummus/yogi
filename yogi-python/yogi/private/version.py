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

from ctypes import c_char_p

yogi.YOGI_GetVersion.restype = c_char_p
yogi.YOGI_GetVersion.argtypes = []


def get_version() -> str:
    """Retrieve the Yogi Core version.

    Returns:
        Version string of the loaded Yogi Core library.
    """
    return yogi.YOGI_GetVersion().decode()
