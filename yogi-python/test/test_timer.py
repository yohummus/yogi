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
import datetime

from .common import TestCase


class TestTimer(TestCase):
    def setUp(self):
        self.context = yogi.Context()
        self.timer = yogi.Timer(self.context)

    def test_start(self):
        fn_res = None
        fn_called = False

        def fn(res):
            nonlocal fn_res, fn_called
            fn_res = res
            fn_called = True

        self.timer.start_async(yogi.Duration.from_milliseconds(1), fn)
        while not fn_called:
            self.context.run_one()

        self.assertIsInstance(fn_res, yogi.Success)
        self.assertEqual(fn_res, yogi.Success())

    def test_cancel(self):
        fn_res = None
        fn_called = False

        def fn(res):
            nonlocal fn_res, fn_called
            fn_res = res
            fn_called = True

        self.assertFalse(self.timer.cancel())
        self.timer.start_async(yogi.Duration.infinity, fn)
        self.assertTrue(self.timer.cancel())
        while not fn_called:
            self.context.run_one()

        self.assertFalse(self.timer.cancel())
        self.assertIsInstance(fn_res, yogi.Failure)
        self.assertEqual(fn_res.error_code, yogi.ErrorCode.CANCELED)


if __name__ == '__main__':
    unittest.main()
