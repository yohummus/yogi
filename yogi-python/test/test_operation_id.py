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


class TestOperationId(TestCase):
    def test_from_value(self):
        oid = yogi.OperationId(5)
        self.assertEqual(oid.value, 5)
        self.assertTrue(oid.is_valid)

        oid = yogi.OperationId(0)
        self.assertEqual(oid.value, 0)
        self.assertFalse(oid.is_valid)

        oid = yogi.OperationId(-10)
        self.assertEqual(oid.value, -10)
        self.assertFalse(oid.is_valid)

    def test_comparison_operators(self):
        a = yogi.OperationId(4)
        b = yogi.OperationId(4)
        c = yogi.OperationId(1)

        self.assertTrue(a == b)
        self.assertFalse(a == c)

        self.assertFalse(a != b)
        self.assertTrue(a != c)

    def test_hash(self):
        a = yogi.OperationId(4)
        b = yogi.OperationId(7)
        self.assertNotEqual(hash(a), hash(b))


if __name__ == '__main__':
    unittest.main()
