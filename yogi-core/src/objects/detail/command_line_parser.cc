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

#include "command_line_parser.h"
#include "../../api/errors.h"

#include "../../objects/logger.h"
#include "../../utils/glob.h"

#include <boost/algorithm/string.hpp>
#include <boost/optional/optional_io.hpp>
#include <sstream>
#include <fstream>

namespace po = boost::program_options;

namespace objects {
namespace detail {

CommandLineParser::CommandLineParser(int argc, const char* const* argv,
                                     api::CommandLineOptions options)
    : argc_(argc),
      argv_(argv),
      options_(options),
      visible_options_("Allowed options"),
      hidden_options_("Hidden options"),
      files_json_({}),
      direct_json_({}) {}

void CommandLineParser::Parse() {
  YOGI_ASSERT(vm_.empty());  // only run once!

  AddHelpOptions();
  AddLoggingOptions();
  AddBranchOptions();
  AddFileOptions();
  AddOverrideOptions();
  AddVariableOptions();

  PopulateVariablesMap();
  HandleHelpOptions();
  ExtractOptions();
  ApplyOverrides();

  LoadConfigFiles();
}

void CommandLineParser::AddHelpOptions() {
  // clang-format off
  visible_options_.add_options()(
    "help,h",
    "Show this help message"
  );

  if (options_ & api::kLoggingOptions) {
    visible_options_.add_options()(
      "help-logging",
      "Shows information about the logging options"
    );
  }
  // clang-format on
}

void CommandLineParser::AddLoggingOptions() {
  // clang-format off
  if (options_ & api::kLoggingOptions) {
    visible_options_.add_options()(
      "log-file", po::value<std::string>()->notifier([&](auto& val) {
        this->LogFileNotifier(val);
      }),
      "Path to the logfile with support for time placeholders; set to NONE to disable"
    )(
      "log-console", po::value<std::string>()->notifier([&](auto& val) {
        this->LogConsoleNotifier(val);
      })->implicit_value("STDERR"),
      "Log to either STDOUT, STDERR or NONE (implicit value is STDERR)"
    )(
      "log-color", po::value<bool>()->notifier([&](auto& val) {
        direct_json_["logging"]["color"] = val;
      })->implicit_value(true),
      "Use color when logging to the console"
    )(
      "log-fmt", po::value<std::string>()->notifier([&](auto& val) {
        direct_json_["logging"]["entry-format"] = val;
      }),
      "Format of a log entry (use entry placeholders)"
    )(
      "log-time-fmt", po::value<std::string>()->notifier([&](auto& val) {
        direct_json_["logging"]["time-format"] = val;
      }),
      "Format of a log entry's timestamp (use time placeholders)"
    )(
      "log-verbosity", po::value<std::vector<std::string>>()->notifier([&](auto& val) {
        this->LogVerbosityNotifier(val);
      }),
      "Logger verbosities (e.g. --log-verbosity='Yogi.*=DEBUG')"
    );
  }
  // clang-format on
}

void CommandLineParser::AddBranchOptions() {
  // clang-format off
  if (options_ & api::kBranchNameOption) {
    visible_options_.add_options()(
      "name", po::value<std::string>()->notifier([&](auto& val) {
        direct_json_["branch"]["name"] = val;
      }),
      "Branch name"
    );
  }

  if (options_ & api::kBranchDescriptionOption) {
    visible_options_.add_options()(
      "description", po::value<std::string>()->notifier([&](auto& val) {
        direct_json_["branch"]["description"] = val;
      }),
      "Branch description"
    );
  }

  if (options_ & api::kBranchNetworkOption) {
    visible_options_.add_options()(
      "network", po::value<std::string>()->notifier([&](auto& val) {
        direct_json_["branch"]["network"] = val;
      }),
      "Network name"
    );
  }

  if (options_ & api::kBranchPasswordOption) {
    visible_options_.add_options()(
      "password", po::value<std::string>()->notifier([&](auto& val) {
        direct_json_["branch"]["password"] = val;
      }),
      "Network password"
    );
  }

  if (options_ & api::kBranchPathOption) {
    visible_options_.add_options()(
      "path", po::value<std::string>()->notifier([&](auto& val) {
        direct_json_["branch"]["path"] = val;
      }),
      "Branch path"
    );
  }

  if (options_ & api::kBranchAdvInterfacesOption) {
    visible_options_.add_options()(
      "adv-ifs", po::value<std::vector<std::string>>()->notifier([&](auto& val) {
        direct_json_["branch"]["advertising_interfaces"] = val;
      }),
      "Branch advertising interfaces (e.g. --adv-ifs 'eth0' --adv-ifs 'localhost' --adv-ifs '11:22:33:44:55:66')"
    );
  }

  if (options_ & api::kBranchAdvAddressOption) {
    visible_options_.add_options()(
      "adv-addr", po::value<std::string>()->notifier([&](auto& val) {
        direct_json_["branch"]["advertising_address"] = val;
      }),
      "Branch advertising address (e.g. --adv-addr 'ff02::8000:2439')"
    );
  }

  if (options_ & api::kBranchAdvPortOption) {
    visible_options_.add_options()(
      "adv-port", po::value<unsigned>()->notifier([&](auto& val) {
        direct_json_["branch"]["advertising_port"] = val;
      }),
      "Branch advertising port (e.g. --adv-port 13531"
    );
  }

  if (options_ & api::kBranchAdvIntervalOption) {
    visible_options_.add_options()(
      "adv-int", po::value<float>()->notifier([&](auto& val) {
        direct_json_["branch"]["advertising_interval"] = val;
      }),
      "Branch advertising interval in seconds (e.g. --adv-int 3.0)"
    );
  }

  if (options_ & api::kBranchTimeoutOption) {
    visible_options_.add_options()(
      "timeout", po::value<float>()->notifier([&](auto& val) {
        direct_json_["branch"]["timeout"] = val;
      }),
      "Ghost mode"
    );
  }

  if (options_ & api::kBranchGhostModeOption) {
    visible_options_.add_options()(
      "ghost", po::value<bool>()->notifier([&](auto& val) {
        direct_json_["branch"]["ghost_mode"] = val;
      })->implicit_value(true),
      "Branch timeout in seconds (e.g. --timeout 3.0)"
    );
  }
  // clang-format on
}

void CommandLineParser::AddFileOptions() {
  // clang-format off
  if (options_ & api::kFileOption || options_ & api::kFileRequiredOption) {
    auto name = "_cfg_files";
    hidden_options_.add_options()(
      name, po::value<std::vector<std::string>>()->notifier([&](auto& val) {
        this->FileNotifier(val);
      }),
      "Configuration files (JSON format); multiple files will be merged"
      " according to JSON Merge Patch (RFC 7386) from left to right"
    );

    positional_options_.add(name, -1);
  }
  // clang-format on
}

void CommandLineParser::AddOverrideOptions() {
  // clang-format off
  if (options_ & api::kOverrideOption) {
    visible_options_.add_options()(
      "override,o", po::value<std::vector<std::string>>()->notifier([&](auto& val) {
        this->OverrideNotifier(val);
      }),
      "Configuration overrides in either JSON Merge Patch (RFC 7386) format"
      " (--override '{\"person\":{\"age\":42}}') or simplified using JSON"
      " Pointer (RFC 6901) format (--override '/person/age=42')"
    );
  }
  // clang-format on
}

void CommandLineParser::AddVariableOptions() {
  // clang-format off
  if (options_ & api::kVariableOption) {
    visible_options_.add_options()(
      "var,v", po::value<std::vector<std::string>>()->notifier([&](auto& val) {
        this->VariableNotifier(val);
      }),
      "Configuration variables (e.g. --var 'DIR=\"/usr/local\"')"
    );
  }
  // clang-format on
}

void CommandLineParser::PopulateVariablesMap() {
  po::options_description options;
  options.add(visible_options_);
  options.add(hidden_options_);

  try {
    po::store(po::command_line_parser(argc_, argv_)
                  .options(options)
                  .positional(positional_options_)
                  .run(),
              vm_);
  } catch (const std::exception& e) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED) << e.what();
  }
}

