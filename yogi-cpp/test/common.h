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

#include <gtest/gtest.h>
#include <yogi.h>
#include <boost/filesystem.hpp>

#define CHECK_ENUM_ELEMENT(enum, element, macro)            \
  EXPECT_EQ(static_cast<int>(yogi::enum ::element), macro); \
  EXPECT_EQ(yogi::ToString(yogi::enum ::element), #element)

void CheckStringMatches(std::string s, std::string pattern);
void RunContextUntilBranchesAreConnected(
    yogi::ContextPtr context, std::initializer_list<yogi::BranchPtr> branches);
void WriteFile(const std::string& filename, const std::string& content);
std::string ReadFile(const std::string& filename);
std::ostream& operator<<(std::ostream& os, const std::chrono::nanoseconds& ns);

class TemporaryWorkdirGuard final {
 public:
  TemporaryWorkdirGuard();
  TemporaryWorkdirGuard(TemporaryWorkdirGuard&& other);
  ~TemporaryWorkdirGuard();

  TemporaryWorkdirGuard(const TemporaryWorkdirGuard&) = delete;
  TemporaryWorkdirGuard& operator=(const TemporaryWorkdirGuard&) = delete;

 private:
  boost::filesystem::path old_working_dir_;
  boost::filesystem::path temp_path_;
};

struct CommandLine final {
  CommandLine(std::initializer_list<std::string> args);
  ~CommandLine();

  CommandLine(const CommandLine&) = delete;
  CommandLine& operator=(const CommandLine&) = delete;

  int argc;
  char** argv;
};
