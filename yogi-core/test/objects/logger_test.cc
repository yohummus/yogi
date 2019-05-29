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
#include "../../src/utils/system.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <time.h>
#include <regex>

class LoggerTest : public TestFixture {
 protected:
  struct Entry {
    int severity;
    long long timestamp;
    int tid;
    std::string file;
    int line;
    std::string component;
    std::string msg;
  };

  static void Hook(int severity, long long timestamp, int tid, const char* file,
                   int line, const char* component, const char* msg,
                   void* userarg) {
    auto& entries = *static_cast<std::vector<Entry>*>(userarg);
    entries.push_back({severity, timestamp, tid, file, line, component, msg});
  }

  virtual void SetUp() override {
    logger_ = nullptr;
    int res = YOGI_LoggerCreate(&logger_, "My.Component");
    ASSERT_OK(res);
    ASSERT_NE(logger_, nullptr);

    res =
        YOGI_ConfigureHookLogging(YOGI_VB_TRACE, &LoggerTest::Hook, &entries_);
    ASSERT_OK(res);
  }

  bool CheckLineMatchesCustomLogFormat(std::string line) {
    std::string re = "^__####_##_##_####-##-##_##_##_##_##:##:##_###_###_###__";
    boost::replace_all(re, "#", "\\d");
    re += std::to_string(utils::GetProcessId()) + "_";
    re += std::to_string(utils::GetCurrentThreadId()) + "_";
    re += "ERR_Hello_myfile\\.cc_123_My\\.Component___\\$_";
    re += "$";

    std::smatch m;

    line = line.substr(0, line.find_first_of("\r\n"));
    return std::regex_match(line, m, std::regex(re));
  }

  void* logger_;
  std::vector<Entry> entries_;
  static constexpr const char* custom_time_fmt_ =
      "_%Y_%m_%d_%F_%H_%M_%S_%T_%3_%6_%9_";
  static constexpr const char* custom_fmt_ =
      "_$t_$P_$T_$s_$m_$f_$l_$c_$<_$>_$$_";
};

TEST_F(LoggerTest, Colours) {
  YOGI_LoggerSetVerbosity(logger_, YOGI_VB_TRACE);

  int streams[] = {YOGI_ST_STDOUT, YOGI_ST_STDERR};
  for (int stream : streams) {
    YOGI_ConfigureConsoleLogging(YOGI_VB_TRACE, stream, YOGI_TRUE, nullptr,
                                 nullptr);

    YOGI_LoggerLog(logger_, YOGI_VB_FATAL, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_WARNING, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_INFO, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_DEBUG, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_TRACE, "myfile.cc", 123, "Hello");
  }
}

TEST_F(LoggerTest, LogToConsole) {
  // Testing the actual output is too hard because yogi_core statically links
  // to the runtime which makes it impossible to capture STDOUT/STDERR easily.

  // Test stdout
  int res = YOGI_ConfigureConsoleLogging(
      YOGI_VB_TRACE, YOGI_ST_STDOUT, YOGI_FALSE, custom_time_fmt_, custom_fmt_);
  ASSERT_OK(res);

  YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");

  // Test stderr
  res = YOGI_ConfigureConsoleLogging(YOGI_VB_TRACE, YOGI_ST_STDERR, YOGI_FALSE,
                                     custom_time_fmt_, custom_fmt_);
  ASSERT_OK(res);

  YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");
}

TEST_F(LoggerTest, FormatErrors) {
  for (auto time_fmt : {"%4", "%%", "", "%%T", "%", "bla%"}) {
    int res = YOGI_ConfigureConsoleLogging(YOGI_VB_TRACE, YOGI_ST_STDOUT,
                                           YOGI_FALSE, time_fmt, nullptr);
    EXPECT_ERR(res, YOGI_ERR_INVALID_PARAM) << time_fmt;

    res = YOGI_ConfigureFileLogging(YOGI_VB_TRACE, "logfile.txt", nullptr, 0,
                                    time_fmt, nullptr);
    EXPECT_ERR(res, YOGI_ERR_INVALID_PARAM) << time_fmt;

    res = YOGI_ConfigureFileLogging(YOGI_VB_TRACE, time_fmt, nullptr, 0,
                                    nullptr, nullptr);
    EXPECT_ERR(res, YOGI_ERR_INVALID_PARAM) << time_fmt;
  }

  for (auto fmt : {"$;", "", "$$$", "$", "bla$"}) {
    int res = YOGI_ConfigureConsoleLogging(YOGI_VB_TRACE, YOGI_ST_STDOUT,
                                           YOGI_FALSE, nullptr, fmt);
    EXPECT_ERR(res, YOGI_ERR_INVALID_PARAM) << fmt;

    res = YOGI_ConfigureFileLogging(YOGI_VB_TRACE, "logfile.txt", nullptr, 0,
                                    nullptr, fmt);
    EXPECT_ERR(res, YOGI_ERR_INVALID_PARAM) << fmt;
  }
}

