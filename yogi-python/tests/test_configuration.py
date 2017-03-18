#!/usr/bin/env python3
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + '/..')

import yogi
import unittest
import os


def make_config_path(suffix: str) -> str:
    return os.path.dirname(os.path.abspath(__file__)) + '/config_{}.json'.format(suffix)


class TestConfiguration(unittest.TestCase):
    def test_empty_configuration(self):
        cfg = yogi.Configuration(argv=None)
        self.assertEqual(yogi.Path('/'), cfg.location)
        self.assertIsNone(cfg.connection_target)
        self.assertIsNone(cfg.connection_timeout)
        self.assertIsNone(cfg.connection_identification)

    def test_update(self):
        cfg = yogi.Configuration(['test.py', make_config_path('a')])
        cfg.update('{"yogi": {"location": "/Home"}}')
        self.assertEqual(yogi.Path("/Home"), cfg.location)
        self.assertRaises(yogi.BadConfiguration, lambda: cfg.update('{'))

    def test_config_file(self):
        cfg = yogi.Configuration(['test.py', make_config_path('a')])
        self.assertEqual(yogi.Path('/Test'), cfg.location)
        self.assertEqual('localhost:12345', cfg.connection_target)
        self.assertAlmostEqual(1.234, cfg.connection_timeout)
        self.assertEqual('Hello World', cfg.connection_identification)

    def test_config_file_priority(self):
        cfg = yogi.Configuration(['test.py', make_config_path('a'), make_config_path('b')])
        self.assertEqual(yogi.Path('/Pudding'), cfg.location)
        self.assertIsNone(cfg.connection_target)

    def test_command_line_override(self):
        cfg = yogi.Configuration(['test.py', make_config_path('a'), '--connection_target=my-host:1234',
                                     '--connection_timeout=0.555', '-i', 'Dude', '--location=/Home'])
        self.assertEqual(yogi.Path('/Home'), cfg.location)
        self.assertEqual('my-host:1234', cfg.connection_target)
        self.assertAlmostEqual(0.555, cfg.connection_timeout)
        self.assertEqual('Dude', cfg.connection_identification)

    def test_json_overrides(self):
        cfg = yogi.Configuration(['test.py', make_config_path('a'), '--json={"my-age": 42}', '-j', '{"my-id": 55}',
                                     '--json={"yogi": {"location": "/Somewhere"}}', '--location=/Home'])
        self.assertEqual(42, cfg.config['my-age'])
        self.assertEqual(55, cfg.config['my-id'])
        self.assertEqual(yogi.Path('/Home'), cfg.location)

    def test_bad_configuration_file(self):
        self.assertRaises(yogi.BadConfiguration, lambda: yogi.Configuration(['test.py', make_config_path('c')]))

    def test_bad_command_line(self):
        self.assertRaises(yogi.BadCommandLine, lambda: yogi.Configuration(['test.py', '--hey-dude']))

    def test_str(self):
        cfg = yogi.Configuration(['test.py', make_config_path('a')])
        self.assertRegexpMatches(str(cfg), r'.*localhost:12345.*')

if __name__ == '__main__':
    unittest.main()

