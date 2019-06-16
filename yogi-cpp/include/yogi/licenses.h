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

#ifndef YOGI_LICENSES_H
#define YOGI_LICENSES_H

//! \file
//!
//! Functions for getting license information.

#include "internal/library.h"

namespace yogi {

_YOGI_DEFINE_API_FN(const char*, YOGI_GetLicense, ())
_YOGI_DEFINE_API_FN(const char*, YOGI_Get3rdPartyLicenses, ())

/// \addtogroup freefn
/// @{

/// Get the license that Yogi is published under.
///
/// Note: The returned string contains the complete description of the license
///       and is therefore very large.
///
/// \returns License information
inline const std::string& GetLicense() {
  static std::string s = internal::YOGI_GetLicense();
  return s;
}

/// Get the license information about the 3rd party libraries used in Yogi.
///
/// Note: The returned string is very large.
///
/// \returns License information
inline const std::string& Get3rdPartyLicenses() {
  static std::string s = internal::YOGI_Get3rdPartyLicenses();
  return s;
}

/// @} freefn

}  // namespace yogi

#endif  // YOGI_LICENSES_H
