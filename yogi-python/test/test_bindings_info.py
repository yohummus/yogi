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

import yogi
import unittest

from .common import TestCase


class TestBindingsInfo(TestCase):
    def test_version(self):
        self.assertEqual(yogi.BindingsInfo.VERSION,
                         self.get_core_macro('YOGI_HDR_VERSION'))

    def test_version_major(self):
        self.assertEqual(yogi.BindingsInfo.VERSION_MAJOR,
                         self.get_core_macro('YOGI_HDR_VERSION_MAJOR'))

    def test_version_minor(self):
        self.assertEqual(yogi.BindingsInfo.VERSION_MINOR,
                         self.get_core_macro('YOGI_HDR_VERSION_MINOR'))

    def test_version_patch(self):
        self.assertEqual(yogi.BindingsInfo.VERSION_PATCH,
                         self.get_core_macro('YOGI_HDR_VERSION_PATCH'))

    def test_version_suffix(self):
        self.assertEqual(yogi.BindingsInfo.VERSION_SUFFIX,
                         self.get_core_macro('YOGI_HDR_VERSION_SUFFIX'))


if __name__ == '__main__':
    unittest.main()
