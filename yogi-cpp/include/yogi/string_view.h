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

#ifndef YOGI_STRING_VIEW_H
#define YOGI_STRING_VIEW_H

//! \file
//!
//! Helpers for passing strings to functions.

#include <string>
#include <cstring>

namespace yogi {

////////////////////////////////////////////////////////////////////////////////
/// Helper class for passing different types of strings to functions that
/// internally require a C string.
///
/// \attention
///   It is imperative that the value passed to any of the view's constructors
///   outlives the view object!
////////////////////////////////////////////////////////////////////////////////
class StringView {
 public:
  /// Constructs a view that evaluates to a nullptr.
  StringView() : s_(nullptr) {}

  /// Constructs a view from a standard string.
  ///
  /// \param s Referenced string.
  StringView(const std::string& s) : s_(s.c_str()) {}

  /// Constructs a view from a NULL-terminated string.
  ///
  /// \param s Referenced NULL-terminated string.
  StringView(const char* s) : s_(s){};

  /// Returns a NULL-terminated string holding the referenced string data.
  ///
  /// \attention
  ///   The returned value is only valid as long as both the view object and the
  ///   parameter passed to any of its constructors are valid.
  ///
  /// \returns NULL-terminated string holding the referenced string data.
  operator const char*() const { return s_; }

  bool operator==(const StringView& rhs) const {
    return std::strcmp(s_, rhs.s_) == 0;
  }

  bool operator!=(const StringView& rhs) const { return !(*this == rhs); }

 private:
  const char* s_;
};

}  // namespace yogi

#endif  // YOGI_STRING_VIEW_H
