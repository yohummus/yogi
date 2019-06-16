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

#ifndef YOGI_IO_H
#define YOGI_IO_H

//! \file
//!
//! Functions for printing various Yogi types.

#include "internal/sfinae.h"

#include <string>

#define _YOGI_TO_STRING_ENUM_CASE(enum_type, enum_element) \
  case enum_type::enum_element:                            \
    return #enum_element;

#define _YOGI_TO_STRING_FLAG_APPENDER(var, enum_type, enum_element) \
  if ((var & enum_type::enum_element) != enum_type::kNone) {        \
    s += " | " #enum_element;                                       \
  }

namespace yogi {

_YOGI_DEFINE_SFINAE_METHOD_TESTER(HasToStringMethod,
                                  .ToString() == std::string())

/// \addtogroup freefn
/// @{

/// Converts a given Yogi enum value or object to a string.
///
/// \tparam T Type of the enum or object.
///
/// \param printable The object to convert to a string.
///
/// \returns Human-readable string name or description of the object.
template <typename T>
inline std::string ToString(const T& printable) {
  static_assert(internal::HasToStringMethod<T>::value,
                "T has no usable ToString() method.");

  return printable.ToString();
}

/// @} freefn

}  // namespace yogi

#endif  // YOGI_IO_H
