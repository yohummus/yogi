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

#pragma once

#include "../config.h"
#include "../../include/yogi_core.h"
#include "../api/errors.h"

#include <stdexcept>
#include <regex>

#define CHECK_PARAM(cond)                       \
  {                                             \
    if (!(cond)) return YOGI_ERR_INVALID_PARAM; \
  }

#define CHECK_BUFFER_PARAMS(buffer, size)        \
  {                                              \
    CHECK_PARAM(buffer != nullptr || size == 0); \
    CHECK_PARAM(size >= 0);                      \
  }

#define CHECK_FLAGS(flags, all_flags) \
  { CHECK_PARAM(!(flags & ~all_flags)); }

#ifdef NDEBUG
#define CATCH_AND_RETURN_INTERNAL_ERRORS                               \
  catch (const std::exception& e) {                                    \
    fprintf(stderr, "%s:%i: INTERNAL ERROR: %s\n", __FILE__, __LINE__, \
            e.what());                                                 \
    return YOGI_ERR_UNKNOWN;                                           \
  }                                                                    \
  catch (...) {                                                        \
    fprintf(stderr, "%s:%i: INTERNAL ERROR: %s\n", __FILE__, __LINE__, \
            "Unknown error");                                          \
    return YOGI_ERR_UNKNOWN;                                           \
  }
#else
#define CATCH_AND_RETURN_INTERNAL_ERRORS
#endif

#define CATCH_AND_RETURN_ERRORS_ONLY \
  catch (const api::Error& e) {      \
    return e.GetErrorCode();         \
  }                                  \
  catch (const std::bad_alloc&) {    \
    return YOGI_ERR_BAD_ALLOC;       \
  }                                  \
  catch (const std::regex_error&) {  \
    return YOGI_ERR_INVALID_REGEX;   \
  }                                  \
  CATCH_AND_RETURN_INTERNAL_ERRORS

#define CATCH_AND_RETURN       \
  CATCH_AND_RETURN_ERRORS_ONLY \
  return YOGI_OK;

#define CATCH_DESCRIPTIVE_AND_RETURN(err, errsize)        \
  catch (const api::DescriptiveError& e) {                \
    CopyStringToUserBuffer(e.GetDetails(), err, errsize); \
    return e.GetErrorCode();                              \
  }                                                       \
  CATCH_AND_RETURN_ERRORS_ONLY                            \
  if (err != nullptr && errsize > 0) {                    \
    *err = '\0';                                          \
  }                                                       \
  return YOGI_OK;
