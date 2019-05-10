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

#pragma once

#include "../config.h"

#include <boost/container/small_vector.hpp>
#include <vector>
#include <memory>
#include <utility>
#include <fstream>

#define YOGI_DEFINE_FLAG_OPERATORS(enum_)                                 \
  inline enum_ operator|(enum_ a, enum_ b) {                              \
    return static_cast<enum_>(static_cast<int>(a) | static_cast<int>(b)); \
  }                                                                       \
  inline enum_ operator&(enum_ a, enum_ b) {                              \
    return static_cast<enum_>(static_cast<int>(a) & static_cast<int>(b)); \
  }

namespace utils {

typedef unsigned char Byte;
typedef std::vector<Byte> ByteVector;
typedef std::shared_ptr<ByteVector> SharedByteVector;
typedef boost::container::small_vector<Byte, 32> SmallByteVector;
typedef std::shared_ptr<SmallByteVector> SharedSmallByteVector;

template <typename... Args>
SharedByteVector MakeSharedByteVector(Args&&... args) {
  return std::make_shared<ByteVector>(std::forward<Args>(args)...);
}

template <typename... Args>
SharedSmallByteVector MakeSharedSmallByteVector(Args&&... args) {
  return std::make_shared<SmallByteVector>(std::forward<Args>(args)...);
}

}  // namespace utils

std::ostream& operator<<(std::ostream& os, const utils::ByteVector& bytes);
