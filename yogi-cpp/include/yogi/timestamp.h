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

#ifndef YOGI_TIMESTAMP_H
#define YOGI_TIMESTAMP_H

//! \file
//!
//! Representation of points in time.

#include "internal/library.h"
#include "internal/error_code_helpers.h"
#include "string_view.h"
#include "duration.h"

#include <chrono>
#include <sstream>
#include <iomanip>

namespace yogi {

_YOGI_DEFINE_API_FN(int, YOGI_GetCurrentTime, (long long* timestamp))

_YOGI_DEFINE_API_FN(int, YOGI_FormatTime,
                    (long long timestamp, char* str, int strsize,
                     const char* timefmt))

_YOGI_DEFINE_API_FN(int, YOGI_ParseTime,
                    (long long* timestamp, const char* str,
                     const char* timefmt))

////////////////////////////////////////////////////////////////////////////////
/// Represents a UTC timestamp.
///
/// Timestamps are expressed in nanoseconds since 01/01/1970 UTC.
////////////////////////////////////////////////////////////////////////////////
class Timestamp {
 public:
  /// Creates a timestamp from a duration since the epoch 01/01/1970 UTC.
  ///
  /// \param dur_since_epoch %Duration since the epoch.
  ///
  /// \returns %Timestamp instance.
  static Timestamp FromDurationSinceEpoch(const Duration& dur_since_epoch) {
    if (!dur_since_epoch.IsFinite() || dur_since_epoch < Duration::kZero) {
      throw ArithmeticException(
          "Invalid duration value for use as a timestamp");
    }

    Timestamp t;
    t.dur_since_epoch_ = dur_since_epoch;
    return t;
  }

  /// Creates a timestamp from the current time.
  ///
  /// \returns %Timestamp corresponding to the current time.
  static Timestamp Now() {
    long long timestamp;
    int res = internal::YOGI_GetCurrentTime(&timestamp);
    internal::CheckErrorCode(res);

    return FromDurationSinceEpoch(Duration::FromNanoseconds(timestamp));
  }

  /// Converts a string into a timestamp.
  ///
  /// The \p timefmt parameter describes the format of the conversion. The
  /// following placeholders are supported:
  ///  - \c \%Y: Four digit year
  ///  - \c \%m: Month name as a decimal 01 to 12
  ///  - \c \%d: Day of the month as decimal 01 to 31
  ///  - \c \%F: Equivalent to %Y-%m-%d (the ISO 8601 date format)
  ///  - \c \%H: The hour as a decimal number using a 24-hour clock (00 to 23)
  ///  - \c \%M: The minute as a decimal 00 to 59
  ///  - \c \%S: Seconds as a decimal 00 to 59
  ///  - \c \%T: Equivalent to %H:%M:%S (the ISO 8601 time format)
  ///  - \c \%3: Milliseconds as decimal number 000 to 999
  ///  - \c \%6: Microseconds as decimal number 000 to 999
  ///  - \c \%9: Nanoseconds as decimal number 000 to 999
  ///
  /// \note
  ///   The placeholder syntax is a modulo sign followed by a single character.
  ///   Any additional characters shown above are for Doxygen.
  ///
  /// By default, the string \p str will be parsed in the format
  /// "2018-04-23T18:25:43.511Z".
  ///
  /// \param str     The string to parse.
  /// \param timefmt Format of the time string.
  ///
  /// \returns The parsed timestamp.
  static Timestamp Parse(const StringView& str,
                         const StringView& timefmt = {}) {
    long long timestamp;
    int res = internal::YOGI_ParseTime(&timestamp, str, timefmt);
    internal::CheckErrorCode(res);
    return FromDurationSinceEpoch(Duration::FromNanoseconds(timestamp));
  }

  /// Constructs the timestamp using the epoch 01/01/1970 UTC.
  Timestamp() {}

  /// Returns the duration since 01/01/1970 UTC.
  ///
  /// \returns %Duration since 01/01/1970 UTC.
  Duration DurationSinceEpoch() const { return dur_since_epoch_; }

