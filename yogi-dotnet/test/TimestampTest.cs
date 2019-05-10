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

using System;
using Xunit;

namespace test
{
    public class TimestampTest : TestCase
    {
        [Fact]
        public void FromDurationSinceEpoch()
        {
            Assert.Equal(Yogi.Duration.FromDays(123), Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromDays(123)).DurationSinceEpoch);
            Assert.Throws<ArithmeticException>(() =>
            {
                var _ = Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.Infinity);
            });
            Assert.Throws<ArithmeticException>(() =>
            {
                var _ = Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(-1));
            });
        }

        [Fact]
        public void Now()
        {
            Assert.True(Yogi.Timestamp.Now.DurationSinceEpoch.TotalSeconds > 1e6);
        }

        [Fact]
        public void Parse()
        {
            var t = Yogi.Timestamp.Parse("2009-02-11T12:53:09.123Z");
            Assert.Equal(1234356789123000000L, t.DurationSinceEpoch.NanosecondsCount);

            t = Yogi.Timestamp.Parse("20090211125309123456789", "%Y%m%d%H%M%S%3%6%9");
            Assert.Equal(1234356789123456789L, t.DurationSinceEpoch.NanosecondsCount);
        }

        [Fact]
        public void Constructor()
        {
            Assert.Equal(0, new Yogi.Timestamp().DurationSinceEpoch.NanosecondsCount);
        }

        [Fact]
        public void DurationSinceEpoch()
        {
            var dur = Yogi.Duration.FromDays(123);
            var t = Yogi.Timestamp.FromDurationSinceEpoch(dur);
            Assert.Equal(dur, t.DurationSinceEpoch);
        }

        [Fact]
        public void Nanoseconds()
        {
            Assert.Equal(789, Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(123456789L)).Nanoseconds);
        }

        [Fact]
        public void Microseconds()
        {
            Assert.Equal(456, Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(123456789L)).Microseconds);
        }

        [Fact]
        public void Milliseconds()
        {
            Assert.Equal(123, Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(123456789L)).Milliseconds);
        }

        [Fact]
        public void Format()
        {
            var t = Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(1234356789123456789L));

            var s = t.Format();
            Assert.Equal("2009-02-11T12:53:09.123Z", s);

            s = t.Format("%Y%m%d%H%M%S%3%6%9");
            Assert.Equal("20090211125309123456789", s);
        }

        [Fact]
        public void ToStringMethod()
        {
            var t = Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(1234356789123456789L));
            Assert.Equal("2009-02-11T12:53:09.123Z", t.ToString());
        }

        [Fact]
        public void Operators()
        {
            var t = new Yogi.Timestamp();
            t += Yogi.Duration.FromNanoseconds(8);
            Assert.Equal(8, t.DurationSinceEpoch.NanosecondsCount);
            Assert.Throws<ArithmeticException>(() =>
            {
                t += Yogi.Duration.FromMilliseconds(-1);
            });

            t -= Yogi.Duration.FromNanoseconds(1);
            Assert.Equal(7, t.DurationSinceEpoch.NanosecondsCount);
            Assert.Throws<ArithmeticException>(() =>
            {
                t -= Yogi.Duration.FromMilliseconds(1);
            });

            Assert.Equal(12, (t + Yogi.Duration.FromNanoseconds(5)
                ).DurationSinceEpoch.NanosecondsCount);
            Assert.Throws<ArithmeticException>(() =>
            {
                var _ = t + Yogi.Duration.FromMilliseconds(-1);
            });

            Assert.Equal(5, (t - Yogi.Duration.FromNanoseconds(2)
                ).DurationSinceEpoch.NanosecondsCount);
            Assert.Equal(4, (t - Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(3))).NanosecondsCount);
            Assert.Throws<ArithmeticException>(() =>
            {
                var _ = t - Yogi.Duration.FromMilliseconds(1);
            });

            Assert.True(t == Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(7)));
            Assert.False(t == Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(8)));
            Assert.False(t != Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(7)));
            Assert.True(t != Yogi.Timestamp.FromDurationSinceEpoch(
                Yogi.Duration.FromNanoseconds(8)));

            Assert.True(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2))
                < Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3)));
            Assert.False(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3))
                < Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2)));
            Assert.False(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3))
                < Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3)));

            Assert.False(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2))
                > Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3)));
            Assert.True(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3))
                > Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2)));
            Assert.False(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3))
                > Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3)));

            Assert.True(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2))
                <= Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3)));
            Assert.False(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3))
                <= Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2)));
            Assert.True(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3))
                <= Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3)));

            Assert.False(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2))
                >= Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3)));
            Assert.True(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3))
                >= Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2)));
            Assert.True(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3))
                >= Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(3)));
        }

        [Fact]
        public void EqualsMethod()
        {
            Assert.True(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1))
                .Equals(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1))));
            Assert.False(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1))
                .Equals(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2))));
            Assert.False(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1))
                .Equals(""));
        }

        [Fact]
        public void GetHashCodeMethod()
        {
            Assert.NotEqual(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1)),
                Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2)));
        }

        [Fact]
        public void CompareTo()
        {
            Assert.Equal(-1, Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1))
                .CompareTo(""));
            Assert.Equal(-1, Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1))
                .CompareTo(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2))));
            Assert.Equal(0, Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1))
                .CompareTo(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1))));
            Assert.Equal(+1, Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(2))
                .CompareTo(Yogi.Timestamp.FromDurationSinceEpoch(Yogi.Duration.FromDays(1))));
        }
    }
}
