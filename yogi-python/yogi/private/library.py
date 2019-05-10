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
import ctypes


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
    yogi = ctypes.cdll.LoadLibrary(lib_filename)
except Exception as e:
    raise Exception('ERROR: Could not load {}: {}. Make sure the library is'
                    ' in your library search path.'.format(lib_filename, e))