void CommandLineParser::HandleHelpOptions() {
  if (vm_.count("help")) {
    std::string binary_name = argv_[0];
    auto pos = binary_name.find_last_of("/\\");
    if (pos != std::string::npos) {
      binary_name = binary_name.substr(pos + 1);
    }

    std::stringstream ss;
    ss << "Usage: " << binary_name << " [options]";
    if (options_ & api::kFileOption || options_ & api::kFileRequiredOption) {
      ss << (options_ & api::kFileRequiredOption ? " config.json"
                                                 : " [config.json]");
      ss << " [config2.json ...]";
    }
    ss << std::endl;

    ss << std::endl;
    ss << visible_options_ << std::endl;

    throw api::DescriptiveError(YOGI_ERR_HELP_REQUESTED) << ss.str();
  }

  if (vm_.count("help-logging")) {
    // clang-format off
    std::stringstream ss;
    ss << "Some of the logging switches support the placeholders listed below." << std::endl;
    ss << std::endl;
    ss << "Time placeholders:" << std::endl;
    ss << "  %Y - Four digit year" << std::endl;
    ss << "  %m - Month name as a decimal 01 to 12" << std::endl;
    ss << "  %d - Day of the month as decimal 01 to 31" << std::endl;
    ss << "  %F - Equivalent to %Y-%m-%d (the ISO 8601 date format)" << std::endl;
    ss << "  %H - The hour as a decimal number using a 24-hour clock (range 00 to 23)" << std::endl;
    ss << "  %M - The minute as a decimal 00 to 59" << std::endl;
    ss << "  %S - Seconds as a decimal 00 to 59" << std::endl;
    ss << "  %T - Equivalent to %H:%M:%S (the ISO 8601 time format)" << std::endl;
    ss << "  %3 - Milliseconds as decimal number 000 to 999" << std::endl;
    ss << "  %6 - Microseconds as decimal number 000 to 999" << std::endl;
    ss << "  %9 - Nanoseconds as decimal number 000 to 999" << std::endl;
    ss << std::endl;
    ss << "Entry placeholders:" << std::endl;
    ss << "  $t - Timestamp, formatted according to the configured time format" << std::endl;
    ss << "  $P - Process ID (PID)" << std::endl;
    ss << "  $T - Thread ID" << std::endl;
    ss << "  $s - Severity as a 3 letter abbreviation (FAT, ERR, WRN, IFO, DBG or TRC)" << std::endl;
    ss << "  $m - Log message" << std::endl;
    ss << "  $f - Source filename" << std::endl;
    ss << "  $l - Source line number" << std::endl;
    ss << "  $c - Component tag" << std::endl;
    ss << "  $< - Set console color corresponding to severity" << std::endl;
    ss << "  $> - Reset the colours (also done after each log entry)" << std::endl;
    ss << "  $$ - A $ sign" << std::endl;
    // clang-format on

    throw api::DescriptiveError(YOGI_ERR_HELP_REQUESTED) << ss.str();
  }
}

