#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest

from test_terminals import AsyncCall
from proto import yogi_00000001_pb2
from proto import yogi_0000040d_pb2


class TestProcess(unittest.TestCase):
    def setUp(self):
        self.pi = yogi.ProcessInterface(yogi.Configuration())
        self.scheduler = yogi.Scheduler()
        self.leaf = yogi.Leaf(self.scheduler)
        self.connection = yogi.LocalConnection(self.pi.leaf, self.leaf)

        self.operational_terminal = yogi.CachedConsumerTerminal('/Process/Operational', yogi_00000001_pb2,
                                                                   leaf=self.leaf)
        self.errors_terminal = yogi.CachedConsumerTerminal('/Process/Errors', yogi_0000040d_pb2, leaf=self.leaf)
        self.warnings_terminal = yogi.CachedConsumerTerminal('/Process/Warnings', yogi_0000040d_pb2, leaf=self.leaf)

        for tm in [self.operational_terminal, self.errors_terminal, self.warnings_terminal]:
            while tm.get_binding_state() is yogi.BindingState.RELEASED:
                pass

            while True:
                try:
                    tm.get_cached_message()
                    break
                except yogi.Failure:
                    pass

    def tearDown(self):
        self.operational_terminal.destroy()
        self.errors_terminal.destroy()
        self.warnings_terminal.destroy()
        self.connection.destroy()
        self.leaf.destroy()
        self.scheduler.destroy()
        self.pi.destroy()

    def test_init(self):
        pass

    def test_initial_errors_and_warnings(self):
        msg = self.errors_terminal.get_cached_message()
        self.assertEqual(0, len(msg.value))

        msg = self.warnings_terminal.get_cached_message()
        self.assertEqual(0, len(msg.value))

    def test_errors(self):
        err1 = yogi.Error('Out of coffee')
        err2 = yogi.Error('Out of soy milk')

        def check(errors, fn):
            with AsyncCall() as wrap:
                def handler_fn(res, msg, cached):
                    self.assertEqual(yogi.Success(), res)
                    self.assertEqual([err.message for err in errors], msg.value)

                self.errors_terminal.async_receive_message(wrap(handler_fn))
                fn()

        # set and clear anomalies
        check([err1], lambda: err1.set())
        check([err1, err2], lambda: err2.set())
        check([err2], lambda: err1.clear())
        check([], lambda: err2.destroy())

        # set timed anomaly
        check([err1], lambda: err1.set(milliseconds=1))
        while len(self.errors_terminal.get_cached_message().value) is not 0:
            pass
        self.assertEqual(0, len(self.errors_terminal.get_cached_message().value))

        err1.destroy()

    def test_warnings(self):
        wrn1 = yogi.Warning('Out of coffee')
        wrn2 = yogi.Warning('Out of soy milk')

        def check(errors, fn):
            with AsyncCall() as wrap:
                def handler_fn(res, msg, cached):
                    self.assertEqual(yogi.Success(), res)
                    self.assertEqual([err.message for err in errors], msg.value)

                self.warnings_terminal.async_receive_message(wrap(handler_fn))
                fn()

        # set and clear anomalies
        check([wrn1], lambda: wrn1.set())
        check([wrn1, wrn2], lambda: wrn2.set())
        check([wrn2], lambda: wrn1.clear())
        check([], lambda: wrn2.destroy())

        # set timed anomaly
        check([wrn1], lambda: wrn1.set(milliseconds=1))
        while len(self.warnings_terminal.get_cached_message().value) is not 0:
            pass
        self.assertEqual(0, len(self.warnings_terminal.get_cached_message().value))

        wrn1.destroy()

    def test_initial_operational_state(self):
        msg = self.operational_terminal.get_cached_message()
        self.assertTrue(msg.value)

    def test_operational_conditions(self):
        oc1 = []
        oc2 = []

        def check(state, fn):
            with AsyncCall() as wrap:
                def handler_fn(res, msg, cached):
                    self.assertEqual(yogi.Success(), res)
                    self.assertEqual(state, msg.value)

                self.operational_terminal.async_receive_message(wrap(handler_fn))
                fn()

        def create_oc1():
            oc1.append(yogi.ManualOperationalCondition('OC 1'))

        def create_oc2():
            oc2.append(yogi.ManualOperationalCondition('OC 2'))

        check(False, create_oc1)
        check(True, lambda: oc1[0].set())
        check(False, create_oc2)
        check(True, lambda: oc2[0].set())
        check(False, lambda: oc2[0].clear())
        check(True, lambda: oc2[0].destroy())
        check(False, lambda: oc1[0].clear())
        check(True, lambda: oc1[0].destroy())

    def test_operational_observer(self):
        moc = yogi.ManualOperationalCondition('Test Condition')
        observer = yogi.OperationalObserver()

        i = [0]

        def fn1(operational):
            if i[0] is 0:
                self.assertFalse(operational)
            elif i[0] is 1:
                self.assertTrue(operational)
            elif i[0] is 2:
                self.assertFalse(operational)
            i[0] += 1

        observer_id = observer.add(fn1)

        j = [0]

        def fn2(operational):
            if j[0] is 0:
                self.assertFalse(operational)
            elif j[0] is 1:
                self.assertTrue(operational)
            elif j[0] is 2:
                self.assertFalse(operational)
            elif j[0] is 3:
                self.assertTrue(operational)
            j[0] += 1

        observer.add(fn2)

        moc.set()
        moc.clear()

        self.assertEqual(0, i[0])
        self.assertEqual(0, j[0])

        observer.start()

        self.assertEqual(1, i[0])
        self.assertEqual(1, j[0])

        moc.set()

        self.assertEqual(2, i[0])
        self.assertEqual(2, j[0])

        observer.remove(observer_id)

        moc.clear()

        self.assertEqual(2, i[0])
        self.assertEqual(3, j[0])

        observer.stop()

        moc.set()

        self.assertEqual(2, i[0])
        self.assertEqual(3, j[0])

        observer.destroy()
        moc.destroy()

    def test_dependency(self):
        tmA1 = yogi.ConsumerTerminal('1', yogi_00000001_pb2, leaf=self.pi.leaf)
        tmA2 = yogi.PublishSubscribeTerminal('2', yogi_00000001_pb2, leaf=self.pi.leaf)
        bd = yogi.Binding(tmA2, '2')

        dep = yogi.Dependency('My Dependency', [tmA1, bd])
        self.assertFalse(self.pi.operational)

        tmB1 = yogi.ProducerTerminal('1', yogi_00000001_pb2, leaf=self.leaf)
        self.assertFalse(self.pi.operational)

        tmB2 = yogi.PublishSubscribeTerminal('2', yogi_00000001_pb2, leaf=self.leaf)
        while not self.pi.operational:
            pass
        self.assertTrue(self.pi.operational)

        tmB2.destroy()
        while self.pi.operational:
            pass
        self.assertFalse(self.pi.operational)

        tmB1.destroy()
        dep.destroy()
        bd.destroy()
        tmA2.destroy()
        tmA1.destroy()

    def test_process_dependency(self):
        tmA = yogi.ConsumerTerminal('A', yogi_00000001_pb2)

        dep = yogi.ProcessDependency('My Process', [tmA])
        self.assertFalse(self.pi.operational)

        tmB = yogi.ProducerTerminal('/A', yogi_00000001_pb2, leaf=self.leaf)
        self.assertFalse(self.pi.operational)

        tmO = yogi.CachedProducerTerminal('/My Process/Process/Operational', yogi_00000001_pb2, leaf=self.leaf)
        self.assertFalse(self.pi.operational)

        msg = tmO.make_message(value=True)
        tmO.try_publish(msg)

        while not self.pi.operational:
            pass
        self.assertTrue(self.pi.operational)

        msg.value = False
        tmO.try_publish(msg)

        while self.pi.operational:
            pass
        self.assertFalse(self.pi.operational)

        tmO.destroy()
        tmB.destroy()
        dep.destroy()
        tmA.destroy()

if __name__ == '__main__':
    unittest.main()

