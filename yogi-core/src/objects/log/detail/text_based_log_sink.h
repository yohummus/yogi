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

#include "../../../config.h"
#include "log_sink.h"

#include <string>

namespace objects {
namespace log {
namespace detail {

class TextBasedLogSink : public LogSink {
 public:
  TextBasedLogSink(api::Verbosity verbosity, std::string time_fmt,
                   std::string fmt, bool ignore_colour);

 protected:
  virtual void WriteEntry(api::Verbosity severity,
                          const utils::Timestamp& timestamp, int tid,
                          const char* file, int line,
                          const std::string& component,
                          const char* msg) override;

  virtual void WritePartialOutput(const std::string& str) = 0;
  virtual void SetOutputColors(api::Verbosity) {}
  virtual void ResetOutputColors() {}
  virtual void Flush() {}

 private:
  const std::string time_fmt_;
  const std::string fmt_;
  const bool ignore_colour_;
};

}  // namespace detail
}  // namespace log
}  // namespace objects
