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

#include "../config.h"

#include <chrono>

namespace utils {

class Timestamp {
 public:
  static Timestamp Now();

  Timestamp() {}
  explicit Timestamp(const std::chrono::nanoseconds& nanoseconds_since_epoch)
      : time_(TimePoint() + nanoseconds_since_epoch){};

  std::chrono::nanoseconds NanosecondsSinceEpoch() const {
    return time_.time_since_epoch();
  }

  int NanosecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch().count() / 1) % 1000);
  }

  int MicrosecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch().count() / 1000) % 1000);
  }

  int MillisecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch().count() / 1000000) % 1000);
  }

  static Timestamp Parse(const std::string& str, const std::string& fmt);
  std::string ToFormattedString(std::string fmt) const;
  std::string ToJavaScriptString() const;

  Timestamp operator+(const std::chrono::nanoseconds& ns) const {
    return Timestamp(NanosecondsSinceEpoch() + ns);
  }

  Timestamp operator-(const std::chrono::nanoseconds& ns) const {
    return Timestamp(NanosecondsSinceEpoch() - ns);
  }

  bool operator==(const Timestamp& rhs) const {
    return NanosecondsSinceEpoch() == rhs.NanosecondsSinceEpoch();
  }

  bool operator!=(const Timestamp& rhs) const {
    return NanosecondsSinceEpoch() != rhs.NanosecondsSinceEpoch();
  }

  bool operator<(const Timestamp& rhs) const {
    return NanosecondsSinceEpoch() < rhs.NanosecondsSinceEpoch();
  }

  bool operator>(const Timestamp& rhs) const {
    return NanosecondsSinceEpoch() > rhs.NanosecondsSinceEpoch();
  }

 private:
  typedef std::chrono::time_point<std::chrono::system_clock,
                                  std::chrono::nanoseconds>
      TimePoint;

  template <int Width>
  static int ParseNumber(const std::string& str,
                         std::string::const_iterator* it, int min, int max);

  static void MatchChar(const std::string& str, std::string::const_iterator* it,
                        char ch);

  Timestamp(const TimePoint& time) : time_(time) {}

  TimePoint time_;
};

}  // namespace utils
