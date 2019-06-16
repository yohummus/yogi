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

#include "log_sink.h"

namespace objects {
namespace log {
namespace detail {

void LogSink::Publish(api::Verbosity severity,
                      const utils::Timestamp& timestamp, int tid,
                      const char* file, int line, const std::string& component,
                      const char* msg) {
  if (severity <= verbosity_) {
    WriteEntry(severity, timestamp, tid, file, line, component, msg);
  }
}

}  // namespace detail
}  // namespace log
}  // namespace objects
