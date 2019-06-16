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
using System.IO;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Xunit;

namespace test
{
    public class ConfigurationTest : TestCase
    {
        public ConfigurationTest()
        {
            tempDir = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            Directory.CreateDirectory(tempDir);
        }

#pragma warning disable xUnit1013
        public new void Dispose()
        {
            Directory.Delete(tempDir, true);
            base.Dispose();
        }
#pragma warning restore xUnit1013

        string tempDir;

        [Fact]
        public void ConfigurationFlagsEnum()
        {
            AssertEnumMatches<Yogi.ConfigurationFlags>("YOGI_CFG_");
        }

        [Fact]
        public void CommandLineOptionsEnum()
        {
            foreach (var elem in GetEnumElements<Yogi.CommandLineOptions>())
            {
                if (elem == Yogi.CommandLineOptions.BranchAll)
                {
                    var exceptions = new List<Yogi.CommandLineOptions>();
                    foreach (var x in GetEnumElements<Yogi.CommandLineOptions>())
                    {
                        if (!x.ToString().StartsWith("Branch"))
                        {
                            exceptions.Add(x);
                        }
                    }

                    AssertFlagCombinationMatches("YOGI_CLO_", elem, exceptions);
                }
                else if (elem == Yogi.CommandLineOptions.All)
                {
                    AssertFlagCombinationMatches("YOGI_CLO_", elem);
                }
                else
                {
                    AssertFlagMatches("YOGI_CLO_", elem);
                }
            }
        }

        [Fact]
        public void CreateFromJson()
        {
            var cfg = new Yogi.Configuration("{\"age\": 42}");
            Assert.Equal(42, (int)cfg.ToJson()["age"]);
        }

        [Fact]
        public void FlagsProperty()
        {
            var cfg = new Yogi.Configuration(Yogi.ConfigurationFlags.MutableCmdLine);
            Assert.Equal(Yogi.ConfigurationFlags.MutableCmdLine, cfg.Flags);
        }

        [Fact]
        public void UpdateFromCommandLine()
        {
            var cfg = new Yogi.Configuration();

            cfg.UpdateFromCommandLine(new[] { "exe", "-o", "{\"age\": 25}" },
                Yogi.CommandLineOptions.Overrides);
            Assert.Equal(25, (int)cfg.ToJson()["age"]);

            cfg.UpdateFromCommandLine(new List<string> { "exe", "-o", "{\"age\": 18}" },
                Yogi.CommandLineOptions.Overrides);
            Assert.Equal(18, (int)cfg.ToJson()["age"]);
        }

        [Fact]
        public void UpdateFromJson()
        {
            var cfg = new Yogi.Configuration();
            cfg.UpdateFromJson("{\"age\": 42}");
            Assert.Equal(42, (int)cfg.ToJson()["age"]);
            cfg.UpdateFromJson(JObject.Parse("{\"age\": 88}"));
            Assert.Equal(88, (int)cfg.ToJson()["age"]);
        }

        [Fact]
        public void UpdateFromFile()
        {
            var filename = Path.Combine(tempDir, "cfg.json");
            File.WriteAllText(filename, "{\"age\": 66}");

            var cfg = new Yogi.Configuration();
            cfg.UpdateFromFile(filename);

            Assert.Equal(66, (int)cfg.ToJson()["age"]);
        }

        [Fact]
        public void Dump()
        {
            var cfg = new Yogi.Configuration(Yogi.ConfigurationFlags.DisableVariables);
            cfg.UpdateFromJson("{\"age\": 42}");

            Assert.Throws<Yogi.FailureException>(() => cfg.Dump(true));

            Assert.DoesNotContain(" ", cfg.Dump());
            Assert.DoesNotContain("\n", cfg.Dump());
            Assert.Contains(" ", cfg.Dump(indentation: 2));
            Assert.Contains("\n", cfg.Dump(indentation: 2));
        }

        [Fact]
        public void ToStringMethod()
        {
            var cfg = new Yogi.Configuration();
            cfg.UpdateFromJson("{\"age\": 42}");
            Assert.Equal(cfg.Dump(), cfg.ToString());
        }

        [Fact]
        public void ToJson()
        {
            var cfg = new Yogi.Configuration(Yogi.ConfigurationFlags.DisableVariables);
            cfg.UpdateFromJson("{\"age\": 42}");

            Assert.Throws<Yogi.FailureException>(() => cfg.ToJson(true));

            Assert.Equal(42, (int)cfg.ToJson()["age"]);
        }

        [Fact]
        public void WriteToFile()
        {
            var filename = Path.Combine(tempDir, "dump.json");

            var cfg = new Yogi.Configuration(Yogi.ConfigurationFlags.DisableVariables);
            cfg.UpdateFromJson("{\"age\": 11}");

            Assert.Throws<Yogi.FailureException>(() => cfg.WriteToFile(filename, true));

            cfg.WriteToFile(filename);
            var content = File.ReadAllText(filename);
            Assert.DoesNotContain(" ", content);
            Assert.DoesNotContain("\n", content);
            Assert.Equal(11, (int)JObject.Parse(content)["age"]);

            cfg.WriteToFile(filename, indentation: 2);
            content = File.ReadAllText(filename);
            Assert.Contains(" ", content);
            Assert.Contains("\n", content);
            Assert.Equal(11, (int)JObject.Parse(content)["age"]);
        }

        [Fact]
        public void Validate()
        {
            var cfg = new Yogi.Configuration(@"
                {
                  'person': {
                    'name': 'Joe',
                    'age': 42
                  }
                }
                ".Replace('\'', '"'));


            var scm = new Yogi.Configuration(@"
                {
                  '$schema': 'http://json-schema.org/draft-07/schema#',
                  'title': 'Test schema',
                  'properties': {
                    'name': {
                      'description': 'Name',
                      'type': 'string'
                    },
                    'age': {
                      'description': 'Age of the person',
                      'type': 'number',
                      'minimum': 2,
                      'maximum': 200
                    }
                  },
                  'required': ['name', 'age'],
                  'type': 'object'
                }
            ".Replace('\'', '"'));

            cfg.Validate("/person", scm);

            try
            {
                cfg.Validate(scm);
                Assert.True(false);  // Should not get here since Validate() should throw
            }
            catch (Yogi.DescriptiveFailureException e)
            {
                Assert.Equal(Yogi.ErrorCode.ConfigurationValidationFailed, e.Failure.ErrorCode);
                Assert.Contains("not found", e.ToString());
            }
        }
    }
}
