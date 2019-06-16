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

#include "timestamp.h"
#include "../api/errors.h"

#include <boost/algorithm/string.hpp>
#include <time.h>
#include <stdio.h>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace utils {

Timestamp Timestamp::Now() {
  return Timestamp(std::chrono::time_point_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now()));
}

Timestamp Timestamp::Parse(const std::string& str, const std::string& fmt) {
  std::tm tm = {};
  tm.tm_year = 70;
  tm.tm_mon = 1;
  tm.tm_mday = 1;
  tm.tm_isdst = 0;

  int msec = 0;
  int usec = 0;
  int nsec = 0;

  auto strit = str.begin();
  bool special = false;
  for (auto fmtch : fmt) {
    if (special) {
      switch (fmtch) {
        case 'Y':
          tm.tm_year = ParseNumber<4>(str, &strit, 0070, 9999) - 1900;
          break;

        case 'm':
          tm.tm_mon = ParseNumber<2>(str, &strit, 01, 12) - 1;
          break;

        case 'd':
          tm.tm_mday = ParseNumber<2>(str, &strit, 01, 31);
          break;

        case 'F':
          tm.tm_year = ParseNumber<4>(str, &strit, 0070, 9999) - 1900;
          MatchChar(str, &strit, '-');
          tm.tm_mon = ParseNumber<2>(str, &strit, 01, 12) - 1;
          MatchChar(str, &strit, '-');
          tm.tm_mday = ParseNumber<2>(str, &strit, 01, 31);
          break;

        case 'H':
          tm.tm_hour = ParseNumber<2>(str, &strit, 00, 23);
          break;

        case 'M':
          tm.tm_min = ParseNumber<2>(str, &strit, 00, 59);
          break;

        case 'S':
          tm.tm_sec = ParseNumber<2>(str, &strit, 00, 59);
          break;

        case 'T':
          tm.tm_hour = ParseNumber<2>(str, &strit, 00, 23);
          MatchChar(str, &strit, ':');
          tm.tm_min = ParseNumber<2>(str, &strit, 00, 59);
          MatchChar(str, &strit, ':');
          tm.tm_sec = ParseNumber<2>(str, &strit, 00, 59);
          break;

        case '3':
          msec = ParseNumber<3>(str, &strit, 000, 999);
          break;

        case '6':
          usec = ParseNumber<3>(str, &strit, 000, 999);
          break;

        case '9':
          nsec = ParseNumber<3>(str, &strit, 000, 999);
          break;

        default:
          throw api::Error(YOGI_ERR_INVALID_TIME_FORMAT);
      }

      special = false;
    } else {
      if (fmtch == '%') {
        special = true;
      } else {
        MatchChar(str, &strit, fmtch);
      }
    }
  }

  if (strit != str.end()) {
    throw api::Error(YOGI_ERR_PARSING_TIME_FAILED);
  }

#ifdef _WIN32
  auto time = _mkgmtime(&tm);
#else
  auto time = timegm(&tm);
#endif

 if (time == -1) {
   throw api::Error(YOGI_ERR_PARSING_TIME_FAILED);
 }

  std::chrono::nanoseconds ns = {};
  ns += std::chrono::seconds(time);
  ns += std::chrono::milliseconds(msec);
  ns += std::chrono::microseconds(usec);
  ns += std::chrono::nanoseconds(nsec);

  return Timestamp(ns);
}

std::string Timestamp::ToFormattedString(std::string fmt) const {
  auto time = std::chrono::system_clock::to_time_t(
      std::chrono::time_point_cast<std::chrono::system_clock::duration>(time_));

  std::tm tm = {};
#ifdef _WIN32
  gmtime_s(&tm, &time);
#else
  gmtime_r(&time, &tm);
#endif

  char tmp[4];
  sprintf(tmp, "%03i", static_cast<int>(MillisecondsFraction()));
  boost::replace_all(fmt, "%3", tmp);
  sprintf(tmp, "%03i", static_cast<int>(MicrosecondsFraction()));
  boost::replace_all(fmt, "%6", tmp);
  sprintf(tmp, "%03i", static_cast<int>(NanosecondsFraction()));
  boost::replace_all(fmt, "%9", tmp);

  std::ostringstream ss;
  ss << std::put_time(&tm, fmt.c_str());
  return ss.str();
}

std::string Timestamp::ToJavaScriptString() const {
  return ToFormattedString("%FT%T.%3Z");
}

template <int Width>
int Timestamp::ParseNumber(const std::string& str,
                           std::string::const_iterator* it, int min, int max) {
  if (std::distance(*it, str.end()) < Width) {
    throw api::Error(YOGI_ERR_PARSING_TIME_FAILED);
  }

  char buf[Width + 1];
  std::copy_n(*it, Width, buf);
  buf[Width] = '\0';

  int multiplier = 1;
  int number = 0;
  for (int i = Width - 1; i >= 0; --i) {
    int digit = buf[i] - '0';
    if (digit < 0 || digit > 9) {
      throw api::Error(YOGI_ERR_PARSING_TIME_FAILED);
    }

    number += digit * multiplier;
    multiplier *= 10;
  }

  if (number < min || number > max) {
    throw api::Error(YOGI_ERR_PARSING_TIME_FAILED);
  }

  *it += Width;;
  return number;
}

void Timestamp::MatchChar(const std::string& str,
                          std::string::const_iterator* it, char ch) {
  if (*it == str.end() || **it != ch) {
    throw api::Error(YOGI_ERR_PARSING_TIME_FAILED);
  }

  ++*it;
}

}  // namespace utils
