# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2018 Johannes Bergmann.
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


class TestConstants(TestCase):
    def test_version(self):
        c = yogi.Constants.VERSION
        self.assertIsInstance(c, str)
        self.assertGreater(len(c), 4)
        self.assertEqual(c, self.get_core_macro("YOGI_HDR_VERSION"))

    def test_version_major(self):
        c = yogi.Constants.VERSION_MAJOR
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(
            c, self.get_core_macro("YOGI_HDR_VERSION_MAJOR"))

    def test_version_minor(self):
        c = yogi.Constants.VERSION_MINOR
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(
            c, self.get_core_macro("YOGI_HDR_VERSION_MINOR"))

    def test_version_patch(self):
        c = yogi.Constants.VERSION_PATCH
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(
            c, self.get_core_macro("YOGI_HDR_VERSION_PATCH"))

    def test_version_suffix(self):
        c = yogi.Constants.VERSION_SUFFIX
        self.assertIsInstance(c, str)
        self.assertEqual(c, self.get_core_macro("YOGI_HDR_VERSION_SUFFIX"))

    def test_default_adv_interfaces(self):
        c = yogi.Constants.DEFAULT_ADV_INTERFACES
        self.assertIsInstance(c, str)
        self.assertGreater(len(c), 1)

    def test_default_adv_address(self):
        c = yogi.Constants.DEFAULT_ADV_ADDRESS
        self.assertIsInstance(c, str)
        self.assertGreater(len(c), 1)

    def test_default_adv_port(self):
        c = yogi.Constants.DEFAULT_ADV_PORT
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, 1024)

    def test_default_adv_interval(self):
        c = yogi.Constants.DEFAULT_ADV_INTERVAL
        self.assertIsInstance(c, yogi.Duration)
        self.assertAlmostEqual(c.total_seconds, 1.0, delta=10.0)

    def test_default_connection_timeout(self):
        c = yogi.Constants.DEFAULT_CONNECTION_TIMEOUT
        self.assertIsInstance(c, yogi.Duration)
        self.assertAlmostEqual(c.total_seconds, 1.0, delta=10.0)

    def test_default_logger_verbosity(self):
        c = yogi.Constants.DEFAULT_LOGGER_VERBOSITY
        self.assertIsInstance(c, yogi.Verbosity)

    def test_default_log_time_format(self):
        c = yogi.Constants.DEFAULT_LOG_TIME_FORMAT
        self.assertIsInstance(c, str)
        self.assertGreater(len(c), 1)

    def test_default_log_format(self):
        c = yogi.Constants.DEFAULT_LOG_TIME_FORMAT
        self.assertIsInstance(c, str)
        self.assertGreater(len(c), 5)

    def test_max_message_payload_size(self):
        c = yogi.Constants.MAX_MESSAGE_PAYLOAD_SIZE
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, 1000)

    def test_default_time_format(self):
        c = yogi.Constants.DEFAULT_TIME_FORMAT
        self.assertIsInstance(c, str)
        self.assertTrue(".%3" in c)

    def test_default_inf_duration_string(self):
        c = yogi.Constants.DEFAULT_INF_DURATION_STRING
        self.assertIsInstance(c, str)
        self.assertTrue("inf" in c)

    def test_default_duration_format(self):
        c = yogi.Constants.DEFAULT_DURATION_FORMAT
        self.assertIsInstance(c, str)
        self.assertTrue(".%3" in c)

    def test_default_invalid_handle_string(self):
        c = yogi.Constants.DEFAULT_INVALID_HANDLE_STRING
        self.assertIsInstance(c, str)
        self.assertTrue("INVALID" in c)

    def test_default_object_format(self):
        c = yogi.Constants.DEFAULT_OBJECT_FORMAT
        self.assertIsInstance(c, str)
        self.assertTrue("$T" in c)

    def test_min_tx_queue_size(self):
        c = yogi.Constants.MIN_TX_QUEUE_SIZE
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, yogi.Constants.MAX_MESSAGE_PAYLOAD_SIZE)

    def test_max_tx_queue_size(self):
        c = yogi.Constants.MAX_TX_QUEUE_SIZE
        self.assertIsInstance(c, int)
        self.assertGreater(c, yogi.Constants.MIN_TX_QUEUE_SIZE)

    def test_default_tx_queue_size(self):
        c = yogi.Constants.DEFAULT_TX_QUEUE_SIZE
        self.assertIsInstance(c, int)
        self.assertLess(c, yogi.Constants.MAX_TX_QUEUE_SIZE)

    def test_min_rx_queue_size(self):
        c = yogi.Constants.MIN_RX_QUEUE_SIZE
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, yogi.Constants.MAX_MESSAGE_PAYLOAD_SIZE)

    def test_max_rx_queue_size(self):
        c = yogi.Constants.MAX_RX_QUEUE_SIZE
        self.assertIsInstance(c, int)
        self.assertGreater(c, yogi.Constants.MIN_RX_QUEUE_SIZE)

    def test_default_rx_queue_size(self):
        c = yogi.Constants.DEFAULT_RX_QUEUE_SIZE
        self.assertIsInstance(c, int)
        self.assertLess(c, yogi.Constants.MAX_RX_QUEUE_SIZE)


if __name__ == '__main__':
    unittest.main()
