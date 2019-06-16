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

#ifndef YOGI_UUID_H
#define YOGI_UUID_H

//! \file
//!
//! Representation of Universal Unique IDentifiers.

#include "io.h"

#include <string.h>

namespace yogi {
namespace internal {

inline char NibbleToHexChar(unsigned char nibble) {
  if (nibble <= 9) {
    return static_cast<char>('0' + nibble);
  } else {
    return static_cast<char>('a' + (nibble - 10));
  }
}

}  // namespace internal

////////////////////////////////////////////////////////////////////////////////
/// Represents a Universal Unique IDentifier.
///
/// A UUID is a Universal Unique IDentifier that is used to unambiguously
/// identify certain Yogi objects like branches.
///
/// \note
///   This class is a POD.
////////////////////////////////////////////////////////////////////////////////
class Uuid {
 public:
  typedef unsigned char value_type;
  typedef value_type* iterator;
  typedef value_type const* const_iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  static constexpr size_type static_size() noexcept { return sizeof(Uuid); }

  constexpr std::size_t size() const { return static_size(); }

  iterator begin() noexcept { return data_; }
  const_iterator begin() const noexcept { return data_; }
  iterator end() noexcept { return data_ + size(); }
  const_iterator end() const noexcept { return data_ + size(); }

  bool operator==(const Uuid& rhs) const noexcept {
    return memcmp(data_, rhs.data_, sizeof(data_)) == 0;
  }

  bool operator!=(const Uuid& rhs) const noexcept { return !(*this == rhs); }

  bool operator<(const Uuid& rhs) const noexcept {
    return memcmp(data_, rhs.data_, sizeof(data_)) < 0;
  }

  bool operator>(const Uuid& rhs) const noexcept { return rhs < *this; }

  bool operator<=(const Uuid& rhs) const noexcept { return !(rhs < *this); }

  bool operator>=(const Uuid& rhs) const noexcept { return !(rhs > *this); }

  void swap(Uuid& rhs) noexcept {
    Uuid tmp = *this;
    *this = rhs;
    rhs = tmp;
  }

  value_type* data() noexcept { return data_; }
  const value_type* data() const noexcept { return data_; }

  /// Returns a string in the format "6ba7b810-9dad-11d1-80b4-00c04fd430c8".
  ///
  /// \returns String representation of the UUID.
  std::string ToString() const {
    std::string s;
    s.reserve(36);

    for (std::size_t i = 0; i < size(); ++i) {
      s += internal::NibbleToHexChar((data_[i] >> 4) & 0x0F);
      s += internal::NibbleToHexChar(data_[i] & 0x0F);

      if (i == 3 || i == 5 || i == 7 || i == 9) {
        s += '-';
      }
    }

    return s;
  }

 private:
  value_type data_[16];
};

}  // namespace yogi

namespace std {

template <>
struct hash<yogi::Uuid> {
  size_t operator()(const yogi::Uuid& uuid) const {
    std::size_t seed = 0;
    for (yogi::Uuid::const_iterator it = uuid.begin(); it != uuid.end(); ++it) {
      seed ^= static_cast<std::size_t>(*it) + 0x9e3779b9 + (seed << 6) +
              (seed >> 2);
    }

    return seed;
  }
};

}  // namespace std

#endif  // YOGI_UUID_H
