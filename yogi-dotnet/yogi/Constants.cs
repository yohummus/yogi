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

public static partial class Yogi
{
    partial class Api
    {
        // === YOGI_GetConstant ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetIntConstantDelegate(ref int dest, int constant);

        public static GetIntConstantDelegate YOGI_GetIntConstant
            = Library.GetDelegateForFunction<GetIntConstantDelegate>(
                "YOGI_GetConstant");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetLongLongConstantDelegate(ref long dest, int constant);

        public static GetLongLongConstantDelegate YOGI_GetLongLongConstant
            = Library.GetDelegateForFunction<GetLongLongConstantDelegate>(
                "YOGI_GetConstant");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetStringConstantDelegate(ref IntPtr dest, int constant);

        public static GetStringConstantDelegate YOGI_GetStringConstant
            = Library.GetDelegateForFunction<GetStringConstantDelegate>(
                "YOGI_GetConstant");
    }

    /// <summary>
    /// Constants built into the Yogi Core library.
    /// </summary>
    public static class Constants
    {
        /// <summary>Complete Yogi Core version number.</summary>
        public static readonly string VersionNumber;

        /// <summary>Yogi Core major version number.</summary>
        public static readonly int VersionMajor;

        /// <summary>Yogi Core minor version number.</summary>
        public static readonly int VersionMinor;

        /// <summary>Yogi Core patch version number.</summary>
        public static readonly int VersionPatch;

        /// <summary>Default network interfaces to use for advertising.</summary>
        public static readonly string DefaultAdvInterfaces;

        /// <summary>Default IP address for advertising.</summary>
        public static readonly string DefaultAdvAddress;

        /// <summary>Default UDP port for advertising.</summary>
        public static readonly int DefaultAdvPort;

        /// <summary>Default time between two advertising messages.</summary>
        public static readonly Duration DefaultAdvInterval;

        /// <summary>Default timeout for connections between two branches.</summary>
        public static readonly Duration DefaultConnectionTimeout;

        /// <summary>Default verbosity for newly created loggers.</summary>
        public static readonly Verbosity DefaultLoggerVerbosity;

        /// <summary>Default format of the time string in log entries.</summary>
        public static readonly string DefaultLogTimeFormat;

        /// <summary>Default format of a log entry.</summary>
        public static readonly string DefaultLogFormat;

        /// <summary>Maximum size of the payload of a message between two branches.</summary>
        public static readonly int MaxMessagePayloadSize;

        /// <summary>Default textual format for timestamps.</summary>
        public static readonly string DefaultTimeFormat;

        /// <summary>Default string to denote an infinite duration.</summary>
        public static readonly string DefaultInfiniteDurationString;

        /// <summary>Default textual format for duration strings.</summary>
        public static readonly string DefaultDurationFormat;

        /// <summary>Default string to denote an invalid object handle.</summary>
        public static readonly string DefaultInvalidHandleString;

        /// <summary>Default textual format for strings describing an object.</summary>
        public static readonly string DefaultObjectFormat;

        /// <summary>Minimum size of a send queue for a remote branch.</summary>
        public static readonly int MinTxQueueSize;

        /// <summary>Maximum size of a send queue for a remote branch.</summary>
        public static readonly int MaxTxQueueSize;

        /// <summary>Default size of a send queue for a remote branch.</summary>
        public static readonly int DefaultTxQueueSize;

        /// <summary>Minimum size of a receive queue for a remote branch.</summary>
        public static readonly int MinRxQueueSize;

        /// <summary>Maximum size of a receive queue for a remote branch.</summary>
        public static readonly int MaxRxQueueSize;

        /// <summary>Default size of a receive queue for a remote branch.</summary>
        public static readonly int DefaultRxQueueSize;

        static Constants()
        {
            IntPtr str = new IntPtr();
            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 1));
            VersionNumber = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetIntConstant(ref VersionMajor, 2));
            CheckErrorCode(Api.YOGI_GetIntConstant(ref VersionMinor, 3));
            CheckErrorCode(Api.YOGI_GetIntConstant(ref VersionPatch, 4));

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 5));
            DefaultAdvInterfaces = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 6));
            DefaultAdvAddress = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetIntConstant(ref DefaultAdvPort, 7));

            long t = -1;
            CheckErrorCode(Api.YOGI_GetLongLongConstant(ref t, 8));
            DefaultAdvInterval = Duration.FromNanoseconds(t);

            CheckErrorCode(Api.YOGI_GetLongLongConstant(ref t, 9));
            DefaultConnectionTimeout = Duration.FromNanoseconds(t);

            int n = -1;
            CheckErrorCode(Api.YOGI_GetIntConstant(ref n, 10));
            DefaultLoggerVerbosity = (Verbosity)n;

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 11));
            DefaultLogTimeFormat = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 12));
            DefaultLogFormat = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetIntConstant(ref MaxMessagePayloadSize, 13));

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 14));
            DefaultTimeFormat = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 15));
            DefaultInfiniteDurationString = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 16));
            DefaultDurationFormat = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 17));
            DefaultInvalidHandleString = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 18));
            DefaultObjectFormat = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetIntConstant(ref MinTxQueueSize, 19));
            CheckErrorCode(Api.YOGI_GetIntConstant(ref MaxTxQueueSize, 20));
            CheckErrorCode(Api.YOGI_GetIntConstant(ref DefaultTxQueueSize, 21));

            CheckErrorCode(Api.YOGI_GetIntConstant(ref MinRxQueueSize, 22));
            CheckErrorCode(Api.YOGI_GetIntConstant(ref MaxRxQueueSize, 23));
            CheckErrorCode(Api.YOGI_GetIntConstant(ref DefaultRxQueueSize, 24));
        }
    }
}
