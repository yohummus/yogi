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
using System.Runtime.InteropServices;
using System.Text;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

public static partial class Yogi
{
    partial class Api
    {
        // === YOGI_FormatDuration ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int FormatDurationDelegate(long dur, int neg,
            [MarshalAs(UnmanagedType.LPStr)] StringBuilder str, int strsize,
            [MarshalAs(UnmanagedType.LPStr)] string durfmt,
            [MarshalAs(UnmanagedType.LPStr)] string inffmt);

        public static FormatDurationDelegate YOGI_FormatDuration
            = Library.GetDelegateForFunction<FormatDurationDelegate>(
                "YOGI_FormatDuration");
    }

    enum InfinityType
    {
        Negative = -1,
        None = 0,
        Positive = 1,
    }

    static long AddSafely(long a, long b)
    {
        if (a > 0 && b > long.MaxValue - a)
        {
            throw new OverflowException("Duration value overflow");
        }
        else if (a < 0 && b < long.MinValue - a)
        {
            throw new OverflowException("Duration value underflow");
        }

        return a + b;
    }

    static long MultiplySafely(long val, double multiplicator)
    {
        if (double.IsNaN(multiplicator))
        {
            throw new ArithmeticException("Trying to multiply duration value and NaN");
        }

        if (multiplicator == 0)
        {
            return 0;
        }

        if (Math.Abs(multiplicator) > 1) {
            long maxVal = (long)(long.MaxValue / multiplicator);
            if (Math.Abs(val) > maxVal)
            {
                throw new OverflowException("Duration value overflow");
            }
        }

        return (long)(val * multiplicator);
    }

    static void CheckDivisor(double divisor)
    {
        if (double.IsNaN(divisor))
        {
            throw new ArithmeticException("Trying to divide duration value by NaN");
        }

        if (divisor == 0)
        {
            throw new DivideByZeroException("Trying to divide duration value by zero");
        }
    }

    static long DivideSafely(long val, double divisor)
    {
        CheckDivisor(divisor);

        if (double.IsInfinity(divisor))
        {
            return 0;
        }

        if (Math.Abs(divisor) < 1)
        {
            long maxVal = (long)(long.MaxValue * divisor);
            if (Math.Abs(val) > maxVal)
            {
                throw new OverflowException("Duration value overflow");
            }
        }

        return (long)(val / divisor);
    }

    static long DurationToApiDuration(Duration duration)
    {
        if (duration < Duration.Zero)
        {
            throw new ArgumentOutOfRangeException("Duration must not be negative");
        }

        return duration.NanosecondsCount;
    }

    /// <summary>
    /// Represents a time duration.
    ///
    /// The resolution of a time duration is nanoseconds. Durations can be
    /// positive or negative. Exceptions are thrown in case of arithmetic errors.
    /// </summary>
    public class Duration : IComparable
    {
        /// <summary>Zero duration.</summary>
        public static Duration Zero { get { return new Duration(InfinityType.None); } }

        /// <summary>Infinite duration.</summary>
        public static Duration Infinity { get { return new Duration(InfinityType.Positive); } }

        /// <summary>Negative infinite duration.</summary>
        public static Duration NegativeInfinity { get { return new Duration(InfinityType.Negative); } }

        static Duration FromTimeUnit(long multiplicator, double val)
        {
            if (double.IsNaN(val))
            {
                throw new ArithmeticException("Cannot construct duration from NaN");
            }

            if (!double.IsInfinity(val))
            {
                long maxVal = (long)(long.MaxValue / multiplicator);
                if (Math.Abs(val) > maxVal)
                {
                    throw new OverflowException("Duration value overflow");
                }

                var dur = new Duration();
                dur.nsCount = (long)(val * multiplicator);
                return dur;
            }

            // infinite
            return val < 0 ? NegativeInfinity : Infinity;
        }

        static Duration FromTimeUnit(long multiplicator, long val)
        {
            long maxVal = long.MaxValue / multiplicator;
            if (Math.Abs(val) > maxVal)
            {
                throw new OverflowException("Duration value overflow");
            }

            var dur = new Duration();
            dur.nsCount = val * multiplicator;
            return dur;
        }

        /// <summary>
        /// Constructs a duration from a number of nanoseconds.
        /// </summary>
        /// <param name="ns">Number of nanoseconds.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromNanoseconds(long ns)
        {
            return FromTimeUnit(1L, ns);
        }

        /// <summary>
        /// Constructs a duration from a number of nanoseconds.
        /// </summary>
        /// <param name="ns">Number of nanoseconds.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromNanoseconds(double ns)
        {
            return FromTimeUnit(1L, ns);
        }

        /// <summary>
        /// Constructs a duration from a number of microseconds.
        /// </summary>
        /// <param name="us">Number of microseconds.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromMicroseconds(long us)
        {
            return FromTimeUnit(1000L, us);
        }

        /// <summary>
        /// Constructs a duration from a number of microseconds.
        /// </summary>
        /// <param name="us">Number of microseconds.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromMicroseconds(double us)
        {
            return FromTimeUnit(1000L, us);
        }

        /// <summary>
        /// Constructs a duration from a number of milliseconds.
        /// </summary>
        /// <param name="ms">Number of milliseconds.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromMilliseconds(long ms)
        {
            return FromTimeUnit(1000000L, ms);
        }

        /// <summary>
        /// Constructs a duration from a number of milliseconds.
        /// </summary>
        /// <param name="ms">Number of milliseconds.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromMilliseconds(double ms)
        {
            return FromTimeUnit(1000000L, ms);
        }

        /// <summary>
        /// Constructs a duration from a number of seconds.
        /// </summary>
        /// <param name="seconds">Number of seconds.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromSeconds(long seconds)
        {
            return FromTimeUnit(1000000000L, seconds);
        }

        /// <summary>
        /// Constructs a duration from a number of seconds.
        /// </summary>
        /// <param name="seconds">Number of seconds.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromSeconds(double seconds)
        {
            return FromTimeUnit(1000000000L, seconds);
        }

        /// <summary>
        /// Constructs a duration from a number of minutes.
        /// </summary>
        /// <param name="minutes">Number of minutes.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromMinutes(long minutes)
        {
            return FromTimeUnit(60 * 1000000000L, minutes);
        }

        /// <summary>
        /// Constructs a duration from a number of minutes.
        /// </summary>
        /// <param name="minutes">Number of minutes.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromMinutes(double minutes)
        {
            return FromTimeUnit(60 * 1000000000L, minutes);
        }

        /// <summary>
        /// Constructs a duration from a number of hours.
        /// </summary>
        /// <param name="hours">Number of hours.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromHours(long hours)
        {
            return FromTimeUnit(60 * 60 * 1000000000L, hours);
        }

        /// <summary>
        /// Constructs a duration from a number of hours.
        /// </summary>
        /// <param name="hours">Number of hours.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromHours(double hours)
        {
            return FromTimeUnit(60 * 60 * 1000000000L, hours);
        }

        /// <summary>
        /// Constructs a duration from a number of days.
        /// </summary>
        /// <param name="days">Number of days.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromDays(long days)
        {
            return FromTimeUnit(24 * 60 * 60 * 1000000000L, days);
        }

        /// <summary>
        /// Constructs a duration from a number of days.
        /// </summary>
        /// <param name="days">Number of days.</param>
        /// <returns>Duration instance.</returns>
        public static Duration FromDays(double days)
        {
            return FromTimeUnit(24 * 60 * 60 * 1000000000L, days);
        }

        internal static Duration FromJson(JToken jt)
        {
            var val = (double)jt;
            return val < 0 ? Infinity : Duration.FromSeconds(val);
        }

        Duration(InfinityType infType)
        {
            this.infType = infType;

            switch (infType)
            {
                case InfinityType.None:
                    nsCount = 0;
                    break;

                case InfinityType.Positive:
                    nsCount = long.MaxValue;
                    break;

                case InfinityType.Negative:
                    nsCount = long.MinValue;
                    break;
            }
        }

        /// <summary>
        /// Constructs a zero nanoseconds duration.
        /// </summary>
        public Duration() : this(InfinityType.None)
        {
        }

        /// <summary>
        /// Constructs a duration from a TimeSpan instance.
        /// </summary>
        /// <param name="ts">The time span to represent.</param>
        public Duration(TimeSpan ts) : this(InfinityType.None)
        {
            nsCount = ts.Ticks * 100;
        }

        /// <summary>
        /// Nanoseconds fraction (0-999) of the duration.
        ///
        /// If the duration is infinite, this value will be undefined.
        /// </summary>
        public int Nanoseconds { get { return (int)(NanosecondsCount % 1000); } }

        /// <summary>
        /// Microseconds fraction (0-999) of the duration.
        ///
        /// If the duration is infinite, this value will be undefined.
        /// </summary>
        public int Microseconds { get { return (int)((NanosecondsCount / 1000L) % 1000); } }

        /// <summary>
        /// Milliseconds fraction (0-999) of the duration.
        ///
        /// If the duration is infinite, this value will be undefined.
        /// </summary>
        public int Milliseconds { get { return (int)((NanosecondsCount / 1000000L) % 1000); } }

        /// <summary>
        /// Seconds fraction (0-59) of the duration.
        ///
        /// If the duration is infinite, this value will be undefined.
        /// </summary>
        public int Seconds { get { return (int)((NanosecondsCount / 1000000000L) % 60); } }

        /// <summary>
        /// Minutes fraction (0-59) of the duration.
        ///
        /// If the duration is infinite, this value will be undefined.
        /// </summary>
        public int Minutes { get { return (int)((NanosecondsCount / (60 * 1000000000L)) % 60); } }

        /// <summary>
        /// Hours fraction (0-23) of the duration.
        ///
        /// If the duration is infinite, this value will be undefined.
        /// </summary>
        public int Hours { get { return (int)((NanosecondsCount / (60 * 60 * 1000000000L)) % 24); } }

        /// <summary>
        /// Days fraction of the duration.
        ///
        /// If the duration is infinite, this value will be undefined.
        /// </summary>
        public int Days { get { return (int)((NanosecondsCount / (24 * 60 * 60 * 1000000000L))); } }

        /// <summary>
        /// Total number of nanoseconds.
        ///
        /// If the duration is infinite, this value will be undefined.
        /// </summary>
        public long NanosecondsCount { get { return nsCount; } }

        double ToTotalTimeUnit(long divisor)
        {
            switch (infType)
            {
                case InfinityType.Positive:
                    return double.PositiveInfinity;

                case InfinityType.Negative:
                    return double.NegativeInfinity;

                default:
                    return (double)(nsCount) / divisor;
            }
        }

        /// <summary>
        /// Total number of nanoseconds.
        ///
        /// If the duration is infinite, this value will be the appropriate infinite value.
        /// </summary>
        public double TotalNanoseconds { get { return ToTotalTimeUnit(1L); } }

        /// <summary>
        /// Total number of microseconds.
        ///
        /// If the duration is infinite, this function will return an undefined value.
        /// </summary>
        public double TotalMicroseconds { get { return ToTotalTimeUnit(1000L); } }

        /// <summary>
        /// Total number of milliseconds.
        ///
        /// If the duration is infinite, this function will return an undefined value.
        /// </summary>
        public double TotalMilliseconds { get { return ToTotalTimeUnit(1000000L); } }

        /// <summary>
        /// Total number of seconds.
        ///
        /// If the duration is infinite, this function will return an undefined value.
        /// </summary>
        public double TotalSeconds { get { return ToTotalTimeUnit(1000000000L); } }

        /// <summary>
        /// Total number of minutes.
        ///
        /// If the duration is infinite, this function will return an undefined value.
        /// </summary>
        public double TotalMinutes { get { return ToTotalTimeUnit(60 * 1000000000L); } }

        /// <summary>
        /// Total number of hours.
        ///
        /// If the duration is infinite, this function will return an undefined value.
        /// </summary>
        public double TotalHours { get { return ToTotalTimeUnit(60 * 60 * 1000000000L); } }

        /// <summary>
        /// Total number of days.
        ///
        /// If the duration is infinite, this function will return an undefined value.
        /// </summary>
        public double TotalDays { get { return ToTotalTimeUnit(24 * 60 * 60 * 1000000000L); } }

        /// <summary>
        /// The negated duration, i.e. multiplied by -1.
        /// </summary>
        public Duration Negated
        {
            get
            {
                switch (infType)
                {
                    case InfinityType.Positive:
                        return NegativeInfinity;

                    case InfinityType.Negative:
                        return Infinity;

                    default:
                        return FromNanoseconds(-nsCount);
                }
            }
        }

        /// <summary>
        /// Converts the duration to a TimeSpan object.
        ///
        /// In case that the duration is infinite, the repective min or max value
        /// of the time span will be returned.
        /// </summary>
        /// <returns>TimeSpan object.</returns>
        public TimeSpan ToTimeSpan()
        {
            switch (infType)
            {
                case InfinityType.Positive:
                    return TimeSpan.MaxValue;

                case InfinityType.Negative:
                    return TimeSpan.MinValue;

                default:
                    return TimeSpan.FromTicks(nsCount / 100);
            }
        }

        /// <summary>
        /// True if the duration is finite and false otherwise.
        /// </summary>
        public bool IsFinite { get { return infType == InfinityType.None; } }

        /// <summary>
        /// Converts the duration to a string.
        ///
        /// The durFmt parameter describes the format of the conversion. The
        /// following placeholders are supported:
        ///  - *%+*: Plus sign if duration is positive; minus sign if it is negative
        ///  - *%-*: Minus sign (only) if duration is negative
        ///  - *%d*: Total number of days
        ///  - *%D*: Total number of days if days > 0
        ///  - *%H*: Fractional hours (range 00 to 23)
        ///  - *%M*: Fractional minutes (range 00 to 59)
        ///  - *%S*: Fractional seconds (range 00 to 59)
        ///  - *%T*: Equivalent to %H:%M:%S
        ///  - *%3*: Fractional milliseconds (range 000 to 999)
        ///  - *%6*: Fractional microseconds (range 000 to 999)
        ///  - *%9*: Fractional nanoseconds (range 000 to 999)
        ///
        /// The infFmt parameter describes the format to use for infinite
        /// durations. The following placeholders are supported:
        ///  - *%+*: Plus sign if duration is positive; minus sign if it is negative
        ///  - *%-*: Minus sign (only) if duration is negative
        ///
        /// By default, the duration will be formatted in the format
        /// "-23d 04:19:33.123456789". If the duration is infinite, then the string
        /// constructed using infFmt will be returned ("inf" and "-inf"
        /// respectively by default).
        /// </summary>
        /// <param name="durFmt">Format of the duration string.</param>
        /// <param name="infFmt">Format to use for infinity.</param>
        /// <returns>The formatted duration string.</returns>
        public string Format([Optional] string durFmt, [Optional] string infFmt)
        {
            var str = new StringBuilder(128);
            int res = Api.YOGI_FormatDuration(IsFinite ? nsCount : -1, nsCount < 0 ? 1 : 0,
                str, str.Capacity, durFmt, infFmt);
            CheckErrorCode(res);
            return str.ToString();
        }

        /// <summary>
        /// Converts the duration to a string.
        ///
        /// The format of the string will be "-23d 04:19:33.123456789".
        /// </summary>
        /// <returns>Duration as a string.</returns>
        public override string ToString()
        {
            return Format();
        }

        public static Duration operator +(Duration lhs, Duration rhs)
        {
            if (lhs.infType == InfinityType.None && rhs.infType == InfinityType.None)
            {
                return FromNanoseconds(AddSafely(lhs.nsCount, rhs.nsCount));
            }

            var infTypeSum = (int)lhs.infType + (int)rhs.infType;
            if (infTypeSum == (int)InfinityType.None)
            {
                throw new ArithmeticException("Trying to add positive and negative infinite duration values");
            }

            return infTypeSum > 0 ? Infinity : NegativeInfinity;
        }

        public static Duration operator -(Duration lhs, Duration rhs)
        {
            return lhs + rhs.Negated;
        }

        public static Duration operator *(Duration lhs, double rhs)
        {
            if (lhs.infType != InfinityType.None && rhs == 0)
            {
                throw new ArithmeticException("Trying to multiply infinite duration value and zero");
            }

            if (lhs.infType == InfinityType.None && !double.IsInfinity(rhs) && !double.IsNaN(rhs))
            {
                return FromNanoseconds(MultiplySafely(lhs.nsCount, rhs));
            }

            var rhsInfType = rhs > 0 ? InfinityType.Positive : InfinityType.Negative;
            return lhs.infType == rhsInfType ? Infinity : NegativeInfinity;
        }

        public static Duration operator /(Duration lhs, double rhs)
        {
            if (lhs.infType == InfinityType.None)
            {
                return FromNanoseconds(DivideSafely(lhs.nsCount, rhs));
            }

            CheckDivisor(rhs);
            var rhsInfType = rhs > 0 ? InfinityType.Positive : InfinityType.Negative;
            return lhs.infType == rhsInfType ? Infinity : NegativeInfinity;
        }

        public static bool operator ==(Duration lhs, Duration rhs)
        {
            if (object.ReferenceEquals(lhs, null))
            {
                return object.ReferenceEquals(rhs, null);
            }

            return lhs.Equals(rhs);
        }

        public static bool operator !=(Duration lhs, Duration rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            if (!(obj is Duration))
            {
                return false;
            }

            if (infType == ((Duration)obj).infType && infType != InfinityType.None)
            {
                return true;
            }

            return nsCount == ((Duration)obj).nsCount;
        }

        public static bool operator <(Duration lhs, Duration rhs)
        {
            if (lhs.infType == rhs.infType && lhs.infType == InfinityType.None)
            {
                return lhs.nsCount < rhs.nsCount;
            }

            return lhs.infType < rhs.infType;
        }

        public static bool operator >(Duration lhs, Duration rhs)
        {
            if (lhs.infType == rhs.infType && lhs.infType == InfinityType.None)
            {
                return lhs.nsCount > rhs.nsCount;
            }

            return lhs.infType > rhs.infType;
        }

        public static bool operator <=(Duration lhs, Duration rhs)
        {
            return !(lhs > rhs);
        }

        public static bool operator >=(Duration lhs, Duration rhs)
        {
            return !(lhs < rhs);
        }

        public override int GetHashCode()
        {
            return infType.GetHashCode() * 17 + nsCount.GetHashCode();
        }

        public int CompareTo(object that)
        {
            if (!(that is Duration)) return -1;
            if (this == (Duration)that) return 0;
            return this < (Duration)that ? -1 : +1;
        }

        InfinityType infType;
        long nsCount;
    }
}
