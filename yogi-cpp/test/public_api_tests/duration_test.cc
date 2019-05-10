/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../common.h"

#include <chrono>
using namespace std::chrono_literals;

using yogi::ArithmeticException;
using yogi::Duration;

TEST(DurationTest, Zero) {
  auto dur = Duration::kZero;
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.NanosecondsCount(), 0);
}

TEST(DurationTest, Infinity) {
  auto dur = Duration::kInfinity;
  EXPECT_FALSE(dur.IsFinite());
  EXPECT_TRUE(dur > Duration::kZero);
}

TEST(DurationTest, NegativeInfinity) {
  auto dur = Duration::kNegativeInfinity;
  EXPECT_FALSE(dur.IsFinite());
  EXPECT_TRUE(dur < Duration::kZero);
}

TEST(DurationTest, FromNanoseconds) {
  auto dur = Duration::FromNanoseconds(123);
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.NanosecondsCount(), 123);

  dur = Duration::FromNanoseconds(444.56);
  EXPECT_EQ(dur.NanosecondsCount(), 444);  // Note: no rounding

  dur = Duration::FromNanoseconds(std::numeric_limits<double>::infinity());
  EXPECT_EQ(dur, Duration::kInfinity);

  dur = Duration::FromNanoseconds(-std::numeric_limits<double>::infinity());
  EXPECT_EQ(dur, Duration::kNegativeInfinity);
}

TEST(DurationTest, FromMicroseconds) {
  auto dur = Duration::FromMicroseconds(123);
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.TotalMicroseconds(), 123);

  dur = Duration::FromMicroseconds(std::numeric_limits<double>::infinity());
  EXPECT_EQ(dur, Duration::kInfinity);

  dur = Duration::FromMicroseconds(0.5);
  EXPECT_EQ(dur.NanosecondsCount(), 500);
}

TEST(DurationTest, FromMilliseconds) {
  auto dur = Duration::FromMilliseconds(123);
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.TotalMilliseconds(), 123);

  dur = Duration::FromMilliseconds(std::numeric_limits<double>::infinity());
  EXPECT_EQ(dur, Duration::kInfinity);

  dur = Duration::FromMilliseconds(0.5);
  EXPECT_EQ(dur.TotalMicroseconds(), 500);
}

TEST(DurationTest, FromSeconds) {
  auto dur = Duration::FromSeconds(123);
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.TotalSeconds(), 123);

  dur = Duration::FromSeconds(std::numeric_limits<double>::infinity());
  EXPECT_EQ(dur, Duration::kInfinity);

  dur = Duration::FromSeconds(0.5);
  EXPECT_EQ(dur.TotalMilliseconds(), 500);
}

TEST(DurationTest, FromMinutes) {
  auto dur = Duration::FromMinutes(123);
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.TotalMinutes(), 123);

  dur = Duration::FromMinutes(std::numeric_limits<double>::infinity());
  EXPECT_EQ(dur, Duration::kInfinity);

  dur = Duration::FromMinutes(0.5);
  EXPECT_EQ(dur.TotalSeconds(), 30);
}

TEST(DurationTest, FromHours) {
  auto dur = Duration::FromHours(123);
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.TotalHours(), 123);

  dur = Duration::FromHours(std::numeric_limits<double>::infinity());
  EXPECT_EQ(dur, Duration::kInfinity);

  dur = Duration::FromHours(0.5);
  EXPECT_EQ(dur.TotalMinutes(), 30);
}

TEST(DurationTest, FromDays) {
  auto dur = Duration::FromDays(123);
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.TotalDays(), 123);

  dur = Duration::FromDays(std::numeric_limits<double>::infinity());
  EXPECT_EQ(dur, Duration::kInfinity);

  dur = Duration::FromDays(0.5);
  EXPECT_EQ(dur.TotalHours(), 12);
}

TEST(DurationTest, DefaultConstructor) {
  auto dur = Duration();
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.NanosecondsCount(), 0);
}

TEST(DurationTest, ChronoConstructor) {
  auto dur = Duration(12s);
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.TotalSeconds(), 12);

  dur = Duration(33ms);
  EXPECT_TRUE(dur.IsFinite());
  EXPECT_EQ(dur.TotalMilliseconds(), 33);
}

TEST(DurationTest, Nanoseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Nanoseconds(), 887);
}

