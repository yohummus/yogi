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
#include "../../src/utils/glob.h"
#include "../../src/utils/algorithm.h"

#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;

struct GlobTest : public TestFixture {
 protected:
  virtual void SetUp() override {
    fs::create_directory("test");
    std::ofstream("test/a.json");
    std::ofstream("test/b.json");
    fs::create_directory("test/config");
    std::ofstream("test/config/common.json");
    std::ofstream("test/config/utils.json");
    std::ofstream("test/config/utils.ini");
    fs::create_directory("test/config/old");
    std::ofstream("test/config/old/one.json");
    std::ofstream("test/config/old/two.json");
    std::ofstream("test/config/old/three.ini");
    fs::create_directory("test/config/new");
    std::ofstream("test/config/new/all.json");
    fs::create_directory("test/stuff");
    std::ofstream("test/stuff/more.ini");
    std::ofstream("test/stuff/even_more.json");

    fs::current_path("test/config");
  }

  void check(std::vector<std::string> patterns,
             std::vector<std::string> expectedFilenames) {
    auto filenames = utils::Glob(patterns);
    EXPECT_EQ(expectedFilenames.size(), filenames.size());

    for (auto& filename : expectedFilenames) {
      bool found = utils::contains_if(
          filenames, [&](auto& s) { return fs::equivalent(filename, s); });
      EXPECT_TRUE(found) << "Filename '" << filename
                         << "' expected but not found by glob()";
    }

    for (auto& filename : filenames) {
      bool found = utils::contains_if(expectedFilenames, [&](auto& s) {
        return fs::equivalent(filename, s);
      });
      EXPECT_TRUE(found) << "Filename '" << filename
                         << "' unexpectedly found by glob()";
    }
  }

  TemporaryWorkdirGuard workdir;
};

TEST_F(GlobTest, ExplicitFilenames) {
  std::vector<std::string> filenames = {
      "old/one.json",
      "./old/three.ini",
      "../config/common.json",
      "../stuff/more.ini",
      ".././../test/./stuff/../config/utils.json",
      "../../test/./stuff/../config/utils.ini"};

  check(filenames, filenames);
}

TEST_F(GlobTest, FilesInWorkingDirectory) {
  check({"*"}, {"common.json", "utils.json", "utils.ini"});

  check({"*.js*"}, {"common.json", "utils.json"});
}

TEST_F(GlobTest, FilesInParentDirectory) {
  check({"../*"}, {"../a.json", "../b.json"});
}

TEST_F(GlobTest, FilesInSubDirectory) {
  check({"old/*.json"}, {"old/one.json", "old/two.json"});
}

TEST_F(GlobTest, FilesInImmediateSubdirs) {
  check({"*/*"},
        {"old/one.json", "old/two.json", "old/three.ini", "new/all.json"});
}

TEST_F(GlobTest, AbsolutePaths) {
  auto pwd = fs::current_path();
  check({(pwd / "old/two.*").string()}, {"old/two.json"});
}

TEST_F(GlobTest, ResultOrder) {
  auto filenames =
      utils::Glob({"old/one.json", "./old/three.ini", "old/one.json"});

  EXPECT_EQ(2u, filenames.size());
  EXPECT_TRUE(fs::equivalent(filenames[0], "old/three.ini"));
  EXPECT_TRUE(fs::equivalent(filenames[1], "old/one.json"));
}

TEST_F(GlobTest, BadConfigurationFilePattern) {
  const char* bad_pattern = "./old/does_not_exist.ini";

  try {
    utils::Glob({"old/one.json", bad_pattern});
    FAIL();
  } catch (const std::exception& e) {
    std::string s = e.what();
    EXPECT_NE(s.find(bad_pattern), std::string::npos);
    EXPECT_NE(s.find("not"), std::string::npos);
    EXPECT_NE(s.find("match"), std::string::npos);
    EXPECT_NE(s.find("files"), std::string::npos);
  }
}
