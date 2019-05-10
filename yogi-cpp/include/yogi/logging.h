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

#ifndef YOGI_LOGGING_H
#define YOGI_LOGGING_H

//! \file
//!
//! Everthing related to logging.

#include "io.h"
#include "object.h"
#include "time.h"
#include "string_view.h"
#include "internal/library.h"

#include <cassert>
#include <functional>
#include <memory>
#include <chrono>
#include <mutex>
#include <sstream>

/// \addtogroup logmacros
/// @{

/// Creates a log entry with fatal severity.
///
/// Examples:
///
/// \code
///   // Logging to the App logger:
///   YOGI_LOG_FATAL("My luck number is " << 42)
///
///   // Logging to a custom logger:
///   float rpm = 123.45;
///   yogi::Logger logger("Engine");
///   YOGI_LOG_FATAL(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_FATAL(...) YOGI_LOG(kFatal, __VA_ARGS__)

/// Creates a log entry with error severity.
///
/// Examples:
///
/// \code
///   // Logging to the App logger:
///   YOGI_LOG_ERROR("My luck number is " << 42)
///
///   // Logging to a custom logger:
///   float rpm = 123.45;
///   yogi::Logger logger("Engine");
///   YOGI_LOG_ERROR(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_ERROR(...) YOGI_LOG(kError, __VA_ARGS__)

/// Creates a log entry with warning severity.
///
/// Examples:
///
/// \code
///   // Logging to the App logger:
///   YOGI_LOG_WARNING("My luck number is " << 42)
///
///   // Logging to a custom logger:
///   float rpm = 123.45;
///   yogi::Logger logger("Engine");
///   YOGI_LOG_WARNING(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_WARNING(...) YOGI_LOG(kWarning, __VA_ARGS__)

/// Creates a log entry with info severity.
///
/// Examples:
///
/// \code
///   // Logging to the App logger:
///   YOGI_LOG_INFO("My luck number is " << 42)
///
///   // Logging to a custom logger:
///   float rpm = 123.45;
///   yogi::Logger logger("Engine");
///   YOGI_LOG_INFO(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_INFO(...) YOGI_LOG(kInfo, __VA_ARGS__)

/// Creates a log entry with debug severity.
///
/// Examples:
///
/// \code
///   // Logging to the App logger:
///   YOGI_LOG_DEBUG("My luck number is " << 42)
///
///   // Logging to a custom logger:
///   float rpm = 123.45;
///   yogi::Logger logger("Engine");
///   YOGI_LOG_DEBUG(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_DEBUG(...) YOGI_LOG(kDebug, __VA_ARGS__)

/// Creates a log entry with trace severity.
///
/// Examples:
///
/// \code
///   // Logging to the App logger:
///   YOGI_LOG_TRACE("My luck number is " << 42)
///
///   // Logging to a custom logger:
///   float rpm = 123.45;
///   yogi::Logger logger("Engine");
///   YOGI_LOG_TRACE(logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG_TRACE(...) YOGI_LOG(kTrace, __VA_ARGS__)

/// Creates a log entry.
///
/// The severity parameter is the name of one of the Verbosity enum values.
///
/// Examples:
///
/// \code
///   // Logging to the App logger:
///   YOGI_LOG(kInfo, "My luck number is " << 42)
///
///   // Logging to a custom logger:
///   float rpm = 123.45;
///   yogi::Logger logger("Engine");
///   YOGI_LOG(kWarning, logger, "Speed exceeded " << rpm << " RPM")
/// \endcode
#define YOGI_LOG(severity, ...)                                               \
  _YOGI_LOG_EXPAND(_YOGI_LOG_IMPL(                                            \
      severity, _YOGI_LOG_SELECT(_YOGI_LOG_LOGGER, __VA_ARGS__)(__VA_ARGS__), \
      _YOGI_LOG_SELECT(_YOGI_LOG_STREAM, __VA_ARGS__)(__VA_ARGS__)))

/// @} logmacros

#define _YOGI_LOG_EXPAND(x) x
#define _YOGI_LOG_CAT(a, b) _YOGI_LOG_CAT2(a, b)
#define _YOGI_LOG_CAT2(a, b) a##b

