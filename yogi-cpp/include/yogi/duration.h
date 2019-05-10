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

#ifndef YOGI_DURATION_H
#define YOGI_DURATION_H

//! \file
//!
//! Representation of time durations.

#include "internal/library.h"
#include "internal/error_code_helpers.h"
#include "string_view.h"

#include <chrono>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <type_traits>

static_assert(std::numeric_limits<double>::has_infinity,
              "Yogi needs a platform with support for infinite double values.");

namespace yogi {

_YOGI_DEFINE_API_FN(int, YOGI_FormatDuration,
                    (long long dur, int neg, char* str, int strsize,
                     const char* durfmt, const char* infstr))

class Duration;

namespace internal {

template <typename T>
inline bool IsFinite(T) {
  return true;
}

template <>
inline bool IsFinite<float>(float val) {
  return std::isfinite(val);
}

template <>
inline bool IsFinite<double>(double val) {
  return std::isfinite(val);
}

template <typename T>
inline bool IsNan(T) {
  return false;
}

template <>
inline bool IsNan<float>(float val) {
  return std::isnan(val);
}

template <>
inline bool IsNan<double>(double val) {
  return std::isnan(val);
}

enum InfinityType {
  kNegative = -1,
  kNone = 0,
  kPositive = 1,
};

inline long long AddSafely(long long a, long long b) {
  if (a > 0 && b > (std::numeric_limits<long long>::max)() - a) {
    throw ArithmeticException("Duration value overflow");
  } else if (a < 0 && b < (std::numeric_limits<long long>::min)() - a) {
    throw ArithmeticException("Duration value underflow");
  }

  return a + b;
}

template <long long Divisor>
inline double ToTotalTimeUnit(InfinityType inf_type, long long ns_count) {
  switch (inf_type) {
    case kPositive:
      return std::numeric_limits<double>::infinity();

    case kNegative:
      return -std::numeric_limits<double>::infinity();

    default:
      return static_cast<double>(ns_count) / Divisor;
  }
}

template <typename T>
inline long long Multiply(long long val, T multiplicator) {
  return static_cast<long long>(val * multiplicator);
}

template <>
inline long long Multiply<float>(long long val, float multiplicator) {
  return static_cast<long long>(static_cast<float>(val) * multiplicator);
}

template <>
inline long long Multiply<double>(long long val, double multiplicator) {
  return static_cast<long long>(static_cast<double>(val) * multiplicator);
}

template <typename T>
inline long long Divide(long long val, T divisor) {
  return static_cast<long long>(val / divisor);
}

template <>
inline long long Divide<float>(long long val, float multiplicator) {
  return static_cast<long long>(static_cast<float>(val) / multiplicator);
}

template <>
inline long long Divide<double>(long long val, double multiplicator) {
  return static_cast<long long>(static_cast<double>(val) / multiplicator);
}

template <typename T>
inline long long MultiplySafely(long long val, T multiplicator) {
  if (IsNan(multiplicator)) {
    throw ArithmeticException("Trying to multiply duration value and NaN");
  }

  if (multiplicator == T{}) {
    return 0;
  }

  if (std::abs(multiplicator) > static_cast<T>(1)) {
    auto max_val =
        Divide((std::numeric_limits<long long>::max)(), multiplicator);
    if (std::abs(val) > max_val) {
      throw ArithmeticException("Duration value overflow");
    }
  }

  return Multiply(val, multiplicator);
}

template <typename T>
void CheckDivisor(T divisor) {
  if (IsNan(divisor)) {
    throw ArithmeticException("Trying to divide duration value by NaN");
  }

  if (divisor == T{}) {
    throw ArithmeticException("Trying to divide duration value by zero");
  }
}

template <typename T>
inline long long DivideSafely(long long val, T divisor) {
  CheckDivisor(divisor);

  if (!IsFinite(divisor)) {
    return 0;
  }

  if (std::abs(divisor) < static_cast<T>(1)) {
    auto max_val = Multiply((std::numeric_limits<long long>::max)(), divisor);
    if (std::abs(val) > max_val) {
      throw ArithmeticException("Duration value overflow.");
    }
  }

  return Divide(val, divisor);
}

template <long long Multiplicator, typename T>
inline Duration DurationFromTimeUnit(T val);

}  // namespace internal

////////////////////////////////////////////////////////////////////////////////
/// Represents a time duration.
///
/// The resolution of a time duration is in nanoseconds. Durations can be
/// positive or negative. Exceptions are thrown in case of arithmetic errors.
////////////////////////////////////////////////////////////////////////////////
class Duration {
 public:
  /// Zero duration
  static const Duration kZero;

