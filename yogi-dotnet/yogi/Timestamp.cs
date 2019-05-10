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

public static partial class Yogi
{
    partial class Api
    {
        // === YOGI_GetCurrentTime ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetCurrentTimeDelegate(ref long timestamp);

        public static GetCurrentTimeDelegate YOGI_GetCurrentTime
            = Library.GetDelegateForFunction<GetCurrentTimeDelegate>(
                "YOGI_GetCurrentTime");

        // === YOGI_FormatTime ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int FormatTimeDelegate(long timestamp,
            [MarshalAs(UnmanagedType.LPStr)] StringBuilder str, int strsize,
            [MarshalAs(UnmanagedType.LPStr)] string timefmt);

        public static FormatTimeDelegate YOGI_FormatTime
            = Library.GetDelegateForFunction<FormatTimeDelegate>(
                "YOGI_FormatTime");

        // === YOGI_ParseTime ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ParseTimeDelegate(ref long timestamp,
            [MarshalAs(UnmanagedType.LPStr)] string str,
            [MarshalAs(UnmanagedType.LPStr)] string timefmt);

        public static ParseTimeDelegate YOGI_ParseTime
            = Library.GetDelegateForFunction<ParseTimeDelegate>(
                "YOGI_ParseTime");
    }

    /// <summary>
    /// Represents a UTC timestamp.
    ///
    /// Timestamps are expressed in nanoseconds since 01/01/1970 UTC.
    /// </summary>
    public class Timestamp : IComparable
    {
        /// <summary>
        /// Creates a timestamp from a duration since the epoch 01/01/1970 UTC.
        /// </summary>
        /// <param name="duration">Duration since the epoch.</param>
        /// <returns>Timestamp object.</returns>
        public static Timestamp FromDurationSinceEpoch(Duration duration)
        {
            if (!duration.IsFinite || duration < Duration.Zero)
            {
                throw new ArithmeticException("Invalid duration value for use as a timestamp");
            }

            var t = new Timestamp();
            t.durSinceEpoch = duration;
            return t;
        }

        /// <summary>
        /// Creates a timestamp from the current time.
        /// </summary>
        /// <returns>Timestamp corresponding to the current time.</returns>
        public static Timestamp Now
        {
            get
            {
                long timestamp = -1;
                int res = Api.YOGI_GetCurrentTime(ref timestamp);
                CheckErrorCode(res);
                return FromDurationSinceEpoch(Duration.FromNanoseconds(timestamp));
            }
        }

        /// <summary>
        /// Converts a string into a timestamp.
        ///
        /// The fmt parameter describes the textual format of the conversion. The
        /// following placeholders are supported:
        ///     %Y: Four digit year.
        ///     %m: Month name as a decimal 01 to 12.
        ///     %d: Day of the month as decimal 01 to 31.
        ///     %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
        ///     %H: The hour as a decimal number using a 24-hour clock (00 to 23).
        ///     %M: The minute as a decimal 00 to 59.
        ///     %S: Seconds as a decimal 00 to 59.
        ///     %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
        ///     %3: Milliseconds as decimal number 000 to 999.
        ///     %6: Microseconds as decimal number 000 to 999.
        ///     %9: Nanoseconds as decimal number 000 to 999.
        ///
        /// By default, the timestamp will be parsed in the format
        /// "2009-02-11T12:53:09.123Z".
        /// </summary>
        /// <param name="s">String to parse.</param>
        /// <param name="fmt">Format of the time string.</param>
        /// <returns>The parsed timestamp.</returns>
        public static Timestamp Parse(string s, [Optional] string fmt)
        {
            long timestamp = -1;
            int res = Api.YOGI_ParseTime(ref timestamp, s, fmt);
            return FromDurationSinceEpoch(Duration.FromNanoseconds(timestamp));
        }

        /// <summary>
        /// Constructs the timestamp using the epoch 01/01/1970 UTC.
        /// </summary>
        public Timestamp()
        {
            durSinceEpoch = Duration.Zero;
        }

        /// <summary>
        /// The duration since 01/01/1970 UTC.
        /// </summary>
        public Duration DurationSinceEpoch { get { return durSinceEpoch; } }

        /// <summary>
        /// The nanoseconds fraction of the timestamp (0-999).
        /// </summary>
        public int Nanoseconds { get { return durSinceEpoch.Nanoseconds; } }

        /// <summary>
        /// The microseconds fraction of the timestamp (0-999).
        /// </summary>
        public int Microseconds { get { return durSinceEpoch.Microseconds; } }

        /// <summary>
        /// The Milliseconds fraction of the timestamp (0-999).
        /// </summary>
        public int Milliseconds { get { return durSinceEpoch.Milliseconds; } }

        /// <summary>
        /// Converts the timestamp to a string.
        ///
        /// The fmt parameter describes the textual format of the conversion. The
        /// following placeholders are supported:
        ///     %Y: Four digit year.
        ///     %m: Month name as a decimal 01 to 12.
        ///     %d: Day of the month as decimal 01 to 31.
        ///     %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
        ///     %H: The hour as a decimal number using a 24-hour clock (00 to 23).
        ///     %M: The minute as a decimal 00 to 59.
        ///     %S: Seconds as a decimal 00 to 59.
        ///     %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
        ///     %3: Milliseconds as decimal number 000 to 999.
        ///     %6: Microseconds as decimal number 000 to 999.
        ///     %9: Nanoseconds as decimal number 000 to 999.
        ///
        /// By default, the timestamp will be formatted as an ISO-8601 string with
        /// up to millisecond resolution, e.g. "2018-04-23T18:25:43.511Z".
        /// </summary>
        /// <param name="fmt">Format of the time string.</param>
        /// <returns>The timestamp as a string.</returns>
        public string Format([Optional] string fmt)
        {
            var str = new StringBuilder(128);
            int res = Api.YOGI_FormatTime(durSinceEpoch.NanosecondsCount, str, str.Capacity, fmt);
            CheckErrorCode(res);
            return str.ToString();
        }

        /// <summary>
        /// Returns the time in ISO-8601 format up to milliseonds.
        ///
        /// Example: "2018-04-23T18:25:43.511Z".
        /// </summary>
        /// <returns>The time in ISO-8601 format.</returns>
        public override string ToString()
        {
            return Format();
        }

        public static Timestamp operator +(Timestamp lhs, Duration rhs)
        {
            return FromDurationSinceEpoch(lhs.durSinceEpoch + rhs);
        }

        public static Timestamp operator -(Timestamp lhs, Duration rhs)
        {
            return FromDurationSinceEpoch(lhs.durSinceEpoch - rhs);
        }

        public static Duration operator -(Timestamp lhs, Timestamp rhs)
        {
            return lhs.durSinceEpoch - rhs.durSinceEpoch;
        }

        public static bool operator ==(Timestamp lhs, Timestamp rhs)
        {
            if (object.ReferenceEquals(lhs, null))
            {
                return object.ReferenceEquals(rhs, null);
            }

            return lhs.Equals(rhs);
        }

        public static bool operator !=(Timestamp lhs, Timestamp rhs)
        {
            return lhs.durSinceEpoch != rhs.durSinceEpoch;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is Timestamp)) return false;
            return durSinceEpoch == ((Timestamp)obj).durSinceEpoch;
        }

        public static bool operator <(Timestamp lhs, Timestamp rhs)
        {
            return lhs.durSinceEpoch < rhs.durSinceEpoch;
        }

        public static bool operator >(Timestamp lhs, Timestamp rhs)
        {
            return lhs.durSinceEpoch > rhs.durSinceEpoch;
        }

        public static bool operator <=(Timestamp lhs, Timestamp rhs)
        {
            return lhs.durSinceEpoch <= rhs.durSinceEpoch;
        }

        public static bool operator >=(Timestamp lhs, Timestamp rhs)
        {
            return lhs.durSinceEpoch >= rhs.durSinceEpoch;
        }

        public override int GetHashCode()
        {
            return durSinceEpoch.GetHashCode();
        }

        public int CompareTo(object that)
        {
            if (!(that is Timestamp)) return -1;
            if (this == (Timestamp)that) return 0;
            return this < (Timestamp)that ? -1 : +1;
        }

        Duration durSinceEpoch;
    }
}
