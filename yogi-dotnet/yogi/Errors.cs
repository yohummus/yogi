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
using System.Diagnostics;

public static partial class Yogi
{
    partial class Api
    {
        // === YOGI_GetErrorString ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr GetErrorStringDelegate(int err);

        public static GetErrorStringDelegate YOGI_GetErrorString
            = Library.GetDelegateForFunction<GetErrorStringDelegate>(
                "YOGI_GetErrorString");
    }

    /// <summary>
    /// Error codes returned by functions from the Yogi Core library.
    /// </summary>
    public enum ErrorCode
    {
        /// <summary>Operation completed successfully.</summary>
        Ok = 0,

        /// <summary>Unknown internal error occured.</summary>
        Unknown = -1,

        /// <summary>The object is still being used by another object.</summary>
        ObjectStillUsed = -2,

        /// <summary>Insufficient memory to complete the operation.</summary>
        BadAlloc = -3,

        /// <summary>Invalid parameter.</summary>
        InvalidParam = -4,

        /// <summary>Invalid Handle.</summary>
        InvalidHandle = -5,

        /// <summary>Object is of the wrong type.</summary>
        WrongObjectType = -6,

        /// <summary>The operation has been canceled.</summary>
        Canceled = -7,

        /// <summary>Operation failed because the object is busy.</summary>
        Busy = -8,

        /// <summary>The operation timed out.</summary>
        Timeout = -9,

        /// <summary>The timer has not been started or already expired.</summary>
        TimerExpired = -10,

        /// <summary>The supplied buffer is too small.</summary>
        BufferTooSmall = -11,

        /// <summary>Could not open a socket.</summary>
        OpenSocketFailed = -12,

        /// <summary>Could not bind a socket.</summary>
        BindSocketFailed = -13,

        /// <summary>Could not listen on socket.</summary>
        ListenSocketFailed = -14,

        /// <summary>Could not set a socket option.</summary>
        SetSocketOptionFailed = -15,

        /// <summary>Invalid regular expression.</summary>
        InvalidRegex = -16,

        /// <summary>Could not read from file.</summary>
        OpenFileFailed = -17,

        /// <summary>Could not read from or write to socket.</summary>
        RwSocketFailed = -18,

        /// <summary>Could not connect a socket.</summary>
        ConnectSocketFailed = -19,

        /// <summary>The magic prefix sent when establishing a connection is wrong.</summary>
        InvalidMagicPrefix = -20,

        /// <summary>The Yogi versions are not compatible.</summary>
        IncompatibleVersion = -21,

        /// <summary>Could not deserialize a message.</summary>
        DeserializeMsgFailed = -22,

        /// <summary>Could not accept a socket.</summary>
        AcceptSocketFailed = -23,

        /// <summary>Attempting to connect branch to itself.</summary>
        LoopbackConnection = -24,

        /// <summary>The passwords of the local and remote branch don't match.</summary>
        PasswordMismatch = -25,

        /// <summary>The net names of the local and remote branch don't match.</summary>
        NetNameMismatch = -26,

        /// <summary>A branch with the same name is already active.</summary>
        DuplicateBranchName = -27,

        /// <summary>A branch with the same path is already active.</summary>
        DuplicateBranchPath = -28,

        /// <summary>Message payload is too large.</summary>
        PayloadTooLarge = -29,

        /// <summary>Parsing the command line failed.</summary>
        ParsingCmdlineFailed = -30,

        /// <summary>Parsing a JSON string failed.</summary>
        ParsingJsonFailed = -31,

        /// <summary>Parsing a configuration file failed.</summary>
        ParsingFileFailed = -32,

        /// <summary>The configuration is not valid.</summary>
        ConfigNotValid = -33,

        /// <summary>Help/usage text requested.</summary>
        HelpRequested = -34,

        /// <summary>Could not write to file.</summary>
        WriteToFileFailed = -35,

        /// <summary>One or more configuration variables are undefined or could not be resolved.</summary>
        UndefinedVariables = -36,

        /// <summary>Support for configuration variables has been disabled.</summary>
        NoVariableSupport = -37,

        /// <summary>A configuration variable has been used in a key.</summary>
        VariableUsedInKey = -38,

        /// <summary>Invalid time format.</summary>
        InvalidTimeFormat = -39,

        /// <summary>Could not parse time string.</summary>
        ParsingTimeFailed = -40,

        /// <summary>A send queue for a remote branch is full.</summary>
        TxQueueFull = -41,

        /// <summary>Invalid operation ID.</summary>
        InvalidOperationId = -42,

        /// <summary>Operation is not running.</summary>
        OperationNotRunning = -43,

        /// <summary>User-supplied data is not valid MessagePack.</summary>
        InvalidUserMsgpack = -44,

        /// <summary>Joining UDP multicast group failed.</summary>
        JoinMulticastGroupFailed = -45,

        /// <summary>Enumerating network interfaces failed.</summary>
        EnumerateNetworkInterfacesFailed = -46,

        /// <summary>The section could not be found in the configuration.</summary>
        ConfigurationSectionNotFound = -47,
    }