TEST(DurationTest, Microseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Microseconds(), 465);
}

TEST(DurationTest, Milliseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Milliseconds(), 132);
}

TEST(DurationTest, Seconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Seconds(), 9);
}

TEST(DurationTest, Minutes) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Minutes(), 33);
}

TEST(DurationTest, Hours) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Hours(), 21);
}

TEST(DurationTest, Days) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Days(), 1428);
}

TEST(DurationTest, NanosecondsCount) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.NanosecondsCount(), 123456789132465887LL);
}

TEST(DurationTest, TotalNanoseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_DOUBLE_EQ(dur.TotalNanoseconds(), 123456789132465887.0);

  EXPECT_EQ(Duration::kInfinity.TotalNanoseconds(),
            std::numeric_limits<double>::infinity());
  EXPECT_EQ(Duration::kNegativeInfinity.TotalNanoseconds(),
            -std::numeric_limits<double>::infinity());
}

TEST(DurationTest, TotalMicroseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_DOUBLE_EQ(dur.TotalMicroseconds(), 123456789132465.887);

  EXPECT_EQ(Duration::kInfinity.TotalMicroseconds(),
            std::numeric_limits<double>::infinity());
  EXPECT_EQ(Duration::kNegativeInfinity.TotalMicroseconds(),
            -std::numeric_limits<double>::infinity());
}

TEST(DurationTest, TotalMilliseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_DOUBLE_EQ(dur.TotalMilliseconds(), 123456789132.465887);

  EXPECT_EQ(Duration::kInfinity.TotalMilliseconds(),
            std::numeric_limits<double>::infinity());
  EXPECT_EQ(Duration::kNegativeInfinity.TotalMilliseconds(),
            -std::numeric_limits<double>::infinity());
}

TEST(DurationTest, TotalSeconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_DOUBLE_EQ(dur.TotalSeconds(), 123456789.132465887);

  EXPECT_EQ(Duration::kInfinity.TotalSeconds(),
            std::numeric_limits<double>::infinity());
  EXPECT_EQ(Duration::kNegativeInfinity.TotalSeconds(),
            -std::numeric_limits<double>::infinity());
}

TEST(DurationTest, TotalMinutes) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_DOUBLE_EQ(dur.TotalMinutes(), 2057613.1522077648);

  EXPECT_EQ(Duration::kInfinity.TotalMinutes(),
            std::numeric_limits<double>::infinity());
  EXPECT_EQ(Duration::kNegativeInfinity.TotalMinutes(),
            -std::numeric_limits<double>::infinity());
}

TEST(DurationTest, TotalHours) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_DOUBLE_EQ(dur.TotalHours(), 34293.55253679608);

  EXPECT_EQ(Duration::kInfinity.TotalHours(),
            std::numeric_limits<double>::infinity());
  EXPECT_EQ(Duration::kNegativeInfinity.TotalHours(),
            -std::numeric_limits<double>::infinity());
}

TEST(DurationTest, TotalDays) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_DOUBLE_EQ(dur.TotalDays(), 1428.8980223665033);

  EXPECT_EQ(Duration::kInfinity.TotalDays(),
            std::numeric_limits<double>::infinity());
  EXPECT_EQ(Duration::kNegativeInfinity.TotalDays(),
            -std::numeric_limits<double>::infinity());
}

TEST(DurationTest, Negated) {
  auto dur = Duration(123ns);
  auto neg_dur = dur.Negated();
  EXPECT_EQ(neg_dur.NanosecondsCount(), -dur.NanosecondsCount());
  EXPECT_TRUE(neg_dur.IsFinite());
}

TEST(DurationTest, ToChronoDuration) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.ToChronoDuration<>(), 123456789132465887ns);
  EXPECT_EQ(dur.ToChronoDuration<std::chrono::seconds>(), 123456789s);

  EXPECT_EQ(Duration::kInfinity.ToChronoDuration<>(),
            (std::chrono::nanoseconds::max)());
  EXPECT_EQ(Duration::kInfinity.ToChronoDuration<std::chrono::minutes>(),
            (std::chrono::minutes::max)());
  EXPECT_EQ(Duration::kNegativeInfinity.ToChronoDuration<>(),
            (std::chrono::nanoseconds::min)());
  EXPECT_EQ(
      Duration::kNegativeInfinity.ToChronoDuration<std::chrono::minutes>(),
      (std::chrono::minutes::min)());
}

