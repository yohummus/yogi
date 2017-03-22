#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest
import time
from test_terminals import AsyncCall


class TestRawTerminals(unittest.TestCase):
    def setUp(self):
        self.scheduler = yogi.Scheduler()
        self.leafA = yogi.Leaf(self.scheduler)
        self.leafB = yogi.Leaf(self.scheduler)
        self.connection = None

        def makeTerminals(leaf):
            terminals = {}
            terminals['dm'] = yogi.RawDeafMuteTerminal('DM Terminal', yogi.Signature(0x12345678), leaf=leaf)
            terminals['ps'] = yogi.RawPublishSubscribeTerminal('PS Terminal', 0x12345678, leaf=leaf)
            terminals['cps'] = yogi.RawCachedPublishSubscribeTerminal('CPS Terminal', 0x12345678, leaf=leaf)
            terminals['sg'] = yogi.RawScatterGatherTerminal('SG Terminal', 0x12345678, leaf=leaf)
            terminals['pcProducer'] = yogi.RawProducerTerminal('PC Terminal', 0x12345678, leaf=leaf)
            terminals['pcConsumer'] = yogi.RawConsumerTerminal('PC Terminal', 0x12345678, leaf=leaf)
            terminals['cpcProducer'] = yogi.RawCachedProducerTerminal('CPC Terminal', 0x12345678, leaf=leaf)
            terminals['cpcConsumer'] = yogi.RawCachedConsumerTerminal('CPC Terminal', 0x12345678, leaf=leaf)
            terminals['msMaster'] = yogi.RawMasterTerminal('MS Terminal', 0x12345678, leaf=leaf)
            terminals['msSlave'] = yogi.RawSlaveTerminal('MS Terminal', 0x12345678, leaf=leaf)
            terminals['cmsMaster'] = yogi.RawCachedMasterTerminal('CMS Terminal', 0x12345678, leaf=leaf)
            terminals['cmsSlave'] = yogi.RawCachedSlaveTerminal('CMS Terminal', 0x12345678, leaf=leaf)
            terminals['scService'] = yogi.RawServiceTerminal('SC Terminal', 0x12345678, leaf=leaf)
            terminals['scClient'] = yogi.RawClientTerminal('SC Terminal', 0x12345678, leaf=leaf)
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

    def test_RawDeafMuteTerminal(self):
        bd = self.bindings['dm']
        self.connect(bd)

    def test_RawPublishSubscribeTerminal(self):
        tmA = self.terminalsA['ps']
        tmB = self.terminalsB['ps']
        bd = self.bindings['ps']

        msg = bytes([1, 2, 3])

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
                self.assertEqual(bytes([1, 2, 3]), msg)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

    def test_RawCachedPublishSubscribeTerminal(self):
        tmA = self.terminalsA['cps']
        tmB = self.terminalsB['cps']
        bd = self.bindings['cps']

        msg = bytes([1, 2, 3])

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
                self.assertEqual(bytes([1, 2, 3]), msg)
                self.assertTrue(cached)

            tmA.async_receive_message(wrap(fn))
            self.connect(bd)

        # successfully receive non-cached message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(bytes([1, 2, 3]), msg)
                self.assertFalse(cached)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

        # get cached message
        msg = tmA.get_cached_message()
        self.assertEqual(bytes([1, 2, 3]), msg)

    def test_RawScatterGatherTerminal(self):
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

        msg = bytes([1, 2, 3])

        with AsyncCall() as wrap:
            def fn(res, scattered_msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(bytes([1, 2, 3]), scattered_msg.message)
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
                self.assertEqual(bytes([1, 2, 3]), scattered_msg.message)

                response = bytes([4, 5, 6])
                scattered_msg.respond(response)

            tmA.async_receive_scattered_message(wrap(fn))

            with AsyncCall() as wrap2:
                def fn2(res, gathered_msg):
                    self.assertEqual(yogi.Success(), res)
                    self.assertTrue(gathered_msg.flags & yogi.GatherFlags.FINISHED)
                    self.assertEqual(bytes([4, 5, 6]), gathered_msg.message)
                    return yogi.ControlFlow.STOP

                tmB.async_scatter_gather(msg, wrap2(fn2))

        # cancel operation
        with AsyncCall() as wrap:
            def fn(res, scattered_msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(bytes([1, 2, 3]), scattered_msg.message)
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

        msg = bytes([1, 2, 3])

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
                self.assertEqual(bytes([1, 2, 3]), msg)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

    def test_CachedProducerConsumerTerminals(self):
        tmA = self.terminalsA['cpcConsumer']
        tmB = self.terminalsB['cpcProducer']

        msg = bytes([1, 2, 3])

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
                self.assertEqual(bytes([1, 2, 3]), msg)
                self.assertTrue(cached)

            tmA.async_receive_message(wrap(fn))
            self.connect(tmA)

        # successfully receive non-cached message
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(bytes([1, 2, 3]), msg)
                self.assertFalse(cached)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

        # get cached message
        msg = tmA.get_cached_message()
        self.assertEqual(bytes([1, 2, 3]), msg)

    def test_MasterSlaveTerminals(self):
        tmA = self.terminalsB['msMaster']
        tmB = self.terminalsA['msSlave']

        msg = bytes([1, 2, 3])

        # publish without the terminals being bound
        self.assertRaises(yogi.Failure, lambda: tmA.publish(msg))
        self.assertEqual(False, tmA.try_publish(msg))

        self.assertRaises(yogi.Failure, lambda: tmB.publish(msg))
        self.assertEqual(False, tmB.try_publish(msg))

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
                self.assertEqual(bytes([1, 2, 3]), msg)

            tmB.async_receive_message(wrap(fn))
            tmA.publish(msg)

        self.assertTrue(tmA.try_publish(msg))

        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(bytes([1, 2, 3]), msg)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

    def test_CachedMasterSlaveTerminals(self):
        tmA = self.terminalsA['cmsMaster']
        tmB = self.terminalsB['cmsSlave']

        msg = bytes([1, 2, 3])

        # publish without the terminals being bound
        self.assertRaises(yogi.Failure, lambda: tmA.publish(msg))
        self.assertEqual(False, tmA.try_publish(msg))

        self.assertRaises(yogi.Failure, lambda: tmB.publish(msg))
        self.assertEqual(False, tmB.try_publish(msg))

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
                self.assertEqual(bytes([1, 2, 3]), msg)
                self.assertTrue(cached)

            tmB.async_receive_message(wrap(fn))

            with AsyncCall() as wrap:
                def fn(res, msg, cached):
                    self.assertEqual(yogi.Success(), res)
                    self.assertEqual(bytes([1, 2, 3]), msg)
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
                self.assertEqual(bytes([1, 2, 3]), msg)
                self.assertFalse(cached)

            tmB.async_receive_message(wrap(fn))
            tmA.publish(msg)

        self.assertTrue(tmA.try_publish(msg))

        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(bytes([1, 2, 3]), msg)
                self.assertFalse(cached)

            tmA.async_receive_message(wrap(fn))
            tmB.publish(msg)

        self.assertTrue(tmB.try_publish(msg))

        # get cached message
        msg = tmA.get_cached_message()
        self.assertEqual(bytes([1, 2, 3]), msg)

        msg = tmB.get_cached_message()
        self.assertEqual(bytes([1, 2, 3]), msg)

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

        msg = bytes([1, 2, 3])

        with AsyncCall() as wrap:
            def fn(res, request):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(bytes([1, 2, 3]), request.message)
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
                self.assertEqual(bytes([1, 2, 3]), request.message)

                response = bytes([4, 5, 6])
                request.respond(response)

            tmA.async_receive_request(wrap(fn))

            with AsyncCall() as wrap2:
                def fn2(res, gathered_msg):
                    self.assertEqual(yogi.Success(), res)
                    self.assertTrue(gathered_msg.flags & yogi.GatherFlags.FINISHED)
                    self.assertEqual(bytes([4, 5, 6]), gathered_msg.message)
                    return yogi.ControlFlow.STOP

                tmB.async_request(msg, wrap2(fn2))

        # cancel operation
        with AsyncCall() as wrap:
            def fn(res, request):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual(bytes([1, 2, 3]), request.message)
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
