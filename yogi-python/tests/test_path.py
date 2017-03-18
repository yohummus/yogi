#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest


class TestPath(unittest.TestCase):
    def test_bad_path(self):
        self.assertRaises(yogi.BadPath, lambda: yogi.Path('//'))
        self.assertRaises(yogi.BadPath, lambda: yogi.Path('/tmp//'))
        self.assertRaises(yogi.BadPath, lambda: yogi.Path('tmp//'))
        self.assertRaises(yogi.BadPath, lambda: yogi.Path('a//b'))

    def test_str(self):
        self.assertEqual('/Test', str(yogi.Path('/Test')))

    def test_len(self):
        self.assertEqual(5, len(yogi.Path('/Test')))

    def test_eq(self):
        self.assertEqual(yogi.Path('/Test/tmp'), yogi.Path('/Test/tmp'))
        self.assertEqual(yogi.Path('/Test/tmp'), '/Test/tmp')

    def test_truediv(self):
        self.assertEqual(yogi.Path('/Test/tmp'), yogi.Path('/Test') / yogi.Path('tmp'))
        self.assertEqual(yogi.Path('/Test/tmp'), yogi.Path('/Test') / 'tmp')
        self.assertRaises(yogi.BadPath, lambda: yogi.Path('/Test') / yogi.Path('/tmp'))

    def test_clear(self):
        path = yogi.Path('/Test')
        path.clear()
        self.assertEqual(yogi.Path(), path)

    def test_is_absolute(self):
        self.assertTrue(yogi.Path('/Test').is_absolute)
        self.assertFalse(yogi.Path('Test').is_absolute)

    def test_is_root(self):
        self.assertTrue(yogi.Path('/').is_root)
        self.assertFalse(yogi.Path('/Test').is_root)
        self.assertFalse(yogi.Path('').is_root)


if __name__ == '__main__':
    unittest.main()

