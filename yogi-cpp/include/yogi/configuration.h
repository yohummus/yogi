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

#ifndef YOGI_CONFIGURATION_H
#define YOGI_CONFIGURATION_H

//! \file
//!
//! Configuration objects.

#include "object.h"
#include "io.h"
#include "json.h"
#include "internal/flags.h"
#include "internal/query_string.h"
#include "string_view.h"
#include "json_view.h"

#include <memory>
#include <type_traits>

namespace yogi {

_YOGI_DEFINE_API_FN(int, YOGI_ConfigurationCreate, (void** config, int flags))

_YOGI_DEFINE_API_FN(int, YOGI_ConfigurationUpdateFromCommandLine,
                    (void* config, int argc, const char* const* argv,
                     int options, char* err, int errsize))

_YOGI_DEFINE_API_FN(int, YOGI_ConfigurationUpdateFromJson,
                    (void* config, const char* json, char* err, int errsize))

_YOGI_DEFINE_API_FN(int, YOGI_ConfigurationUpdateFromFile,
                    (void* config, const char* filename, char* err,
                     int errsize))

_YOGI_DEFINE_API_FN(int, YOGI_ConfigurationDump,
                    (void* config, char* json, int jsonsize, int resvars,
                     int indent))

_YOGI_DEFINE_API_FN(int, YOGI_ConfigurationWriteToFile,
                    (void* config, const char* filename, int resvars,
                     int indent))

/// \addtogroup enums
/// @{

////////////////////////////////////////////////////////////////////////////////
/// Flags used to change a configuration object's behaviour.
////////////////////////////////////////////////////////////////////////////////
enum class ConfigurationFlags {
  /// No flags.
  kNone = 0,

  /// Disables support for variables in the configuration.
  kDisableVariables = (1 << 0),

  /// Makes configuration options given directly on the command line
  /// overridable.
  kMutableCmdLine = (1 << 1),
};

_YOGI_DEFINE_FLAG_OPERATORS(ConfigurationFlags)

template <>
inline std::string ToString<ConfigurationFlags>(
    const ConfigurationFlags& flags) {
  switch (flags) {
    _YOGI_TO_STRING_ENUM_CASE(ConfigurationFlags, kNone)
    _YOGI_TO_STRING_ENUM_CASE(ConfigurationFlags, kDisableVariables)
    _YOGI_TO_STRING_ENUM_CASE(ConfigurationFlags, kMutableCmdLine)
  }

  std::string s;
  _YOGI_TO_STRING_FLAG_APPENDER(flags, ConfigurationFlags, kDisableVariables)
  _YOGI_TO_STRING_FLAG_APPENDER(flags, ConfigurationFlags, kMutableCmdLine)
  return s.substr(3);
}

////////////////////////////////////////////////////////////////////////////////
/// Flags used to adjust how command line options are parsed.
////////////////////////////////////////////////////////////////////////////////
enum class CommandLineOptions {
  /// No options.
  kNone = 0,

  /// Include logging configuration for file logging.
  kLogging = (1 << 0),

  /// Include branch name configuration.
  kBranchName = (1 << 1),

  /// Include branch description configuration.
  kBranchDescription = (1 << 2),

  /// Include network name configuration.
  kBranchNetwork = (1 << 3),

  /// Include network password configuration.
  kBranchPassword = (1 << 4),

  /// Include branch path configuration.
  kBranchPath = (1 << 5),

  /// Include branch advertising interfaces configuration.
  kBranchAdvIfs = (1 << 6),

  /// Include branch advertising address configuration.
  kBranchAdvAddr = (1 << 7),

  /// Include branch advertising port configuration.
  kBranchAdvPort = (1 << 8),

  /// Include branch advertising interval configuration.
  kBranchAdvInt = (1 << 9),

  /// Include branch timeout configuration.
  kBranchTimeout = (1 << 10),

  /// Include ghost mode configuration.
  kBranchGhostMode = (1 << 11),

  /// Parse configuration files given on the command line.
  kFiles = (1 << 12),

  /// Same as the Files option but at least one configuration file must be
  /// given.
  kFilesRequired = (1 << 13),

  /// Allow overriding arbitrary configuration sections.
  kOverrides = (1 << 14),

  /// Allow setting variables via a dedicated switch.
  kVariables = (1 << 15),

  /// Combination of all branch flags.
  kBranchAll = kBranchName | kBranchDescription | kBranchNetwork |
               kBranchPassword | kBranchPath | kBranchAdvIfs | kBranchAdvAddr |
               kBranchAdvPort | kBranchAdvInt | kBranchTimeout |
               kBranchGhostMode,

