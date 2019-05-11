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

#include "../common.h"
#include "../../src/objects/detail/command_line_parser.h"

using objects::detail::CommandLineParser;
namespace fs = boost::filesystem;

class CommandLineParserTest : public TestFixture {
 protected:
  nlohmann::json CheckParsingSucceeds(const CommandLine& cmdline,
                                      api::CommandLineOptions options,
                                      const char* section_name) {
    CommandLineParser parser(cmdline.argc, cmdline.argv, options);
    EXPECT_NO_THROW(parser.Parse());

    auto section = parser.GetDirectConfiguration()[section_name];
    EXPECT_FALSE(section.empty());

    return section;
  }

  void CheckParsingFailsWithNoOptions(const CommandLine& cmdline) {
    CommandLineParser parser(cmdline.argc, cmdline.argv, api::kNoOptions);
    EXPECT_THROW_DESCRIPTIVE_ERROR(parser.Parse(),
                                   YOGI_ERR_PARSING_CMDLINE_FAILED);
  }

  template <typename T>
  void CheckParsingSingleValue(const char* cmdline_option, T val,
                               api::CommandLineOptions options,
                               const char* section_name, const char* key) {
    CommandLine cmdline{
        cmdline_option,
        std::to_string(val),
    };

    auto section = CheckParsingSucceeds(cmdline, options, section_name);
    ASSERT_TRUE(!!section.count(key));
    EXPECT_FLOAT_EQ(static_cast<float>(section[key].get<T>()),
                    static_cast<float>(val));

    CheckParsingFailsWithNoOptions(cmdline);
  }

  void CheckParsingSingleValue(const char* cmdline_option, const char* val,
                               api::CommandLineOptions options,
                               const char* section_name, const char* key) {
    CommandLine cmdline{
        cmdline_option,
        val,
    };

    auto section = CheckParsingSucceeds(cmdline, options, section_name);
    ASSERT_TRUE(!!section.count(key));
    EXPECT_EQ(section[key].get<std::string>(), val);

    CheckParsingFailsWithNoOptions(cmdline);
  }
};

TEST_F(CommandLineParserTest, HelpOption) {
  // clang-format off
  CommandLine cmdline{
    "--help",
  };
  // clang-format on

  CommandLineParser parser(cmdline.argc, cmdline.argv, api::kNoOptions);

  EXPECT_THROW_DESCRIPTIVE_ERROR(parser.Parse(), YOGI_ERR_HELP_REQUESTED);
}

TEST_F(CommandLineParserTest, HelpLoggingOption) {
  // clang-format off
  CommandLine cmdline{
    "--help-logging",
  };
  // clang-format on

  CommandLineParser parser(cmdline.argc, cmdline.argv, api::kLoggingOptions);

  EXPECT_THROW_DESCRIPTIVE_ERROR(parser.Parse(), YOGI_ERR_HELP_REQUESTED);

  CheckParsingFailsWithNoOptions(cmdline);
}

TEST_F(CommandLineParserTest, LoggingOptions) {
  // clang-format off
  CommandLine cmdline{
    "--log-file", "/tmp/logfile.txt",
    "--log-console=STDOUT",
    "--log-color",
    "--log-fmt", "some entry format",
    "--log-time-fmt", "some time format",
    "--log-verbosity", "Yogi.*=WARNING",
    "--log-verbosity=App=DEBUG",
  };
  // clang-format on

  auto section = CheckParsingSucceeds(cmdline, api::kLoggingOptions, "logging");

  EXPECT_EQ(section.value("file", "NOT FOUND"), "/tmp/logfile.txt");
  EXPECT_EQ(section.value("console", "NOT FOUND"), "STDOUT");
  EXPECT_EQ(section.value("color", false), true);
  EXPECT_EQ(section.value("entry-format", "NOT FOUND"), "some entry format");
  EXPECT_EQ(section.value("time-format", "NOT FOUND"), "some time format");

  auto verbosities = section["verbosity"];
  EXPECT_FALSE(verbosities.empty());

  EXPECT_EQ(verbosities.value("Yogi.*", "NOT FOUND"), "WARNING");
  EXPECT_EQ(verbosities.value("App", "NOT FOUND"), "DEBUG");

  CheckParsingFailsWithNoOptions(cmdline);
}

TEST_F(CommandLineParserTest, BranchNameOption) {
  CheckParsingSingleValue("--name", "My Branch", api::kBranchNameOption,
                          "branch", "name");
}

TEST_F(CommandLineParserTest, BranchDescriptionOption) {
  CheckParsingSingleValue("--description", "Some text...",
                          api::kBranchDescriptionOption, "branch",
                          "description");
}

TEST_F(CommandLineParserTest, BranchNetworkOption) {
  CheckParsingSingleValue("--network", "General", api::kBranchNetworkOption,
                          "branch", "network");
}

TEST_F(CommandLineParserTest, BranchPasswordOption) {
  CheckParsingSingleValue("--password", "Secret", api::kBranchPasswordOption,
                          "branch", "password");
}

