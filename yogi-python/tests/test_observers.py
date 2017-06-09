#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest

from proto import yogi_0000d007_pb2


class TestObservers(unittest.TestCase):
    def setUp(self):
        self.scheduler = yogi.Scheduler()
        self.leafA = yogi.Leaf(self.scheduler)
        self.leafB = yogi.Leaf(self.scheduler)
        self.connection = None

        self.producer = yogi.ProducerTerminal('T', yogi_0000d007_pb2, leaf=self.leafA)
        self.consumer = yogi.ConsumerTerminal('T', yogi_0000d007_pb2, leaf=self.leafB)
        self.cachedProducer = yogi.CachedProducerTerminal('T', yogi_0000d007_pb2, leaf=self.leafA)
        self.cachedConsumer = yogi.CachedConsumerTerminal('T', yogi_0000d007_pb2, leaf=self.leafB)
        self.scatterGatherA = yogi.ScatterGatherTerminal('T', yogi_0000d007_pb2, leaf=self.leafA)
        self.scatterGatherB = yogi.ScatterGatherTerminal('T', yogi_0000d007_pb2, leaf=self.leafB)
        self.scatterGatherBinding = yogi.Binding(self.scatterGatherA, 'T')
        self.service = yogi.ServiceTerminal('T', yogi_0000d007_pb2, leaf=self.leafA)
        self.client = yogi.ClientTerminal('T', yogi_0000d007_pb2, leaf=self.leafB)

    def tearDown(self):
        self.producer.destroy()
        self.consumer.destroy()
        self.cachedProducer.destroy()
        self.cachedConsumer.destroy()
        self.scatterGatherBinding.destroy()
        self.scatterGatherA.destroy()
        self.scatterGatherB.destroy()
        self.service.destroy()
        self.client.destroy()

        try:
            self.connection.destroy()
        except:
            pass

        self.leafA.destroy()
        self.leafB.destroy()
        self.scheduler.destroy()

    def connect(self, binder, subscribable):
        self.assertIs(yogi.BindingState.RELEASED, binder.get_binding_state())
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)
        while binder.get_binding_state() is yogi.BindingState.RELEASED:
            pass
        while subscribable.get_subscription_state() is yogi.SubscriptionState.UNSUBSCRIBED:
            pass
        self.assertIs(yogi.BindingState.ESTABLISHED, binder.get_binding_state())

    def test_init(self):
        pass

    def test_BindingObserver(self):
        observer = yogi.BindingObserver(self.consumer)

        i = [0]

        def fn1(state):
            if i[0] is 0:
                self.assertIs(yogi.BindingState.RELEASED, state)
            elif i[0] is 1:
                self.assertIs(yogi.BindingState.ESTABLISHED, state)
            elif i[0] is 2:
                self.assertIs(yogi.BindingState.RELEASED, state)
            i[0] += 1

        observer_id = observer.add(fn1)

        j = [0]

        def fn2(state):
            if j[0] is 0:
                self.assertIs(yogi.BindingState.RELEASED, state)
            elif j[0] is 1:
                self.assertIs(yogi.BindingState.ESTABLISHED, state)
            elif j[0] is 2:
                self.assertIs(yogi.BindingState.RELEASED, state)
            elif j[0] is 3:
                self.assertIs(yogi.BindingState.ESTABLISHED, state)
            j[0] += 1

        observer.add(fn2)

        self.assertIs(0, i[0])
        self.assertIs(0, j[0])

        observer.start()
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)
        while self.consumer.get_binding_state() is yogi.BindingState.RELEASED:
            pass
        self.connection.destroy()
        while self.consumer.get_binding_state() is yogi.BindingState.ESTABLISHED:
            pass

        observer.remove(observer_id)
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)
        while self.consumer.get_binding_state() is yogi.BindingState.RELEASED:
            pass

        observer.stop()
        self.connection.destroy()
        while self.consumer.get_binding_state() is yogi.BindingState.ESTABLISHED:
            pass

        self.assertIs(3, i[0])
        self.assertIs(4, j[0])

    def test_SubscriptionObserver(self):
        observer = yogi.SubscriptionObserver(self.producer)

        i = [0]

        def fn1(state):
            if i[0] is 0:
                self.assertIs(yogi.SubscriptionState.UNSUBSCRIBED, state)
            elif i[0] is 1:
                self.assertIs(yogi.SubscriptionState.SUBSCRIBED, state)
            elif i[0] is 2:
                self.assertIs(yogi.SubscriptionState.UNSUBSCRIBED, state)
            i[0] += 1

        observer_id = observer.add(fn1)

        j = [0]

        def fn2(state):
            if j[0] is 0:
                self.assertIs(yogi.SubscriptionState.UNSUBSCRIBED, state)
            elif j[0] is 1:
                self.assertIs(yogi.SubscriptionState.SUBSCRIBED, state)
            elif j[0] is 2:
                self.assertIs(yogi.SubscriptionState.UNSUBSCRIBED, state)
            elif j[0] is 3:
                self.assertIs(yogi.SubscriptionState.SUBSCRIBED, state)
            j[0] += 1

        observer.add(fn2)

        self.assertIs(0, i[0])
        self.assertIs(0, j[0])

        observer.start()
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)
        while self.consumer.get_binding_state() is yogi.BindingState.RELEASED:
            pass
        self.connection.destroy()
        while self.consumer.get_binding_state() is yogi.BindingState.ESTABLISHED:
            pass

        observer.remove(observer_id)
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)
        while self.consumer.get_binding_state() is yogi.BindingState.RELEASED:
            pass

        observer.stop()
        self.connection.destroy()
        while self.consumer.get_binding_state() is yogi.BindingState.ESTABLISHED:
            pass

        self.assertIs(3, i[0])
        self.assertIs(4, j[0])

    def test_MessageObserverPublishNonCached(self):
        observer = yogi.MessageObserver(self.consumer)

        i = [0]

        def fn1(msg):
            i[0] += 1
            self.assertEqual(123, msg.value)

        callback_id = observer.add(fn1)

        def fn2(msg, cached=False):
            i[0] += 1
            self.assertEqual(123, msg.value)
            self.assertFalse(cached)

        observer.add(fn2)

        self.assertIs(0, i[0])

        observer.start()
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)

        msg = self.producer.make_message(value=123)
        while not self.producer.try_publish(msg):
            pass
        while i[0] < 2:
            pass
        self.assertIs(2, i[0])

        observer.remove(callback_id)
        self.producer.publish(msg)
        while i[0] < 3:
            pass
        self.assertIs(3, i[0])

        observer.stop()
        self.producer.publish(msg)
        self.assertIs(3, i[0])

    def test_MessageObserverPublishCached(self):
        observer = yogi.MessageObserver(self.cachedConsumer)

        i = [0]

        def fn(msg, cached):
            i[0] += 1
            self.assertEqual(123, msg.value)
            if i[0] < 3:
                self.assertTrue(cached)
            else:
                self.assertFalse(cached)

        callback_id = observer.add(fn)
        observer.add(fn)

        self.assertIs(0, i[0])

        msg = self.cachedProducer.make_message(value=123)
        self.cachedProducer.try_publish(msg)

        observer.start()
        self.connection = yogi.LocalConnection(self.leafA, self.leafB)

        while i[0] < 2:
            pass
        self.assertIs(2, i[0])

        observer.remove(callback_id)
        self.cachedProducer.publish(msg)
        while i[0] < 3:
            pass
        self.assertIs(3, i[0])

        observer.stop()
        self.cachedProducer.publish(msg)
        self.assertIs(3, i[0])

    def test_MessageObserverScatterGather(self):
        self.connect(self.scatterGatherBinding, self.scatterGatherB)

        observer = yogi.MessageObserver(self.scatterGatherA)

        i = [0]
        msg = self.scatterGatherB.make_scatter_message()
        msg.value = 'Hello'

        def fn1(res, response):
            self.assertTrue(res)
            self.assertTrue(response.flags & yogi.GatherFlags.DEAF)
            i[0] += 1
            return yogi.ControlFlow.STOP

        self.scatterGatherB.async_scatter_gather(msg, fn1)

        while i[0] < 1:
            pass
        self.assertIs(1, i[0])

        observer.start()

        def fn2(res, response):
            self.assertTrue(res)
            self.assertTrue(response.flags & yogi.GatherFlags.IGNORED)
            i[0] += 1
            return yogi.ControlFlow.STOP

        self.scatterGatherB.async_scatter_gather(msg, fn2)

        while i[0] < 2:
            pass
        self.assertIs(2, i[0])

        def fn3(request):
            self.assertEqual('Hello', request.message.value)
            msg = self.scatterGatherA.make_gather_message()
            msg.value = 123
            request.respond(msg)

        observer.set(fn3)

        def fn4(res, response):
            self.assertTrue(res)
            self.assertEqual(123, response.message.value)
            i[0] += 1
            return yogi.ControlFlow.STOP

        self.scatterGatherB.async_scatter_gather(msg, fn4)

        while i[0] < 3:
            pass
        self.assertIs(3, i[0])

        observer.clear()

        self.scatterGatherB.async_scatter_gather(msg, fn2)

        while i[0] < 4:
            pass
        self.assertIs(4, i[0])

    def test_MessageObserverRequestResponse(self):
        self.connect(self.service, self.client)

        observer = yogi.MessageObserver(self.service)

        i = [0]
        msg = self.client.make_request_message()
        msg.value = 'Hello'

        def fn1(res, response):
            self.assertTrue(res)
            self.assertTrue(response.flags & yogi.GatherFlags.DEAF)
            i[0] += 1
            return yogi.ControlFlow.STOP

        self.client.async_request(msg, fn1)

        while i[0] < 1:
            pass
        self.assertIs(1, i[0])

        observer.start()

        def fn2(res, response):
            self.assertTrue(res)
            self.assertTrue(response.flags & yogi.GatherFlags.IGNORED)
            i[0] += 1
            return yogi.ControlFlow.STOP

        self.client.async_request(msg, fn2)

        while i[0] < 2:
            pass
        self.assertIs(2, i[0])

        def fn3(request):
            self.assertEqual('Hello', request.message.value)
            msg = self.scatterGatherA.make_gather_message()
            msg.value = 123
            request.respond(msg)

        observer.set(fn3)

        def fn4(res, response):
            self.assertTrue(res)
            self.assertEqual(123, response.message.value)
            i[0] += 1
            return yogi.ControlFlow.STOP

        self.client.async_request(msg, fn4)

        while i[0] < 3:
            pass
        self.assertIs(3, i[0])

        observer.clear()

        self.client.async_request(msg, fn2)

        while i[0] < 4:
            pass
        self.assertIs(4, i[0])


if __name__ == '__main__':
    unittest.main()
