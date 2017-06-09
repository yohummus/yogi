#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest
import os
import tempfile


class TestFreeFunctions(unittest.TestCase):
    def test_get_version(self):
        self.assertRegexpMatches(yogi.get_version(), r'^\d+\.\d+\.\d+(-[a-zA-Z0-9_-]+)?$')

    def test_set_log_file(self):
        filename = tempfile.mktemp(suffix='.log', prefix='yogi-')
        yogi.set_log_file(filename=filename, verbosity=yogi.Verbosity.DEBUG)
        self.assertTrue(os.path.isfile(filename))
        self.assertGreater(os.path.getsize(filename), 10)


if __name__ == '__main__':
    unittest.main()
