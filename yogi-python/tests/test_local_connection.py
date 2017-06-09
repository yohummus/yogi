#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest


class TestLocalConnection(unittest.TestCase):
    def setUp(self):
        self.scheduler = yogi.Scheduler()
        self.endpointA = yogi.Leaf(self.scheduler)
        self.endpointB = yogi.Leaf(self.scheduler)
        self.connection = yogi.LocalConnection(self.endpointA, self.endpointB)

    def test_init(self):
        pass

    def test_description(self):
        self.assertRegexpMatches(self.connection.description, r'.*Local.*')

    def test_remote_version(self):
        self.assertEqual(self.connection.remote_version, yogi.get_version())

    def test_remote_identification(self):
        self.assertIs(None, self.connection.remote_identification)


if __name__ == '__main__':
    unittest.main()

