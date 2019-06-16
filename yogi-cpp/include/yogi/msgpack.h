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

#ifndef YOGI_MSG_PACK_H
#define YOGI_MSG_PACK_H

//! \file
//!
//! Includes the msgpack-c MessagePack library.

#ifdef MSGPACK_VERSION_MAJOR

#if MSGPACK_VERSION_MAJOR != 3
#error "Incompatible version of the msgpack-c library has been included"
#endif

#else

#include "internal/push_diagnostics_ignore_warnings.h"
#include "3rd_party/msgpack/msgpack.hpp"
#include "internal/pop_diagnostics.h"

#endif

namespace yogi {

/// Alias for the msgpack-c library.
namespace msgpack = ::msgpack;

}  // namespace yogi

#endif  // YOGI_MSG_PACK_H
