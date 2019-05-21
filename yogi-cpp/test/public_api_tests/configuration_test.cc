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

#include <yogi_core.h>

TEST(ConfigurationTest, CreateFromJson) {
  auto cfg = yogi::Configuration::Create("{\"age\": 42}");
  auto json = yogi::Json::parse(cfg->Dump());
  EXPECT_EQ(json.value("age", -1), 42);
}

TEST(ConfigurationTest, ConfigurationFlags) {
  // clang-format off
  CHECK_ENUM_ELEMENT(ConfigurationFlags, kNone,             YOGI_CFG_NONE);
  CHECK_ENUM_ELEMENT(ConfigurationFlags, kDisableVariables, YOGI_CFG_DISABLE_VARIABLES);
  CHECK_ENUM_ELEMENT(ConfigurationFlags, kMutableCmdLine,   YOGI_CFG_MUTABLE_CMD_LINE);
  // clang-format on

  auto flags = yogi::ConfigurationFlags::kNone;
  EXPECT_EQ(yogi::ToString(flags), "kNone");
  flags = flags | yogi::ConfigurationFlags::kDisableVariables;
  EXPECT_EQ(yogi::ToString(flags), "kDisableVariables");
  flags |= yogi::ConfigurationFlags::kMutableCmdLine;
  EXPECT_EQ(yogi::ToString(flags), "kDisableVariables | kMutableCmdLine");
}

TEST(ConfigurationTest, CommandLineOptions) {
  // clang-format off
  CHECK_ENUM_ELEMENT(CommandLineOptions, kNone,              YOGI_CLO_NONE);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kLogging,           YOGI_CLO_LOGGING);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchName,        YOGI_CLO_BRANCH_NAME);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchDescription, YOGI_CLO_BRANCH_DESCRIPTION);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchNetwork,     YOGI_CLO_BRANCH_NETWORK);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchPassword,    YOGI_CLO_BRANCH_PASSWORD);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchPath,        YOGI_CLO_BRANCH_PATH);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchAdvIfs,      YOGI_CLO_BRANCH_ADV_IFS);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchAdvAddr,     YOGI_CLO_BRANCH_ADV_ADDR);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchAdvPort,     YOGI_CLO_BRANCH_ADV_PORT);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchAdvInt,      YOGI_CLO_BRANCH_ADV_INT);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchTimeout,     YOGI_CLO_BRANCH_TIMEOUT);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchGhostMode,   YOGI_CLO_BRANCH_GHOST_MODE);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kFiles,             YOGI_CLO_FILES);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kFilesRequired,     YOGI_CLO_FILES_REQUIRED);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kOverrides,         YOGI_CLO_OVERRIDES);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kVariables,         YOGI_CLO_VARIABLES);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchAll,         YOGI_CLO_BRANCH_ALL);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kAll,               YOGI_CLO_ALL);
  // clang-format on

  auto options = yogi::CommandLineOptions::kNone;
  EXPECT_EQ(yogi::ToString(options), "kNone");

  options = options | yogi::CommandLineOptions::kLogging;
  EXPECT_EQ(yogi::ToString(options), "kLogging");

  options |= yogi::CommandLineOptions::kBranchName;
  EXPECT_EQ(yogi::ToString(options), "kLogging | kBranchName");

  options |= yogi::CommandLineOptions::kBranchDescription;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription");

  options |= yogi::CommandLineOptions::kBranchNetwork;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork");

  options |= yogi::CommandLineOptions::kBranchPassword;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword");

  options |= yogi::CommandLineOptions::kBranchPath;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath");

  options |= yogi::CommandLineOptions::kBranchAdvIfs;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath | kBranchAdvIfs");

  options |= yogi::CommandLineOptions::kBranchAdvAddr;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath | kBranchAdvIfs | kBranchAdvAddr");

  options |= yogi::CommandLineOptions::kBranchAdvPort;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath | kBranchAdvIfs | kBranchAdvAddr | "
            "kBranchAdvPort");

  options |= yogi::CommandLineOptions::kBranchAdvInt;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath | kBranchAdvIfs | kBranchAdvAddr | "
            "kBranchAdvPort | "
            "kBranchAdvInt");

  options |= yogi::CommandLineOptions::kBranchTimeout;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath | kBranchAdvIfs | kBranchAdvAddr | "
            "kBranchAdvPort | "
            "kBranchAdvInt | kBranchTimeout");

  options |= yogi::CommandLineOptions::kBranchGhostMode;
  EXPECT_EQ(yogi::ToString(options), "kLogging | kBranchAll");

  options |= yogi::CommandLineOptions::kFiles;
  EXPECT_EQ(yogi::ToString(options), "kLogging | kBranchAll | kFiles");

  options |= yogi::CommandLineOptions::kFilesRequired;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchAll | kFiles | kFilesRequired");

  options |= yogi::CommandLineOptions::kOverrides;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchAll | kFiles | kFilesRequired | kOverrides");

  options |= yogi::CommandLineOptions::kVariables;
  EXPECT_EQ(yogi::ToString(options), "kAll");
}