TEST(DurationTest, IsInfinite) {
  EXPECT_TRUE(Duration::kZero.IsFinite());
  EXPECT_TRUE(Duration(123ns).IsFinite());
  EXPECT_FALSE(Duration::kInfinity.IsFinite());
  EXPECT_FALSE(Duration::kNegativeInfinity.IsFinite());
}

TEST(DurationTest, Format) {
  auto dur = Duration(123456789123456789ns);

  auto s = dur.Format();
  EXPECT_EQ(s, "1428d 21:33:09.123456789");

  s = dur.Format("%S");
  EXPECT_EQ(s, "09");

  s = Duration::kInfinity.Format("%S", "abc");
  EXPECT_EQ(s, "abc");

  s = Duration::kInfinity.Format();
  EXPECT_EQ(s, "inf");

  s = Duration::kNegativeInfinity.Format();
  EXPECT_EQ(s, "-inf");

  s = Duration::kInfinity.Format("%9", "%+bla");
  EXPECT_EQ(s, "+bla");
}

TEST(DurationTest, ToString) {
  auto dur = Duration(123456789123456789ns);

  auto s = dur.ToString();
  EXPECT_EQ(s, "1428d 21:33:09.123456789");
}

TEST(DurationTest, PlusOperator) {
  auto dur1 = Duration(10ns);
  auto dur2 = Duration(3us);
  EXPECT_EQ((dur1 + dur2).NanosecondsCount(), 3010);

  EXPECT_EQ(dur1 + Duration::kInfinity, Duration::kInfinity);
  EXPECT_EQ(Duration::kInfinity + Duration::kInfinity, Duration::kInfinity);
  EXPECT_EQ(dur1 + Duration::kNegativeInfinity, Duration::kNegativeInfinity);
  EXPECT_EQ(Duration::kNegativeInfinity + Duration::kNegativeInfinity,
            Duration::kNegativeInfinity);
  EXPECT_THROW(Duration::kInfinity + Duration::kNegativeInfinity,
               ArithmeticException);
}

TEST(DurationTest, MinusOperator) {
  auto dur1 = Duration(10ns);
  auto dur2 = Duration(3us);
  EXPECT_EQ((dur1 - dur2).NanosecondsCount(), -2990);

  EXPECT_EQ(dur1 - Duration::kInfinity, Duration::kNegativeInfinity);
  EXPECT_EQ(Duration::kInfinity - Duration::kNegativeInfinity,
            Duration::kInfinity);
  EXPECT_EQ(dur1 - Duration::kNegativeInfinity, Duration::kInfinity);
  EXPECT_EQ(Duration::kNegativeInfinity - Duration::kInfinity,
            Duration::kNegativeInfinity);
  EXPECT_THROW(Duration::kNegativeInfinity - Duration::kNegativeInfinity,
               ArithmeticException);
}

TEST(DurationTest, MultiplicationOperator) {
  auto dur = Duration(8ns);

  EXPECT_EQ((dur * 3).NanosecondsCount(), 24);
  EXPECT_EQ((dur * 3.5).NanosecondsCount(), 28);

  EXPECT_EQ(Duration::kInfinity * 5, Duration::kInfinity);
  EXPECT_EQ(Duration::kInfinity * -5, Duration::kNegativeInfinity);
  EXPECT_THROW(Duration::kInfinity * 0, ArithmeticException);
}

TEST(DurationTest, DivisionOperator) {
  auto dur = Duration(28ns);

  EXPECT_EQ((dur / 2).NanosecondsCount(), 14);
  EXPECT_EQ((dur / 3.5).NanosecondsCount(), 8);

  EXPECT_EQ(Duration::kInfinity / 5, Duration::kInfinity);
  EXPECT_EQ(Duration::kInfinity / -5, Duration::kNegativeInfinity);
  EXPECT_THROW(dur / 0, ArithmeticException);
}

TEST(DurationTest, PlusEqualsOperator) {
  auto dur1 = Duration(10ns);
  auto dur2 = Duration(3us);
  auto dur3 = dur1 += dur2;
  EXPECT_EQ(dur1, dur3);
  EXPECT_EQ(dur1.NanosecondsCount(), 3010);
}

