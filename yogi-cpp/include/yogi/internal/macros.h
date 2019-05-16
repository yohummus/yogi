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

#ifndef YOGI_INTERNAL_MACROS_H
#define YOGI_INTERNAL_MACROS_H

//! \file
//!
//! Internally used macros.

#ifdef _WIN32
#define _YOGI_WEAK_SYMBOL __declspec(selectany)
#else
#define _YOGI_WEAK_SYMBOL __attribute__((weak))
#endif

#define _YOGI_DEFINE_API_FN(ret_type, name, arg_types)           \
  namespace internal {                                           \
  _YOGI_WEAK_SYMBOL ret_type(*name) arg_types =                  \
      Library::GetFunctionAddress<ret_type(*) arg_types>(#name); \
  }

#endif  // YOGI_INTERNAL_MACROS_H