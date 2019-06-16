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
import msgpack

from .common import TestCase


class TestPayloadView(TestCase):
    def test_from_json_view(self):
        jsn = yogi.JsonView("[1]")
        view = yogi.PayloadView(jsn)
        self.assertEqual(view.data, jsn.data)
        self.assertEqual(view.size, jsn.size)

    def test_from_msgpack_view(self):
        mp = yogi.MsgpackView([1, 2])
        view = yogi.PayloadView(mp)
        self.assertEqual(view.data, mp.data)
        self.assertEqual(view.size, mp.size)

    def test_from_buffer(self):
        buf = bytearray([1, 2, 0])
        view = yogi.PayloadView(buf, len(buf), yogi.EncodingType.JSON)
        self.assertEqual(view.data, buf)
        self.assertEqual(view.size, len(buf))
        self.assertEqual(view.encoding, yogi.EncodingType.JSON)

        buf = bytearray([0x92, 1, 2])
        view = yogi.PayloadView(buf, len(buf), yogi.EncodingType.MSGPACK)
        self.assertEqual(view.data, buf)
        self.assertEqual(view.size, len(buf))
        self.assertEqual(view.encoding, yogi.EncodingType.MSGPACK)

    def test_len_operator(self):
        view = yogi.MsgpackView([1, 2, 3])
        self.assertEqual(len(view), view.size)

    def test_comparison_operators_payload_view(self):
        a = yogi.PayloadView(bytearray([0x92, 1, 0]), 3,
                             yogi.EncodingType.MSGPACK)
        b = yogi.PayloadView(bytearray([0x92, 1, 0]), 3,
                             yogi.EncodingType.MSGPACK)
        c = yogi.PayloadView(bytearray([0x92, 1, 0]), 3,
                             yogi.EncodingType.JSON)

        self.assertTrue(a == b)
        self.assertFalse(a == c)

        self.assertFalse(a != b)
        self.assertTrue(a != c)

    def test_comparison_operators_json_view(self):
        view = yogi.PayloadView(yogi.JsonView("[1]"))
        a = yogi.JsonView("[1]")
        b = yogi.JsonView("[2]")

        self.assertTrue(view == a)
        self.assertFalse(view == b)

        self.assertFalse(view != a)
        self.assertTrue(view != b)

    def test_comparison_operators_msgpack_view(self):
        view = yogi.PayloadView(yogi.MsgpackView([1]))
        a = yogi.MsgpackView([1])
        b = yogi.MsgpackView([2])

        self.assertTrue(view == a)
        self.assertFalse(view == b)

        self.assertFalse(view != a)
        self.assertTrue(view != b)


if __name__ == '__main__':
    unittest.main()
