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

#include "common.h"

#include <chrono>
#include <regex>
#include <sstream>
#include <fstream>

using namespace std::chrono_literals;
using namespace std::string_literals;

namespace fs = boost::filesystem;

void CheckStringMatches(std::string s, std::string pattern) {
  std::regex re(pattern);
  std::smatch m;
  EXPECT_TRUE(std::regex_match(s, m, re))
      << "String: " << s << " Pattern: " << pattern;
}

void RunContextUntilBranchesAreConnected(
    yogi::ContextPtr context, std::initializer_list<yogi::BranchPtr> branches) {
  std::map<yogi::BranchPtr, std::set<yogi::Uuid>> uuids;
  for (auto branch : branches) {
    uuids[branch] = {};
  }

  for (auto& entry : uuids) {
    for (auto branch : branches) {
      if (branch != entry.first) {
        entry.second.insert(branch->GetUuid());
      }
    }
  }

  auto start = std::chrono::steady_clock::now();

  while (!uuids.empty()) {
    context->Poll();

    auto entry = uuids.begin();
    auto infos = entry->first->GetConnectedBranches();
    for (auto& info : infos) {
      auto uuid = info.first;
      entry->second.erase(uuid);
    }

    if (entry->second.empty()) {
      uuids.erase(entry);
    }

    if (std::chrono::steady_clock::now() - start > 3s) {
      throw std::runtime_error("Branches did not connect");
    }
  }
}

void WriteFile(const std::string& filename, const std::string& content) {
  std::ofstream file(filename);
  file << content;
}

std::string ReadFile(const std::string& filename) {
  std::ifstream f(filename);
  EXPECT_TRUE(f.is_open()) << filename;
  std::string content((std::istreambuf_iterator<char>(f)),
                      (std::istreambuf_iterator<char>()));
  return content;
}

std::ostream& operator<<(std::ostream& os, const std::chrono::nanoseconds& ns) {
  return os << ns.count() << "ns";
}

TemporaryWorkdirGuard::TemporaryWorkdirGuard() {
  temp_path_ = fs::temp_directory_path() / fs::unique_path();
  fs::create_directory(temp_path_);
  old_working_dir_ = fs::current_path();
  fs::current_path(temp_path_);
}

TemporaryWorkdirGuard::TemporaryWorkdirGuard(TemporaryWorkdirGuard&& other) {
  old_working_dir_ = other.old_working_dir_;
  temp_path_ = other.temp_path_;
  other.temp_path_.clear();
}

TemporaryWorkdirGuard::~TemporaryWorkdirGuard() {
  if (!temp_path_.empty()) {
    fs::current_path(old_working_dir_);
    fs::remove_all(temp_path_);
  }
}

CommandLine::CommandLine(std::initializer_list<std::string> args) {
  argc = static_cast<int>(args.size() + 1);
  argv = new char*[static_cast<std::size_t>(argc)];

  std::string exe = "executable-name";
  argv[0] = new char[exe.size() + 1];
  std::memcpy(argv[0], exe.c_str(), exe.size() + 1);

  auto it = args.begin();
  for (int i = 1; i < argc; ++i) {
    auto& arg = *it;
    argv[i] = new char[arg.size() + 1];
    std::memcpy(argv[i], arg.c_str(), arg.size() + 1);
    ++it;
  }
}

CommandLine::~CommandLine() {
  for (int i = 0; i < argc; ++i) {
    delete[] argv[i];
  }

  delete[] argv;
}
