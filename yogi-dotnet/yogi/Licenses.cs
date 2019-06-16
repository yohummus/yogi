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
        // === YOGI_GetLicense ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr GetLicenseDelegate();

        public static GetLicenseDelegate YOGI_GetLicense
            = Library.GetDelegateForFunction<GetLicenseDelegate>(
                "YOGI_GetLicense");

        // === YOGI_Get3rdPartyLicenses ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr Get3rdPartyLicensesDelegate();

        public static Get3rdPartyLicensesDelegate YOGI_Get3rdPartyLicenses
            = Library.GetDelegateForFunction<Get3rdPartyLicensesDelegate>(
                "YOGI_Get3rdPartyLicenses");
    }

    /// <summary>
    /// String containing the Yogi license.
    /// </summary>
    public static string License
    {
        get
        {
            return Marshal.PtrToStringAnsi(Api.YOGI_GetLicense());
        }
    }

    /// <summary>
    /// String containing licenses of the 3rd party libraries compiled into Yogi Core.
    /// </summary>
    public static string ThirdPartyLicenses
    {
        get
        {
            return Marshal.PtrToStringAnsi(Api.YOGI_Get3rdPartyLicenses());
        }
    }
}