TEST_F(CommandLineParserTest, BranchPathOption) {
  CheckParsingSingleValue("--path", "/some/path", api::kBranchPathOption,
                          "branch", "path");
}

TEST_F(CommandLineParserTest, BranchAdvertisingInterfacesOption) {
  // clang-format off
  CommandLine cmdline{
    "--adv-ifs", "localhost",
    "--adv-ifs", "wlan0",
  };
  // clang-format on

  auto section = CheckParsingSucceeds(cmdline, api::kBranchAdvInterfacesOption,
                                      "branch")["advertising_interfaces"];

  EXPECT_EQ(section[0].get<std::string>(), "localhost");
  EXPECT_EQ(section[1].get<std::string>(), "wlan0");
}

TEST_F(CommandLineParserTest, BranchAdvertisingAddressOption) {
  CheckParsingSingleValue("--adv-addr", "0::0", api::kBranchAdvAddressOption,
                          "branch", "advertising_address");
}

TEST_F(CommandLineParserTest, BranchAdvertisingPortOption) {
  CheckParsingSingleValue("--adv-port", 12345, api::kBranchAdvPortOption,
                          "branch", "advertising_port");
}

TEST_F(CommandLineParserTest, BranchAdvertisingIntervalOption) {
  CheckParsingSingleValue("--adv-int", 3.5, api::kBranchAdvIntervalOption,
                          "branch", "advertising_interval");
}

TEST_F(CommandLineParserTest, BranchTimeoutOption) {
  CheckParsingSingleValue("--timeout", 6.6, api::kBranchTimeoutOption, "branch",
                          "timeout");
}

TEST_F(CommandLineParserTest, BranchGhostModeOption) {
  CheckParsingSingleValue("--ghost", true, api::kBranchGhostModeOption,
                          "branch", "ghost_mode");
  CheckParsingSingleValue("--ghost", false, api::kBranchGhostModeOption,
                          "branch", "ghost_mode");
}

TEST_F(CommandLineParserTest, FileOption) {
  TemporaryWorkdirGuard workdir;
  {
    fs::ofstream file("a.json");
    file << "{\"person\": {\"name\": \"Joe\", \"age\": 42}}";
  }
  {
    fs::ofstream file("bcde.json");
    file << "{\"person\": {\"age\": 88}}";
  }

  // clang-format off
  CommandLine cmdline{
    "a.json",
    "bc*.json",
  };
  // clang-format on

  CommandLineParser parser(cmdline.argc, cmdline.argv, api::kFileOption);
  EXPECT_NO_THROW(parser.Parse());

  auto section = parser.GetFilesConfiguration()["person"];
  EXPECT_FALSE(section.empty());

  EXPECT_EQ(section.value("name", "NOT FOUND"), "Joe");
  EXPECT_EQ(section.value("age", -1), 88);

  CheckParsingFailsWithNoOptions(cmdline);
}

TEST_F(CommandLineParserTest, FileOptionCorruptFile) {
  TemporaryWorkdirGuard workdir;
  {
    fs::ofstream file("bad.json");
    file << "{\"person\": {\"age\": 44}";
  }

  // clang-format off
  CommandLine cmdline{
    "bad.json",
  };
  // clang-format on

  CommandLineParser parser(cmdline.argc, cmdline.argv, api::kFileOption);
  EXPECT_THROW_DESCRIPTIVE_ERROR(parser.Parse(), YOGI_ERR_PARSING_FILE_FAILED);
}

TEST_F(CommandLineParserTest, OverrideOption) {
  // clang-format off
  CommandLine cmdline{
    "--override", "{\"person\":{\"age\":42}}",
    "--o={\"person\":{\"name\":\"Joe\"}}",
    "-o", "/person/name=Marc",
  };
  // clang-format on

  auto section = CheckParsingSucceeds(cmdline, api::kOverrideOption, "person");

  EXPECT_EQ(section.value("age", -1), 42);
  EXPECT_EQ(section.value("name", "NOT FOUND"), "Marc");

  CheckParsingFailsWithNoOptions(cmdline);
}

TEST_F(CommandLineParserTest, VariableOptions) {
  // clang-format off
  CommandLine cmdline{
    "--var", "DIR=\"/usr/local\"",
    "--var=NAME=Yohummus",
    "--v=NUM=55",
    "--v", "PI=3.14",
  };
  // clang-format on

  auto section =
      CheckParsingSucceeds(cmdline, api::kVariableOption, "variables");

  EXPECT_EQ(section.value("DIR", "NOT FOUND"), "/usr/local");
  EXPECT_EQ(section.value("NAME", "NOT FOUND"), "Yohummus");
  EXPECT_EQ(section.value("NUM", -1), 55);
  EXPECT_FLOAT_EQ(section.value("PI", -1.0f), 3.14f);

  CheckParsingFailsWithNoOptions(cmdline);
}
