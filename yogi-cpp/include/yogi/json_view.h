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

#ifndef YOGI_JSON_VIEW_H
#define YOGI_JSON_VIEW_H

//! \file
//!
//! Helpers for passing JSON-encoded data to functions.

#include "json.h"

#include <string>
#include <cstring>
#include <vector>

namespace yogi {

////////////////////////////////////////////////////////////////////////////////
/// Helper class for passing different types of JSON to functions that
/// internally require a C string holding the serialized JSON data.
///
/// \attention
///   It is imperative that the objects passed to any of the view's constructors
///   outlive the view object!
////////////////////////////////////////////////////////////////////////////////
class JsonView {
 public:
  /// Constructs a view that evaluates to a nullptr.
  JsonView() : s_(nullptr), size_(0) {}

  /// Constructs a view from a vector.
  ///
  /// \param data Buffer to use.
  JsonView(const std::vector<char>& data)
      : s_(data.data()), size_(static_cast<int>(data.size())) {}

  /// Constructs a view from a standard string.
  ///
  /// \param s Referenced string.
  JsonView(const std::string& s)
      : s_(s.c_str()), size_(static_cast<int>(s.size() + 1)) {}

  /// Constructs a view from a NULL-terminated string.
  ///
  /// \param s Referenced NULL-terminated string.
  JsonView(const char* s) : s_(s), size_(static_cast<int>(strlen(s) + 1)){};

  /// Constructs a view from a JSON object.
  ///
  /// \param json The JSON object to reference.
  JsonView(const Json& json)
      : tmp_(json.dump()),
        s_(tmp_.c_str()),
        size_(static_cast<int>(tmp_.size() + 1)){};

  /// Returns a NULL-terminated string holding the serialized JSON data.
  ///
  /// \attention
  ///   The returned value is only valid as long as both the view object and the
  ///   parameter passed to any of its constructors are valid.
  ///
  /// \returns NULL-terminated string holding the serialized JSON data.
  const char* Data() const { return s_; }

  /// Returns the length of the serialized JSON data in bytes.
  ///
  /// \returns Length of the serialized JSON data, including the trailing '\0'.
  int Size() const { return size_; }

  /// Returns a NULL-terminated string holding the serialized JSON data.
  ///
  /// \attention
  ///   The returned value is only valid as long as both the view object and the
  ///   parameter passed to any of its constructors are valid.
  ///
  /// \returns NULL-terminated string holding the serialized JSON data.
  operator const char*() const { return s_; }

  bool operator==(const JsonView& rhs) const {
    if (Size() != rhs.Size()) return false;
    return std::memcmp(Data(), rhs.Data(), static_cast<std::size_t>(Size())) ==
           0;
  }

  bool operator!=(const JsonView& rhs) const { return !(*this == rhs); }

 private:
  std::string tmp_;
  const char* s_;
  int size_;
};

}  // namespace yogi

#endif  // YOGI_JSON_VIEW_H
