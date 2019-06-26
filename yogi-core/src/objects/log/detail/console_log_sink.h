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
#include "text_based_log_sink.h"

namespace objects {
namespace log {
namespace detail {

class ConsoleLogSink : public TextBasedLogSink {
 public:
  ConsoleLogSink(api::Verbosity verbosity, FILE* stream, bool color,
                 std::string time_fmt, std::string fmt);

 protected:
  virtual void WritePartialOutput(const std::string& str) override;
  virtual void SetOutputColors(api::Verbosity severity) override;
  virtual void ResetOutputColors() override;
  virtual void Flush() override;

 private:
  FILE* const stream_;
};

typedef std::unique_ptr<ConsoleLogSink> ConsoleLogSinkPtr;

}  // namespace detail
}  // namespace log
}  // namespace objects
