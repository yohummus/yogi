#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest

from proto import yogi_0000d007_pb2
from test_terminals import AsyncCall


class TestBinding(unittest.TestCase):
    def setUp(self):
        self.scheduler = yogi.Scheduler()
        self.leafA = yogi.Leaf(self.scheduler)
        self.leafB = yogi.Leaf(self.scheduler)
        self.connection = None

        self.tmA = yogi.DeafMuteTerminal('T', yogi_0000d007_pb2, leaf=self.leafA)
        self.tmB = yogi.DeafMuteTerminal('T', yogi_0000d007_pb2, leaf=self.leafB)
        self.bd = yogi.Binding(self.tmA, self.tmB.name)

    def tearDown(self):
        self.bd.destroy()
        self.tmA.destroy()
        self.tmB.destroy()

        if self.connection:
            self.connection.destroy()

        self.leafA.destroy()
        self.leafB.destroy()
        self.scheduler.destroy()

    def connect(self):
        self.assertIs(yogi.BindingState.RELEASED, self.bd.get_binding_state())
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)
        while self.bd.get_binding_state() is yogi.BindingState.RELEASED:
            pass
        self.assertIs(yogi.BindingState.ESTABLISHED, self.bd.get_binding_state())

    def test_ctor(self):
        pass

    def test_get_binding_state(self):
        self.connect()

    def test_getters(self):
        self.assertEqual(self.tmA.name, self.bd.targets)
        self.assertIs(self.tmA, self.bd.terminal)

    def test_async_get_binding_state(self):
        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(yogi.BindingState.RELEASED, state)

            self.bd.async_get_binding_state(wrap(fn))

        self.connect()

        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(yogi.BindingState.ESTABLISHED, state)

            self.bd.async_get_binding_state(wrap(fn))

    def test_async_await_binding_state_changed(self):
        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(yogi.BindingState.ESTABLISHED, state)

            self.bd.async_await_binding_state_change(wrap(fn))
            self.connect()

        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(yogi.BindingState.RELEASED, state)

            self.bd.async_await_binding_state_change(wrap(fn))
            self.connection.destroy()
            self.connection = None

    def test_cancel_await_binding_state_changed(self):
        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Canceled(), res)

            self.bd.async_await_binding_state_change(wrap(fn))
            self.bd.cancel_await_binding_state_change()


if __name__ == '__main__':
    unittest.main()