#define _YOGI_LOG_SELECT(macro, ...)                                  \
  _YOGI_LOG_CAT(macro, _YOGI_LOG_SELECT2(__VA_ARGS__, _CUSTOM_LOGGER, \
                                         _APP_LOGGER, _INVALID))

#define _YOGI_LOG_SELECT2(_0, _1, suffix, ...) suffix
#define _YOGI_LOG_LOGGER_CUSTOM_LOGGER(logger, stream) logger
#define _YOGI_LOG_LOGGER_APP_LOGGER(stream) ::yogi::app_logger
#define _YOGI_LOG_STREAM_CUSTOM_LOGGER(logger, stream) stream
#define _YOGI_LOG_STREAM_APP_LOGGER(stream) stream

#define _YOGI_LOG_IMPL(severity, logger, stream)                     \
  {                                                                  \
    if (::yogi::Verbosity::severity <= (logger)->GetVerbosity()) {   \
      std::stringstream ss;                                          \
      ss << stream;                                                  \
      (logger)->Log(::yogi::Verbosity::severity, ss.str(), __FILE__, \
                    __LINE__);                                       \
    }                                                                \
  }

namespace yogi {

_YOGI_DEFINE_API_FN(int, YOGI_ConfigureConsoleLogging,
                    (int verbosity, int stream, int color, const char* timefmt,
                     const char* fmt))

_YOGI_DEFINE_API_FN(int, YOGI_ConfigureHookLogging,
                    (int verbosity,
                     void (*fn)(int severity, long long timestamp, int tid,
                                const char* file, int line, const char* comp,
                                const char* msg, void* userarg),
                     void* userarg))

_YOGI_DEFINE_API_FN(int, YOGI_ConfigureFileLogging,
                    (int verbosity, const char* filename, char* genfn,
                     int genfnsize, const char* timefmt, const char* fmt))

_YOGI_DEFINE_API_FN(int, YOGI_LoggerCreate,
                    (void** logger, const char* component))

_YOGI_DEFINE_API_FN(int, YOGI_LoggerGetVerbosity,
                    (void* logger, int* verbosity))

_YOGI_DEFINE_API_FN(int, YOGI_LoggerSetVerbosity, (void* logger, int verbosity))

_YOGI_DEFINE_API_FN(int, YOGI_LoggerSetComponentsVerbosity,
                    (const char* components, int verbosity, int* count))

_YOGI_DEFINE_API_FN(int, YOGI_LoggerLog,
                    (void* logger, int severity, const char* file, int line,
                     const char* msg))

/// \addtogroup enums
/// @{

////////////////////////////////////////////////////////////////////////////////
/// Levels of how verbose logging output is.
///
/// The term _severity_ is refers to the same type.
////////////////////////////////////////////////////////////////////////////////
enum class Verbosity {
  /// Fatal errors are errors that require a process restart.
  kFatal = 0,

  /// Errors that the system can recover from.
  kError = 1,

  /// Warnings.
  kWarning = 2,

  /// Useful general information about the system state.
  kInfo = 3,

  /// Information for debugging.
  kDebug = 4,

  /// Detailed debugging information.
  kTrace = 5,
};

template <>
inline std::string ToString<Verbosity>(const Verbosity& vb) {
  switch (vb) {
    _YOGI_TO_STRING_ENUM_CASE(Verbosity, kFatal)
    _YOGI_TO_STRING_ENUM_CASE(Verbosity, kError)
    _YOGI_TO_STRING_ENUM_CASE(Verbosity, kWarning)
    _YOGI_TO_STRING_ENUM_CASE(Verbosity, kInfo)
    _YOGI_TO_STRING_ENUM_CASE(Verbosity, kDebug)
    _YOGI_TO_STRING_ENUM_CASE(Verbosity, kTrace)
  }

  bool should_never_get_here = false;
  assert(should_never_get_here);
  return {};
}

////////////////////////////////////////////////////////////////////////////////
/// Output streams for writing to the terminal.
////////////////////////////////////////////////////////////////////////////////
enum class Stream {
  /// Standard output.
  kStdout = 0,

