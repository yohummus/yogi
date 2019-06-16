# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2019 Johannes Bergmann.
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
import math

from .common import TestCase


class TestDuration(TestCase):
    def test_zero(self):
        dur = yogi.Duration.zero
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.nanoseconds_count, 0)

    def test_infinity(self):
        dur = yogi.Duration.infinity
        self.assertFalse(dur.is_finite)
        self.assertTrue(dur > yogi.Duration.zero)

    def test_negative_infinity(self):
        dur = yogi.Duration.negative_infinity
        self.assertFalse(dur.is_finite)
        self.assertTrue(dur < yogi.Duration.zero)

    def test_from_nanoseconds(self):
        dur = yogi.Duration.from_nanoseconds(123)
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.nanoseconds_count, 123)

        dur = yogi.Duration.from_nanoseconds(444.56)
        self.assertEqual(dur.nanoseconds_count, 444)  # Note: no rounding

        dur = yogi.Duration.from_nanoseconds(math.inf)
        self.assertEqual(dur, yogi.Duration.infinity)

        dur = yogi.Duration.from_nanoseconds(-math.inf)
        self.assertEqual(dur, yogi.Duration.negative_infinity)

    def test_from_microseconds(self):
        dur = yogi.Duration.from_microseconds(123)
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.total_microseconds, 123)

        dur = yogi.Duration.from_microseconds(math.inf)
        self.assertEqual(dur, yogi.Duration.infinity)

        dur = yogi.Duration.from_microseconds(0.5)
        self.assertEqual(dur.nanoseconds_count, 500)

    def test_from_milliseconds(self):
        dur = yogi.Duration.from_milliseconds(123)
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.total_milliseconds, 123)

        dur = yogi.Duration.from_milliseconds(math.inf)
        self.assertEqual(dur, yogi.Duration.infinity)

        dur = yogi.Duration.from_milliseconds(0.5)
        self.assertEqual(dur.total_microseconds, 500)

    def test_from_seconds(self):
        dur = yogi.Duration.from_seconds(123)
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.total_seconds, 123)

        dur = yogi.Duration.from_seconds(math.inf)
        self.assertEqual(dur, yogi.Duration.infinity)

        dur = yogi.Duration.from_seconds(0.5)
        self.assertEqual(dur.total_milliseconds, 500)

    def test_from_minutes(self):
        dur = yogi.Duration.from_minutes(123)
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.total_minutes, 123)

        dur = yogi.Duration.from_minutes(math.inf)
        self.assertEqual(dur, yogi.Duration.infinity)

        dur = yogi.Duration.from_minutes(0.5)
        self.assertEqual(dur.total_seconds, 30)

    def test_from_hours(self):
        dur = yogi.Duration.from_hours(123)
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.total_hours, 123)

        dur = yogi.Duration.from_hours(math.inf)
        self.assertEqual(dur, yogi.Duration.infinity)

        dur = yogi.Duration.from_hours(0.5)
        self.assertEqual(dur.total_minutes, 30)

    def test_from_days(self):
        dur = yogi.Duration.from_days(123)
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.total_days, 123)

        dur = yogi.Duration.from_days(math.inf)
        self.assertEqual(dur, yogi.Duration.infinity)

        dur = yogi.Duration.from_days(0.5)
        self.assertEqual(dur.total_hours, 12)

    def test_constructor(self):
        dur = yogi.Duration()
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.nanoseconds_count, 0)

        dur = yogi.Duration(datetime.timedelta(seconds=12))
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.total_seconds, 12)

        dur = yogi.Duration(datetime.timedelta(milliseconds=33))
        self.assertTrue(dur.is_finite)
        self.assertEqual(dur.total_milliseconds, 33)

    def test_nanoseconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertEqual(dur.nanoseconds, 887)

    def test_microseconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertEqual(dur.microseconds, 465)

    def test_milliseconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertEqual(dur.milliseconds, 132)

    def test_seconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertEqual(dur.seconds, 9)

    def test_minutes(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertEqual(dur.minutes, 33)

    def test_hours(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertEqual(dur.hours, 21)

    def test_days(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertEqual(dur.days, 1428)

    def test_nanoseconds_count(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertEqual(dur.nanoseconds_count, 123456789132465887)

    def test_total_nanoseconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertAlmostEqual(dur.total_nanoseconds, 123456789132465887)

        self.assertEqual(yogi.Duration.infinity.total_nanoseconds, math.inf)
        self.assertEqual(yogi.Duration.negative_infinity.total_nanoseconds,
                         -math.inf)

    def test_total_microseconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertAlmostEqual(dur.total_microseconds, 123456789132465.887)

        self.assertEqual(yogi.Duration.infinity.total_microseconds, math.inf)
        self.assertEqual(yogi.Duration.negative_infinity.total_microseconds,
                         -math.inf)

    def test_total_milliseconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertAlmostEqual(dur.total_milliseconds, 123456789132.465887)

        self.assertEqual(yogi.Duration.infinity.total_milliseconds, math.inf)
        self.assertEqual(yogi.Duration.negative_infinity.total_milliseconds,
                         -math.inf)

    def test_total_seconds(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertAlmostEqual(dur.total_seconds, 123456789.132465887)

        self.assertEqual(yogi.Duration.infinity.total_seconds, math.inf)
        self.assertEqual(yogi.Duration.negative_infinity.total_seconds,
                         -math.inf)

    def test_total_minutes(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertAlmostEqual(dur.total_minutes, 2057613.1522077648)

        self.assertEqual(yogi.Duration.infinity.total_minutes, math.inf)
        self.assertEqual(yogi.Duration.negative_infinity.total_minutes,
                         -math.inf)

    def test_total_hours(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertAlmostEqual(dur.total_hours, 34293.55253679608)

        self.assertEqual(yogi.Duration.infinity.total_hours, math.inf)
        self.assertEqual(yogi.Duration.negative_infinity.total_hours,
                         -math.inf)

    def test_total_days(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465887)
        self.assertAlmostEqual(dur.total_days, 1428.8980223665033)

        self.assertEqual(yogi.Duration.infinity.total_days, math.inf)
        self.assertEqual(yogi.Duration.negative_infinity.total_days,
                         -math.inf)

    def test_negated(self):
        dur = yogi.Duration.from_nanoseconds(123)
        neg_dur = dur.negated
        self.assertEqual(-neg_dur.nanoseconds_count, dur.nanoseconds_count)
        self.assertTrue(neg_dur.is_finite)

    def test_to_timedelta(self):
        dur = yogi.Duration.from_nanoseconds(123456789132465000)
        self.assertEqual(dur.to_timedelta(),
                         datetime.timedelta(microseconds=123456789132465))

        self.assertEqual(yogi.Duration.infinity.to_timedelta(),
                         datetime.timedelta.max)
        self.assertEqual(yogi.Duration.negative_infinity.to_timedelta(),
                         datetime.timedelta.min)

    def test_is_finite(self):
        self.assertTrue(yogi.Duration.zero.is_finite)
        self.assertTrue(yogi.Duration.from_nanoseconds(123).is_finite)
        self.assertFalse(yogi.Duration.infinity.is_finite)
        self.assertFalse(yogi.Duration.negative_infinity.is_finite)

    def test_format(self):
        dur = yogi.Duration.from_nanoseconds(123456789123456789)

        s = dur.format()
        self.assertEqual(s, "1428d 21:33:09.123456789")

        s = dur.format("%S")
        self.assertEqual(s, "09")

        s = yogi.Duration.infinity.format("%S", "abc")
        self.assertEqual(s, "abc")

        s = yogi.Duration.infinity.format()
        self.assertEqual(s, "inf")

        s = yogi.Duration.negative_infinity.format()
        self.assertEqual(s, "-inf")

        s = yogi.Duration.infinity.format("%9", "%+bla")
        self.assertEqual(s, "+bla")

    def test_str(self):
        dur = yogi.Duration.from_nanoseconds(123456789123456789)
        s = str(dur)
        self.assertEqual(s, "1428d 21:33:09.123456789")

    def test_add(self):
        dur1 = yogi.Duration.from_nanoseconds(10)
        dur2 = yogi.Duration.from_microseconds(3)
        self.assertEqual((dur1 + dur2).nanoseconds_count, 3010)

        self.assertEqual(dur1 + yogi.Duration.infinity, yogi.Duration.infinity)
        self.assertEqual(yogi.Duration.infinity + yogi.Duration.infinity,
                         yogi.Duration.infinity)
        self.assertEqual(dur1 + yogi.Duration.negative_infinity,
                         yogi.Duration.negative_infinity)
        self.assertEqual(yogi.Duration.negative_infinity +
                         yogi.Duration.negative_infinity,
                         yogi.Duration.negative_infinity)
        self.assertRaises(ArithmeticError, lambda: yogi.Duration.infinity
                          + yogi.Duration.negative_infinity)

    def test_sub(self):
        dur1 = yogi.Duration.from_nanoseconds(10)
        dur2 = yogi.Duration.from_microseconds(3)
        self.assertEqual((dur1 - dur2).nanoseconds_count, -2990)

        self.assertEqual(dur1 - yogi.Duration.infinity,
                         yogi.Duration.negative_infinity)
        self.assertEqual(yogi.Duration.infinity -
                         yogi.Duration.negative_infinity,
                         yogi.Duration.infinity)
        self.assertEqual(dur1 - yogi.Duration.negative_infinity,
                         yogi.Duration.negative_infinity)
        self.assertEqual(yogi.Duration.negative_infinity -
                         yogi.Duration.infinity,
                         yogi.Duration.negative_infinity)
        self.assertRaises(ArithmeticError,
                          lambda: yogi.Duration.negative_infinity
                          - yogi.Duration.negative_infinity)

    def test_mul(self):
        dur = yogi.Duration.from_nanoseconds(8)

        self.assertEqual((dur * 3).nanoseconds_count, 24)
        self.assertEqual((dur * 3.5).nanoseconds_count, 28)

        self.assertEqual(yogi.Duration.infinity * 5, yogi.Duration.infinity)
        self.assertEqual(yogi.Duration.infinity * -5,
                         yogi.Duration.negative_infinity)

        self.assertRaises(ArithmeticError, lambda: yogi.Duration.infinity * 0)

    def test_truediv(self):
        dur = yogi.Duration.from_nanoseconds(28)

        self.assertEqual((dur / 2).nanoseconds_count, 14)
        self.assertEqual((dur / 3.5).nanoseconds_count, 8)

        self.assertEqual(yogi.Duration.infinity / 5, yogi.Duration.infinity)
        self.assertEqual(yogi.Duration.infinity / -5,
                         yogi.Duration.negative_infinity)

        self.assertRaises(ZeroDivisionError, lambda: dur / 0)

    def test_eq(self):
        dur1 = yogi.Duration.from_nanoseconds(1000)
        dur2 = yogi.Duration.from_microseconds(1)
        dur3 = yogi.Duration.from_nanoseconds(2)
        self.assertTrue(dur1 == dur2)
        self.assertFalse(dur1 == dur3)

    def test_ne(self):
        dur1 = yogi.Duration.from_nanoseconds(1000)
        dur2 = yogi.Duration.from_microseconds(1)
        dur3 = yogi.Duration.from_nanoseconds(2)
        self.assertFalse(dur1 != dur2)
        self.assertTrue(dur1 != dur3)

    def test_lt(self):
        dur1 = yogi.Duration.from_nanoseconds(1)
        dur2 = yogi.Duration.from_nanoseconds(1)
        dur3 = yogi.Duration.from_nanoseconds(2)
        self.assertFalse(dur1 < dur2)
        self.assertTrue(dur2 < dur3)
        self.assertFalse(dur3 < dur1)

        self.assertFalse(yogi.Duration.infinity < yogi.Duration.infinity)
        self.assertTrue(yogi.Duration.negative_infinity <
                        yogi.Duration.infinity)
        self.assertFalse(yogi.Duration.infinity < yogi.Duration.zero)
        self.assertTrue(yogi.Duration.negative_infinity < yogi.Duration.zero)

    def test_gt(self):
        dur1 = yogi.Duration.from_nanoseconds(1)
        dur2 = yogi.Duration.from_nanoseconds(1)
        dur3 = yogi.Duration.from_nanoseconds(2)
        self.assertFalse(dur1 > dur2)
        self.assertFalse(dur2 > dur3)
        self.assertTrue(dur3 > dur1)

        self.assertFalse(yogi.Duration.infinity > yogi.Duration.infinity)
        self.assertFalse(yogi.Duration.negative_infinity >
                         yogi.Duration.infinity)
        self.assertTrue(yogi.Duration.infinity > yogi.Duration.zero)
        self.assertFalse(yogi.Duration.negative_infinity > yogi.Duration.zero)

    def test_le(self):
        dur1 = yogi.Duration.from_nanoseconds(1)
        dur2 = yogi.Duration.from_nanoseconds(1)
        dur3 = yogi.Duration.from_nanoseconds(2)
        self.assertTrue(dur1 <= dur2)
        self.assertTrue(dur2 <= dur3)
        self.assertFalse(dur3 <= dur1)

        self.assertTrue(yogi.Duration.infinity <= yogi.Duration.infinity)
        self.assertTrue(yogi.Duration.negative_infinity <=
                        yogi.Duration.infinity)
        self.assertFalse(yogi.Duration.infinity <= yogi.Duration.zero)
        self.assertTrue(yogi.Duration.negative_infinity <= yogi.Duration.zero)

    def test_ge(self):
        dur1 = yogi.Duration.from_nanoseconds(1)
        dur2 = yogi.Duration.from_nanoseconds(1)
        dur3 = yogi.Duration.from_nanoseconds(2)
        self.assertTrue(dur1 >= dur2)
        self.assertFalse(dur2 >= dur3)
        self.assertTrue(dur3 >= dur1)

        self.assertTrue(yogi.Duration.infinity >= yogi.Duration.infinity)
        self.assertFalse(yogi.Duration.negative_infinity >=
                         yogi.Duration.infinity)
        self.assertTrue(yogi.Duration.infinity >= yogi.Duration.zero)
        self.assertFalse(yogi.Duration.negative_infinity >= yogi.Duration.zero)

    def test_hash(self):
        dur1 = yogi.Duration.from_days(1)
        dur2 = yogi.Duration.from_days(2)
        self.assertNotEqual(hash(dur1), hash(dur2))


if __name__ == '__main__':
    unittest.main()
