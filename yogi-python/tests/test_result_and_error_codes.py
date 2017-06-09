#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest


class TestResult(unittest.TestCase):
    def setUp(self):
        self.okResult = yogi.Result(0)
        self.idResult = yogi.Result(123)
        self.errorResult = yogi.Result(-2)

    def test_value(self):
        self.assertEqual(0, self.okResult.value)
        self.assertEqual(123, self.idResult.value)
        self.assertEqual(-2, self.errorResult.value)

    def test_bool(self):
        self.assertTrue(self.okResult)
        self.assertTrue(self.idResult)
        self.assertFalse(self.errorResult)

    def test_comparison(self):
        self.assertNotEqual(self.okResult, self.idResult)
        self.assertEqual(self.idResult, yogi.Result(123))

    def test_str(self):
        self.assertEqual('[0] Success', str(self.okResult))
        self.assertEqual('[123] Success', str(self.idResult))
        self.assertEqual('[-2] Invalid object handle', str(self.errorResult))


class TestFailure(unittest.TestCase):
    def test_init(self):
        self.assertRaises(Exception, lambda: yogi.Failure(0))
        self.assertRaises(Exception, lambda: yogi.Failure(1))
        yogi.Failure(-1)

    def test_comparison(self):
        self.assertNotEqual(yogi.Failure(-1), yogi.Result(1))
        self.assertEqual(yogi.Failure(-1), yogi.Result(-1))


class TestSuccess(unittest.TestCase):
    def test_init(self):
        self.assertRaises(Exception, lambda: yogi.Success(-1))
        yogi.Success(1)
        yogi.Success(0)
        yogi.Success()

    def test_comparison(self):
        self.assertNotEqual(yogi.Success(), yogi.Result(123))
        self.assertEqual(yogi.Success(123), yogi.Result(123))
        self.assertEqual(yogi.Success(), yogi.Result(0))


if __name__ == '__main__':
    unittest.main()

