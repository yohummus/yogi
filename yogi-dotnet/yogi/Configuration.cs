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
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

public static partial class Yogi
{
    partial class Api
    {
        // === YOGI_ConfigurationCreate ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ConfigurationCreateDelegate(ref IntPtr config,
            ConfigurationFlags flags);

        public static ConfigurationCreateDelegate YOGI_ConfigurationCreate
            = Library.GetDelegateForFunction<ConfigurationCreateDelegate>(
                "YOGI_ConfigurationCreate");

        // === YOGI_ConfigurationUpdateFromCommandLine ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ConfigurationUpdateFromCommandLineDelegate(SafeObjectHandle config,
            int argc, string[] argv, CommandLineOptions options,
            [MarshalAs(UnmanagedType.LPStr)] StringBuilder err, int errsize);

        public static ConfigurationUpdateFromCommandLineDelegate
            YOGI_ConfigurationUpdateFromCommandLine
            = Library.GetDelegateForFunction<ConfigurationUpdateFromCommandLineDelegate>(
                "YOGI_ConfigurationUpdateFromCommandLine");

        // === YOGI_ConfigurationUpdateFromJson ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ConfigurationUpdateFromJsonDelegate(SafeObjectHandle config,
            byte[] json, [MarshalAs(UnmanagedType.LPStr)] StringBuilder err, int errsize);

        public static ConfigurationUpdateFromJsonDelegate YOGI_ConfigurationUpdateFromJson
            = Library.GetDelegateForFunction<ConfigurationUpdateFromJsonDelegate>(
                "YOGI_ConfigurationUpdateFromJson");

        // === YOGI_ConfigurationUpdateFromFile ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ConfigurationUpdateFromFileDelegate(SafeObjectHandle config,
            string filename, [MarshalAs(UnmanagedType.LPStr)] StringBuilder err, int errsize);

        public static ConfigurationUpdateFromFileDelegate YOGI_ConfigurationUpdateFromFile
            = Library.GetDelegateForFunction<ConfigurationUpdateFromFileDelegate>(
                "YOGI_ConfigurationUpdateFromFile");

        // === YOGI_ConfigurationDump ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ConfigurationDumpDelegate(SafeObjectHandle config,
            [MarshalAs(UnmanagedType.LPStr)] StringBuilder json, int jsonsize,
            int resvars, int indent);

        public static ConfigurationDumpDelegate YOGI_ConfigurationDump
            = Library.GetDelegateForFunction<ConfigurationDumpDelegate>(
                "YOGI_ConfigurationDump");

        // === YOGI_ConfigurationWriteToFile ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ConfigurationWriteToFileDelegate(SafeObjectHandle config,
            string filename, int resvars, int indent);

        public static ConfigurationWriteToFileDelegate YOGI_ConfigurationWriteToFile
            = Library.GetDelegateForFunction<ConfigurationWriteToFileDelegate>(
                "YOGI_ConfigurationWriteToFile");
    }

    /// <summary>
    /// Flags used to change a configuration object's behaviour.
    /// </summary>
    [Flags]
    public enum ConfigurationFlags
    {
        /// <summary>No flags.</summary>
        None = 0,

        /// <summary>Disables support for variables in the configuration.</summary>
        DisableVariables = (1 << 0),

        /// <summary>Makes configuration options given directly on the command line
        /// overridable.</summary>
        MutableCmdLine = (1 << 1)
    }

    /// <summary>
    /// Flags used to adjust how command line options are parsed.
    /// </summary>
    [Flags]
    public enum CommandLineOptions
    {
        /// <summary>No options.</summary>
        None = 0,

        /// <summary>Include logging configuration for file logging.</summary>
        Logging = (1 << 0),

        /// <summary>Include branch name configuration.</summary>
        BranchName = (1 << 1),

        /// <summary>Include branch description configuration.</summary>
        BranchDescription = (1 << 2),

        /// <summary>Include network name configuration.</summary>
        BranchNetwork = (1 << 3),

        /// <summary>Include network password configuration.</summary>
        BranchPassword = (1 << 4),

        /// <summary>Include branch path configuration.</summary>
        BranchPath = (1 << 5),

        /// <summary>Include branch advertising interfaces configuration.</summary>
        BranchAdvIfs = (1 << 6),

        /// <summary>Include branch advertising address configuration.</summary>
        BranchAdvAddr = (1 << 7),

        /// <summary>Include branch advertising port configuration.</summary>
        BranchAdvPort = (1 << 8),

        /// <summary>Include branch advertising interval configuration.</summary>
        BranchAdvInt = (1 << 9),

        /// <summary>Include branch timeout configuration.</summary>
        BranchTimeout = (1 << 10),

        /// <summary>Include ghost mode configuration.</summary>
        BranchGhostMode = (1 << 11),

        /// <summary>Parse configuration files given on the command line.</summary>
        Files = (1 << 12),

        /// <summary>Same as the Files option but at least one configuration file must be
        /// given.</summary>
        FilesRequired = (1 << 13),

        /// <summary>Allow overriding arbitrary configuration sections.</summary>
        Overrides = (1 << 14),

        /// <summary>Allow setting variables via a dedicated switch.</summary>
        Variables = (1 << 15),

        /// <summary>Combination of all branch flags.</summary>
        BranchAll = BranchName | BranchDescription | BranchNetwork | BranchPassword | BranchPath
            | BranchAdvIfs | BranchAdvAddr | BranchAdvPort | BranchAdvInt | BranchTimeout
            | BranchGhostMode,

        /// <summary>Combination of all flags.</summary>
        All = Logging | BranchAll | Files | FilesRequired | Overrides | Variables,
    }

    /// <summary>
    /// A configuration represents a set of parameters that usually remain constant
    /// throughout the runtime of a program. Parameters can come from different
    /// sources such as the command line or a file. Configurations are used for
    /// other parts of the library such as application objects, however, they are
    /// also intended to store user-defined parameters.
    /// </summary>
    public class Configuration : Object
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="flags">Flags for behaviour adjustments.</param>
        public Configuration(ConfigurationFlags flags = ConfigurationFlags.None)
        : base(Create(flags))
        {
            Flags = flags;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="json">Initial JSON to use.</param>
        /// <param name="flags">Flags for behaviour adjustments.</param>
        public Configuration(JsonView json, ConfigurationFlags flags = ConfigurationFlags.None)
        : this(flags)
        {
            UpdateFromJson(json);
        }

        /// <summary>
        /// Updates the configuration from command line options.
        ///
        /// If parsing the command line, files or any given JSON string fails, or
        /// if help is requested (e.g. by using the --help switch) then a
        /// DescriptiveFailure exception will be raised containing detailed
        /// information about the error or the help text.
        /// </summary>
        /// <param name="argv">List of command line arguments including the script name.</param>
        /// <param name="options">Options to provide on the command line.</param>
        public void UpdateFromCommandLine(IEnumerable<string> argv,
            CommandLineOptions options = CommandLineOptions.None)
        {
            var args = new List<string>();
            args.AddRange(argv);

            CheckDescriptiveErrorCode((err) =>
            {
                return Api.YOGI_ConfigurationUpdateFromCommandLine(Handle, args.Count,
                    args.ToArray(), options, err, err.Capacity);
            });
        }

        /// <summary>
        /// Updates the configuration from a JSON object serialized to a string.
        ///
        /// If parsing fails then a DescriptiveFailure exception will be raised
        /// containing detailed information about the error.
        /// </summary>
        /// <param name="json">JSON object.</param>
        public void UpdateFromJson(JsonView json)
        {
            CheckDescriptiveErrorCode((err) =>
            {
                return Api.YOGI_ConfigurationUpdateFromJson(Handle, json.Data, err, err.Capacity);
            });
        }

        /// <summary>
        /// Updates the configuration from a JSON file.
        ///
        /// If parsing the file fails then a DescriptiveFailure exception will be raised
        /// containing detailed information about the error.
        /// </summary>
        /// <param name="filename">Path to the JSON file.</param>
        public void UpdateFromFile(string filename)
        {
            CheckDescriptiveErrorCode((err) =>
            {
                return Api.YOGI_ConfigurationUpdateFromFile(Handle, filename, err, err.Capacity);
            });
        }

        /// <summary>
        /// Retrieves the configuration as a JSON-formatted string.
        /// </summary>
        /// <param name="resolveVariables">Resolve all configuration variables.
        /// By default, variables get resolved if the configuration supports them.</param>
        /// <param name="indentation">Number of space characters to use for indentation.
        /// By default, no spaces are used and new lines are omitted as well.</param>
        /// <returns>The configuration serialized to a string.</returns>
        public string Dump([Optional] bool? resolveVariables, [Optional] int? indentation)
        {
            if (resolveVariables == null)
            {
                resolveVariables = (Flags & ConfigurationFlags.DisableVariables)
                                    != ConfigurationFlags.DisableVariables;
            }

            if (indentation == null)
            {
                indentation = -1;
            }

            StringBuilder json;
            var size = 512;
            int res;
            do
            {
                json = new StringBuilder(size);
                size *= 2;
                res = Api.YOGI_ConfigurationDump(Handle, json, json.Capacity,
                    resolveVariables == true ? 1 : 0, (int)indentation);
            }
            while (res == (int)ErrorCode.BufferTooSmall);
            CheckErrorCode(res);

            return json.ToString();
        }

        /// <summary>
        /// Retrieves the configuration as a JSON object.
        /// </summary>
        /// <param name="resolveVariables">Resolve all configuration variables.
        /// By default, variables get resolved if the configuration supports them.</param>
        /// <returns>JSON object representing the configuration.</returns>
        public JObject ToJson([Optional] bool? resolveVariables)
        {
            return JObject.Parse(Dump(resolveVariables));
        }

        /// <summary>
        /// Writes the configuration to a file in JSON format.
        /// </summary>
        /// <param name="filename">Path to the output file.</param>
        /// <param name="resolveVariables">Resolve all configuration variables.
        /// By default, variables get resolved if the configuration supports them.</param>
        /// <param name="indentation">Number of space characters to use for indentation.
        /// By default, no spaces are used and new lines are omitted as well.</param>
        public void WriteToFile(string filename, [Optional] bool? resolveVariables,
            [Optional] int? indentation)
        {
            if (resolveVariables == null)
            {
                resolveVariables = (Flags & ConfigurationFlags.DisableVariables)
                                    != ConfigurationFlags.DisableVariables;
            }

            if (indentation == null)
            {
                indentation = -1;
            }

            int res = Api.YOGI_ConfigurationWriteToFile(Handle, filename,
                resolveVariables == true ? 1 : 0, (int)indentation);
            CheckErrorCode(res);
        }

        /// <summary>
        /// Configuration flags.
        /// </summary>
        public readonly ConfigurationFlags Flags;

        /// <summary>
        /// Serializes the configuration to a string.
        /// </summary>
        /// <returns>Configuration serialized to a string.</returns>
        public override string ToString()
        {
            return Dump();
        }

        static IntPtr Create(ConfigurationFlags flags)
        {
            var handle = new IntPtr();
            int res = Api.YOGI_ConfigurationCreate(ref handle, flags);
            CheckErrorCode(res);
            return handle;
        }
    }
}
