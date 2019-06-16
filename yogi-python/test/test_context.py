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
import threading

from .common import TestCase


one_ms = yogi.Duration.from_milliseconds(1)


class TestContext(TestCase):
    def setUp(self):
        self.context = yogi.Context()

    def test_poll(self):
        self.assertEqual(self.context.poll(), 0)
        self.context.post(lambda: None)
        self.context.post(lambda: None)
        self.assertEqual(self.context.poll(), 2)

    def test_poll_one(self):
        self.assertEqual(self.context.poll_one(), 0)
        self.context.post(lambda: None)
        self.context.post(lambda: None)
        self.assertEqual(self.context.poll_one(), 1)

    def test_run(self):
        self.assertEqual(self.context.run(one_ms), 0)
        self.context.post(lambda: None)
        self.context.post(lambda: None)
        self.assertEqual(self.context.run(one_ms), 2)

    def test_run_one(self):
        self.assertEqual(self.context.run_one(one_ms), 0)
        self.context.post(lambda: None)
        self.context.post(lambda: None)
        self.assertEqual(self.context.run_one(one_ms), 1)

    def test_run_in_background(self):
        called = False

        def fn():
            nonlocal called
            called = True

        self.context.run_in_background()
        self.context.post(fn)
        while not called:
            pass

    def test_stop(self):
        def thread_fn():
            self.context.wait_for_running()
            self.context.stop()

        thread = threading.Thread(target=thread_fn)
        thread.start()
        self.context.run()
        thread.join()

    def test_wait_for_running_and_stopped(self):
        self.assertTrue(self.context.wait_for_stopped())
        self.assertTrue(self.context.wait_for_stopped(one_ms))
        self.assertFalse(self.context.wait_for_running(one_ms))

        self.context.run_in_background()

        self.assertTrue(self.context.wait_for_running())
        self.assertTrue(self.context.wait_for_running(one_ms))
        self.assertFalse(self.context.wait_for_stopped(one_ms))

        self.context.stop()

        self.assertTrue(self.context.wait_for_stopped())


if __name__ == '__main__':
    unittest.main()