  /// Returns the nanoseconds fraction of the timestamp.
  ///
  /// \returns Nanoseconds fraction of the timestamp (0-999).
  int Nanoseconds() const { return dur_since_epoch_.Nanoseconds(); }

  /// Returns the microseconds fraction of the timestamp.
  ///
  /// \returns Microseconds fraction of the timestamp (0-999).
  int Microseconds() const { return dur_since_epoch_.Microseconds(); }

  /// Returns the Milliseconds fraction of the timestamp.
  ///
  /// \returns Milliseconds fraction of the timestamp (0-999).
  int Milliseconds() const { return dur_since_epoch_.Milliseconds(); }

  /// Converts the timestamp to a string.
  ///
  /// The \p timefmt parameter describes the format of the conversion. The
  /// following placeholders are supported:
  ///  - \c \%Y: Four digit year
  ///  - \c \%m: Month name as a decimal 01 to 12
  ///  - \c \%d: Day of the month as decimal 01 to 31
  ///  - \c \%F: Equivalent to %Y-%m-%d (the ISO 8601 date format)
  ///  - \c \%H: The hour as a decimal number using a 24-hour clock (00 to 23)
  ///  - \c \%M: The minute as a decimal 00 to 59
  ///  - \c \%S: Seconds as a decimal 00 to 59
  ///  - \c \%T: Equivalent to %H:%M:%S (the ISO 8601 time format)
  ///  - \c \%3: Milliseconds as decimal number 000 to 999
  ///  - \c \%6: Microseconds as decimal number 000 to 999
  ///  - \c \%9: Nanoseconds as decimal number 000 to 999
  ///
  /// \note
  ///   The placeholder syntax is a modulo sign followed by a single character.
  ///   Any additional characters shown above are for Doxygen.
  ///
  /// By default, the timestamp will be formatted as an ISO-8601 string with
  /// up to millisecond resolution, e.g. "2018-04-23T18:25:43.511Z".
  ///
  /// \param timefmt Format of the time string.
  ///
  /// \returns The formatted time string.
  std::string Format(const StringView& timefmt = {}) const {
    char str[128];
    int res = internal::YOGI_FormatTime(dur_since_epoch_.NanosecondsCount(),
                                        str, sizeof(str), timefmt);
    internal::CheckErrorCode(res);
    return str;
  }

  /// Returns the time in ISO-8601 format up to milliseonds.
  ///
  /// Example: "2018-04-23T18:25:43.511Z".
  ///
  /// \returns The time in ISO-8601 format.
  std::string ToString() const { return Format(); }

  Timestamp operator+(const Duration& dur) const {
    return FromDurationSinceEpoch(dur_since_epoch_ + dur);
  }

  Timestamp operator-(const Duration& dur) const {
    return FromDurationSinceEpoch(dur_since_epoch_ - dur);
  }

  Duration operator-(const Timestamp& rhs) const {
    return dur_since_epoch_ - rhs.dur_since_epoch_;
  }

  Timestamp& operator+=(const Duration& dur) {
    *this = *this + dur;
    return *this;
  }

  Timestamp& operator-=(const Duration& dur) {
    *this = *this - dur;
    return *this;
  }

  bool operator==(const Timestamp& rhs) const {
    return dur_since_epoch_ == rhs.dur_since_epoch_;
  }

  bool operator!=(const Timestamp& rhs) const {
    return dur_since_epoch_ != rhs.dur_since_epoch_;
  }

  bool operator<(const Timestamp& rhs) const {
    return dur_since_epoch_ < rhs.dur_since_epoch_;
  }

  bool operator>(const Timestamp& rhs) const {
    return dur_since_epoch_ > rhs.dur_since_epoch_;
  }

  bool operator<=(const Timestamp& rhs) const {
    return dur_since_epoch_ <= rhs.dur_since_epoch_;
  }

  bool operator>=(const Timestamp& rhs) const {
    return dur_since_epoch_ >= rhs.dur_since_epoch_;
  }

 private:
  Duration dur_since_epoch_;
};

}  // namespace yogi

#endif  // YOGI_TIMESTAMP_H
