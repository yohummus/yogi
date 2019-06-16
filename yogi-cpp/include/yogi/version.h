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

#ifndef YOGI_VERSION_H
#define YOGI_VERSION_H

//! \file
//!
//! Version information.

#include "internal/library.h"

namespace yogi {

_YOGI_DEFINE_API_FN(const char*, YOGI_GetVersion, ())

_YOGI_DEFINE_API_FN(int, YOGI_CheckBindingsCompatibility,
                    (const char* bindver, char* err, int errsize))

/// \addtogroup freefn
/// @{

/// Returns the version string of the loaded Yogi Core library.
///
/// \returns Version string of the loaded Yogi Core library.
inline const std::string& GetVersion() {
  static std::string s = internal::YOGI_GetVersion();
  return s;
}

/// @} freefn

}  // namespace yogi

#endif  // YOGI_VERSION_H
