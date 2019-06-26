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

#include "../common.h"

#include <yogi_core.h>
#include <boost/filesystem.hpp>
#include <regex>
#include <vector>

class LoggingTest : public testing::Test {
 protected:
  virtual void SetUp() override {
    temp_dir_ = boost::filesystem::unique_path();
    boost::filesystem::create_directory(temp_dir_);
  }

  virtual void TearDown() override {
    yogi::DisableConsoleLogging();
    yogi::DisableHookLogging();
    yogi::DisableFileLogging();

    boost::filesystem::remove_all(temp_dir_);
  }

  boost::filesystem::path temp_dir_;
};

TEST_F(LoggingTest, VerbosityEnum) {
  // clang-format off
  CHECK_ENUM_ELEMENT(Verbosity, kFatal,   YOGI_VB_FATAL);
  CHECK_ENUM_ELEMENT(Verbosity, kError,   YOGI_VB_ERROR);
  CHECK_ENUM_ELEMENT(Verbosity, kWarning, YOGI_VB_WARNING);
  CHECK_ENUM_ELEMENT(Verbosity, kInfo,    YOGI_VB_INFO);
  CHECK_ENUM_ELEMENT(Verbosity, kDebug,   YOGI_VB_DEBUG);
  CHECK_ENUM_ELEMENT(Verbosity, kTrace,   YOGI_VB_TRACE);
  // clang-format on
}

TEST_F(LoggingTest, StreamEnum) {
  // clang-format off
  CHECK_ENUM_ELEMENT(Stream, kStdout, YOGI_ST_STDOUT);
  CHECK_ENUM_ELEMENT(Stream, kStderr, YOGI_ST_STDERR);
  // clang-format on
}

TEST_F(LoggingTest, SetupConsoleLogging) {
  yogi::SetupConsoleLogging(yogi::Verbosity::kInfo, yogi::Stream::kStdout,
                            true);
  yogi::app_logger->Log(yogi::Verbosity::kWarning, "Warning message");
  yogi::SetupConsoleLogging(yogi::Verbosity::kDebug, yogi::Stream::kStdout,
                            false, "%S.%3", "$t - $m");
  yogi::app_logger->Log(yogi::Verbosity::kError, "Error message");
}

TEST_F(LoggingTest, SetupHookLogging) {
  bool called = false;
  yogi::SetupHookLogging(yogi::Verbosity::kDebug,
                         [&](auto severity, auto timestamp, int tid, auto file,
                             int line, auto comp, auto msg) {
                           EXPECT_EQ(severity, yogi::Verbosity::kWarning);
                           EXPECT_LE(timestamp, yogi::GetCurrentTime());
                           EXPECT_GT(tid, 0);
                           EXPECT_EQ(file, "file.cc");
                           EXPECT_EQ(line, 123);
                           EXPECT_FALSE(comp.empty());
                           EXPECT_EQ(msg, "A warning");
                           called = true;
                         });

  yogi::app_logger->Log(yogi::Verbosity::kWarning, "A warning", "file.cc", 123);
  EXPECT_TRUE(called);

  called = false;
  yogi::SetupHookLogging(yogi::Verbosity::kDebug,
                         [&](auto, auto, int, auto file, int line, auto, auto) {
                           EXPECT_TRUE(file.empty());
                           EXPECT_EQ(line, 0);
                           called = true;
                         });

  yogi::app_logger->Log(yogi::Verbosity::kWarning, "A warning");
  EXPECT_TRUE(called);
}

TEST_F(LoggingTest, SetupFileLogging) {
  auto file_prefix = (temp_dir_ / "logfile_%Y_").string();

  auto filename =
      yogi::SetupFileLogging(yogi::Verbosity::kInfo, file_prefix + "1");
  EXPECT_EQ(filename.find("%Y"), std::string::npos);
  EXPECT_TRUE(boost::filesystem::exists(filename));

  filename = yogi::SetupFileLogging(yogi::Verbosity::kInfo, file_prefix + "2",
                                    "%S.%3", "$t - $m");
  EXPECT_EQ(filename.find("%Y"), std::string::npos);
  EXPECT_TRUE(boost::filesystem::exists(filename));
}

TEST_F(LoggingTest, SetComponentsVerbosity) {
  yogi::Logger::SetComponentsVerbosity("App", yogi::Verbosity::kDebug);
  EXPECT_EQ(yogi::Verbosity::kDebug, yogi::app_logger->GetVerbosity());
  yogi::Logger::SetComponentsVerbosity("App", yogi::Verbosity::kInfo);
  EXPECT_EQ(yogi::Verbosity::kInfo, yogi::app_logger->GetVerbosity());
}

