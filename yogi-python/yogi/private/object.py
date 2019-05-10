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
from .errors import FailureException, ErrorCode, api_result_handler

import sys
from ctypes import c_void_p, c_char_p, c_int, create_string_buffer, sizeof
from typing import List, Any


yogi.YOGI_FormatObject.restype = api_result_handler
yogi.YOGI_FormatObject.argtypes = [c_void_p, c_char_p, c_int, c_char_p,
                                   c_char_p]

yogi.YOGI_Destroy.restype = api_result_handler
yogi.YOGI_Destroy.argtypes = [c_void_p]


class Object:
    """Base class for all "creatable" objects."""

    def __init__(self, handle: c_void_p, dependecies: List[Any] = []):
        """Constructs the object.

        The objects in the depencies list are stored in order to prevent the
        garbage collector to destroy them before this objects gets destroyed.
        For example, it would prevent the destruction of the context object in
        the following case:
            timer = yogi.Timer(yogi.Context())

        Args:
            handle:       Native handle representing the object.
            dependencies: Other objects that this object depends on.
        """
        self._handle = handle
        self._dependencies = dependecies

    def format(self, fmt: str = None, nullstr: str = None) -> str:
        """Creates a string describing the object.

        The fmt parameter describes the format of the string. The following
        placeholders are supported:
            $T: Type of the object (e.g. Branch)
            $x: Handle of the object in lower-case hex notation
            $X: Handle of the object in upper-case hex notation

        By default, the object will be formatted in the format
        "Branch [44fdde]" with the hex value in brackets being the object's
        handle.

        If, for any reason, the object's handle is None, this function returns
        the nullstr parameter value ("INVALID HANDLE" by default).

        Args:
            fmt:     Format of the string
            nullstr: String to use if the handle is None

        Returns:
            Formatted string.
        """
        fmt = None if fmt is None else fmt.encode()
        nullstr = None if nullstr is None else nullstr.encode()
        s = create_string_buffer(128)
        yogi.YOGI_FormatObject(self._handle, s, sizeof(s), fmt, nullstr)
        return s.value.decode("utf-8")

    def __str__(self):
        return self.format()

    def __del__(self):
        assert self._handle, "Looks like the ctor of the inheriting class" \
            "forgot to call Object.__init__()."

        try:
            yogi.YOGI_Destroy(self._handle)
            self._handle = None
        except FailureException as e:
            info = ""
            if e.failure.error_code is ErrorCode.OBJECT_STILL_USED:
                info = " Check that you don't have circular dependencies on" \
                    " Yogi objects."

            print("Could not destroy {} instance: {}.{}".format(
                self.__class__.__name__, e.failure, info), file=sys.stderr)

            assert(False)
