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

#ifndef YOGI_TIME_H
#define YOGI_TIME_H

//! \file
//!
//! Functions for handling time.

#include "timestamp.h"

namespace yogi {

/// \addtogroup freefn
/// @{

/// Get the current time.
///
/// \returns The current time since 01/01/1970 UTC.
inline Timestamp GetCurrentTime() { return Timestamp::Now(); }

/// @} freefn

}  // namespace yogi

#endif  // YOGI_TIME_H