  /// Stanard error.
  kStderr = 1,
};

template <>
inline std::string ToString<Stream>(const Stream& st) {
  switch (st) {
    _YOGI_TO_STRING_ENUM_CASE(Stream, kStdout)
    _YOGI_TO_STRING_ENUM_CASE(Stream, kStderr)
  }

  bool should_never_get_here = false;
  assert(should_never_get_here);
  return {};
}

/// @} enums

/// \addtogroup freefn
/// @{

/// Configures logging to the console.
///
/// This function supports colourizing the output if the terminal that the
/// process is running in supports it. The color used for a log entry depends
/// on the entry's severity. For example, errors will be printed in red and
/// warnings in yellow.
///
/// Each log entry contains the _component_ tag which describes which part of
/// a program issued the log entry. For entries created by the library itself,
/// this parameter is prefixed with the string "Yogi.", followed by the
/// internal component name. For example, the component tag for a branch would
/// be "Yogi.Branch".
///
/// The \p timefmt parameter describes the textual format of a log entry's
/// timestamp. The following placeholders are supported:
///  - \c \%Y: Four digit year.
///  - \c \%m: Month name as a decimal 01 to 12.
///  - \c \%d: Day of the month as decimal 01 to 31.
///  - \c \%F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
///  - \c \%H: The hour as a decimal number using a 24-hour clock (00 to 23).
///  - \c \%M: The minute as a decimal 00 to 59.
///  - \c \%S: Seconds as a decimal 00 to 59.
///  - \c \%T: Equivalent to %H:%M:%S (the ISO 8601 time format).
///  - \c \%3: Milliseconds as decimal number 000 to 999.
///  - \c \%6: Microseconds as decimal number 000 to 999.
///  - \c \%9: Nanoseconds as decimal number 000 to 999.
///
/// The \p fmt parameter describes the textual format of the complete log entry
/// as it will appear on the console. The supported placeholders are:
///  - \c $t: Timestamp, formatted according to the timefmt parameter.
///  - \c $P: Process ID (PID).
///  - \c $T: Thread ID.
///  - \c $s: Severity as 3 letters (FAT, ERR, WRN, IFO, DBG or TRC).
///  - \c $m: Log message.
///  - \c $f: Source file name.
///  - \c $l: Source line number.
///  - \c $c: Component tag.
///  - \c $<: Set console color corresponding to severity.
///  - \c $>: Reset the colours (also done after each log entry).
///  - \c $$: A $ sign.
///
/// \note
///   The placeholder syntax is a modulo or dollar sign repectively, followed by
///   a single character. Any additional characters shown above are for Doxygen.
///
/// \param verbosity Maximum verbosity of messages to log
/// \param stream    The stream to use
/// \param color     Use colors in output
/// \param timefmt   Format of the timestamp (see above for placeholders)
/// \param fmt       Format of a log entry (see above for placeholders)
inline void SetupConsoleLogging(Verbosity verbosity,
                                Stream stream = Stream::kStderr,
                                bool color = true,
                                const StringView& timefmt = {},
                                const StringView& fmt = {}) {
  int res = internal::YOGI_ConfigureConsoleLogging(static_cast<int>(verbosity),
                                                   static_cast<int>(stream),
                                                   color ? 1 : 0, timefmt, fmt);
  internal::CheckErrorCode(res);
}

/// Disables logging to the console.
inline void DisableConsoleLogging() {
  int res = internal::YOGI_ConfigureConsoleLogging(-1, 0, 0, nullptr, nullptr);
  internal::CheckErrorCode(res);
}

/// @} freefn

/// Handler function type for the log hook.
///
/// \param severity  Severity (verbosity) of the entry
/// \param timestamp UTC timestamp of the entry
/// \param tid       ID of the thread that created the entry
/// \param file      Source file name
/// \param line      Source file line number
/// \param comp      Component that created the entry
/// \param msg       Log message
using LogHookFn = std::function<void(Verbosity severity, Timestamp timestamp,
                                     int tid, std::string file, int line,
                                     std::string comp, std::string msg)>;

namespace internal {
struct LogToHookData {
  static std::mutex mutex;
  static std::unique_ptr<LogHookFn> log_hook_fn;
};

_YOGI_WEAK_SYMBOL std::mutex LogToHookData::mutex;
_YOGI_WEAK_SYMBOL std::unique_ptr<LogHookFn> LogToHookData::log_hook_fn;

}  // namespace internal

/// \addtogroup freefn
/// @{

/// Configures logging to a user-defined function.
///
/// This function can be used to get notified whenever the Yogi library itself
/// or the user produces log messages. These messages can then be processed
/// further in user code.
///
/// \param verbosity Maximum verbosity of messages to log.
/// \param fn        Callback function.
inline void SetupHookLogging(Verbosity verbosity, LogHookFn fn) {
  auto fn_ptr = std::make_unique<LogHookFn>(fn);
  static auto wrapper = [](int severity, long long timestamp, int tid,
                           const char* file, int line, const char* comp,
                           const char* msg, void* userarg) {
    auto fn_ptr = (static_cast<LogHookFn*>(userarg));
    (*fn_ptr)(
        static_cast<Verbosity>(severity),
        Timestamp::FromDurationSinceEpoch(Duration::FromNanoseconds(timestamp)),
        tid, file ? file : "", line, comp, msg ? msg : "");
  };
  std::lock_guard<std::mutex> lock(internal::LogToHookData::mutex);

  int res = internal::YOGI_ConfigureHookLogging(static_cast<int>(verbosity),
                                                wrapper, fn_ptr.get());
  internal::CheckErrorCode(res);

  internal::LogToHookData::log_hook_fn = std::move(fn_ptr);
}

/// Disables logging to user-defined functions.
inline void DisableHookLogging() {
  std::lock_guard<std::mutex> lock(internal::LogToHookData::mutex);

  int res = internal::YOGI_ConfigureHookLogging(-1, nullptr, nullptr);
  internal::CheckErrorCode(res);

  internal::LogToHookData::log_hook_fn = {};
}

/// Configures logging to a file.
///
/// This function opens a file to write library-internal and user logging
/// information to. If the file with the given filename already exists then it
/// will be overwritten.
///
/// The \p timefmt and \p fmt parameters describe the textual format for a log
/// entry. The \p filename parameter supports all placeholders that are valid
/// for \p timefmt. See the LogToConsole() method for supported placeholders.
///
/// \note
///   The color-related placeholders are ignored when writing to log files.
///
/// \param verbosity Maximum verbosity of messages to log
/// \param filename  Path to the log file (see above for placeholders)
/// \param timefmt   Format of the timestamp (see above for placeholders)
/// \param fmt       Format of a log entry (see above for placeholders)
///
/// \returns The generated filename with all placeholders resolved
inline std::string SetupFileLogging(Verbosity verbosity,
                                    const StringView& filename,
                                    const StringView& timefmt = {},
                                    const StringView& fmt = {}) {
  char genfn[256];
  int res =
      internal::YOGI_ConfigureFileLogging(static_cast<int>(verbosity), filename,
                                          genfn, sizeof(genfn), timefmt, fmt);
  internal::CheckErrorCode(res);
  return genfn;
}

/// Disables logging to a file
inline void DisableFileLogging() {
  int res = internal::YOGI_ConfigureFileLogging(-1, nullptr, nullptr, 0,
                                                nullptr, nullptr);
  internal::CheckErrorCode(res);
}

/// @} freefn

class Logger;

/// Shared pointer to a logger.
using LoggerPtr = std::shared_ptr<Logger>;

////////////////////////////////////////////////////////////////////////////////
/// Allows generating log entries.
///
/// A logger is an object used for generating log entries that are tagged with
/// the logger's component tag. A logger's component tag does not have to be
/// unique, i.e. multiple loggers can be created using identical component
/// tags.
///
/// The verbosity of a logger acts as a filter. Only messages with a verbosity
/// less than or equal to the logger's verbosity are being logged.
///
/// \note
///   The verbosity of a logger affects only messages logged through that
///   particular logger, i.e. if two loggers have identical component tags their
///   verbosity settings are still independent from each other.
////////////////////////////////////////////////////////////////////////////////
class Logger : public ObjectT<Logger> {
 public:
  /// Sets the verbosity of all loggers matching a given component tag.
  ///
  /// This function finds all loggers whose component tag matches the regular
  /// expression given in the components parameter and sets their verbosity
  /// to the value of the verbosity parameter.
  ///
  /// \param components Regex (ECMAScript) for the component tags to match
  /// \param verbosity  Maximum verbosity entries to be logged
  ///
  /// \returns Number of matching loggers
  static int SetComponentsVerbosity(const StringView& components,
                                    Verbosity verbosity) {
    int count;
    int res = internal::YOGI_LoggerSetComponentsVerbosity(
        components, static_cast<int>(verbosity), &count);
    internal::CheckErrorCode(res);
    return count;
  }

