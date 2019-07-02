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

#pragma once

#include "../../config.h"
#include "../../api/object.h"
#include "detail/console_log_sink.h"
#include "detail/hook_log_sink.h"
#include "detail/file_log_sink.h"

#include <atomic>
#include <mutex>
#include <vector>
#include <sstream>
#include <regex>

// To be used only within class that inherit LoggerUser and in files that have
// defined a global internall logger using YOGI_DEFINE_INTERNAL_LOGGER().
#define LOG_FAT(...) YOGI_INTERNAL_LOG(kFatal, __VA_ARGS__)
#define LOG_ERR(...) YOGI_INTERNAL_LOG(kError, __VA_ARGS__)
#define LOG_WRN(...) YOGI_INTERNAL_LOG(kWarning, __VA_ARGS__)
#define LOG_IFO(...) YOGI_INTERNAL_LOG(kInfo, __VA_ARGS__)
#define LOG_DBG(...) YOGI_INTERNAL_LOG(kDebug, __VA_ARGS__)
#define LOG_TRC(...) YOGI_INTERNAL_LOG(kTrace, __VA_ARGS__)

#define YOGI_INTERNAL_LOG(severity, stream)                                 \
  {                                                                         \
    auto& logger = file_global_internal_logger;                             \
    if (::api::Verbosity::severity <= (logger)->GetVerbosity()) {           \
      std::stringstream ss;                                                 \
      if (this->HasLoggingPrefix()) ss << this->GetLoggingPrefix() << ": "; \
      ss << stream;                                                         \
      (logger)->Log(::api::Verbosity::severity, __FILE__, __LINE__,         \
                    ss.str().c_str());                                      \
    }                                                                       \
  }

#define YOGI_DEFINE_INTERNAL_LOGGER(component)                     \
  namespace {                                                      \
  const objects::log::LoggerPtr file_global_internal_logger =      \
      objects::log::Logger::CreateStaticInternalLogger(component); \
  }

namespace objects {
namespace log {

class Logger : public api::ExposedObjectT<Logger, api::ObjectType::kLogger> {
 public:
  static api::Verbosity StringToVerbosity(const std::string& str);
  static void SetSink(detail::ConsoleLogSinkPtr&& sink);
  static void SetSink(detail::HookLogSinkPtr&& sink);
  static void SetSink(detail::FileLogSinkPtr&& sink);
  static std::shared_ptr<Logger> GetAppLogger() { return app_logger_; }
  static std::shared_ptr<Logger> CreateStaticInternalLogger(
      const std::string& component);  // Internal loggers must be static!
  static const std::vector<std::weak_ptr<Logger>>& GetInternalLoggers() {
    return InternalLoggers();
  }
  static int SetComponentsVerbosity(const std::regex& components,
                                    api::Verbosity verbosity);

  Logger(std::string component);

  const std::string& GetComponent() const { return component_; }
  api::Verbosity GetVerbosity() const { return verbosity_; }
  void SetVerbosity(api::Verbosity verbosity) { verbosity_ = verbosity; }
  void Log(api::Verbosity severity, const char* file, int line,
           const char* msg);

 private:
  static std::mutex sinks_mutex_;
  static detail::LogSinkPtr console_sink_;
  static detail::LogSinkPtr hook_sink_;
  static detail::LogSinkPtr file_sink_;
  static std::shared_ptr<Logger> app_logger_;

  static std::vector<std::weak_ptr<Logger>>& InternalLoggers();

  const std::string component_;
  std::atomic<api::Verbosity> verbosity_;
};

typedef std::shared_ptr<Logger> LoggerPtr;

class LoggerUser {
 public:
  virtual ~LoggerUser() {}

  bool HasLoggingPrefix() const { return !prefix_.empty(); }
  const std::string& GetLoggingPrefix() const { return prefix_; }

 protected:
  void SetLoggingPrefix(const api::ExposedObject& obj);
  void SetLoggingPrefix(std::string prefix);
  void ClearLoggingPrefix();

 private:
  std::string prefix_;
};

}  // namespace log
}  // namespace objects
