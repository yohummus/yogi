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

#include "base64.h"

#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
using namespace boost::archive::iterators;

namespace utils {

std::string EncodeBase64(std::string plain) {
  using Iterator = insert_linebreaks<
      base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>,
      76>;

  size_t padding((3 - plain.size() % 3) % 3);
  plain.append(padding, 0);
  std::string encoded(
      Iterator(plain.begin()),
      Iterator(plain.end() - static_cast<Iterator::difference_type>(padding)));
  encoded.append(padding, '=');

  return encoded;
}

std::string DecodeBase64(std::string encoded) {
  using Iterator = transform_width<
      binary_from_base64<remove_whitespace<std::string::const_iterator>>, 8, 6>;

  encoded.append((4 - encoded.size() % 4) % 4, '=');
  auto padding = std::count(encoded.begin(), encoded.end(), '=');

  try {
    std::replace(encoded.begin(), encoded.end(), '=', 'A');

    std::string plain;
    plain.assign(Iterator(encoded.begin()), Iterator(encoded.end()));
    plain.erase(plain.end() - padding, plain.end());
    return plain;
  } catch (const std::exception&) {
    return {};
  }
}

}  // namespace utils