  /// Creates a logger.
  ///
  /// The verbosity of new loggers is Verbosity::kInfo by default.
  ///
  /// \param component The component tag to use
  ///
  /// \returns Newly created logger
  static LoggerPtr Create(const StringView& component) {
    return LoggerPtr(new Logger(component));
  }

  /// Returns the verbosity of the logger.
  ///
  /// \returns Verbosity of the logger
  Verbosity GetVerbosity() const {
    int vb;
    int res = internal::YOGI_LoggerGetVerbosity(GetHandle(), &vb);
    internal::CheckErrorCode(res);
    return static_cast<Verbosity>(vb);
  }

  /// Sets the verbosity of the logger.
  ///
  /// \param verbosity Verbosity
  void SetVerbosity(Verbosity verbosity) {
    int res = internal::YOGI_LoggerSetVerbosity(GetHandle(),
                                                static_cast<int>(verbosity));
    internal::CheckErrorCode(res);
  }

  /// Creates a log entry.
  ///
  /// \param severity Severity (verbosity) of the entry
  /// \param msg      Log message
  /// \param file     Source file name
  /// \param line     Source file line number
  void Log(Verbosity severity, const StringView& msg, const StringView& file,
           int line) {
    const char* short_file = file;
    if (short_file) {
      for (const char* ch = short_file; *ch; ++ch) {
        if (*ch == '/' || *ch == '\\') {
          short_file = ch + 1;
        }
      }
    }

    int res = internal::YOGI_LoggerLog(GetHandle(), static_cast<int>(severity),
                                       short_file, line, msg);
    internal::CheckErrorCode(res);
  }

