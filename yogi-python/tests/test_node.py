#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest

from test_terminals import AsyncCall
from proto import yogi_0000d007_pb2


class TestNode(unittest.TestCase):
    def setUp(self):
        self.scheduler = yogi.Scheduler()
        self.node = yogi.Node(self.scheduler)
        self.leaf = yogi.Leaf(self.scheduler)
        self.connection = yogi.LocalConnection(self.node, self.leaf)

    def tearDown(self):
        self.connection.destroy()
        self.leaf.destroy()
        self.node.destroy()
        self.scheduler.destroy()

    def test_init(self):
        pass

    def test_scheduler(self):
        self.assertIs(self.scheduler, self.node.scheduler)

    def test_get_known_terminals(self):
        tmA = yogi.ProducerTerminal('One', yogi_0000d007_pb2, leaf=self.leaf)
        tmB = yogi.ServiceTerminal('Two', yogi_0000d007_pb2, leaf=self.leaf)

        while len(self.node.get_known_terminals()) < 2:
            pass

        terminals = self.node.get_known_terminals()
        self.assertEqual(2, len(terminals))

        self.assertIs(yogi.ProducerTerminal, terminals[0].type)
        self.assertEqual(0x0000d007, terminals[0].signature.raw)
        self.assertEqual('One', terminals[0].name)
        self.assertIs(yogi.ServiceTerminal, terminals[1].type)
        self.assertEqual(0x0000d007, terminals[1].signature.raw)
        self.assertEqual('Two', terminals[1].name)

        tmA.destroy()
        tmB.destroy()

    def test_async_await_known_terminals_change(self):
        tm = None
        with AsyncCall() as wrap:
            def fn(res, info, change):
                self.assertEqual(yogi.Success(), res)
                self.assertIs(yogi.ProducerTerminal, info.type)
                self.assertEqual(0x0000d007, info.signature.raw)
                self.assertEqual('One', info.name)
                self.assertEqual(yogi.ChangeType.ADDED, change)

            self.node.async_await_known_terminals_change(wrap(fn))
            tm = yogi.ProducerTerminal('One', yogi_0000d007_pb2, leaf=self.leaf)

        with AsyncCall() as wrap:
            def fn(res, info, change):
                self.assertEqual(yogi.Success(), res)
                self.assertIs(yogi.ProducerTerminal, info.type)
                self.assertEqual(0x0000d007, info.signature.raw)
                self.assertEqual('One', info.name)
                self.assertEqual(yogi.ChangeType.REMOVED, change)

            self.node.async_await_known_terminals_change(wrap(fn))
            tm.destroy()


    def test_cancel_await_known_terminals_change(self):
        with AsyncCall() as wrap:
            def fn(res, info, change):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(info)
                self.assertIsNone(change)

            self.node.async_await_known_terminals_change(wrap(fn))
            self.node.cancel_await_known_terminals_change()


if __name__ == '__main__':
    unittest.main()

