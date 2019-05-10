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

#ifndef YOGI_INTERNAL_SFINAE_H
#define YOGI_INTERNAL_SFINAE_H

//! \file
//!
//! Helpers related to Substitution Failure Is Not An Error.

#include <type_traits>
#include <string>

#define _YOGI_DEFINE_SFINAE_METHOD_TESTER(class_name, test_expression)    \
  namespace internal {                                                    \
  template <typename T>                                                   \
  struct class_name {                                                     \
   private:                                                               \
    template <typename U>                                                 \
    static auto test(int)                                                 \
        -> decltype(std::declval<U>() test_expression, std::true_type()); \
                                                                          \
    template <typename>                                                   \
    static std::false_type test(...);                                     \
                                                                          \
   public:                                                                \
    static constexpr bool value =                                         \
        std::is_same<decltype(test<T>(0)), std::true_type>::value;        \
  };                                                                      \
  }  // namespace internal

#endif  // YOGI_INTERNAL_SFINAE_H