  /// Infinite duration
  static const Duration kInfinity;

  /// Negative infinite duration
  static const Duration kNegativeInfinity;

  /// Construct a duration from a number of nanoseconds
  ///
  /// \tparam T Arithmetic type of \p ns
  ///
  /// \param ns Number of nanoseconds
  ///
  /// \returns Duration instance
  template <typename T>
  static Duration FromNanoseconds(T ns) {
    return internal::DurationFromTimeUnit<1LL>(ns);
  }

  /// Construct a duration from a number of microseconds
  ///
  /// \tparam T Arithmetic type of \p us
  ///
  /// \param us Number of microseconds
  ///
  /// \returns Duration instance
  template <typename T>
  static Duration FromMicroseconds(T us) {
    return internal::DurationFromTimeUnit<1'000LL>(us);
  }

  /// Construct a duration from a number of milliseconds
  ///
  /// \tparam T Arithmetic type of \p ms
  ///
  /// \param ms Number of milliseconds
  ///
  /// \returns Duration instance
  template <typename T>
  static Duration FromMilliseconds(T ms) {
    return internal::DurationFromTimeUnit<1'000'000LL>(ms);
  }

  /// Construct a duration from a number of seconds
  ///
  /// \tparam T Arithmetic type of \p seconds
  ///
  /// \param seconds Number of seconds
  ///
  /// \returns Duration instance
  template <typename T>
  static Duration FromSeconds(T seconds) {
    return internal::DurationFromTimeUnit<1'000'000'000LL>(seconds);
  }

  /// Construct a duration from a number of minutes
  ///
  /// \tparam T Arithmetic type of \p minutes
  ///
  /// \param minutes Number of minutes
  ///
  /// \returns Duration instance
  template <typename T>
  static Duration FromMinutes(T minutes) {
    return internal::DurationFromTimeUnit<60 * 1'000'000'000LL>(minutes);
  }

  /// Construct a duration from a number of hours
  ///
  /// \tparam T Arithmetic type of \p hours
  ///
  /// \param hours Number of hours
  ///
  /// \returns Duration instance
  template <typename T>
  static Duration FromHours(T hours) {
    return internal::DurationFromTimeUnit<60 * 60 * 1'000'000'000LL>(hours);
  }

  /// Construct a duration from a number of days
  ///
  /// \tparam T Arithmetic type of \p days
  ///
  /// \param days Number of days
  ///
  /// \returns Duration instance
  template <typename T>
  static Duration FromDays(T days) {
    return internal::DurationFromTimeUnit<24 * 60 * 60 * 1'000'000'000LL>(days);
  }

  /// Constructs a zero nanoseconds duration
  Duration() : Duration(internal::kNone) {}

  /// Constructs a duration from a duration value from the standard library
  ///
  /// \tparam Rep    Arithmetic type representing the number of ticks
  /// \tparam Period Ratio type representing the tick period
  ///
  /// \param dur Duration
  template <typename Rep, typename Period>
  Duration(const std::chrono::duration<Rep, Period>& dur)
      : Duration(internal::kNone) {
    ns_count_ =
        std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
  }

