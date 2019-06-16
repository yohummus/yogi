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
using System.Text;
using System.Diagnostics;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Collections.Generic;

public static partial class Yogi
{
    partial class Api
    {
        // === YOGI_FormatObject ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int FormatObjectDelegate(IntPtr obj,
            [MarshalAs(UnmanagedType.LPStr)] StringBuilder str, int strsize,
            [MarshalAs(UnmanagedType.LPStr)] string objfmt,
            [MarshalAs(UnmanagedType.LPStr)] string nullstr);

        public static FormatObjectDelegate YOGI_FormatObject
            = Library.GetDelegateForFunction<FormatObjectDelegate>(
                "YOGI_FormatObject");

        // === YOGI_Destroy ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int DestroyDelegate(IntPtr handle);

        public static DestroyDelegate YOGI_Destroy
            = Library.GetDelegateForFunction<DestroyDelegate>(
                "YOGI_Destroy");
    }

    internal class SafeObjectHandle : SafeHandle
    {
        public SafeObjectHandle(string objectTypeName, IntPtr handle)
        : base(IntPtr.Zero, true)
        {
            this.ObjectTypeName = objectTypeName;
            this.handle = handle;
        }

        public string ObjectTypeName { get; }

        public override bool IsInvalid
        {
            get { return handle == IntPtr.Zero; }
        }

        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
        protected override bool ReleaseHandle()
        {
            Result res = ErrorCodeToResult(Api.YOGI_Destroy(DangerousGetHandle()));
            if (!res)
            {
                string info = "";
                if (res.ErrorCode == ErrorCode.ObjectStillUsed)
                {
                    info = " Check that you don't have circular dependencies on Yogi objects.";
                }

                string s = $"Could not destroy {ObjectTypeName}: {res}.{info}";
                Console.Error.WriteLine(s);

                return false;
            }

            return true;
        }
    }

    /// <summary>
    /// Base class for all "creatable" objects.
    /// </summary>
    public class Object : IDisposable
    {
        /// <summary>
        /// Disposes the object.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Creates a string describing the object.
        ///
        /// The fmt parameter describes the format of the string. The following
        /// placeholders are supported:
        ///     $T: Type of the object (e.g. Branch)
        ///     $x: Handle of the object in lower-case hex notation
        ///     $X: Handle of the object in upper-case hex notation
        ///
        /// By default, the object will be formatted in the format
        /// "Branch [44fdde]" with the hex value in brackets being the object's
        /// handle.
        ///
        /// If, for any reason, the object's handle is null, this function returns
        /// the nullstr parameter value ("INVALID HANDLE" by default).
        /// </summary>
        /// <param name="fmt">Format of the string.</param>
        /// <param name="nullstr">String to use if the handle is null.</param>
        /// <returns>Formatted string.</returns>
        public string Format([Optional] string fmt, [Optional] string nullstr)
        {
            var str = new StringBuilder(128);
            int res = Api.YOGI_FormatObject(handle.DangerousGetHandle(), str, str.Capacity,
                fmt, nullstr);
            CheckErrorCode(res);
            return str.ToString();
        }

        /// <summary>
        /// Returns a string representation of the object.
        /// </summary>
        /// <returns>String representation of the object.</returns>
        public override string ToString()
        {
            return Format();
        }

        /// <summary>
        /// Returns true if the Dispose() function has been called for this object.
        /// </summary>
        public bool Disposed { get; private set; }

        /// <summary>
        /// Constructs the object.
        ///
        /// The objects in the depencies list are stored in order to prevent the
        /// garbage collector to destroy them before this objects gets destroyed.
        /// For example, it would prevent the destruction of the context object in
        /// the following case:
        ///    var timer = new Yogi.Timer(new Yogi.Context())
        /// </summary>
        /// <param name="nativeHandle">Native handle representing the object.</param>
        /// <param name="dependencies">Other objects that this object depends on.</param>
        protected Object(IntPtr nativeHandle, [Optional] Object[] dependencies)
        {
            this.handle = new SafeObjectHandle(GetType().Name, nativeHandle);
            this.dependencies = dependencies;
            Disposed = false;
            refCounter = 1;

            if (dependencies != null)
            {
                foreach (var dependency in dependencies)
                {
                    Debug.Assert(!dependency.Disposed);
                    dependency.IncRefCounter();
                }
            }
        }

        internal SafeObjectHandle Handle
        {
            get
            {
                if (Disposed)
                {
                    return new SafeObjectHandle(GetType().Name, IntPtr.Zero);
                }
                else
                {
                    return handle;
                }
            }

            private set
            {
                handle = value;
            }
        }

        ~Object()
        {
            Dispose(false);
        }

        void IncRefCounter()
        {
            lock (handle)
            {
                Debug.Assert(refCounter > 0);
                ++refCounter;
            }
        }

        void DecRefCounter()
        {

            lock (handle)
            {
                Debug.Assert(refCounter > 0);
                --refCounter;
                if (refCounter == 0)
                {
                    handle.Dispose();
                }
            }

            if (dependencies != null)
            {
                foreach (var dependency in dependencies)
                {
                    dependency.DecRefCounter();
                }
            }
        }

        void Dispose(bool disposing)
        {
            if (Disposed) return;
            Disposed = true;

            DecRefCounter();
        }

        SafeObjectHandle handle;
        Object[] dependencies;
        volatile int refCounter;
    }
}
