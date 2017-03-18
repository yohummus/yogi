#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest

from proto import yogi_0000d007_pb2
from test_terminals import AsyncCall


class TestSubscription(unittest.TestCase):
    def setUp(self):
        self.scheduler = yogi.Scheduler()
        self.leafA = yogi.Leaf(self.scheduler)
        self.leafB = yogi.Leaf(self.scheduler)
        self.connection = None

        self.tmA = yogi.PublishSubscribeTerminal('T', yogi_0000d007_pb2, leaf=self.leafA)
        self.tmB = yogi.PublishSubscribeTerminal('T', yogi_0000d007_pb2, leaf=self.leafB)
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
        self.assertIs(yogi.SubscriptionState.UNSUBSCRIBED, self.tmB.get_subscription_state())
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)
        while self.tmB.get_subscription_state() is yogi.SubscriptionState.UNSUBSCRIBED:
            pass
        self.assertIs(yogi.SubscriptionState.SUBSCRIBED, self.tmB.get_subscription_state())

    def test_ctor(self):
        pass

    def test_async_get_subscription_state(self):
        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(yogi.SubscriptionState.UNSUBSCRIBED, state)

            self.tmB.async_get_subscription_state(wrap(fn))

        self.connect()

        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(yogi.SubscriptionState.SUBSCRIBED, state)

            self.tmB.async_get_subscription_state(wrap(fn))

    def test_async_await_subscription_state_changed(self):
        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(yogi.SubscriptionState.SUBSCRIBED, state)

            self.tmB.async_await_subscription_state_change(wrap(fn))
            self.connect()

        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(yogi.SubscriptionState.UNSUBSCRIBED, state)

            self.tmB.async_await_subscription_state_change(wrap(fn))
            self.connection.destroy()
            self.connection = None

    def test_cancel_await_subscription_state_changed(self):
        with AsyncCall() as wrap:
            def fn(res, state):
                self.assertEqual(yogi.Canceled(), res)

            self.tmB.async_await_subscription_state_change(wrap(fn))
            self.tmB.cancel_await_subscription_state_change()


if __name__ == '__main__':
    unittest.main()
