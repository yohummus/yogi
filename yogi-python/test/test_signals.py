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

from .common import TestCase


class TestSignals(TestCase):
    def setUp(self):
        self.context = yogi.Context()

    def test_signals_enum(self):
        for sig in yogi.Signals:
            if sig is yogi.Signals.ALL:
                self.assertFlagCombinationMatches("YOGI_SIG_", sig, [])
            else:
                self.assertFlagMatches("YOGI_SIG_", sig)

    def test_raise_signal_cleanup_handler(self):
        fn_sigarg = None
        fn_called = False

        def fn(sigarg):
            nonlocal fn_sigarg, fn_called
            fn_sigarg = sigarg
            fn_called = True

        yogi.raise_signal(yogi.Signals.USR1, None, fn)
        self.assertTrue(fn_called)
        self.assertEqual(fn_sigarg, None)

        fn_called = False
        yogi.raise_signal(yogi.Signals.USR2, 55, fn)

        self.assertTrue(fn_called)
        self.assertEqual(fn_sigarg, 55)

        def fn2():
            nonlocal fn_called
            fn_called = True

        fn_called = False
        yogi.raise_signal(yogi.Signals.USR3, None, fn2)
        self.assertTrue(fn_called)

        fn_called = False

        yogi.raise_signal(yogi.Signals.USR4, "Hello", fn2)
        self.assertTrue(fn_called)

    def test_await_signal(self):
        fn_res = None
        fn_signal = None
        fn_sigarg = None
        fn_called = False

        def fn(res, signal, sigarg):
            nonlocal fn_res, fn_signal, fn_sigarg, fn_called
            fn_res = res
            fn_signal = signal
            fn_sigarg = sigarg
            fn_called = True

        sigset = yogi.SignalSet(self.context, yogi.Signals.TERM |
                                yogi.Signals.USR5)
        sigset.await_signal_async(fn)
        yogi.raise_signal(yogi.Signals.TERM, 123)
        self.context.poll()

        self.assertTrue(fn_called)
        self.assertIsInstance(fn_res, yogi.Success)
        self.assertEqual(fn_res, yogi.Success())
        self.assertIsInstance(fn_signal, yogi.Signals)
        self.assertEqual(fn_signal, yogi.Signals.TERM)
        self.assertEqual(fn_sigarg, 123)

        fn_called = False
        sigset.await_signal_async(fn)
        yogi.raise_signal(yogi.Signals.TERM)
        self.context.poll()

        self.assertTrue(fn_called)
        self.assertEqual(fn_sigarg, None)

        def fn2(res, signal):
            nonlocal fn_called
            fn_called = True

        fn_called = False
        sigset.await_signal_async(fn2)
        yogi.raise_signal(yogi.Signals.TERM, 123)
        self.context.poll()

        self.assertTrue(fn_called)

    def test_cancel_await_signal(self):
        fn_res = None
        fn_signal = None
        fn_sigarg = None
        fn_called = False

        def fn(res, signal, sigarg):
            nonlocal fn_res, fn_signal, fn_sigarg, fn_called
            fn_res = res
            fn_signal = signal
            fn_sigarg = sigarg
            fn_called = True

        sigset = yogi.SignalSet(self.context, yogi.Signals.TERM)

        self.assertFalse(sigset.cancel_await_signal())

        sigset.await_signal_async(fn)
        self.assertTrue(sigset.cancel_await_signal())
        self.context.poll()

        self.assertTrue(fn_called)
        self.assertIsInstance(fn_res, yogi.Failure)
        self.assertEqual(fn_res.error_code, yogi.ErrorCode.CANCELED)
        self.assertIsInstance(fn_signal, yogi.Signals)
        self.assertEqual(fn_signal, yogi.Signals.NONE)
        self.assertEqual(fn_sigarg, None)


if __name__ == '__main__':
    unittest.main()
