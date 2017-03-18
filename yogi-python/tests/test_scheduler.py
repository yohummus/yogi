#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest


class TestScheduler(unittest.TestCase):
    def setUp(self):
        self.scheduler = yogi.Scheduler()

    def test_set_thread_pool_size(self):
        self.scheduler.set_thread_pool_size(2)
        self.assertRaises(yogi.Failure, lambda: self.scheduler.set_thread_pool_size(1000000))


if __name__ == '__main__':
    unittest.main()

