#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest


class TestLeaf(unittest.TestCase):
    def setUp(self):
        self.scheduler = yogi.Scheduler()
        self.leaf = yogi.Leaf(self.scheduler)

    def tearDown(self):
        self.leaf.destroy()
        self.scheduler.destroy()

    def test_init(self):
        pass

    def test_scheduler(self):
        self.assertIs(self.scheduler, self.leaf.scheduler)


if __name__ == '__main__':
    unittest.main()

