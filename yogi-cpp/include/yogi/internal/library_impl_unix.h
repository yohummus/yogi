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

#ifndef YOGI_INTERNAL_LIBRARY_IMPL_UNIX_H
#define YOGI_INTERNAL_LIBRARY_IMPL_UNIX_H

//! \file
//!
//! Unix-specific parts for handling the Yogi-Core library.

#include <dlfcn.h>

namespace yogi {
namespace internal {

const char* Library::GetFilename() {
  const char* s = std::getenv("YOGI_CORE_LIBRARY");
  if (s) return s;

#ifdef __APPLE__
  return "libyogi-core.dylib";
#else
  return "libyogi-core.so";
#endif
}

Library::LibraryHandle Library::LoadLibrary(const char* filename) {
  dlerror();  // Clear any previous errors
  return dlopen(filename, RTLD_NOW);
}

Library::ProcAddress Library::GetProcAddress(LibraryHandle handle,
                                             const char* name) {
  dlerror();  // Clear any previous errors
  return dlsym(handle, name);
}

std::string Library::GetLastErrorString() {
  std::string s;
  auto err = dlerror();
  if (err) {
    s.assign(err);
  }

  return s;
}

}  // namespace internal
}  // namespace yogi

#endif  // YOGI_INTERNAL_LIBRARY_IMPL_UNIX_H
