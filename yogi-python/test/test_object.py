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

from ctypes import c_void_p


class FakeObject:
    def __init__(self):
        self._handle = c_void_p()


class TestContext(TestCase):
    def test_format(self):
        obj = yogi.Context()

        s = obj.format()
        self.assertRegex(s, "Context \\[[1-9a-f][0-9a-f]+\\]")

        s = obj.format("$T-[$X]")
        self.assertRegex(s, "Context-\\[[1-9A-F][0-9A-F]+\\]")

        obj = FakeObject()
        s = yogi.Object.format(obj)
        self.assertTrue(s.startswith("INVALID"))

        s = yogi.Object.format(obj, nullstr="MOO")
        self.assertEqual(s, "MOO")

    def test_str(self):
        obj = yogi.Context()
        self.assertRegex(str(obj), "Context \\[[1-9a-f][0-9a-f]+\\]")


if __name__ == '__main__':
    unittest.main()
