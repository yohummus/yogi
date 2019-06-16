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

#ifndef YOGI_INTERNAL_FLAGS_H
#define YOGI_INTERNAL_FLAGS_H

//! \file
//!
//! Helpers for defining flags.

#define _YOGI_DEFINE_FLAG_OPERATORS(enum_type)                                 \
  inline enum_type operator|(enum_type a, enum_type b) {                      \
    return static_cast<enum_type>(static_cast<int>(a) | static_cast<int>(b)); \
  }                                                                           \
  inline enum_type operator&(enum_type a, enum_type b) {                      \
    return static_cast<enum_type>(static_cast<int>(a) & static_cast<int>(b)); \
  }                                                                           \
  inline enum_type& operator|=(enum_type& a, enum_type b) { return a = a | b; }

#endif  // YOGI_INTERNAL_FLAGS_H
