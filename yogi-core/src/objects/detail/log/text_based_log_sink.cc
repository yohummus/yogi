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

#include "text_based_log_sink.h"
#include "../../../utils/system.h"

#include <sstream>

namespace objects {
namespace detail {

TextBasedLogSink::TextBasedLogSink(api::Verbosity verbosity,
                                   std::string time_fmt, std::string fmt,
                                   bool ignore_colour)
    : LogSink(verbosity),
      time_fmt_(time_fmt),
      fmt_(fmt),
      ignore_colour_(ignore_colour) {}

void TextBasedLogSink::WriteEntry(api::Verbosity severity,
                                  const utils::Timestamp& timestamp, int tid,
                                  const char* file, int line,
                                  const std::string& component,
                                  const char* msg) {
  std::stringstream ss;
  bool colour_cleared = true;

  std::string::size_type old_pos = 0;
  std::string::size_type pos = fmt_.find('$');
  while (pos != std::string::npos) {
    if (pos > old_pos) {
      ss.write(fmt_.c_str() + old_pos,
               static_cast<std::streamsize>(pos - old_pos));
    }

    switch (fmt_[pos + 1]) {
      case 't':
        ss << timestamp.ToFormattedString(time_fmt_);
        break;

      case 'P':
        ss << utils::GetProcessId();
        break;

      case 'T':
        ss << tid;
        break;

      case 's':
        switch (severity) {
          case api::kFatal:
            ss << "FAT";
            break;

          case api::kError:
            ss << "ERR";
            break;

          case api::kWarning:
            ss << "WRN";
            break;

          case api::kInfo:
            ss << "IFO";
            break;

          case api::kDebug:
            ss << "DBG";
            break;

          case api::kTrace:
            ss << "TRC";
            break;

          default:
            YOGI_NEVER_REACHED;
            break;
        }
        break;

      case 'm':
        ss << msg;
        break;

      case 'f':
        ss << file;
        break;

      case 'l':
        ss << line;
        break;

      case 'c':
        ss << component;
        break;

      case '<':
        if (!ignore_colour_ && colour_cleared) {
          if (ss.rdbuf()->in_avail()) {
            WritePartialOutput(ss.str());
            ss.str(std::string());
          }

          SetOutputColors(severity);
          colour_cleared = false;
        }
        break;

      case '>':
        if (!ignore_colour_ && !colour_cleared) {
          if (ss.rdbuf()->in_avail()) {
            WritePartialOutput(ss.str());
            ss.str(std::string());
          }

          ResetOutputColors();
          colour_cleared = true;
        }
        break;

      case '$':
        ss << '$';
        break;
    }

    old_pos = pos + 2;  // skip placeholder
    pos = fmt_.find('$', old_pos);
  }

  ss << fmt_.substr(old_pos);
  ss << std::endl;
  WritePartialOutput(ss.str());
  if (!colour_cleared) {
    ResetOutputColors();
  }

  Flush();
}

}  // namespace detail
}  // namespace objects
