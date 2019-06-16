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

#include "configuration.h"

#include <boost/algorithm/string.hpp>
#include <string>
#include <sstream>
#include <fstream>
using namespace std::string_literals;

YOGI_DEFINE_INTERNAL_LOGGER("Configuration")

namespace objects {
namespace config {

Configuration::Configuration(api::ConfigurationFlags flags)
    : variables_supported_(!(flags & api::kDisableVariables)),
      mutable_cmdline_(flags & api::kMutableCmdLine),
      json_({}),
      immutable_json_({}) {
  SetLoggingPrefix(*this);
}

void Configuration::UpdateFromCommandLine(int argc, const char* const* argv,
                                          api::CommandLineOptions options) {
  detail::CommandLineParser parser(argc, argv, options);
  parser.Parse();

  VerifyAndMerge(parser.GetFilesConfiguration(),
                 parser.GetDirectConfiguration());

  if (!mutable_cmdline_) {
    immutable_json_ = parser.GetDirectConfiguration();
  }
}

void Configuration::UpdateFromString(const std::string& json_str) {
  nlohmann::json json;

  try {
    json = nlohmann::json::parse(json_str);
  } catch (const nlohmann::json::exception& e) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_JSON_FAILED)
        << "Could not parse JSON string: " << e.what();
  }

  VerifyAndMerge(json, immutable_json_);
}

void Configuration::UpdateFromFile(const std::string& filename) {
  std::ifstream f(filename);
  if (!f.is_open() || f.fail()) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_FILE_FAILED)
        << "Could not open " << filename;
  }

  nlohmann::json json;
  try {
    f >> json;
  } catch (const std::exception& e) {
    throw api::DescriptiveError(YOGI_ERR_PARSING_FILE_FAILED)
        << "Could not parse " << filename << ": " << e.what();
  }

  VerifyAndMerge(json, immutable_json_);
}

nlohmann::json Configuration::GetJson(
    const nlohmann::json::json_pointer& target) const {
  return GetJson(variables_supported_, target);
}

nlohmann::json Configuration::GetJson(
    bool resolve_variables, const nlohmann::json::json_pointer& target) const {
  nlohmann::json json;

  if (resolve_variables) {
    if (!variables_supported_) {
      throw api::Error(YOGI_ERR_NO_VARIABLE_SUPPORT);
    }

    json = ResolveVariables(json_)[target];
  } else {
    json = json_[target];
  }

  return json;
}

std::string Configuration::Dump(bool resolve_variables,
                                int indentation_width) const {
  return GetJson(resolve_variables).dump(indentation_width);
}

void Configuration::WriteToFile(const std::string& filename,
                                bool resolve_variables,
                                int indentation_width) const {
  if (!variables_supported_ && resolve_variables) {
    throw api::Error(YOGI_ERR_NO_VARIABLE_SUPPORT);
  }

  std::ofstream f(filename);
  if (!f.is_open() || f.fail()) {
    throw api::Error(YOGI_ERR_READ_FILE_FAILED);
  }

  try {
    if (resolve_variables) {
      f << ResolveVariables(json_).dump(indentation_width);
    } else {
      f << json_.dump(indentation_width);
    }

    if (indentation_width != -1) {
      f << std::endl;
    }
  } catch (const std::exception& e) {
    LOG_ERR("Could not write configuration to " << filename << ": "
                                                << e.what());
    throw api::Error(YOGI_ERR_WRITE_FILE_FAILED);
  }
}

void Configuration::CheckCircularVariableDependency(
    const std::string& var_ref, const nlohmann::json& var_val) {
  if (var_val.is_string()) {
    auto str = var_val.get<std::string>();
    if (str.find(var_ref) != std::string::npos) {
      throw api::DescriptiveError(YOGI_ERR_UNDEFINED_VARIABLES)
          << "Circular dependency in variable \""
          << var_ref.substr(2, var_ref.size() - 3) << '"';
    }
  }
}

void Configuration::ResolveVariablesSections(nlohmann::json* vars) {
  for (auto it = vars->begin(); it != vars->end(); ++it) {
    auto var_ref = "${"s + it.key() + '}';
    auto var_val = it.value();
    CheckCircularVariableDependency(var_ref, var_val);

    for (auto& elem : *vars) {
      ResolveSingleVariable(&elem, var_ref, var_val);
    }
  }
}

void Configuration::ResolveSingleVariable(nlohmann::json* elem,
                                          const std::string& var_ref,
                                          const nlohmann::json& var_val) {
  if (!elem->is_string()) {
    return;
  }

  auto val = elem->get<std::string>();
  if (val == var_ref) {
    *elem = var_val;
  } else {
    auto replacement =
        var_val.is_string() ? var_val.get<std::string>() : var_val.dump();
    boost::replace_all(val, var_ref, replacement);
    *elem = val;
  }
}

nlohmann::json Configuration::ResolveVariables(
    const nlohmann::json& unresolved_json) {
  auto json = unresolved_json;
  if (!json.count("variables")) {
    return json;
  }

  auto& vars = json["variables"];

  ResolveVariablesSections(&vars);

  WalkAllElements(&json, [&](const auto&, auto* elem) {
    for (auto it = vars.cbegin(); it != vars.cend(); ++it) {
      auto var_ref = "${"s + it.key() + '}';
      ResolveSingleVariable(elem, var_ref, it.value());
    }
  });

  return json;
}

template <typename Fn>
void Configuration::WalkAllElements(nlohmann::json* json, Fn fn) {
  for (auto it = json->begin(); it != json->end(); ++it) {
    if (it.value().is_structured()) {
      WalkAllElements(&it.value(), fn);
    }

    if (json->is_object()) {
      fn(it.key(), &it.value());
    } else {
      static const std::string dummy;
      fn(dummy, &it.value());
    }
  }
}

void Configuration::CheckVariablesOnlyUsedInValues(nlohmann::json* json) {
  WalkAllElements(json, [=](const auto& key, auto) {
    if (key.find("${") != std::string::npos) {
      throw api::DescriptiveError(YOGI_ERR_VARIABLE_USED_IN_KEY)
          << "Found syntax for variable in key: " << key;
    }
  });
}

void Configuration::CheckAllVariablesAreResolved(nlohmann::json* json) {
  WalkAllElements(json, [=](const auto&, const auto* elem) {
    if (!elem->is_string()) return;
    auto val = elem->template get<std::string>();
    auto pos = val.find("${");
    if (pos != std::string::npos) {
      throw api::DescriptiveError(YOGI_ERR_UNDEFINED_VARIABLES)
          << "Variable \""
          << val.substr(pos + 2, val.find('}', pos + 2) - pos - 2)
          << "\" could not be resolved";
    }
  });
}

void Configuration::VerifyAndMerge(const nlohmann::json& json_to_merge,
                                   const nlohmann::json& immutable_json) {
  auto new_json = json_;
  new_json.merge_patch(json_to_merge);
  new_json.merge_patch(immutable_json);

  if (variables_supported_) {
    CheckVariablesOnlyUsedInValues(&new_json);
    auto resolved_json = ResolveVariables(new_json);
    CheckAllVariablesAreResolved(&resolved_json);
  }

  json_ = new_json;
}

}  // namespace config
}  // namespace objects
