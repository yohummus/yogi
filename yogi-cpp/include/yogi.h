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

#ifndef YOGI_H
#define YOGI_H

//! \file
//!
//! Includes the whole library.

//! \namespace yogi
//!
//! Main namespace for yogi-cpp.

//! \addtogroup enums Enums
//! Enumerations.

//! \addtogroup freefn Free Functions
//! Free functions.

//! \addtogroup logmacros Logging Macros
//! Macros for creating log entries.

#include "yogi/branch.h"
#include "yogi/buffer.h"
#include "yogi/bindings_info.h"
#include "yogi/configuration.h"
#include "yogi/constants.h"
#include "yogi/context.h"
#include "yogi/duration.h"
#include "yogi/errors.h"
#include "yogi/io.h"
#include "yogi/json_view.h"
#include "yogi/json.h"
#include "yogi/licenses.h"
#include "yogi/logging.h"
#include "yogi/msgpack_view.h"
#include "yogi/msgpack.h"
#include "yogi/object.h"
#include "yogi/operation_id.h"
#include "yogi/payload_view.h"
#include "yogi/signals.h"
#include "yogi/stream_io.h"
#include "yogi/string_view.h"
#include "yogi/time.h"
#include "yogi/timer.h"
#include "yogi/timestamp.h"
#include "yogi/uuid.h"
#include "yogi/version.h"

#endif  // YOGI_H