  /// Combination of all flags.
  kAll =
      kLogging | kBranchAll | kFiles | kFilesRequired | kOverrides | kVariables,
};

_YOGI_DEFINE_FLAG_OPERATORS(CommandLineOptions)

template <>
inline std::string ToString<CommandLineOptions>(
    const CommandLineOptions& options) {
  switch (options) {
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kNone)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kLogging)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchName)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchDescription)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchNetwork)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchPassword)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchPath)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchAdvIfs)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchAdvAddr)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchAdvPort)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchAdvInt)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchTimeout)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchGhostMode)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kFiles)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kFilesRequired)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kOverrides)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kVariables)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchAll)
    _YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kAll)
  }

  std::string s;
  _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kLogging)
  if ((options & CommandLineOptions::kBranchAll) ==
      CommandLineOptions::kBranchAll) {
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchAll)
  } else {
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchName)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions,
                                  kBranchDescription)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchNetwork)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchPassword)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchPath)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchAdvIfs)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchAdvAddr)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchAdvPort)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchAdvInt)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchTimeout)
    _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchGhostMode)
  }

  _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kFiles)
  _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kFilesRequired)
  _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kOverrides)
  _YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kVariables)
  return s.substr(3);
}

/// @} enums

class Configuration;

/// Shared pointer to a configuration.
using ConfigurationPtr = std::shared_ptr<Configuration>;

////////////////////////////////////////////////////////////////////////////////
/// A configuration represents a set of parameters that usually remain constant
/// throughout the runtime of a program.
///
/// Parameters can come from different sources such as the command line or a
/// file. Configurations are used for other parts of the library such as
/// application objects, however, they are also intended to store user-defined
/// parameters.
////////////////////////////////////////////////////////////////////////////////
class Configuration : public ObjectT<Configuration> {
 public:
  /// Creates a configuration.
  ///
  /// \param flags Flags for behaviour adjustments.
  ///
  /// \returns The created configuration.
  static ConfigurationPtr Create(
      ConfigurationFlags flags = ConfigurationFlags::kNone) {
    return ConfigurationPtr(new Configuration(flags));
  }

  /// Creates a configuration from JSON.
  ///
  /// \param json  JSON to use initially.
  /// \param flags Flags for behaviour adjustments.
  ///
  /// \returns The created configuration.
  static ConfigurationPtr Create(
      const JsonView& json,
      ConfigurationFlags flags = ConfigurationFlags::kNone) {
    auto config = Create(flags);
    config->UpdateFromJson(json);
    return config;
  }

  /// Returns the flags set for the configuration.
  ///
  /// \return Flags set for the configuration.
  ConfigurationFlags GetFlags() const { return flags_; }

  /// Updates the configuration from command line options.
  ///
  /// If parsing the command line, files or any given JSON string fails, or
  /// if help is requested (e.g. by using the --help switch) then a
  /// DescriptiveFailure exception will be raised containing detailed
  /// information about the error or the help text.
  ///
  /// \param argc    Number of command line arguments in \p argv.
  /// \param argv    Command line arguments as passed to main().
  /// \param options Options to provide on the command line.
  void UpdateFromCommandLine(
      int argc, const char* const* argv,
      CommandLineOptions options = CommandLineOptions::kNone) {
    internal::CheckDescriptiveErrorCode([&](auto err, auto size) {
      return internal::YOGI_ConfigurationUpdateFromCommandLine(
          this->GetHandle(), argc, argv, static_cast<int>(options), err, size);
    });
  }

  /// Updates the configuration from a serialized JSON object.
  ///
  /// If parsing fails then a DescriptiveFailure exception will be raised
  /// containing detailed information about the error.
  ///
  /// \param json JSON object or serialized JSON.
  void UpdateFromJson(const JsonView& json) {
    internal::CheckDescriptiveErrorCode([&](auto err, auto size) {
      return internal::YOGI_ConfigurationUpdateFromJson(this->GetHandle(), json,
                                                        err, size);
    });
  }

  /// Updates the configuration from a JSON file.
  ///
  /// If parsing the file fails then a DescriptiveFailure exception will be
  /// raised containing detailed information about the error.
  ///
  /// \param filename Path to the JSON file.
  void UpdateFromFile(const StringView& filename) {
    internal::CheckDescriptiveErrorCode([&](auto err, auto size) {
      return internal::YOGI_ConfigurationUpdateFromFile(this->GetHandle(),
                                                        filename, err, size);
    });
  }

