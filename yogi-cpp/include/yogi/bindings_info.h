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

#ifndef YOGI_BINDINGS_INFO_H
#define YOGI_BINDINGS_INFO_H

/// Major version number of the bindings.
#define YOGI_BINDINGS_VERSION_MAJOR 0

/// Minor version number of the bindings.
#define YOGI_BINDINGS_VERSION_MINOR 0

/// Patch version number of the bindings.
#define YOGI_BINDINGS_VERSION_PATCH 3

/// Version suffix of the bindings.
#define YOGI_BINDINGS_VERSION_SUFFIX "-alpha"

/// Whole version string of the bindings.
#define YOGI_BINDINGS_VERSION "0.0.3" YOGI_BINDINGS_VERSION_SUFFIX

#include "internal/macros.h"

#include <string>

namespace yogi {

////////////////////////////////////////////////////////////////////////////////
/// Information about the bindings.
////////////////////////////////////////////////////////////////////////////////
struct bindings_info {
  /// Whole version string of the bindings.
  static const std::string kVersion;

  /// Major version number of the bindings.
  static const int kVersionMajor;

  /// Minor version number of the bindings.
  static const int kVersionMinor;

  /// Patch version number of the bindings.
  static const int kVersionPatch;

  /// Version suffix of the bindings.
  static const std::string kVersionSuffix;
};

_YOGI_WEAK_SYMBOL const std::string bindings_info::kVersion =
    YOGI_BINDINGS_VERSION;

_YOGI_WEAK_SYMBOL const int bindings_info::kVersionMajor =
    YOGI_BINDINGS_VERSION_MAJOR;

_YOGI_WEAK_SYMBOL const int bindings_info::kVersionMinor =
    YOGI_BINDINGS_VERSION_MINOR;

_YOGI_WEAK_SYMBOL const int bindings_info::kVersionPatch =
    YOGI_BINDINGS_VERSION_PATCH;

_YOGI_WEAK_SYMBOL const std::string bindings_info::kVersionSuffix =
    YOGI_BINDINGS_VERSION_SUFFIX;

}  // namespace yogi

#endif  // YOGI_BINDINGS_INFO_H