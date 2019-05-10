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
import msgpack

from .common import TestCase


class TestMsgpackView(TestCase):
    def test_from_bytes(self):
        view = yogi.MsgpackView(bytes([0x92, 0x1, 0x2]))
        self.assertEqual(view.data, bytes([0x92, 0x1, 0x2]))
        self.assertEqual(view.size, 3)

    def test_from_object(self):
        view = yogi.MsgpackView([1, 2])
        self.assertEqual(view.data, bytes([0x92, 0x1, 0x2]))
        self.assertEqual(view.size, 3)

    def test_len_operator(self):
        view = yogi.MsgpackView([1, 2, 3])
        self.assertEqual(len(view), view.size)

    def test_comparison_operators(self):
        a = yogi.MsgpackView([1])
        b = yogi.MsgpackView([1])
        c = yogi.MsgpackView([2])

        self.assertTrue(a == b)
        self.assertFalse(a == c)

        self.assertFalse(a != b)
        self.assertTrue(a != c)

    def test_hash(self):
        a = yogi.MsgpackView([1])
        b = yogi.MsgpackView([2])
        self.assertNotEqual(hash(a), hash(b))


if __name__ == '__main__':
    unittest.main()
