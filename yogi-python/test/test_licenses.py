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

import yogi
import unittest

from .common import TestCase


class TestLicenses(TestCase):
    def test_license(self):
        lic = yogi.get_license()
        self.assertIsInstance(lic, str)
        self.assertGreater(len(lic), 100)

    def test_3rd_party_licenses(self):
        lic = yogi.get_3rd_party_licenses()
        self.assertIsInstance(lic, str)
        self.assertGreater(len(lic), 100)
        self.assertNotEqual(yogi.get_license(), lic)


if __name__ == '__main__':
    unittest.main()