TEST_F(LoggingTest, LoggerVerbosity) {
  auto logger = yogi::Logger::Create("My logger");
  EXPECT_EQ(logger->GetVerbosity(), yogi::Verbosity::kInfo);
  logger->SetVerbosity(yogi::Verbosity::kFatal);
  EXPECT_EQ(logger->GetVerbosity(), yogi::Verbosity::kFatal);
}

TEST_F(LoggingTest, Log) {
  auto logger = yogi::Logger::Create("My logger");

  bool called = false;
  yogi::SetupHookLogging(
      yogi::Verbosity::kDebug,
      [&](auto severity, auto, int, auto file, int line, auto comp, auto msg) {
        EXPECT_EQ(severity, yogi::Verbosity::kWarning);
        EXPECT_EQ(file, "file.cc");
        EXPECT_EQ(line, 123);
        EXPECT_EQ(comp, "My logger");
        EXPECT_EQ(msg, "Hey dude");
        called = true;
      });

  logger->Log(yogi::Verbosity::kWarning, "Hey dude", "file.cc", 123);
  EXPECT_TRUE(called);
}

TEST_F(LoggingTest, AppLogger) {
  auto logger = yogi::AppLogger::Create();
  logger->SetVerbosity(yogi::Verbosity::kWarning);
  EXPECT_EQ(logger->GetVerbosity(), yogi::app_logger->GetVerbosity());

  logger->SetVerbosity(yogi::Verbosity::kTrace);
  EXPECT_EQ(logger->GetVerbosity(), yogi::app_logger->GetVerbosity());
}

TEST_F(LoggingTest, Macros) {
  struct Entry {
    yogi::Verbosity severity;
    std::string file;
    int line;
    std::string comp;
    std::string msg;
  };
  std::vector<Entry> entries;

  yogi::SetupHookLogging(
      yogi::Verbosity::kTrace,
      [&](auto severity, auto, int, auto file, int line, auto comp, auto msg) {
        Entry entry = {severity, file, line, comp, msg};
        entries.push_back(entry);
      });

  auto logger = yogi::Logger::Create("Engine");
  logger->SetVerbosity(yogi::Verbosity::kTrace);
  yogi::app_logger->SetVerbosity(yogi::Verbosity::kTrace);

  YOGI_LOG_FATAL(logger, "a"
                             << "b");
  YOGI_LOG_FATAL(123 << 45);
  YOGI_LOG_ERROR(logger, "a"
                             << "b");
  YOGI_LOG_ERROR(123 << 45);
  YOGI_LOG_WARNING(logger, "a"
                               << "b");
  YOGI_LOG_WARNING(123 << 45);
  YOGI_LOG_INFO(logger, "a"
                            << "b");
  YOGI_LOG_INFO(123 << 45);
  YOGI_LOG_DEBUG(logger, "a"
                             << "b");
  YOGI_LOG_DEBUG(123 << 45);
  YOGI_LOG_TRACE(logger, "a"
                             << "b");
  YOGI_LOG_TRACE(123 << 45);
  YOGI_LOG(kInfo, logger,
           "a"
               << "b");
  YOGI_LOG(kInfo, 123 << 45);

  ASSERT_EQ(entries.size(), 14u);
  EXPECT_EQ(entries[0].severity, yogi::Verbosity::kFatal);
  EXPECT_EQ(entries[1].severity, yogi::Verbosity::kFatal);
  EXPECT_EQ(entries[2].severity, yogi::Verbosity::kError);
  EXPECT_EQ(entries[3].severity, yogi::Verbosity::kError);
  EXPECT_EQ(entries[4].severity, yogi::Verbosity::kWarning);
  EXPECT_EQ(entries[5].severity, yogi::Verbosity::kWarning);
  EXPECT_EQ(entries[6].severity, yogi::Verbosity::kInfo);
  EXPECT_EQ(entries[7].severity, yogi::Verbosity::kInfo);
  EXPECT_EQ(entries[8].severity, yogi::Verbosity::kDebug);
  EXPECT_EQ(entries[9].severity, yogi::Verbosity::kDebug);
  EXPECT_EQ(entries[10].severity, yogi::Verbosity::kTrace);
  EXPECT_EQ(entries[11].severity, yogi::Verbosity::kTrace);
  EXPECT_EQ(entries[12].severity, yogi::Verbosity::kInfo);
  EXPECT_EQ(entries[13].severity, yogi::Verbosity::kInfo);

  std::string file = __FILE__;
  file = file.substr(file.find_last_of("\\/") + 1);

  for (std::size_t i = 0; i < entries.size(); ++i) {
    auto entry = entries[i];
    EXPECT_EQ(entry.file, file);
    EXPECT_GT(entry.line, 0);
    EXPECT_LT(entry.line, __LINE__);
    EXPECT_EQ(entry.comp, i % 2 ? "App" : "Engine");
    EXPECT_EQ(entry.msg, i % 2 ? "12345" : "ab");
  }
}