void CommandLineParser::ExtractOptions() {
  try {
    po::notify(vm_);
  } catch (const po::error& e) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED) << e.what();
  }
}

void CommandLineParser::ApplyOverrides() {
  try {
    for (auto& ovr : overrides_) {
      if (ovr.json_pointer_syntax) {
        direct_json_[ovr.path] = ovr.value;
      } else {
        direct_json_.merge_patch(ovr.value);
      }
    }
  } catch (const nlohmann::json::exception& e) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED) << e.what();
  }
}

void CommandLineParser::LoadConfigFiles() {
  for (auto& file : config_files_) {
    std::ifstream f(file);
    if (!f.is_open()) {
      throw api::DescriptiveError(YOGI_ERR_PARSING_FILE_FAILED)
          << "Could not open " << file;
    }

    nlohmann::json json;
    try {
      f >> json;
    } catch (const std::exception& e) {
      throw api::DescriptiveError(YOGI_ERR_PARSING_FILE_FAILED)
          << "Could not parse " << file << ": " << e.what();
    }

    files_json_.merge_patch(json);
  }
}

void CommandLineParser::LogFileNotifier(const std::string& val) {
  if (boost::to_upper_copy(val) == "NONE") {
    direct_json_["logging"]["file"] = nullptr;
  } else {
    direct_json_["logging"]["file"] = val;
  }
}