    /// <summary>
    /// Represents a result of an operation.
    ///
    /// This is a wrapper around the result code returned by the functions from
    /// the Yogi Core library. A result is represented by a number which is >= 0
    /// in case of success and < 0 in case of a failure.
    /// </summary>
    public class Result : IComparable
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="value">Number as returned by the Yogi Core library function.</param>
        public Result(int value)
        {
            Value = value;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="ec">Error code.</param>
        public Result(ErrorCode ec)
        {
            Value = (int)ec;
        }

        /// <summary>
        /// The number as returned by the Yogi Core library function.
        /// </summary>
        public int Value { get; }

        /// <summary>
        /// Error code associated with this result.
        /// </summary>
        public ErrorCode ErrorCode
        {
            get
            {
                return Value < 0 ? (ErrorCode)Value : ErrorCode.Ok;
            }
        }

        /// <summary>
        /// Returns a human-readable string describing the result.
        /// </summary>
        public override string ToString()
        {
            return Marshal.PtrToStringAnsi(Api.YOGI_GetErrorString(Value));
        }

        public static bool operator <(Result lhs, Result rhs)
        {
            return lhs.Value < rhs.Value;
        }

        public static bool operator >(Result lhs, Result rhs)
        {
            return lhs.Value > rhs.Value;
        }

        public static bool operator ==(Result lhs, Result rhs)
        {
            if (object.ReferenceEquals(lhs, null))
            {
                return object.ReferenceEquals(rhs, null);
            }

            return lhs.Equals(rhs);
        }

        public static bool operator !=(Result lhs, Result rhs)
        {
            return lhs.Value != rhs.Value;
        }

        public static bool operator <=(Result lhs, Result rhs)
        {
            return lhs.Value <= rhs.Value;
        }

        public static bool operator >=(Result lhs, Result rhs)
        {
            return lhs.Value >= rhs.Value;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is Result)) return false;
            return Value == ((Result)obj).Value;
        }

        public static bool operator true(Result res)
        {
            return res.Value >= 0;
        }

        public static bool operator false(Result res)
        {
            return res.Value < 0;
        }

        public static implicit operator bool(Result res)
        {
            return res.Value >= 0;
        }

        public override int GetHashCode()
        {
            return Value.GetHashCode();
        }

        public int CompareTo(object that)
        {
            if (!(that is Result)) return -1;
            if (this == (Result)that) return 0;
            return this < (Result)that ? -1 : +1;
        }
    }

    /// <summary>
    /// Represents the success of an operation.
    /// </summary>
    public class Success : Result
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="value">Number as returned by the Yogi Core library function.</param>
        /// <returns></returns>
        public Success(int value = 0)
        : base(value)
        {
            Debug.Assert(value >= 0);
        }
    }

    /// <summary>
    /// Represents the failure of an operation.
    /// </summary>
    public class Failure : Result
    {

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="ec">Associated error code.</param>
        public Failure(ErrorCode ec)
        : base(ec)
        {
        }
    }

    /// <summary>
    /// A failure of an operation that includes a description.
    /// </summary>
    public class DescriptiveFailure : Failure
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="ec">Error code.</param>
        /// <param name="description">More detailed information about the error.</param>
        /// <returns></returns>
        public DescriptiveFailure(ErrorCode ec, string description)
        : base(ec)
        {
            Description = description;
        }

        /// <summary>
        /// More detailed information about the error.
        /// </summary>
        public string Description { get; }

        /// <summary>
        /// Returns a human-readable string describing the error in detail.
        /// </summary>
        public override string ToString()
        {
            return base.ToString() + ". Description: " + Description;
        }
    }

    /// <summary>
    /// Base class for all Yogi exceptions.
    /// </summary>
    public abstract class Exception : System.Exception
    {
        /// <summary>
        /// The wrapped Failure or DescriptiveFailure object.
        /// </summary>
        public abstract Failure Failure { get; }

        /// <summary>
        /// The exception message.
        /// </summary>
        public override string Message
        {
            get
            {
                return Failure.ToString();
            }
        }
    }

    /// <summary>
    /// Exception wrapping a Failure object.
    ///
    /// This exception type is used for failures without a detailed description.
    /// </summary>
    public class FailureException : Exception
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="ec">Error code associated with the failure.</param>
        public FailureException(ErrorCode ec)
        {
            Failure = new Failure(ec);
        }

        public override Failure Failure { get; }
    }

    /// <summary>
    /// Exception wrapping a DescriptiveFailure object.
    ///
    /// This exception type is used for failures that have detailed information available.
    /// </summary>
    public class DescriptiveFailureException : FailureException
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="ec">Error code associated with the failure.</param>
        /// <param name="description">Detailed description of the failure.</param>
        public DescriptiveFailureException(ErrorCode ec, string description) : base(ec)
        {
            Failure = new DescriptiveFailure(ec, description);
        }

        public override Failure Failure { get; }
    }

    static void CheckErrorCode(int res)
    {
        if (res < 0)
        {
            throw new FailureException((ErrorCode)res);
        }
    }

    static bool FalseIfSpecificErrorElseThrow(int res, ErrorCode ec)
    {
        if (res == (int)ec)
        {
            return false;
        }
        else
        {
            CheckErrorCode(res);
            return true;
        }
    }

    static void CheckDescriptiveErrorCode(Func<StringBuilder, int> fn)
    {
        var err = new StringBuilder(256);
        int res = fn(err);
        if (res < 0)
        {
            var s = err.ToString();
            if (s.Length > 0)
            {
                throw new DescriptiveFailureException((ErrorCode)res, err.ToString());
            }
            else
            {
                throw new FailureException((ErrorCode)res);
            }
        }
    }

    static Result ErrorCodeToResult(int res)
    {
        if (res < 0)
        {
            return new Failure((ErrorCode)res);
        }
        else
        {
            return new Success(res);
        }
    }
}
