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
using System.Reflection;

public static partial class Yogi
{
    /// <summary>
    /// CInformation about the bindings.
    /// </summary>
    public static class BindingsInfo
    {
        /// <summary>Whole version string of the bindings.</summary>
        public static readonly string Version;

        /// <summary>Major version of the bindings.</summary>
        public static readonly int VersionMajor;

        /// <summary>Minor version of the bindings.</summary>
        public static readonly int VersionMinor;

        /// <summary>Patch version of the bindings.</summary>
        public static readonly int VersionPatch;

        /// <summary>Version suffix of the bindings.</summary>
        public static readonly string VersionSuffix;

        static BindingsInfo()
        {
            var assembly = Assembly.GetExecutingAssembly();
            var assemblyVersion = assembly.GetName().Version;
            VersionMajor = assemblyVersion.Major;
            VersionMinor = assemblyVersion.Minor;
            VersionPatch = assemblyVersion.Build;

            var attr = assembly.GetCustomAttribute<AssemblyInformationalVersionAttribute>();
            Version = attr.InformationalVersion;

            VersionSuffix = Version.Substring(assemblyVersion.ToString(3).Length);
        }
    }
}
