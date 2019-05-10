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

#pragma once

#include "../../config.h"
#include "../../api/errors.h"
#include "../../api/enums.h"

#include <nlohmann/json.hpp>
#include <boost/program_options.hpp>
#include <vector>
#include <string>

namespace objects {
namespace detail {

class CommandLineParser {
 public:
  CommandLineParser(int argc, const char* const* argv,
                    api::CommandLineOptions options);

  void Parse();
  const nlohmann::json& GetFilesConfiguration() const { return files_json_; }
  const nlohmann::json& GetDirectConfiguration() const { return direct_json_; }

 private:
  struct Override {
    bool json_pointer_syntax;
    nlohmann::json::json_pointer path;
    nlohmann::json value;
  };

  void AddHelpOptions();
  void AddLoggingOptions();
  void AddBranchOptions();
  void AddFileOptions();
  void AddOverrideOptions();
  void AddVariableOptions();

  void PopulateVariablesMap();
  void HandleHelpOptions();
  void ExtractOptions();
  void ApplyOverrides();
  void LoadConfigFiles();

  void LogFileNotifier(const std::string& val);
  void LogConsoleNotifier(const std::string& val);
  void LogVerbosityNotifier(const std::vector<std::string>& val);
  void FileNotifier(const std::vector<std::string>& val);
  void OverrideNotifier(const std::vector<std::string>& val);
  void VariableNotifier(const std::vector<std::string>& val);

  const int argc_;
  const char* const* const argv_;
  const api::CommandLineOptions options_;

  boost::program_options::options_description visible_options_;
  boost::program_options::options_description hidden_options_;
  boost::program_options::positional_options_description positional_options_;
  boost::program_options::variables_map vm_;
  nlohmann::json files_json_;
  nlohmann::json direct_json_;

  std::vector<std::string> config_files_;
  std::vector<Override> overrides_;
};

}  // namespace detail
}  // namespace objects
