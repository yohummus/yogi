#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest

import time


class TestTimestamp(unittest.TestCase):
    def setUp(self):
        self.t = yogi.Timestamp.now()

    def test_now(self):
        t1 = yogi.Timestamp.now()
        time.sleep(0.001)
        t2 = yogi.Timestamp.now()
        self.assertNotEqual(t1, t2)

    def test_getters(self):
        import future.types.newint
        self.assertTrue(isinstance(self.t.milliseconds, (int, future.types.newint)))
        self.assertTrue(isinstance(self.t.microseconds, (int, future.types.newint)))
        self.assertTrue(isinstance(self.t.nanoseconds, (int, future.types.newint)))
        self.assertTrue(isinstance(self.t.ns_since_epoch, (int, future.types.newint)))

    def test_to_string(self):
        base = '^\d\d/\d\d/\d\d\d\d \d\d:\d\d:\d\d'
        sub = '\.\d\d\d'
        self.assertRegexpMatches(self.t.to_string(precision=yogi.Timestamp.Precision.SECONDS), base + '$')
        self.assertRegexpMatches(self.t.to_string(precision=yogi.Timestamp.Precision.MILLISECONDS), base + sub + '$')
        self.assertRegexpMatches(self.t.to_string(precision=yogi.Timestamp.Precision.MICROSECONDS), base + sub * 2 + '$')
        self.assertRegexpMatches(self.t.to_string(precision=yogi.Timestamp.Precision.NANOSECONDS), base + sub * 3 + '$')
        self.assertEqual(self.t.to_string(), str(self.t))


if __name__ == '__main__':
    unittest.main()
