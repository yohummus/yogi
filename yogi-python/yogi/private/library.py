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

import os
import platform
from ctypes import cdll, c_int, c_char_p, create_string_buffer, sizeof

from .bindings_info import BindingsInfo


# Determine shared library path and filename
lib_filename = os.getenv('YOGI_CORE_LIBRARY')
if lib_filename is None:
    if platform.system() == 'Windows':
        lib_filename = "yogi-core.dll"
    elif platform.system() == 'Darwin':
        lib_filename = "libyogi-core.dylib"
    elif platform.system() == 'Linux':
        lib_filename = "libyogi-core.so"
    else:
        raise Exception(platform.system() + ' is not supported')

# Load the shared library
try:
    yogi = cdll.LoadLibrary(lib_filename)
except Exception as e:
    raise Exception('ERROR: Could not load {}: {}. Make sure the library is'
                    ' in your library search path.'.format(lib_filename, e))

# Check if the bindings version is compatible with the core version
yogi.YOGI_CheckBindingsCompatibility.restype = int
yogi.YOGI_CheckBindingsCompatibility.argtypes = [c_char_p, c_char_p, c_int]

version = BindingsInfo.VERSION.encode()

err = create_string_buffer(256)
if yogi.YOGI_CheckBindingsCompatibility(version, err, sizeof(err)) != 0:
    raise Exception('FATAL: ' + err.value.decode())