TEST(DurationTest, MinusEqualsOperator) {
  auto dur1 = Duration(10ns);
  auto dur2 = Duration(3us);
  auto dur3 = dur1 -= dur2;
  EXPECT_EQ(dur1, dur3);
  EXPECT_EQ(dur1.NanosecondsCount(), -2990);
}

TEST(DurationTest, MultiplyEqualsOperator) {
  auto dur1 = Duration(3ns);
  auto dur2 = dur1 *= 3;
  EXPECT_EQ(dur1, dur2);
  EXPECT_EQ(dur1.NanosecondsCount(), 9);
}

TEST(DurationTest, DivideEqualsOperator) {
  auto dur1 = Duration(12ns);
  auto dur2 = dur1 /= 3;
  EXPECT_EQ(dur1, dur2);
  EXPECT_EQ(dur1.NanosecondsCount(), 4);
}

TEST(DurationTest, EqualityOperator) {
  auto dur1 = Duration(1000ns);
  auto dur2 = Duration(1us);
  auto dur3 = Duration(2ns);
  EXPECT_TRUE(dur1 == dur2);
  EXPECT_FALSE(dur1 == dur3);
}

TEST(DurationTest, NotEqualsOperator) {
  auto dur1 = Duration(1000ns);
  auto dur2 = Duration(1us);
  auto dur3 = Duration(2ns);
  EXPECT_FALSE(dur1 != dur2);
  EXPECT_TRUE(dur1 != dur3);
}

TEST(DurationTest, LessThanOperator) {
  auto dur1 = Duration(1ns);
  auto dur2 = Duration(1ns);
  auto dur3 = Duration(2ns);
  EXPECT_FALSE(dur1 < dur2);
  EXPECT_TRUE(dur2 < dur3);
  EXPECT_FALSE(dur3 < dur1);

  EXPECT_FALSE(Duration::kInfinity < Duration::kInfinity);
  EXPECT_TRUE(Duration::kNegativeInfinity < Duration::kInfinity);
  EXPECT_FALSE(Duration::kInfinity < Duration::kZero);
  EXPECT_TRUE(Duration::kNegativeInfinity < Duration::kZero);
}

TEST(DurationTest, GreaterThanOperator) {
  auto dur1 = Duration(1ns);
  auto dur2 = Duration(1ns);
  auto dur3 = Duration(2ns);
  EXPECT_FALSE(dur1 > dur2);
  EXPECT_FALSE(dur2 > dur3);
  EXPECT_TRUE(dur3 > dur1);

  EXPECT_FALSE(Duration::kInfinity > Duration::kInfinity);
  EXPECT_FALSE(Duration::kNegativeInfinity > Duration::kInfinity);
  EXPECT_TRUE(Duration::kInfinity > Duration::kZero);
  EXPECT_FALSE(Duration::kNegativeInfinity > Duration::kZero);
}

TEST(DurationTest, LessOrEqualOperator) {
  auto dur1 = Duration(1ns);
  auto dur2 = Duration(1ns);
  auto dur3 = Duration(2ns);
  EXPECT_TRUE(dur1 <= dur2);
  EXPECT_TRUE(dur2 <= dur3);
  EXPECT_FALSE(dur3 <= dur1);

  EXPECT_TRUE(Duration::kInfinity <= Duration::kInfinity);
  EXPECT_TRUE(Duration::kNegativeInfinity <= Duration::kInfinity);
  EXPECT_FALSE(Duration::kInfinity <= Duration::kZero);
  EXPECT_TRUE(Duration::kNegativeInfinity <= Duration::kZero);
}

TEST(DurationTest, GreaterOrEqualOperator) {
  auto dur1 = Duration(1ns);
  auto dur2 = Duration(1ns);
  auto dur3 = Duration(2ns);
  EXPECT_TRUE(dur1 >= dur2);
  EXPECT_FALSE(dur2 >= dur3);
  EXPECT_TRUE(dur3 >= dur1);

  EXPECT_TRUE(Duration::kInfinity >= Duration::kInfinity);
  EXPECT_FALSE(Duration::kNegativeInfinity >= Duration::kInfinity);
  EXPECT_TRUE(Duration::kInfinity >= Duration::kZero);
  EXPECT_FALSE(Duration::kNegativeInfinity >= Duration::kZero);
}