  /// Retrieves the configuration as a JSON-formatted string.
  ///
  /// \param resolve_variables Resolve all configuration variables.
  /// \param indentation       Number of space characters to use for
  ///                          indentation; must be >= 0.
  ///
  /// \returns The configuration serialized to a string.
  std::string Dump(bool resolve_variables, int indentation) const {
    if (indentation < 0) {
      throw Failure(ErrorCode::kInvalidParam);
    }

    return DumpImpl(resolve_variables, indentation);
  }

  /// Retrieves the configuration as a JSON-formatted string.
  ///
  /// No indentation and no newlines will be generated; i.e. the returned string
  /// will be as compact as possible.
  ///
  /// \param resolve_variables Resolve all configuration variables.
  ///
  /// \returns The configuration serialized to a string.
  std::string Dump(bool resolve_variables) const {
    return DumpImpl(resolve_variables, -1);
  }

  /// Retrieves the configuration as a JSON-formatted string.
  ///
  /// %Configuration variables get resolved if the configuration supports them.
  ///
  /// \param indentation Number of space characters to use for indentation;
  ///                    must be >= 0.
  ///
  /// \returns The configuration serialized to a string.
  std::string Dump(int indentation) const {
    if (indentation < 0) {
      throw Failure(ErrorCode::kInvalidParam);
    }

    return DumpImpl((flags_ & ConfigurationFlags::kDisableVariables) ==
                        ConfigurationFlags::kNone,
                    indentation);
  }

  /// Retrieves the configuration as a JSON-formatted string.
  ///
  /// %Configuration variables get resolved if the configuration supports them.
  ///
  /// No indentation and no newlines will be generated; i.e. the returned string
  /// will be as compact as possible.
  ///
  /// \returns The configuration serialized to a string.
  std::string Dump() const {
    return DumpImpl((flags_ & ConfigurationFlags::kDisableVariables) ==
                        ConfigurationFlags::kNone,
                    -1);
  }

  /// Writes the configuration to a file in JSON format.
  ///
  /// \param filename          Path to the output file.
  /// \param resolve_variables Resolve all configuration variables.
  /// \param indentation       Number of space characters to use for
  ///                          indentation; must be >= 0.
  void WriteToFile(const StringView& filename, bool resolve_variables,
                   int indentation) const {
    if (indentation < 0) {
      throw Failure(ErrorCode::kInvalidParam);
    }

    WriteToFileImpl(filename, resolve_variables, indentation);
  }

  /// Writes the configuration to a file in JSON format.
  ///
  /// No indentation and no newlines will be generated; i.e. the returned string
  /// will be as compact as possible.
  ///
  /// \param filename          Path to the output file.
  /// \param resolve_variables Resolve all configuration variables.
  void WriteToFile(const StringView& filename, bool resolve_variables) const {
    WriteToFileImpl(filename, resolve_variables, -1);
  }

  /// Writes the configuration to a file in JSON format.
  ///
  /// %Configuration variables get resolved if the configuration supports them.
  ///
  /// \param filename    Path to the output file.
  /// \param indentation Number of space characters to use for indentation; must
  ///                    be >= 0.
  void WriteToFile(const StringView& filename, int indentation) const {
    if (indentation < 0) {
      throw Failure(ErrorCode::kInvalidParam);
    }

    WriteToFileImpl(filename,
                    (flags_ & ConfigurationFlags::kDisableVariables) ==
                        ConfigurationFlags::kNone,
                    indentation);
  }

  /// Writes the configuration to a file in JSON format.
  ///
  /// %Configuration variables get resolved if the configuration supports them.
  ///
  /// No indentation and no newlines will be generated; i.e. the returned string
  /// will be as compact as possible.
  ///
  /// \param filename Path to the output file.
  void WriteToFile(const StringView& filename) const {
    WriteToFileImpl(filename,
                    (flags_ & ConfigurationFlags::kDisableVariables) ==
                        ConfigurationFlags::kNone,
                    -1);
  }

 private:
  Configuration(ConfigurationFlags flags)
      : ObjectT(internal::CallApiCreate(internal::YOGI_ConfigurationCreate,
                                        static_cast<int>(flags)),
                {}),
        flags_(flags) {}

  std::string DumpImpl(bool resolve_variables, int indentation) const {
    return internal::QueryString([&](auto json, auto size) {
      return internal::YOGI_ConfigurationDump(this->GetHandle(), json, size,
                                              resolve_variables ? 1 : 0,
                                              indentation);
    });
  }

  void WriteToFileImpl(const char* filename, bool resolve_variables,
                       int indentation) const {
    int res = internal::YOGI_ConfigurationWriteToFile(
        this->GetHandle(), filename, resolve_variables ? 1 : 0, indentation);
    internal::CheckErrorCode(res);
  }

  const ConfigurationFlags flags_;
};

}  // namespace yogi

#endif  // YOGI_CONFIGURATION_H