TEST_F(LoggerTest, LogToHook) {
  YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");
  ASSERT_FALSE(entries_.empty());

  auto& entry = entries_.back();
  EXPECT_EQ(entry.severity, YOGI_VB_ERROR);
  EXPECT_GT(entry.timestamp, time(nullptr) * 999999999LL);
  EXPECT_EQ(entry.tid, utils::GetCurrentThreadId());
  EXPECT_EQ(entry.file, "myfile.cc");
  EXPECT_EQ(entry.line, 123);
  EXPECT_EQ(entry.component, "My.Component");
  EXPECT_EQ(entry.msg, "Hello");
}

TEST_F(LoggerTest, LogToFile) {
  char filename[100];
  int res = YOGI_ConfigureFileLogging(YOGI_VB_TRACE, "%F_%H%M%S.log", filename,
                                      sizeof(filename), custom_time_fmt_,
                                      custom_fmt_);
  ASSERT_OK(res);
  ASSERT_TRUE(boost::filesystem::exists(filename));

  YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");

  // Check file content
  std::ifstream file(filename);
  std::string content((std::istreambuf_iterator<char>(file)),
                      (std::istreambuf_iterator<char>()));
  file.close();
  EXPECT_TRUE(CheckLineMatchesCustomLogFormat(content)) << content;

  // Close the logfile
  res = YOGI_ConfigureFileLogging(YOGI_VB_NONE, nullptr, nullptr, 0, nullptr,
                                  nullptr);
  EXPECT_OK(res);

  boost::filesystem::remove(filename);
}

TEST_F(LoggerTest, GetAndSetVerbosity) {
  int verbosity = -1;
  int res = YOGI_LoggerGetVerbosity(logger_, &verbosity);
  EXPECT_OK(res);
  EXPECT_NE(verbosity, YOGI_VB_ERROR);

  res = YOGI_LoggerSetVerbosity(logger_, YOGI_VB_ERROR);
  EXPECT_OK(res);

  res = YOGI_LoggerGetVerbosity(logger_, &verbosity);
  EXPECT_OK(res);
  EXPECT_EQ(verbosity, YOGI_VB_ERROR);

  YOGI_LoggerLog(logger_, YOGI_VB_FATAL, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_WARNING, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_INFO, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_DEBUG, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_TRACE, "myfile.cc", 123, "Hello");

  ASSERT_EQ(entries_.size(), 2);
  EXPECT_EQ(entries_[0].severity, YOGI_VB_FATAL);
  EXPECT_EQ(entries_[1].severity, YOGI_VB_ERROR);
}

TEST_F(LoggerTest, SetComponentsVerbosity) {
  void* logger2;
  int res = YOGI_LoggerCreate(&logger2, "Another Logger");
  ASSERT_OK(res);

  YOGI_LoggerSetVerbosity(logger_, YOGI_VB_INFO);
  YOGI_LoggerSetVerbosity(logger2, YOGI_VB_INFO);

  int count = -1;
  res = YOGI_LoggerSetComponentsVerbosity("My\\..*", YOGI_VB_ERROR, &count);
  EXPECT_OK(res);
  EXPECT_EQ(count, 1);

  YOGI_LoggerLog(logger_, YOGI_VB_INFO, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger2, YOGI_VB_INFO, "myfile.cc", 123, "Hello");

  ASSERT_EQ(entries_.size(), 1);
  EXPECT_EQ(entries_.back().component, "Another Logger");
}

TEST_F(LoggerTest, SetComponentsVerbosityAppLogger) {
  int count = -1;
  int res = YOGI_LoggerSetComponentsVerbosity("App", YOGI_VB_ERROR, &count);
  EXPECT_OK(res);
  EXPECT_EQ(count, 1);
}

TEST_F(LoggerTest, SetComponentsVerbosityInternalLoggers) {
  void* context;
  int res = YOGI_ContextCreate(&context);
  ASSERT_OK(res);

  int count = -1;
  res = YOGI_LoggerSetComponentsVerbosity("Yogi\\..*", YOGI_VB_ERROR, &count);
  EXPECT_OK(res);
  EXPECT_GT(count, 0);
}

TEST_F(LoggerTest, Log) {
  int res = YOGI_LoggerLog(logger_, YOGI_VB_FATAL, "myfile.cc", 123, "Hello");
  EXPECT_OK(res);

  res = YOGI_LoggerLog(nullptr, YOGI_VB_TRACE, "myfile.cc", 123, "Hello");
  EXPECT_OK(res);
}

TEST_F(LoggerTest, DefaultLoggerVerbosity) {
  YOGI_LoggerLog(logger_, YOGI_VB_INFO, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_DEBUG, "myfile.cc", 123, "Hello");

  ASSERT_EQ(entries_.size(), 1);
  EXPECT_EQ(entries_.back().severity, YOGI_VB_INFO);
}

TEST_F(LoggerTest, AppLogger) {
  YOGI_LoggerLog(nullptr, YOGI_VB_FATAL, "myfile.cc", 123, "Hello");
  ASSERT_FALSE(entries_.empty());

  auto& entry = entries_.back();
  EXPECT_EQ(entry.component, "App");
}
