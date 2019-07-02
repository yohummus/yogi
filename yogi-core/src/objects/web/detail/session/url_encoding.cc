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

#include "url_encoding.h"

#include <sstream>

namespace objects {
namespace web {
namespace detail {

boost::optional<std::string> DecodeUrl(boost::beast::string_view url) {
  std::string out;
  out.reserve(url.size());

  for (std::size_t i = 0; i < url.size(); ++i) {
    if (url[i] == '%') {
      if (i + 3 > url.size()) return {};
      int value = 0;
      std::istringstream is(url.substr(i + 1, 2).to_string());
      if (!(is >> std::hex >> value)) return {};
      out += static_cast<char>(value);
      i += 2;
    } else if (url[i] == '+') {
      out += ' ';
    } else {
      out += url[i];
    }
  }

  return out;
}

}  // namespace detail
}  // namespace web
}  // namespace objects
