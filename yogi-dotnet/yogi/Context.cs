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
        // === YOGI_ContextCreate ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextCreateDelegate(ref IntPtr context);

        public static ContextCreateDelegate YOGI_ContextCreate
            = Library.GetDelegateForFunction<ContextCreateDelegate>(
                "YOGI_ContextCreate");

        // === YOGI_ContextPoll ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextPollDelegate(SafeObjectHandle context, ref int count);

        public static ContextPollDelegate YOGI_ContextPoll
            = Library.GetDelegateForFunction<ContextPollDelegate>(
                "YOGI_ContextPoll");

        // === YOGI_ContextPollOne ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextPollOneDelegate(SafeObjectHandle context, ref int count);

        public static ContextPollOneDelegate YOGI_ContextPollOne
            = Library.GetDelegateForFunction<ContextPollOneDelegate>(
                "YOGI_ContextPollOne");

        // === YOGI_ContextRun ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextRunDelegate(SafeObjectHandle context, ref int count,
            long duration);

        public static ContextRunDelegate YOGI_ContextRun
            = Library.GetDelegateForFunction<ContextRunDelegate>(
                "YOGI_ContextRun");

        // === YOGI_ContextRunOne ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextRunOneDelegate(SafeObjectHandle context, ref int count,
            long duration);

        public static ContextRunOneDelegate YOGI_ContextRunOne
            = Library.GetDelegateForFunction<ContextRunOneDelegate>(
                "YOGI_ContextRunOne");

        // === YOGI_ContextRunInBackground ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextRunInBackgroundDelegate(SafeObjectHandle context);

        public static ContextRunInBackgroundDelegate YOGI_ContextRunInBackground
            = Library.GetDelegateForFunction<ContextRunInBackgroundDelegate>(
                "YOGI_ContextRunInBackground");

        // === YOGI_ContextStop ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextStopDelegate(SafeObjectHandle context);

        public static ContextStopDelegate YOGI_ContextStop
            = Library.GetDelegateForFunction<ContextStopDelegate>(
                "YOGI_ContextStop");

        // === YOGI_ContextWaitForRunning ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextWaitForRunningDelegate(SafeObjectHandle context, long duration);

        public static ContextWaitForRunningDelegate YOGI_ContextWaitForRunning
            = Library.GetDelegateForFunction<ContextWaitForRunningDelegate>(
                "YOGI_ContextWaitForRunning");

        // === YOGI_ContextWaitForStopped ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextWaitForStoppedDelegate(SafeObjectHandle context, long duration);

        public static ContextWaitForStoppedDelegate YOGI_ContextWaitForStopped
            = Library.GetDelegateForFunction<ContextWaitForStoppedDelegate>(
                "YOGI_ContextWaitForStopped");

        // === YOGI_ContextPost ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ContextPostFnDelegate(IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ContextPostDelegate(SafeObjectHandle context,
            ContextPostFnDelegate fn, IntPtr userarg);

        public static ContextPostDelegate YOGI_ContextPost
            = Library.GetDelegateForFunction<ContextPostDelegate>(
                "YOGI_ContextPost");
    }

    /// <summary>
    /// Scheduler for the execution of asynchronous operations.
    ///
    /// Once an asynchronous operation finishes, the corresponding handler function
    /// is added to the context's event queue and executed through once of the
    /// Poll... or Run... functions.
    /// </summary>
    public class Context : Object
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        public Context()
        : base(Create())
        {
        }

        /// <summary>
        /// Runs the context's event processing loop to execute ready handlers.
        ///
        /// This function runs handlers (internal and user-supplied such as functions
        /// registered through Post()) that are ready to run, without blocking, until
        /// the Stop() function has been called or there are no more ready handlers.
        ///
        /// This function must be called from outside any handler functions that are
        /// being executed through the context.
        /// </summary>
        /// <returns>Number of executed handlers.</returns>
        public int Poll()
        {
            int count = -1;
            int res = Api.YOGI_ContextPoll(Handle, ref count);
            CheckErrorCode(res);
            return count;
        }

        /// <summary>
        /// Runs the context's event processing loop to execute at most one ready handler.
        ///
        /// This function runs at most one handler (internal and user-supplied such as
        /// functions registered through Post()) that are ready to run, without blocking.
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        /// <returns>Number of executed handlers.</returns>
        public int PollOne()
        {
            int count = -1;
            int res = Api.YOGI_ContextPollOne(Handle, ref count);
            CheckErrorCode(res);
            return count;
        }

        /// <summary>
        /// Runs the context's event processing loop for the specified duration.
        ///
        /// This function blocks while running the context's event processing loop and
        /// calling dispatched handlers (internal and user-supplied such as functions
        /// registered through Post()) for the specified duration unless Stop() is called
        /// within that time.
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        /// <param name="duration">Duration (infinity by default).</param>
        /// <returns>Number of executed handlers.</returns>
        public int Run([Optional] Duration duration)
        {
            long dur = duration == null ? -1 : DurationToApiDuration(duration);

            int count = -1;
            int res = Api.YOGI_ContextRun(Handle, ref count, dur);
            CheckErrorCode(res);
            return count;
        }

        /// <summary>
        /// Runs the context's event processing loop for the specified duration.
        ///
        /// This function blocks while running the context's event processing loop and
        /// calling dispatched handlers (internal and user-supplied such as functions
        /// registered through Post()) for the specified duration unless Stop() is called
        /// within that time.
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        /// <param name="duration">Duration.</param>
        /// <returns>Number of executed handlers.</returns>
        public int Run(TimeSpan duration)
        {
            return Run(new Duration(duration));
        }

        /// <summary>
        /// Runs the context's event processing loop for the specified duration to execute
        /// at most one handler.
        ///
        /// This function blocks while running the context's event processing loop and
        /// calling dispatched handlers (internal and user-supplied such as functions
        /// registered through Post()) for the specified duration until a single handler
        /// function has been executed, unless Stop() is called within that time.
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        /// <param name="duration">Duration (infinity by default).</param>
        /// <returns>Number of executed handlers.</returns>
        public int RunOne([Optional] Duration duration)
        {
            long dur = duration == null ? -1 : DurationToApiDuration(duration);

            int count = -1;
            int res = Api.YOGI_ContextRunOne(Handle, ref count, dur);
            CheckErrorCode(res);
            return count;
        }

        /// <summary>
        /// Runs the context's event processing loop for the specified duration to execute
        /// at most one handler.
        ///
        /// This function blocks while running the context's event processing loop and
        /// calling dispatched handlers (internal and user-supplied such as functions
        /// registered through Post()) for the specified duration until a single handler
        /// function has been executed, unless Stop() is called within that time.
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        /// <param name="duration">Duration.</param>
        /// <returns>Number of executed handlers.</returns>
        public int RunOne(TimeSpan duration)
        {
            return RunOne(new Duration(duration));
        }

        /// <summary>
        /// Starts an internal thread for running the context's event processing loop.
        ///
        /// This function starts a threads that runs the context's event processing loop
        /// in the background. It relieves the user from having to start a thread and
        /// calling the appropriate Run... or Poll... functions themself. The thread can
        /// be stopped using Stop().
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        public void RunInBackground()
        {
            int res = Api.YOGI_ContextRunInBackground(Handle);
            CheckErrorCode(res);
        }

        /// <summary>
        /// Stops the context's event processing loop.
        ///
        /// This function signals the context to stop running its event processing loop.
        /// This causes Run... functions to return as soon as possible and it terminates
        /// the thread started via RunInBackground().
        /// </summary>
        public void Stop()
        {
            int res = Api.YOGI_ContextStop(Handle);
            CheckErrorCode(res);
        }

        /// <summary>
        /// Blocks until the context's event processing loop is being run or until the
        /// specified timeout is reached.
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        /// <param name="duration">Maximum time to wait (infinity by default).</param>
        /// <returns>True if the context's event processing loop is running within the
        /// specified duration and false otherwise.</returns>
        public bool WaitForRunning([Optional] Duration duration)
        {
            long dur = duration == null ? -1 : DurationToApiDuration(duration);

            int res = Api.YOGI_ContextWaitForRunning(Handle, dur);
            if ((ErrorCode)res == ErrorCode.Timeout) return false;
            CheckErrorCode(res);
            return true;
        }

        /// <summary>
        /// Blocks until the context's event processing loop is being run or until the
        /// specified timeout is reached.
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        /// <param name="duration">Maximum time to wait.</param>
        /// <returns>True if the context's event processing loop is running within the
        /// specified duration and false otherwise.</returns>
        public bool WaitForRunning(TimeSpan duration)
        {
            return WaitForRunning(new Duration(duration));
        }

        /// <summary>
        /// Blocks until no thread is running the context's event processing
        /// loop or until the specified timeout is reached.
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        /// <param name="duration">Maximum time to wait (infinity by default).</param>
        /// <returns>True if the context's event processing loop is not running within the
        /// specified duration and false otherwise.</returns>
        public bool WaitForStopped([Optional] Duration duration)
        {
            long dur = duration == null ? -1 : DurationToApiDuration(duration);

            int res = Api.YOGI_ContextWaitForStopped(Handle, dur);
            if ((ErrorCode)res == ErrorCode.Timeout) return false;
            CheckErrorCode(res);
            return true;
        }

        /// <summary>
        /// Blocks until no thread is running the context's event processing
        /// loop or until the specified timeout is reached.
        ///
        /// This function must be called from outside any handler functions that are being
        /// executed through the context.
        /// </summary>
        /// <param name="duration">Maximum time to wait.</param>
        /// <returns>True if the context's event processing loop is not running within the
        /// specified duration and false otherwise.</returns>
        public bool WaitForStopped(TimeSpan duration)
        {
            return WaitForStopped(new Duration(duration));
        }

        /// <summary>
        /// Adds the given function to the context's event processing queue to
        /// be executed and returns immediately.
        ///
        /// The handler fn will only be executed after this function returns and
        /// only by a thread running the context's event processing loop.
        /// </summary>
        /// <param name="fn">The function to call from within the context.</param>
        public void Post(Action fn)
        {
            Api.ContextPostFnDelegate wrapper = (userarg) => {
                try
                {
                    fn();
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
                int res = Api.YOGI_ContextPost(Handle, wrapper, wrapperPtr);
                CheckErrorCode(res);
            }
            catch
            {
                wrapperHandle.Free();
                throw;
            }

        }

        static IntPtr Create()
        {
            var handle = new IntPtr();
            int res = Api.YOGI_ContextCreate(ref handle);
            CheckErrorCode(res);
            return handle;
        }
    }
}