  /// Nanoseconds fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Nanoseconds fraction (0-999)
  int Nanoseconds() const {
    return static_cast<int>(NanosecondsCount() % 1'000);
  }

  /// Microseconds fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Microseconds fraction (0-999)
  int Microseconds() const {
    return static_cast<int>((NanosecondsCount() / 1'000LL) % 1'000);
  }

  /// Milliseconds fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Milliseconds fraction (0-999)
  int Milliseconds() const {
    return static_cast<int>((NanosecondsCount() / 1'000'000LL) % 1'000);
  }

  /// Seconds fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Seconds fraction (0-59)
  int Seconds() const {
    return static_cast<int>(NanosecondsCount() / 1'000'000'000LL) % 60;
  }

  /// Minutes fraction of the duration
  ///
  /// \returns Minutes fraction (0-59)
  int Minutes() const {
    return static_cast<int>(NanosecondsCount() / (60 * 1'000'000'000LL)) % 60;
  }

  /// Hours fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Hours fraction (0-23)
  int Hours() const {
    return static_cast<int>(NanosecondsCount() / (60 * 60 * 1'000'000'000LL)) %
           24;
  }

  /// Days fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Days fraction
  int Days() const {
    return static_cast<int>(NanosecondsCount() /
                            (24 * 60 * 60 * 1'000'000'000LL));
  }

  /// Total number of nanoseconds
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Total number of nanoseconds
  long long NanosecondsCount() const { return ns_count_; }

  /// Total number of nanoseconds
  ///
  /// If the duration is infinite, this function will return the appropriate
  /// infinite value.
  ///
  /// \returns Total number of nanoseconds
  double TotalNanoseconds() const {
    return internal::ToTotalTimeUnit<1LL>(inf_type_, ns_count_);
  }

  /// Total number of microseconds
  ///
  /// If the duration is infinite, this function will return the appropriate
  /// infinite value.
  ///
  /// \returns Total number of microseconds
  double TotalMicroseconds() const {
    return internal::ToTotalTimeUnit<1'000LL>(inf_type_, ns_count_);
  }

  /// Total number of milliseconds
  ///
  /// If the duration is infinite, this function will return the appropriate
  /// infinite value.
  ///
  /// \returns Total number of milliseconds
  double TotalMilliseconds() const {
    return internal::ToTotalTimeUnit<1'000'000LL>(inf_type_, ns_count_);
  }

  /// Total number of seconds
  ///
  /// If the duration is infinite, this function will return the appropriate
  /// infinite value.
  ///
  /// \returns Total number of seconds
  double TotalSeconds() const {
    return internal::ToTotalTimeUnit<1'000'000'000LL>(inf_type_, ns_count_);
  }

  /// Total number of minutes
  ///
  /// If the duration is infinite, this function will return the appropriate
  /// infinite value.
  ///
  /// \returns Total number of minutes
  double TotalMinutes() const {
    return internal::ToTotalTimeUnit<60 * 1'000'000'000LL>(inf_type_,
                                                           ns_count_);
  }

  /// Total number of hours
  ///
  /// If the duration is infinite, this function will return the appropriate
  /// infinite value.
  ///
  /// \returns Total number of hours
  double TotalHours() const {
    return internal::ToTotalTimeUnit<60 * 60 * 1'000'000'000LL>(inf_type_,
                                                                ns_count_);
  }

  /// Total number of days
  ///
  /// If the duration is infinite, this function will return the appropriate
  /// infinite value.
  ///
  /// \returns Total number of days
  double TotalDays() const {
    return internal::ToTotalTimeUnit<24 * 60 * 60 * 1'000'000'000LL>(inf_type_,
                                                                     ns_count_);
  }

  /// Returns the negated duration, i.e. multiplied by -1
  ///
  /// \returns Negated duration
  Duration Negated() const {
    switch (inf_type_) {
      case internal::kPositive:
        return kNegativeInfinity;

      case internal::kNegative:
        return kInfinity;

      default:
        return FromNanoseconds(-ns_count_);
    }
  }

  /// Converts the duration to a standard library duration
  ///
  /// In case that the duration is infinite, the respective min or max value
  /// of the requested type will be returned.
  ///
  /// \tparam T Standard library duration type
  ///
  /// \returns Duration as a standard library duration
  template <typename T = std::chrono::nanoseconds>
  T ToChronoDuration() const {
    switch (inf_type_) {
      case internal::kPositive:
        return (T::max)();

      case internal::kNegative:
        return (T::min)();

      default:  // internal::kNone
        return std::chrono::duration_cast<T>(
            std::chrono::nanoseconds(ns_count_));
    }
  }

  /// Checks if the duration is finite or not
  ///
  /// \returns True if the duration is finite
  bool IsFinite() const { return inf_type_ == internal::kNone; }

  /// Converts the duration to a string.
  ///
  /// The \p dur_fmt parameter describes the format of the conversion. The
  /// following placeholders are supported:
  ///  - *%+*: Plus sign if duration is positive; minus sign if it is negative
  ///  - *%-*: Minus sign (only) if duration is negative
  ///  - *%d*: Total number of days
  ///  - *%D*: Total number of days if days > 0
  ///  - *%H*: Fractional hours (range 00 to 23)
  ///  - *%M*: Fractional minutes (range 00 to 59)
  ///  - *%S*: Fractional seconds (range 00 to 59)
  ///  - *%T*: Equivalent to %H:%M:%S
  ///  - *%3*: Fractional milliseconds (range 000 to 999)
  ///  - *%6*: Fractional microseconds (range 000 to 999)
  ///  - *%9*: Fractional nanoseconds (range 000 to 999)
  ///
  /// The \p inf_fmt parameter describes the format to use for infinite
  /// durations. The following placeholders are supported:
  ///  - *%+*: Plus sign if duration is positive; minus sign if it is negative
  ///  - *%-*: Minus sign (only) if duration is negative
  ///
  /// By default, the duration will be formatted in the format
  /// "-23d 04:19:33.123456789". If the duration is infinite, then the string
  /// constructed using \p inf_fmt will be returned ("inf" and "-inf"
  /// respectively by default).
  ///
  /// \param[in] dur_fmt Format of the duration string
  /// \param[in] inf_fmt Format to use for infinity
  ///
  /// \returns The formatted duration string
  std::string Format(const StringView& dur_fmt = {},
                     const StringView& inf_fmt = {}) const {
    char str[128];
    int res = internal::YOGI_FormatDuration(IsFinite() ? ns_count_ : -1,
                                            ns_count_ < 0 ? 1 : 0, str,
                                            sizeof(str), dur_fmt, inf_fmt);
    internal::CheckErrorCode(res);
    return str;
  }

  /// Converts the duration to a string
  ///
  /// The format of the string will be "-23d 04:19:33.123456789".
  ///
  /// \returns Duration as a string
  std::string ToString() const { return Format(); }

  Duration operator+(const Duration& rhs) const {
    if (inf_type_ == internal::kNone && rhs.inf_type_ == internal::kNone) {
      return FromNanoseconds(internal::AddSafely(ns_count_, rhs.ns_count_));
    }

    auto inf_type_sum = inf_type_ + rhs.inf_type_;
    if (inf_type_sum == internal::kNone) {
      throw ArithmeticException(
          "Trying to add positive and negative infinite duration values.");
    }

    return inf_type_sum > 0 ? kInfinity : kNegativeInfinity;
  }

  Duration operator-(const Duration& rhs) const {
    return *this + rhs.Negated();
  }

  template <typename T>
  Duration operator*(T rhs) const {
    static_assert(std::is_arithmetic<T>::value,
                  "T must be an arithmetic type.");

    if (inf_type_ != internal::kNone && rhs == T{}) {
      throw ArithmeticException(
          "Trying to multiply infinite duration value and zero.");
    }

    if (inf_type_ == internal::kNone && internal::IsFinite(rhs)) {
      return FromNanoseconds(internal::MultiplySafely(ns_count_, rhs));
    }

    auto rhs_inf_type = rhs > T{} ? internal::kPositive : internal::kNegative;
    return inf_type_ == rhs_inf_type ? kInfinity : kNegativeInfinity;
  }

  template <typename T>
  Duration operator/(T rhs) const {
    static_assert(std::is_arithmetic<T>::value,
                  "T must be an arithmetic type.");

    if (inf_type_ == internal::kNone) {
      return FromNanoseconds(internal::DivideSafely(ns_count_, rhs));
    }

    internal::CheckDivisor(rhs);
    auto rhs_inf_type = rhs > T{} ? internal::kPositive : internal::kNegative;
    return inf_type_ == rhs_inf_type ? kInfinity : kNegativeInfinity;
  }

  Duration& operator+=(const Duration& rhs) {
    *this = *this + rhs;
    return *this;
  }

  Duration& operator-=(const Duration& rhs) {
    *this = *this - rhs;
    return *this;
  }

  Duration& operator*=(int rhs) {
    *this = *this * rhs;
    return *this;
  }

  Duration& operator*=(float rhs) {
    *this = *this * rhs;
    return *this;
  }

  Duration& operator/=(int rhs) {
    *this = *this / rhs;
    return *this;
  }

  Duration& operator/=(float rhs) {
    *this = *this / rhs;
    return *this;
  }

  bool operator==(const Duration& rhs) const {
    if (inf_type_ == rhs.inf_type_ && inf_type_ != internal::kNone) {
      return true;
    }

    return ns_count_ == rhs.ns_count_;
  }

  bool operator!=(const Duration& rhs) const { return !(*this == rhs); }

  bool operator<(const Duration& rhs) const {
    if (inf_type_ == rhs.inf_type_ && inf_type_ == internal::kNone) {
      return ns_count_ < rhs.ns_count_;
    }

    return inf_type_ < rhs.inf_type_;
  }

  bool operator>(const Duration& rhs) const {
    if (inf_type_ == rhs.inf_type_ && inf_type_ == internal::kNone) {
      return ns_count_ > rhs.ns_count_;
    }

    return inf_type_ > rhs.inf_type_;
  }

  bool operator<=(const Duration& rhs) const { return !(*this > rhs); }

  bool operator>=(const Duration& rhs) const { return !(*this < rhs); }

 private:
  explicit Duration(internal::InfinityType inf_type) : inf_type_(inf_type) {
    switch (inf_type) {
      case internal::kPositive:
        ns_count_ = (std::numeric_limits<long long>::max)();
        break;

      case internal::kNegative:
        ns_count_ = (std::numeric_limits<long long>::min)();
        break;

      default:  // internal::kNone
        ns_count_ = 0;
        break;
    }
  }

  internal::InfinityType inf_type_;
  long long ns_count_;
};

_YOGI_WEAK_SYMBOL const Duration Duration::kZero;
_YOGI_WEAK_SYMBOL const Duration Duration::kInfinity(internal::kPositive);
_YOGI_WEAK_SYMBOL const Duration
    Duration::kNegativeInfinity(internal::kNegative);

namespace internal {

template <long long Multiplicator, typename T>
inline Duration DurationFromTimeUnitImpl(T val) {
  static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type.");

  if (IsNan(val)) {
    throw ArithmeticException("Cannot construct duration from NaN");
  }

  if (IsFinite(val)) {
    auto max_val =
        static_cast<T>((std::numeric_limits<T>::max)() / Multiplicator);
    if (std::abs(val) > max_val) {
      throw ArithmeticException("Duration value overflow");
    }

    return Duration(std::chrono::nanoseconds(
        static_cast<std::chrono::nanoseconds::rep>(val * Multiplicator)));
  }

  // infinite
  return val < 0 ? Duration::kNegativeInfinity : Duration::kInfinity;
}

template <bool IsIntegral>
struct DurationFromTimeUnitSelector {
  template <long long Multiplicator, typename T>
  inline static Duration Fn(T val) {
    return DurationFromTimeUnitImpl<Multiplicator, T>(val);
  }
};

template <>
struct DurationFromTimeUnitSelector<true> {
  template <long long Multiplicator, typename T>
  inline static Duration Fn(T val) {
    return DurationFromTimeUnitImpl<Multiplicator, long long>(val);
  }
};

template <long long Multiplicator, typename T>
inline Duration DurationFromTimeUnit(T val) {
  using Selector = DurationFromTimeUnitSelector<std::is_integral<T>::value>;
  return Selector::template Fn<Multiplicator>(val);
}

}  // namespace internal
}  // namespace yogi

#endif  // YOGI_DURATION_H
