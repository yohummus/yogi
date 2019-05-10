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
import datetime

from .common import TestCase


class TestTimestamp(TestCase):
    def test_from_duration_since_epoch(self):
        self.assertEqual(yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(123)).duration_since_epoch,
            yogi.Duration.from_days(123))
        self.assertRaises(ArithmeticError,
                          lambda: yogi.Timestamp.from_duration_since_epoch(
                              yogi.Duration.infinity))
        self.assertRaises(ArithmeticError,
                          lambda: yogi.Timestamp.from_duration_since_epoch(
                              yogi.Duration.from_days(-1)))

    def test_now(self):
        dt = yogi.Timestamp.now().to_datetime()
        now = datetime.datetime.now(datetime.timezone.utc)
        delta = datetime.timedelta(seconds=1)
        self.assertGreater(dt, now - delta)
        self.assertLess(dt, now + delta)

    def test_parse(self):
        t = yogi.Timestamp.parse("2009-02-11T12:53:09.123Z")
        self.assertEqual(t.duration_since_epoch.nanoseconds_count,
                         1234356789123000000)

        t = yogi.Timestamp.parse("20090211125309123456789",
                                 "%Y%m%d%H%M%S%3%6%9")
        self.assertEqual(t.duration_since_epoch.nanoseconds_count,
                         1234356789123456789)

    def test_constructor(self):
        t = yogi.Timestamp()
        self.assertEqual(t.duration_since_epoch.nanoseconds_count, 0)

    def test_duration_since_epoch(self):
        dur = yogi.Duration.from_days(123)
        t = yogi.Timestamp.from_duration_since_epoch(dur)
        self.assertEqual(t.duration_since_epoch, dur)

    def test_nanoseconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789)
        t = yogi.Timestamp.from_duration_since_epoch(dur)
        self.assertEqual(t.nanoseconds, 789)

    def test_microseconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789)
        t = yogi.Timestamp.from_duration_since_epoch(dur)
        self.assertEqual(t.microseconds, 456)

    def test_milliseconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789)
        t = yogi.Timestamp.from_duration_since_epoch(dur)
        self.assertEqual(t.milliseconds, 123)

    def test_to_datetime(self):
        dur = yogi.Duration.from_nanoseconds(1234356789123456789)
        dt = yogi.Timestamp.from_duration_since_epoch(dur).to_datetime()
        self.assertIsInstance(dt, datetime.datetime)
        self.assertEqual(dt.microsecond, 123456)

    def test_format(self):
        dur = yogi.Duration.from_nanoseconds(1234356789123456789)
        t = yogi.Timestamp.from_duration_since_epoch(dur)

        s = t.format()
        self.assertEqual(s, "2009-02-11T12:53:09.123Z")

        s = t.format("%Y%m%d%H%M%S%3%6%9")
        self.assertEqual(s, "20090211125309123456789")

    def test_str(self):
        dur = yogi.Duration.from_nanoseconds(1234356789123456789)
        t = yogi.Timestamp.from_duration_since_epoch(dur)
        self.assertEqual(str(t), "2009-02-11T12:53:09.123Z")

    def test_add(self):
        t = yogi.Timestamp()
        t = t + yogi.Duration.from_nanoseconds(8)
        self.assertEqual(t.duration_since_epoch.nanoseconds_count, 8)
        self.assertRaises(ArithmeticError,
                          lambda: t + yogi.Duration.from_milliseconds(-1))

    def test_sub(self):
        t = yogi.Timestamp()
        t += yogi.Duration.from_nanoseconds(8)
        t = t - yogi.Duration.from_nanoseconds(2)
        self.assertEqual(t.duration_since_epoch.nanoseconds_count, 6)
        self.assertEqual((t - yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_nanoseconds(2))).nanoseconds_count, 4)
        self.assertRaises(ArithmeticError,
                          lambda: t - yogi.Duration.from_milliseconds(1))

    def test_eq(self):
        t1 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(2))
        t2 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(3))

        self.assertTrue(t1 == t1)
        self.assertFalse(t2 == t1)

    def test_ne(self):
        t1 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(2))
        t2 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(3))

        self.assertFalse(t1 != t1)
        self.assertTrue(t2 != t1)

    def test_lt(self):
        t1 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(2))
        t2 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(3))

        self.assertTrue(t1 < t2)
        self.assertFalse(t2 < t1)
        self.assertFalse(t2 < t2)

    def test_gt(self):
        t1 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(2))
        t2 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(3))

        self.assertFalse(t1 > t2)
        self.assertTrue(t2 > t1)
        self.assertFalse(t2 > t2)

    def test_le(self):
        t1 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(2))
        t2 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(3))

        self.assertTrue(t1 <= t2)
        self.assertFalse(t2 <= t1)
        self.assertTrue(t2 <= t2)

    def test_ge(self):
        t1 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(2))
        t2 = yogi.Timestamp.from_duration_since_epoch(
            yogi.Duration.from_days(3))

        self.assertFalse(t1 >= t2)
        self.assertTrue(t2 >= t1)
        self.assertTrue(t2 >= t2)

    def test_hash(self):
        self.assertNotEqual(hash(yogi.Timestamp.now()), hash(yogi.Timestamp()))
