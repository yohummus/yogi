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

from .library import yogi

from ctypes import c_char_p

yogi.YOGI_GetLicense.restype = c_char_p
yogi.YOGI_GetLicense.argtypes = []


def get_license() -> str:
    """Retrieve the Yogi license.

    Returns:
        String containing the Yogi license.
    """
    return yogi.YOGI_GetLicense().decode()


yogi.YOGI_Get3rdPartyLicenses.restype = c_char_p
yogi.YOGI_Get3rdPartyLicenses.argtypes = []


def get_3rd_party_licenses() -> str:
    """Retrieve the licenses of the 3rd party libraries compiled into Yogi
    Core.

    Returns:
        String containing the 3rd party licenses.
    """
    return yogi.YOGI_Get3rdPartyLicenses().decode()
