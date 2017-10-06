#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest
import time

from proto import yogi_0000d007_pb2


class AsyncCall:
    def __init__(self):
        self._called = False

    def __enter__(self):
        def wrap(fn):
            def clb(res, *args):
                self._called = True
                fn(res, *args)
            return clb

        return wrap

    def __exit__(self, exc_type, exc_val, exc_tb):
        while exc_type is None and self._called is False:
            pass


class TestTerminals(unittest.TestCase):
    def setUp(self):
        self.scheduler = yogi.Scheduler()
        self.leafA = yogi.Leaf(self.scheduler)
        self.leafB = yogi.Leaf(self.scheduler)
        self.connection = None

        def makeTerminals(leaf):
            terminals = {}
            terminals['dm'] = yogi.DeafMuteTerminal('DM Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['ps'] = yogi.PublishSubscribeTerminal('PS Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['cps'] = yogi.CachedPublishSubscribeTerminal('CPS Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['sg'] = yogi.ScatterGatherTerminal('SG Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['pcProducer'] = yogi.ProducerTerminal('PC Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['pcConsumer'] = yogi.ConsumerTerminal('PC Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['cpcProducer'] = yogi.CachedProducerTerminal('CPC Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['cpcConsumer'] = yogi.CachedConsumerTerminal('CPC Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['msMaster'] = yogi.MasterTerminal('MS Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['msSlave'] = yogi.SlaveTerminal('MS Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['cmsMaster'] = yogi.CachedMasterTerminal('CMS Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['cmsSlave'] = yogi.CachedSlaveTerminal('CMS Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['scService'] = yogi.ServiceTerminal('SC Terminal', yogi_0000d007_pb2, leaf=leaf)
            terminals['scClient'] = yogi.ClientTerminal('SC Terminal', yogi_0000d007_pb2, leaf=leaf)
            return terminals

        self.terminalsA = makeTerminals(self.leafA)
        self.terminalsB = makeTerminals(self.leafB)

        self.bindings = {
            'dm': yogi.Binding(self.terminalsA['dm'], self.terminalsA['dm'].name),
            'ps': yogi.Binding(self.terminalsA['ps'], self.terminalsA['ps'].name),
            'cps': yogi.Binding(self.terminalsA['cps'], self.terminalsA['cps'].name),
            'sg': yogi.Binding(self.terminalsA['sg'], self.terminalsA['sg'].name)
        }

    def tearDown(self):
        for _, binding in self.bindings.items():
            binding.destroy()

        for _, terminal in self.terminalsA.items():
            terminal.destroy()

        for _, terminal in self.terminalsB.items():
            terminal.destroy()

        if self.connection:
            self.connection.destroy()

        self.leafA.destroy()
        self.leafB.destroy()
        self.scheduler.destroy()

    def connect(self, binder, subscribable=None):
        self.assertIs(yogi.BindingState.RELEASED, binder.get_binding_state())
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)
        while binder.get_binding_state() is yogi.BindingState.RELEASED:
            pass
        while subscribable and subscribable.get_subscription_state() is yogi.SubscriptionState.UNSUBSCRIBED:
            pass
        self.assertIs(yogi.BindingState.ESTABLISHED, binder.get_binding_state())

    def test_init(self):
        pass

    def test_DeafMuteTerminal(self):
        bd = self.bindings['dm']
        self.connect(bd)

    def test_PublishSubscribeTerminal(self):
        tmA = self.terminalsA['ps']
        tmB = self.terminalsB['ps']
        bd = self.bindings['ps']

        msg = tmB.make_message(value=123)

        # publish without the terminal being bound
        self.assertRaises(yogi.Failure, lambda: tmB.publish(msg))
        self.assertEqual(False, tmB.try_publish(msg))

        # cancel receive message
        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(msg)

            tmA.async_receive_message(wrap(fn))
            tmA.cancel_receive_message()

        # successfully receive message
        self.connect(bd)
        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(123, msg.value)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

    def test_CachedPublishSubscribeTerminal(self):
        tmA = self.terminalsA['cps']
        tmB = self.terminalsB['cps']
        bd = self.bindings['cps']

        msg = tmB.make_message(value=123)

        # publish without the terminal being bound
        self.assertRaises(yogi.Failure, lambda: tmB.publish(msg))
        self.assertEqual(False, tmB.try_publish(msg))

        # cancel receive message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(msg)
            tmA.async_receive_message(wrap(fn))
            tmA.cancel_receive_message()

        # successfully receive cached message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(123, msg.value)
                self.assertTrue(cached)

            tmA.async_receive_message(wrap(fn))
            self.connect(bd)

        # successfully receive non-cached message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(123, msg.value)
                self.assertFalse(cached)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

        # get cached message
        msg = tmA.get_cached_message()
        self.assertEqual(123, msg.value)

    def test_ScatterGatherTerminal(self):
        tmA = self.terminalsA['sg']
        tmB = self.terminalsB['sg']
        bd = self.bindings['sg']

        # cancel receive message
        with AsyncCall() as wrap:
            def fn(res, scattered_msg):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(scattered_msg)

            tmA.async_receive_scattered_message(wrap(fn))
            tmA.cancel_receive_scattered_message()

        # ignore message
        self.connect(bd, tmB)

        msg = tmB.make_scatter_message()
        msg.value = "test"

        with AsyncCall() as wrap:
            def fn(res, scattered_msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual("test", scattered_msg.message.value)
                scattered_msg.ignore()
            tmA.async_receive_scattered_message(wrap(fn))

            with AsyncCall() as wrap2:
                def fn2(res, gathered_msg):
                    self.assertEqual(yogi.Success(), res)
                    self.assertTrue(gathered_msg.flags & yogi.GatherFlags.IGNORED)
                    return yogi.ControlFlow.STOP

                tmB.async_scatter_gather(msg, wrap2(fn2))

        # respond to message
        with AsyncCall() as wrap:
            def fn(res, scattered_msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual("test", scattered_msg.message.value)

                response = tmA.make_gather_message()
                response.value = 123
                scattered_msg.respond(response)

            tmA.async_receive_scattered_message(wrap(fn))

            with AsyncCall() as wrap2:
                def fn2(res, gathered_msg):
                    self.assertEqual(yogi.Success(), res)
                    self.assertTrue(gathered_msg.flags & yogi.GatherFlags.FINISHED)
                    self.assertEqual(123, gathered_msg.message.value)
                    return yogi.ControlFlow.STOP

                tmB.async_scatter_gather(msg, wrap2(fn2))

        # cancel operation
        with AsyncCall() as wrap:
            def fn(res, scattered_msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual("test", scattered_msg.message.value)
                # do not respond

            tmA.async_receive_scattered_message(wrap(fn))

            with AsyncCall() as wrap2:
                def fn2(res, gathered_msg):
                    self.assertEqual(yogi.Canceled(), res)
                    self.assertIsNone(gathered_msg)

                operation = tmB.async_scatter_gather(msg, wrap2(fn2))
                operation.cancel()

    def test_ProducerConsumerTerminals(self):
        tmA = self.terminalsB['pcConsumer']
        tmB = self.terminalsA['pcProducer']

        msg = tmB.make_message(value=123)

        # publish without the terminal being bound
        self.assertRaises(yogi.Failure, lambda: tmB.publish(msg))
        self.assertEqual(False, tmB.try_publish(msg))

        # cancel receive message
        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(msg)

            tmA.async_receive_message(wrap(fn))
            tmA.cancel_receive_message()

        # successfully receive message
        self.connect(tmA)
        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(123, msg.value)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

    def test_CachedProducerConsumerTerminals(self):
        tmA = self.terminalsA['cpcConsumer']
        tmB = self.terminalsB['cpcProducer']

        msg = tmB.make_message(value=123)

        # publish without the terminal being bound
        self.assertRaises(yogi.Failure, lambda: tmB.publish(msg))
        self.assertEqual(False, tmB.try_publish(msg))

        # cancel receive message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(msg)
            tmA.async_receive_message(wrap(fn))
            tmA.cancel_receive_message()

        # successfully receive cached message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(123, msg.value)
                self.assertTrue(cached)

            tmA.async_receive_message(wrap(fn))
            self.connect(tmA)

        # successfully receive non-cached message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(123, msg.value)
                self.assertFalse(cached)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

        # get cached message
        msg = tmA.get_cached_message()
        self.assertEqual(123, msg.value)

    def test_MasterSlaveTerminals(self):
        tmA = self.terminalsB['msMaster']
        tmB = self.terminalsA['msSlave']

        master_msg = tmA.make_message(value='Hello')
        slave_msg = tmB.make_message(value=123)

        # publish without the terminals being bound
        self.assertRaises(yogi.Failure, lambda: tmA.publish(master_msg))
        self.assertEqual(False, tmA.try_publish(master_msg))

        self.assertRaises(yogi.Failure, lambda: tmB.publish(slave_msg))
        self.assertEqual(False, tmB.try_publish(slave_msg))

        # cancel receive message
        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(msg)

            tmA.async_receive_message(wrap(fn))
            tmA.cancel_receive_message()

        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(msg)

            tmB.async_receive_message(wrap(fn))
            tmB.cancel_receive_message()

        # establish connection
        self.connect(tmB, tmA)
        while tmA.get_binding_state() == yogi.BindingState.RELEASED:
            pass
        while tmB.get_subscription_state() == yogi.SubscriptionState.UNSUBSCRIBED:
            pass

        # successfully receive message
        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual('Hello', msg.value)

            tmB.async_receive_message(wrap(fn))
            tmA.publish(master_msg)

        self.assertTrue(tmA.try_publish(master_msg))

        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(123, msg.value)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(slave_msg)

        self.assertTrue(tmB.try_publish(slave_msg))

    def test_CachedMasterSlaveTerminals(self):
        tmA = self.terminalsA['cmsMaster']
        tmB = self.terminalsB['cmsSlave']

        master_msg = tmA.make_message(value='Hello')
        slave_msg = tmB.make_message(value=123)

        # publish without the terminals being bound
        self.assertRaises(yogi.Failure, lambda: tmA.publish(master_msg))
        self.assertEqual(False, tmA.try_publish(master_msg))

        self.assertRaises(yogi.Failure, lambda: tmB.publish(slave_msg))
        self.assertEqual(False, tmB.try_publish(slave_msg))

        # cancel receive message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(msg)
            tmA.async_receive_message(wrap(fn))
            tmA.cancel_receive_message()

        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(msg)
            tmB.async_receive_message(wrap(fn))
            tmB.cancel_receive_message()

        # successfully receive cached message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual('Hello', msg.value)
                self.assertTrue(cached)

            tmB.async_receive_message(wrap(fn))

            with AsyncCall() as wrap:
                def fn(res, msg, cached):
                    self.assertEqual(yogi.Success(), res)
                    self.assertEqual(123, msg.value)
                    self.assertTrue(cached)

                tmA.async_receive_message(wrap(fn))

                self.connect(tmB, tmA)
                while tmA.get_binding_state() == yogi.BindingState.RELEASED:
                    pass
                while tmB.get_subscription_state() == yogi.SubscriptionState.UNSUBSCRIBED:
                    pass

        # successfully receive non-cached message
        time.sleep(0.01) # takes care of tmB receiving the bounced-back cached message it sent to tmA on connection
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual('Hello', msg.value)
                self.assertFalse(cached)

            tmB.async_receive_message(wrap(fn))
            tmA.publish(master_msg)

        self.assertTrue(tmA.try_publish(master_msg))

        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(123, msg.value)
                self.assertFalse(cached)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(slave_msg)

        self.assertTrue(tmB.try_publish(slave_msg))

        # get cached message
        slave_msg = tmA.get_cached_message()
        self.assertEqual(123, slave_msg.value)

        master_msg = tmB.get_cached_message()
        self.assertEqual('Hello', master_msg.value)

    def test_ServiceClientTerminals(self):
        tmA = self.terminalsA['scService']
        tmB = self.terminalsB['scClient']

        # cancel receive message
        with AsyncCall() as wrap:
            def fn(res, request):
                self.assertEqual(yogi.Canceled(), res)
                self.assertIsNone(request)

            tmA.async_receive_request(wrap(fn))
            tmA.cancel_receive_request()

        # ignore message
        self.connect(tmA, tmB)

        msg = tmB.make_request_message()
        msg.value = "test"

        with AsyncCall() as wrap:
            def fn(res, request):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual("test", request.message.value)
                request.ignore()
            tmA.async_receive_request(wrap(fn))

            with AsyncCall() as wrap2:
                def fn2(res, gathered_msg):
                    self.assertEqual(yogi.Success(), res)
                    self.assertTrue(gathered_msg.flags & yogi.GatherFlags.IGNORED)
                    return yogi.ControlFlow.STOP

                tmB.async_request(msg, wrap2(fn2))

        # respond to message
        with AsyncCall() as wrap:
            def fn(res, request):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual("test", request.message.value)

                response = tmA.make_response_message()
                response.value = 123
                request.respond(response)

            tmA.async_receive_request(wrap(fn))

            with AsyncCall() as wrap2:
                def fn2(res, gathered_msg):
                    self.assertEqual(yogi.Success(), res)
                    self.assertTrue(gathered_msg.flags & yogi.GatherFlags.FINISHED)
                    self.assertEqual(123, gathered_msg.message.value)
                    return yogi.ControlFlow.STOP

                tmB.async_request(msg, wrap2(fn2))

        # cancel operation
        with AsyncCall() as wrap:
            def fn(res, request):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual("test", request.message.value)
                # do not respond

            tmA.async_receive_request(wrap(fn))

            with AsyncCall() as wrap2:
                def fn2(res, gathered_msg):
                    self.assertEqual(yogi.Canceled(), res)
                    self.assertIsNone(gathered_msg)

                operation = tmB.async_request(msg, wrap2(fn2))
                operation.cancel()


if __name__ == '__main__':
    unittest.main()
