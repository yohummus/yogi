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

from .errors import Failure, Success, error_code_to_result

from ctypes import pythonapi, py_object

inc_ref_cnt = pythonapi.Py_IncRef
inc_ref_cnt.argtypes = [py_object]
inc_ref_cnt.restype = None

dec_ref_cnt = pythonapi.Py_DecRef
dec_ref_cnt.argtypes = [py_object]
dec_ref_cnt.restype = None


class Handler:
    def __init__(self, c_function_type, fn):
        self._fn_obj = None

        def clb(res, *args):
            dec_ref_cnt(self._fn_obj)
            return fn(error_code_to_result(res), *args[:-1])

        self._wrapped_fn = c_function_type(clb)
        self._fn_obj = py_object(self._wrapped_fn)

    def __enter__(self):
        inc_ref_cnt(self._fn_obj)
        return self._wrapped_fn

    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_type is not None:
            dec_ref_cnt(self._fn_obj)
