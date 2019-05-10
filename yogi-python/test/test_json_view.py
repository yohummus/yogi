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


class TestJsonView(TestCase):
    def test_from_string(self):
        view = yogi.JsonView("[1]")
        self.assertEqual(view.data, '[1]\0'.encode())
        self.assertEqual(view.size, 4)

    def test_from_object(self):
        view = yogi.JsonView([5])
        self.assertEqual(view.data, '[5]\0'.encode())
        self.assertEqual(view.size, 4)

    def test_len_operator(self):
        view = yogi.JsonView("[1]")
        self.assertEqual(len(view), view.size)

    def test_comparison_operators(self):
        a = yogi.JsonView("[1]")
        b = yogi.JsonView("[1]")
        c = yogi.JsonView("[2]")

        self.assertTrue(a == b)
        self.assertFalse(a == c)

        self.assertFalse(a != b)
        self.assertTrue(a != c)

    def test_hash(self):
        a = yogi.JsonView("[1]")
        b = yogi.JsonView("[2]")
        self.assertNotEqual(hash(a), hash(b))


if __name__ == '__main__':
    unittest.main()
