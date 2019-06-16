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

#include "logger.h"
#include "../api/constants.h"
#include "../utils/system.h"

#include <boost/algorithm/string.hpp>
#include <stdexcept>

using namespace std::string_literals;

namespace objects {

api::Verbosity Logger::StringToVerbosity(const std::string& str) {
  if (boost::iequals(str, "NONE")) return api::Verbosity::kNone;
  if (boost::iequals(str, "FATAL")) return api::Verbosity::kFatal;
  if (boost::iequals(str, "ERROR")) return api::Verbosity::kError;
  if (boost::iequals(str, "WARNING")) return api::Verbosity::kWarning;
  if (boost::iequals(str, "INFO")) return api::Verbosity::kInfo;
  if (boost::iequals(str, "DEBUG")) return api::Verbosity::kDebug;
  if (boost::iequals(str, "TRACE")) return api::Verbosity::kTrace;

  throw std::runtime_error("Invalid verbosity \""s + str + "\"");
}

void Logger::SetSink(detail::ConsoleLogSinkPtr&& sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  console_sink_ = std::move(sink);
}

void Logger::SetSink(detail::HookLogSinkPtr&& sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  hook_sink_ = std::move(sink);
}

void Logger::SetSink(detail::FileLogSinkPtr&& sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  file_sink_ = std::move(sink);
}

int Logger::SetComponentsVerbosity(const std::regex& components,
                                   api::Verbosity verbosity) {
  int count = 0;
  std::smatch m;

  auto fn = [&](const LoggerPtr& log) {
    if (std::regex_match(log->GetComponent(), m, components)) {
      log->SetVerbosity(verbosity);
      ++count;
    }
  };

  // App logger
  fn(GetAppLogger());

  // Loggers created by the user
  for (auto& log : api::ObjectRegister::GetAll<Logger>()) {
    fn(log);
  }

  // Internal loggers
  for (auto& weak_log : GetInternalLoggers()) {
    auto log = weak_log.lock();
    if (log) {
      fn(log);
    }
  }

  return count;
}

LoggerPtr Logger::CreateStaticInternalLogger(const std::string& component) {
  auto logger = std::make_shared<Logger>(std::string("Yogi.") + component);
  InternalLoggers().push_back(logger);
  return logger;
}

Logger::Logger(std::string component)
    : component_(component),
      verbosity_(static_cast<api::Verbosity>(api::kDefaultLoggerVerbosity)) {}

void Logger::Log(api::Verbosity severity, const char* file, int line,
                 const char* msg) {
  if (severity > verbosity_) {
    return;
  }

  auto timestamp = utils::Timestamp::Now();
  int tid = utils::GetCurrentThreadId();

  std::lock_guard<std::mutex> lock(sinks_mutex_);
  detail::LogSinkPtr* sinks[] = {&console_sink_, &hook_sink_, &file_sink_};
  for (auto sink : sinks) {
    if (*sink) {
      (*sink)->Publish(severity, timestamp, tid, file, line, component_, msg);
    }
  }
}

std::vector<std::weak_ptr<Logger>>& Logger::InternalLoggers() {
  static std::vector<std::weak_ptr<Logger>> vec;
  return vec;
}

std::mutex Logger::sinks_mutex_;
detail::LogSinkPtr Logger::console_sink_;
detail::LogSinkPtr Logger::hook_sink_;
detail::LogSinkPtr Logger::file_sink_;
LoggerPtr Logger::app_logger_ = Logger::Create("App");

void LoggerUser::SetLoggingPrefix(const api::ExposedObject& obj) {
  YOGI_ASSERT(!HasLoggingPrefix());
  prefix_ = obj.Format("[$x]");
}

void LoggerUser::SetLoggingPrefix(std::string prefix) { prefix_ = prefix; }

}  // namespace objects