  /// Creates a log entry without file and line number information.
  ///
  /// \param severity Severity (verbosity) of the entry
  /// \param msg      Log message
  void Log(Verbosity severity, const StringView& msg) {
    Log(severity, msg, {}, 0);
  }

 protected:
  // For the AppLogger
  Logger() : ObjectT(nullptr, {}) {}

 private:
  Logger(const char* component)
      : ObjectT(internal::CallApiCreate(internal::YOGI_LoggerCreate, component),
                {}) {}
};

class AppLogger;

/// Shared pointer to an app logger.
using AppLoggerPtr = std::shared_ptr<AppLogger>;

////////////////////////////////////////////////////////////////////////////////
/// Represents the App logger singleton.
///
/// The App logger always exists and uses "App" as its component tag.
/// Instances of this class can be created; however, they will always point to
/// the same logger, i.e. changing its verbosity will change the verbosity of
/// every AppLogger instance.
////////////////////////////////////////////////////////////////////////////////
class AppLogger : public Logger {
 public:
  /// Creates an app logger.
  ///
  /// \returns Newly created app logger.
  static AppLoggerPtr Create() { return AppLoggerPtr(new AppLogger()); }

  virtual std::string ToString() const override { return "AppLogger"; }
};

/// Static app logger instance to use in user code.
static const AppLoggerPtr app_logger = AppLogger::Create();

}  // namespace yogi

#endif  // YOGI_LOGGING_H
