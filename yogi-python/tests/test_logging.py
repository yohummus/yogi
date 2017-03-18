#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest
import json

from test_terminals import AsyncCall
from proto import yogi_000009cd_pb2
from proto import yogi_00000004_pb2


class TestLogging(unittest.TestCase):
    def change_verbosity(self, verbosity_terminal, verbosity):
        with AsyncCall() as wrap:
            def fn(res, msg, cached):
                self.assertEqual(yogi.Success(), res)
                self.assertFalse(cached)
                self.assertEqual(verbosity.value, msg.value)

            verbosity_terminal.async_receive_message(wrap(fn))
            verbosity_terminal.publish(verbosity_terminal.make_message(value=verbosity.value))

    def test_AppLogger(self):
        yogi.log_info('Hey dude')

    def test_ComponentLogger(self):
        logger = yogi.Logger('My Component')
        logger.log_info('Hey dude ', 123, '!')
        logger.log(yogi.Verbosity.INFO, 'Hello', ' you')

    def test_NonColourised(self):
        yogi.Logger.app_logger.stdout_verbosity = yogi.Verbosity.TRACE

        yogi.log_fatal('FATAL')
        yogi.log_error('ERROR')
        yogi.log_warning('WARNING')
        yogi.log_info('INFO')
        yogi.log_debug('DEBUG')
        yogi.log_trace('TRACE')

    def test_Colourised(self):
        yogi.Logger.colourised_stdout = True
        yogi.Logger.app_logger.stdout_verbosity = yogi.Verbosity.TRACE

        yogi.log_fatal('FATAL')
        yogi.log_error('ERROR')
        yogi.log_warning('WARNING')
        yogi.log_info('INFO')
        yogi.log_debug('DEBUG')
        yogi.log_trace('TRACE')

    def test_EffectiveVerbosity(self):
        yogi.Logger().yogi_verbosity = yogi.Verbosity.INFO
        yogi.Logger.max_yogi_verbosity = yogi.Verbosity.ERROR

        self.assertIs(yogi.Verbosity.ERROR, yogi.Logger().effective_yogi_verbosity)
        yogi.Logger.max_yogi_verbosity = yogi.Verbosity.DEBUG
        self.assertIs(yogi.Verbosity.INFO, yogi.Logger().effective_yogi_verbosity)

        yogi.Logger().stdout_verbosity = yogi.Verbosity.INFO
        yogi.Logger.max_stdout_verbosity = yogi.Verbosity.ERROR
        self.assertIs(yogi.Verbosity.ERROR, yogi.Logger().effective_stdout_verbosity)
        yogi.Logger.max_stdout_verbosity = yogi.Verbosity.DEBUG
        self.assertIs(yogi.Verbosity.INFO, yogi.Logger().effective_stdout_verbosity)

        yogi.Logger().yogi_verbosity = yogi.Verbosity.TRACE
        self.assertIs(yogi.Verbosity.DEBUG, yogi.Logger().max_effective_verbosity)

    def test_ProcessInterface(self):
        yogi.Logger.colourised_stdout = False

        argv = ['-j', '''
            {
                "logging": {
                    "stdout": {
                        "colourised": true,
                        "max-verbosity": "DEBUG",
                        "component-verbosity": {
                            "app"      : "ERROR",
                            "cppyogi" : "FATAL",
                            "test"     : "TRACE"
                        }
                    },
                    "yogi": {
                        "max-verbosity": "INFO",
                        "component-verbosity": {
                            "app"      : "DEBUG",
                            "cppyogi" : "INFO",
                            "test"     : "WARNING"
                        }
                    }
                }
            }''']

        scheduler = yogi.Scheduler()
        leaf = yogi.Leaf(scheduler)
        pi = yogi.ProcessInterface(argv)

        # check initial configuration
        self.assertTrue(yogi.Logger.colourised_stdout)
        self.assertIs(yogi.Verbosity.DEBUG, yogi.Logger.max_stdout_verbosity)
        self.assertIs(yogi.Verbosity.ERROR, yogi.Logger().stdout_verbosity)
        self.assertIs(yogi.Verbosity.ERROR, yogi.Logger('app').stdout_verbosity)
        self.assertIs(yogi.Verbosity.FATAL, yogi.Logger('cppyogi').stdout_verbosity)
        self.assertIs(yogi.Verbosity.TRACE, yogi.Logger('test').stdout_verbosity)

        self.assertIs(yogi.Verbosity.INFO, yogi.Logger.max_yogi_verbosity)
        self.assertIs(yogi.Verbosity.DEBUG, yogi.Logger().yogi_verbosity)
        self.assertIs(yogi.Verbosity.DEBUG, yogi.Logger('app').yogi_verbosity)
        self.assertIs(yogi.Verbosity.INFO, yogi.Logger('cppyogi').yogi_verbosity)
        self.assertIs(yogi.Verbosity.WARNING, yogi.Logger('test').yogi_verbosity)

        # check logging over YOGI
        log_terminal = yogi.ConsumerTerminal('/Process/Log', yogi_000009cd_pb2, leaf=leaf)
        connection = yogi.LocalConnection(leaf, pi.leaf)
        while log_terminal.get_binding_state() is yogi.BindingState.RELEASED:
            pass

        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual('Hello', msg.value.first)
                info = json.loads(msg.value.second)
                self.assertEqual('app', info['component'])
                self.assertEqual('FATAL', info['severity'])
                self.assertGreater(info['thread'], 0)

            log_terminal.async_receive_message(wrap(fn))
            yogi.log_fatal('Hello')

        # check changing verbosity levels at runtime
        logger = yogi.Logger('My Component')
        max_verbosity_terminal = yogi.CachedSlaveTerminal('/Process/YOGI Log Verbosity/Max Verbosity',
                                                             yogi_00000004_pb2, leaf=leaf)
        while max_verbosity_terminal.get_binding_state() is yogi.BindingState.RELEASED:
            pass
        while max_verbosity_terminal.get_subscription_state() is yogi.SubscriptionState.UNSUBSCRIBED:
            pass

        self.change_verbosity(max_verbosity_terminal, yogi.Verbosity.INFO)

        verbosity_terminal = yogi.CachedSlaveTerminal('/Process/YOGI Log Verbosity/Components/My Component',
                                                         yogi_00000004_pb2, leaf=leaf)
        while verbosity_terminal.get_binding_state() is yogi.BindingState.RELEASED:
            pass
        while verbosity_terminal.get_subscription_state() is yogi.SubscriptionState.UNSUBSCRIBED:
            pass

        self.change_verbosity(verbosity_terminal, yogi.Verbosity.DEBUG)

        # check that new verbosity levels are being applied
        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual('Go', msg.value.first)

            log_terminal.async_receive_message(wrap(fn))
            logger.log_info('Go')

        self.change_verbosity(verbosity_terminal, yogi.Verbosity.WARNING)

        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual('Vegan', msg.value.first)

            log_terminal.async_receive_message(wrap(fn))
            logger.log_info('Welcome')
            logger.log_warning('Vegan')

        self.change_verbosity(max_verbosity_terminal, yogi.Verbosity.ERROR)

        with AsyncCall() as wrap:
            def fn(res, msg):
                self.assertEqual(yogi.Success(), res)
                self.assertEqual('Mate', msg.value.first)

            log_terminal.async_receive_message(wrap(fn))
            logger.log_warning('Dude')
            logger.log_error('Mate')

        # clean up
        connection.destroy()
        verbosity_terminal.destroy()
        max_verbosity_terminal.destroy()
        log_terminal.destroy()
        leaf.destroy()
        scheduler.destroy()
        pi.destroy()

if __name__ == '__main__':
    unittest.main()