TEST(ConfigurationTest, GetFlags) {
  auto cfg =
      yogi::Configuration::Create(yogi::ConfigurationFlags::kMutableCmdLine);
  EXPECT_EQ(cfg->GetFlags(), yogi::ConfigurationFlags::kMutableCmdLine);
}

TEST(ConfigurationTest, UpdateFromCommandLine) {
  auto cfg = yogi::Configuration::Create();

  // clang-format off
  CommandLine cmdline{
    "-o", "{\"age\": 25}"
  };
  // clang-format on

  cfg->UpdateFromCommandLine(cmdline.argc, cmdline.argv,
                             yogi::CommandLineOptions::kOverrides);
  auto json = yogi::Json::parse(cfg->Dump());
  EXPECT_EQ(json.value("age", -1), 25);

  // clang-format off
  CommandLine cmdline2{
    "-o", "{\"age\": 18}"
  };
  // clang-format on

  cfg->UpdateFromCommandLine(cmdline2.argc, cmdline2.argv,
                             yogi::CommandLineOptions::kOverrides);
  json = yogi::Json::parse(cfg->Dump());
  EXPECT_EQ(json.value("age", -1), 18);
}

TEST(ConfigurationTest, UpdateFromJson) {
  auto cfg = yogi::Configuration::Create();

  cfg->UpdateFromJson("{\"age\": 42}");
  auto json = yogi::Json::parse(cfg->Dump());
  EXPECT_EQ(json.value("age", -1), 42);

  cfg->UpdateFromJson(yogi::Json::parse("{\"age\": 88}"));
  json = yogi::Json::parse(cfg->Dump());
  EXPECT_EQ(json.value("age", -1), 88);
}

TEST(ConfigurationTest, UpdateFromFile) {
  TemporaryWorkdirGuard workdir;
  auto filename = "cfg.json";
  WriteFile(filename, "{\"age\": 66}");

  auto cfg = yogi::Configuration::Create();

  cfg->UpdateFromFile(filename);
  auto json = yogi::Json::parse(cfg->Dump());
  EXPECT_EQ(json.value("age", -1), 66);
}

TEST(ConfigurationTest, Dump) {
  auto cfg =
      yogi::Configuration::Create(yogi::ConfigurationFlags::kDisableVariables);
  cfg->UpdateFromJson("{\"age\": 42}");

  EXPECT_THROW(cfg->Dump(true), yogi::FailureException);

  EXPECT_EQ(cfg->Dump().find(" "), std::string::npos);
  EXPECT_EQ(cfg->Dump().find("\n"), std::string::npos);
  EXPECT_NE(cfg->Dump(2).find(" "), std::string::npos);
  EXPECT_NE(cfg->Dump(2).find("\n"), std::string::npos);
}

TEST(ConfigurationTest, WriteToFile) {
  TemporaryWorkdirGuard workdir;
  auto filename = "dump.json";

  auto cfg =
      yogi::Configuration::Create(yogi::ConfigurationFlags::kDisableVariables);
  cfg->UpdateFromJson("{\"age\": 11}");

  EXPECT_THROW(cfg->WriteToFile(filename, true), yogi::FailureException);

  cfg->WriteToFile(filename);
  auto content = ReadFile(filename);
  EXPECT_EQ(content.find(" "), std::string::npos);
  EXPECT_EQ(content.find("\n"), std::string::npos);
  auto json = yogi::Json::parse(content);
  EXPECT_EQ(json.value("age", -1), 11);

  cfg->WriteToFile(filename, 2);
  content = ReadFile(filename);
  EXPECT_NE(content.find(" "), std::string::npos);
  EXPECT_NE(content.find("\n"), std::string::npos);
  json = yogi::Json::parse(content);
  EXPECT_EQ(json.value("age", -1), 11);
}
