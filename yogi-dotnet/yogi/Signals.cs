/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2019 Johannes Bergmann.
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
        // === YOGI_RaiseSignal ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void RaiseSignalFnDelegate(IntPtr sigarg, IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int RaiseSignalDelegate(Signals signal, IntPtr sigarg,
            RaiseSignalFnDelegate fn, IntPtr userarg);

        public static RaiseSignalDelegate YOGI_RaiseSignal
            = Library.GetDelegateForFunction<RaiseSignalDelegate>(
                "YOGI_RaiseSignal");

        // === YOGI_SignalSetCreate ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int SignalSetCreateDelegate(ref IntPtr sigset,
            SafeObjectHandle context, Signals signals);

        public static SignalSetCreateDelegate YOGI_SignalSetCreate
            = Library.GetDelegateForFunction<SignalSetCreateDelegate>(
                "YOGI_SignalSetCreate");

        // === YOGI_SignalSetAwaitSignalAsync ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void SignalSetAwaitSignalFnDelegate(int res, Signals signal,
            IntPtr sigarg, IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int SignalSetAwaitSignalDelegate(SafeObjectHandle sigset,
            SignalSetAwaitSignalFnDelegate fn, IntPtr userarg);

        public static SignalSetAwaitSignalDelegate YOGI_SignalSetAwaitSignalAsync
            = Library.GetDelegateForFunction<SignalSetAwaitSignalDelegate>(
                "YOGI_SignalSetAwaitSignalAsync");

        // === YOGI_SignalSetCancelAwaitSignal ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int SignalSetCancelAwaitSignalDelegate(SafeObjectHandle sigset);

        public static SignalSetCancelAwaitSignalDelegate YOGI_SignalSetCancelAwaitSignal
            = Library.GetDelegateForFunction<SignalSetCancelAwaitSignalDelegate>(
                "YOGI_SignalSetCancelAwaitSignal");
    }

    /// <summary>
    /// Signals.
    /// </summary>
    [Flags]
    public enum Signals
    {
        /// <summary>No signal.</summary>
        None = 0,

        /// <summary>Interrupt (e.g. by pressing STRG + C).</summary>
        Int = (1 << 0),

        /// <summary>Termination request.</summary>
        Term = (1 << 1),

        /// <summary>User-defined signal 1.</summary>
        Usr1 = (1 << 24),

        /// <summary>User-defined signal 2.</summary>
        Usr2 = (1 << 25),

        /// <summary>User-defined signal 3.</summary>
        Usr3 = (1 << 26),

        /// <summary>User-defined signal 4.</summary>
        Usr4 = (1 << 27),

        /// <summary>User-defined signal 5.</summary>
        Usr5 = (1 << 28),

        /// <summary>User-defined signal 6.</summary>
        Usr6 = (1 << 29),

        /// <summary>User-defined signal 7.</summary>
        Usr7 = (1 << 30),

        /// <summary>User-defined signal 8.</summary>
        Usr8 = (1 << 31),

        /// <summary>All signals.</summary>
        All = Int | Term | Usr1 | Usr2 | Usr3 | Usr4 | Usr5 | Usr6 | Usr7 | Usr8
    }

    /// <summary>
    /// Delegate for the raise signal cleanup handler function.
    /// </summary>
    /// <param name="sigarg">Value of the sigarg parameter passed to RaiseSignal().</param>
    /// <typeparam name="T">Type of the sigarg parameter.</typeparam>
    public delegate void RaiseSignalFnDelegate<T>([Optional] T sigarg) where T : class;

    /// <summary>
    /// Raises a signal.
    ///
    /// Signals in Yogi are intended to be used similarly to POSIX signals. They
    /// have to be raised explicitly by the user (e.g. when receiving an actual
    /// POSIX signal like SIGINT) via this function. A signal will be received by
    /// all signal sets containing that signal.
    ///
    /// The sigarg parameter can be used to deliver user-defined data to the
    /// signal handlers.
    ///
    /// The cleanup handler fn will be called once all signal handlers have been
    /// called. Once fn has been called, the sigarg parameter is not used any more
    /// and can be cleaned up.
    ///
    /// Note: The cleanup handler fn can get called either from within the
    ///       RaiseSignal() function or from any context within the program.
    /// </summary>
    /// <typeparam name="T">Type of the sigarg parameter.</typeparam>
    /// <param name="signal">The signal to raise.</param>
    /// <param name="sigarg">Object to pass to the signal sets.</param>
    /// <param name="fn">Function call after all signal sets have been notified.</param>
    public static void RaiseSignal<T>(Signals signal, T sigarg,
        [Optional] RaiseSignalFnDelegate<T> fn) where T : class
    {
        Api.RaiseSignalFnDelegate wrapper = (sigargPtr, userarg) =>
        {
            try
            {
                if (fn != null)
                {
                    T obj = null;
                    if (sigargPtr != IntPtr.Zero)
                    {
                        obj = (T)GCHandle.FromIntPtr(sigargPtr).Target;
                    }

                    fn(obj);
                }
            }
            finally
            {
                GCHandle.FromIntPtr(sigargPtr).Free();
                GCHandle.FromIntPtr(userarg).Free();
            }
        };
        var wrapperHandle = GCHandle.Alloc(wrapper);
        var sigargHandle = GCHandle.Alloc(sigarg);

        try
        {
            var wrapperPtr = GCHandle.ToIntPtr(wrapperHandle);
            var sigargPtr = GCHandle.ToIntPtr(sigargHandle);
            int res = Api.YOGI_RaiseSignal(signal, sigargPtr, wrapper, wrapperPtr);
            CheckErrorCode(res);
        }
        catch
        {
            sigargHandle.Free();
            wrapperHandle.Free();
            throw;
        }
    }

    /// <summary>
    /// Raises a signal.
    ///
    /// Signals in Yogi are intended to be used similarly to POSIX signals. They
    /// have to be raised explicitly by the user (e.g. when receiving an actual
    /// POSIX signal like SIGINT) via this function. A signal will be received by
    /// all signal sets containing that signal.
    ///
    /// The cleanup handler fn will be called once all signal handlers have been
    /// called.
    ///
    /// Note: The cleanup handler fn can get called either from within the
    ///       RaiseSignal() function or from any context within the program.
    /// </summary>
    /// <param name="signal">The signal to raise.</param>
    /// <param name="fn">Function to call after all signal sets have been notified.</param>
    public static void RaiseSignal(Signals signal, [Optional] Action fn)
    {
        RaiseSignal<object>(signal, null, (sigarg) =>
        {
            if (fn != null)
            {
                fn();
            }
        });
    }

    /// <summary>
    /// Manages a set of signals that the user can wait on
    ///
    /// Signal sets are used to receive signals raised via RaiseSignal(). The signals
    /// are queued until they can be delivered by means of calls to AwaitSignalAsync().
    /// </summary>
    public class SignalSet : Object
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="context">The context to use.</param>
        /// <param name="signals">Signals to listen for.</param>
        public SignalSet(Context context, Signals signals)
        : base(Create(context, signals), new Object[] { context })
        {
        }

        /// <summary>
        /// Delegate for the AwaitSignalAsync<T>() function.
        /// </summary>
        /// <typeparam name="T">Type of the sigarg parameter passed to RaiseSignal().</typeparam>
        /// <param name="res">Result of the wait operation.</param>
        /// <param name="signal">The raised signal.</param>
        /// <param name="sigarg">Value of the sigarg parameter passed to RaiseSignal().</param>
        public delegate void AwaitSignalFnDelegate<T>(Result res, Signals signal,
            [Optional] T sigarg) where T : class;

        /// <summary>
        /// Waits for a signal to be raised.
        ///
        /// The handler \p fn will be called after one of the signals in the set
        /// is caught.
        /// </summary>
        /// <typeparam name="T">Type of the sigarg parameter passed to RaiseSignal().</typeparam>
        /// <param name="fn">Handler function to call.</param>
        public void AwaitSignalAsync<T>(AwaitSignalFnDelegate<T> fn) where T : class
        {
            Api.SignalSetAwaitSignalFnDelegate wrapper = (ec, signal, sigargPtr, userarg) =>
            {
                try
                {
                    T obj = null;
                    if (sigargPtr != IntPtr.Zero)
                    {
                        obj = (T)GCHandle.FromIntPtr(sigargPtr).Target;
                    }

                    fn(ErrorCodeToResult(ec), signal, obj);
                }
                finally
                {
                    GCHandle.FromIntPtr(userarg).Free();
                }
            };
            var wrapperHandle = GCHandle.Alloc(wrapper);

            try
            {
                var wrapperPtr = GCHandle.ToIntPtr(wrapperHandle);
                int res = Api.YOGI_SignalSetAwaitSignalAsync(Handle, wrapper, wrapperPtr);
                CheckErrorCode(res);
            }
            catch
            {
                wrapperHandle.Free();
                throw;
            }
        }

        /// <summary>
        /// Delegate for the AwaitSignalAsync() function.
        /// </summary>
        /// <param name="res">Result of the wait operation.</param>
        /// <param name="signal">The raised signal.</param>
        public delegate void AwaitSignalFnDelegate(Result res, Signals signal);

        /// <summary>
        /// Waits for a signal to be raised.
        ///
        /// The handler fn will be called after one of the signals in the set
        /// is caught.
        /// </summary>
        /// <param name="fn">Handler function to call.</param>
        public void AwaitSignalAsync(AwaitSignalFnDelegate fn)
        {
            AwaitSignalAsync<object>((res, signal, sigarg) => { fn(res, signal); });
        }

        /// <summary>
        /// Cancels waiting for a signal.
        ///
        /// This causes the handler function registered via AwaitSignalAsync() to be
        /// called with a cancellation error.
        /// </summary>
        /// <returns>True if the wait operation was cancelled successfully.</returns>
        public bool CancelAwaitSignal()
        {
            int res = Api.YOGI_SignalSetCancelAwaitSignal(Handle);
            return FalseIfSpecificErrorElseThrow(res, ErrorCode.OperationNotRunning);
        }

        static IntPtr Create(Context context, Signals signals)
        {
            var handle = new IntPtr();
            int res = Api.YOGI_SignalSetCreate(ref handle, context.Handle, signals);
            CheckErrorCode(res);
            return handle;
        }
    }
}
