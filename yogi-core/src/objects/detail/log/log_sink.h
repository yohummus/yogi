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

#include "../../../config.h"
#include "../../../api/enums.h"
#include "../../../utils/timestamp.h"

#include <memory>

namespace objects {
namespace detail {

class LogSink {
 public:
  LogSink(api::Verbosity verbosity) : verbosity_(verbosity) {}
  virtual ~LogSink() {}

  void Publish(api::Verbosity severity, const utils::Timestamp& timestamp,
               int tid, const char* file, int line,
               const std::string& component, const char* msg);

 protected:
  virtual void WriteEntry(api::Verbosity severity,
                          const utils::Timestamp& timestamp, int tid,
                          const char* file, int line,
                          const std::string& component, const char* msg) = 0;

 private:
  const api::Verbosity verbosity_;
};

typedef std::unique_ptr<LogSink> LogSinkPtr;

}  // namespace detail
}  // namespace objects
