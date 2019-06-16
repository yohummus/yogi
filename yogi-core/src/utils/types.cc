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

#include "types.h"

#include <ios>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const utils::ByteVector& bytes) {
  auto flags = os.flags();

  os << '[';
  os << std::setw(2) << std::setfill('0') << std::hex;
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    os << bytes[i];
    if (i < bytes.size() - 1) {
      os << ", ";
    }
  }
  os << ']';

  os.flags(flags);
  return os;
}
