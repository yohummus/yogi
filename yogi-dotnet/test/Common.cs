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
using System.IO;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using Xunit;

namespace test
{
    public class TestCase : IDisposable
    {
        static string yogiCoreH;

        static TestCase()
        {
            string filename = new System.Diagnostics.StackTrace(true).GetFrame(0).GetFileName();
            string path = Path.Combine(new FileInfo(filename).Directory.ToString(),
                "../../yogi-core/include/yogi_core.h");
            yogiCoreH = File.ReadAllText(path);
        }

        public static string GetCoreMacroString(string macroName)
        {
            var regex = new Regex($"#define {macroName} ([^ \\s]*).*");
            Match m = regex.Match(yogiCoreH);
            if (!m.Success)
            {
                throw new IOException($"Macro {macroName} not found in yogi_core.h");
            }

            if (m.Groups[1].Value.StartsWith('('))
            {
                regex = new Regex($"#define {macroName} (\\(.+?\\)).*");
                m = regex.Match(yogiCoreH);
                if (!m.Success)
                {
                    throw new IOException($"Cannot parse macro {macroName} in yogi_core.h");
                }
            }

            var str = m.Groups[1].Value;
            str = str.Trim();
            if (str.StartsWith('"'))
            {
                str = str.Substring(1, str.Length - 2);
            }

            return str;
        }

        public static int GetCoreMacroInt(string macroName)
        {
            var str = GetCoreMacroString(macroName);
            var regex = new Regex("\\( *1 *<< *(\\d+) *\\)");
            Match m = regex.Match(str);
            if (m.Success)
            {
                var n = int.Parse(m.Groups[1].Value);
                return 1 << n;
            }

            return int.Parse(str);
        }

        public static IEnumerable<T> GetEnumElements<T>()
        {
            var type = typeof(T);
            return from name in Enum.GetNames(type) select (T)Enum.Parse(type, name);
        }

        public static void AssertEnumElementMatches<T>(string macroPrefix, T enumElement)
        {
            var enumElementName = Enum.GetName(typeof(T), enumElement);
            var macroName = macroPrefix + Regex.Replace(enumElementName,
                "(?<=.)([A-Z])", "_$0").ToUpper();
            var macroVal = GetCoreMacroInt(macroName);
            Assert.Equal((int)(object)enumElement, macroVal);
        }

        public static void AssertEnumMatches<T>(string macroPrefix)
        {
            foreach (var elem in GetEnumElements<T>())
            {
                AssertEnumElementMatches<T>(macroPrefix, elem);
            }
        }

        public static void AssertFlagMatches<T>(string macroPrefix, T enumElement)
        {
            AssertEnumElementMatches<T>(macroPrefix, enumElement);
        }

        public static void AssertFlagCombinationMatches<T>(string macroPrefix, T enumElement,
                                                           [Optional] IEnumerable<T> exceptions)
        {
            var enumElementName = Enum.GetName(typeof(T), enumElement);
            var macroName = macroPrefix + Regex.Replace(enumElementName,
                "(?<=.)([A-Z])", "_$0").ToUpper();
            GetCoreMacroString(macroName);

            if (exceptions == null) exceptions = new List<T>();
            exceptions.Append(enumElement);

            var expected = 0;
            foreach (var elem in GetEnumElements<T>())
            {
                if (!exceptions.Contains(elem))
                {
                    expected |= (int)(object)elem;
                }
            }

            Assert.Equal(expected, (int)(object)enumElement);
        }

        public static void RunContextUntilBranchesAreConnected(Yogi.Context context,
                                                               params Yogi.Branch[] branches)
        {
            var uuids = new Dictionary<Yogi.Branch, List<Guid>>();
            foreach (var branch in branches)
            {
                uuids[branch] = new List<Guid>();
            }

            foreach (var entry in uuids)
            {
                foreach (var branch in branches)
                {
                    if (branch != entry.Key)
                    {
                        entry.Value.Add(branch.Uuid);
                    }
                }
            }

            var start = DateTime.Now;

            while (uuids.Count > 0)
            {
                context.Poll();

                var entry = uuids.First();
                var infos = entry.Key.GetConnectedBranches();
                foreach (var info in infos)
                {
                    var uuid = info.Key;
                    entry.Value.Remove(uuid);
                }

                if (entry.Value.Count == 0)
                {
                    uuids.Remove(entry.Key);
                }

                if (DateTime.Now - start > TimeSpan.FromSeconds(3))
                {
                    throw new Exception("Branches did not connect");
                }
            }
        }

        public void Dispose()
        {
            Yogi.DisableConsoleLogging();
            Yogi.DisableHookLogging();
            Yogi.DisableFileLogging();

            GC.Collect();
        }
    }
}