void CommandLineParser::LogConsoleNotifier(const std::string& val) {
  auto s = boost::to_upper_copy(val);
  if (s != "STDERR" && s != "STDOUT" && s != "NONE") {
    throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED)
        << "Invalid value \"" << val
        << "\"for --log-console. Allowed values are STDOUT, STDERR and NONE.";
  }

  if (s == "NONE") {
    direct_json_["logging"]["console"] = nullptr;
  } else {
    direct_json_["logging"]["console"] = s;
  }
}

void CommandLineParser::LogVerbosityNotifier(
    const std::vector<std::string>& val) {
  for (auto& str : val) {
    auto sep_pos = str.find('=');
    if (sep_pos == std::string::npos) {
      throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED)
          << "Invalid log verbosity string format \"" << str << "\"";
    }

    auto comp = str.substr(0, sep_pos);
    auto verb = str.substr(sep_pos + 1);

    try {
      Logger::StringToVerbosity(verb);
    } catch (const std::exception& e) {
      throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED) << e.what();
    }

    direct_json_["logging"]["verbosity"][comp] = verb;
  }
}

void CommandLineParser::FileNotifier(const std::vector<std::string>& val) {
  try {
    config_files_ = utils::Glob(val);
  } catch (const std::exception& e) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED) << e.what();
  }

  if (options_ & api::kFileRequiredOption && config_files_.empty()) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED)
        << "No configuration files specified.";
  }
}

void CommandLineParser::OverrideNotifier(const std::vector<std::string>& val) {
  for (auto& str : val) {
    Override ovr;

    if (str.front() == '{') {
      ovr.json_pointer_syntax = false;

      try {
        ovr.value = nlohmann::json::parse(str);
      } catch (const std::exception& e) {
        throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED)
            << "Parsing \"" << str << "\" failed: " << e.what();
      }
    } else {
      ovr.json_pointer_syntax = true;

      auto sep_pos = str.find('=');
      if (sep_pos == std::string::npos) {
        throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED)
            << "Invalid override format \"" << str << "\"";
      }

      ovr.path = nlohmann::json::json_pointer(str.substr(0, sep_pos));
      auto value_str = str.substr(sep_pos + 1);
      try {
        ovr.value = nlohmann::json::parse(value_str);
      } catch (const nlohmann::json::exception&) {
        ovr.value = value_str;
      }
    }

    overrides_.push_back(ovr);
  }
}

void CommandLineParser::VariableNotifier(const std::vector<std::string>& val) {
  for (auto& str : val) {
    auto sep_pos = str.find('=');
    if (sep_pos == std::string::npos) {
      throw api::DescriptiveError(YOGI_ERR_PARSING_CMDLINE_FAILED)
          << "Invalid veriable format \"" << str << "\"";
    }

    auto name = str.substr(0, sep_pos);
    auto value = str.substr(sep_pos + 1);

    auto& target = direct_json_["variables"][name];
    try {
      target = nlohmann::json::parse(value);
    } catch (const nlohmann::json::exception&) {
      target = value;
    }
  }
}

}  // namespace detail
}  // namespace objects
