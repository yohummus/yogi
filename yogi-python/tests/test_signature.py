#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest


class TestSignature(unittest.TestCase):
    def test_init(self):
        yogi.Signature()
        yogi.Signature(123)

    def test_raw(self):
        sig = yogi.Signature(123)
        self.assertEqual(123, sig.raw)

    def test_comparison(self):
        sig1 = yogi.Signature(123)
        sig2 = yogi.Signature(456)
        sig3 = yogi.Signature(456)
        sig4 = yogi.Signature()
        sig5 = yogi.Signature()

        self.assertNotEqual(sig1, 456)
        self.assertNotEqual(sig1, sig2)
        self.assertNotEqual(sig3, sig4)

        self.assertEqual(sig2, 456)
        self.assertEqual(sig4, 0)
        self.assertEqual(sig2, sig3)
        self.assertEqual(sig2, sig3)
        self.assertEqual(sig4, sig5)

    def test_str(self):
        sig = yogi.Signature(0x12345678)
        self.assertRegexpMatches(str(sig), r'.*12345678.*')


if __name__ == '__main__':
    unittest.main()

