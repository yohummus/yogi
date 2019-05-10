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

#include "glob.h"
#include "../utils/algorithm.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <vector>
#include <regex>
#include <stdexcept>

namespace fs = boost::filesystem;
using namespace std::string_literals;

namespace utils {
namespace {

std::vector<std::string> SplitPatternByDirectoryAndSimplify(
    const std::string& pattern) {
  std::vector<std::string> parts;
  boost::split(parts, pattern, boost::is_any_of("/\\"));

  std::vector<std::string> simplified_parts;
  for (auto& part : parts) {
    if (part == ".") {
      continue;
    }

    bool previous_part_was_dir =
        !simplified_parts.empty() && (simplified_parts.back() != "..");
    if ((part == "..") && previous_part_was_dir) {
      simplified_parts.pop_back();
    } else {
      simplified_parts.push_back(part);
    }
  }

  return simplified_parts;
}

std::regex GlobExpressionToRegex(std::string pattern) {
  boost::replace_all(pattern, "\\", "\\\\");
  boost::replace_all(pattern, "^", "\\^");
  boost::replace_all(pattern, ".", "\\.");
  boost::replace_all(pattern, "$", "\\$");
  boost::replace_all(pattern, "|", "\\|");
  boost::replace_all(pattern, "(", "\\(");
  boost::replace_all(pattern, ")", "\\)");
  boost::replace_all(pattern, "[", "\\[");
  boost::replace_all(pattern, "]", "\\]");
  boost::replace_all(pattern, "*", "\\*");
  boost::replace_all(pattern, "+", "\\+");
  boost::replace_all(pattern, "?", "\\?");
  boost::replace_all(pattern, "/", "\\/");
  boost::replace_all(pattern, "\\?", ".");
  boost::replace_all(pattern, "\\*", ".*");

  return std::regex(pattern);
}

void RecursivelyFindFiles(std::vector<std::string>* filenames,
                          const fs::path& path,
                          std::vector<std::string> pattern_parts) {
  std::regex regex(GlobExpressionToRegex(pattern_parts.front()));

  for (fs::directory_iterator it(path); it != fs::directory_iterator(); ++it) {
    auto filename = it->path().filename();
    if (std::regex_match(filename.string(), regex)) {
      if (pattern_parts.size() == 1 && fs::is_regular_file(it->path())) {
        filenames->push_back(it->path().string());
      } else if (fs::is_directory(it->path()) && pattern_parts.size() > 1) {
        auto child_path = path / filename;
        auto child_pattern_parts = std::vector<std::string>(
            pattern_parts.begin() + 1, pattern_parts.end());
        RecursivelyFindFiles(filenames, child_path, child_pattern_parts);
      }
    }
  }
}

std::vector<std::string> GlobSinglePattern(const std::string& pattern) {
  std::vector<std::string> filenames;

  auto pattern_parts = SplitPatternByDirectoryAndSimplify(pattern);
  fs::path start_path;

  // we got an absolute path
  if (pattern_parts.front().empty()) {
    start_path = "/";
    pattern_parts.erase(pattern_parts.begin());
  }
  // we got a drive letter (Windows) e.g. C:
  else if (pattern_parts.front().size() == 2 &&
           pattern_parts.front()[1] == ':') {
    start_path = pattern_parts.front() + "\\";
    pattern_parts.erase(pattern_parts.begin());
  }
  // .. or a file/directory
  else {
    while (pattern_parts.front() == "..") {
      start_path /= "..";
      pattern_parts.erase(pattern_parts.begin());
    }

    if (start_path.empty()) {
      start_path = ".";
    }
  }

  // take a shortcut by going to the deepest directory directly if possible
  while (!pattern_parts.empty() &&
         fs::is_directory(start_path / pattern_parts.front())) {
    start_path /= pattern_parts.front();
    pattern_parts.erase(pattern_parts.begin());
  }

  RecursivelyFindFiles(&filenames, start_path, pattern_parts);

  return filenames;
}

void MergeVectors(std::vector<std::string>* filenames,
                  const std::vector<std::string>& new_filenames) {
  for (auto& filename : new_filenames) {
    auto it = utils::find(*filenames, filename);
    if (it != filenames->end()) {
      // move filename to end of vector
      std::rotate(it, it + 1, filenames->end());
    } else {
      filenames->push_back(filename);
    }
  }
}

}  // anonymous namespace

std::vector<std::string> Glob(const std::vector<std::string>& patterns) {
  std::vector<std::string> filenames;

  for (auto& pattern : patterns) {
    auto new_filenames = GlobSinglePattern(pattern);
    if (new_filenames.empty()) {
      throw std::runtime_error("Pattern \"" + pattern +
                               "\" did not match any files");
    }

    MergeVectors(&filenames, new_filenames);
  }

  return filenames;
}

}  // namespace utils
