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

#ifndef YOGI_INTERNAL_DURATION_CONVERSION_H
#define YOGI_INTERNAL_DURATION_CONVERSION_H

//! \file
//!
//! Helpers for converting durations.

#include "../errors.h"
#include "../duration.h"

namespace yogi {
namespace internal {

inline long long ToCoreDuration(const Duration& duration) {
  if (duration < Duration::kZero) {
    throw FailureException(ErrorCode::kInvalidParam);
  }

  return duration.IsFinite() ? duration.NanosecondsCount() : -1;
}

}  // namespace internal
}  // namespace yogi

#endif  // YOGI_INTERNAL_DURATION_CONVERSION_H
