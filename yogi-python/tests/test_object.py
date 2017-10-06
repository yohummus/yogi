#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest
import ctypes


class TestObject(unittest.TestCase):
    def test_init(self):
        obj = yogi.Object(ctypes.c_void_p())
        scheduler = yogi.Scheduler()

    def test_destroy(self):
        obj = yogi.Object(ctypes.c_void_p())
        self.assertRaises(yogi.Failure, lambda: obj.destroy())

        scheduler = yogi.Scheduler()
        scheduler.destroy()
        self.assertRaises(yogi.Failure, lambda: scheduler.destroy())

    def test_str(self):
        obj = yogi.Object(ctypes.c_void_p())
        self.assertRegexpMatches(str(obj), r'.*Object.*INVALID.*')
        scheduler = yogi.Scheduler()
        self.assertRegexpMatches(str(scheduler), r'.*Scheduler.*0x[0-9a-f]{8}.*')


if __name__ == '__main__':
    unittest.main()

