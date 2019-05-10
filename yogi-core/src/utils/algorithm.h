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

#include <algorithm>

namespace utils {

template <typename Container, typename T>
inline typename Container::iterator find(Container& container, const T& value) {
  return std::find(container.begin(), container.end(), value);
}

template <typename Container, typename Predicate>
inline typename Container::iterator find_if(Container& container,
                                            Predicate pred) {
  return std::find_if(container.begin(), container.end(), pred);
}

template <typename Container, typename Predicate>
inline bool contains_if(Container& container, Predicate pred) {
  return find_if(container, pred) != std::end(container);
}

template <typename Container, typename T>
inline typename Container::iterator remove_erase(Container& container,
                                                 const T& value) {
  return container.erase(std::remove(container.begin(), container.end(), value),
                         container.end());
}

template <typename Container, typename Predicate>
inline typename Container::iterator remove_erase_if(Container& container,
                                                    Predicate pred) {
  return container.erase(
      std::remove_if(container.begin(), container.end(), pred),
      container.end());
}

}  // namespace utils
