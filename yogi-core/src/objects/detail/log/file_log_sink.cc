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

#include "file_log_sink.h"
#include "../../../api/errors.h"

namespace objects {
namespace detail {

FileLogSink::FileLogSink(api::Verbosity verbosity, std::string filename,
                         std::string time_fmt, std::string fmt)
    : TextBasedLogSink(verbosity, time_fmt, fmt, true),
      filename_(utils::Timestamp::Now().ToFormattedString(filename)) {
  file_.open(filename_, std::ios::out | std::ios::trunc);
  if (!file_.is_open()) {
    throw api::Error(YOGI_ERR_OPEN_FILE_FAILED);
  }
}

void FileLogSink::WritePartialOutput(const std::string& str) { file_ << str; }

void FileLogSink::Flush() { file_ << std::flush; }

}  // namespace detail
}  // namespace objects
