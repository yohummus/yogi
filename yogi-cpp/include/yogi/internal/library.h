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

#ifndef YOGI_INTERNAL_LIBRARY_H
#define YOGI_INTERNAL_LIBRARY_H

//! \file
//!
//! Helpers for handling the Yogi Core library.

#include "../bindings_info.h"
#include "macros.h"

#include <cstdlib>
#include <string>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace yogi {
namespace internal {

class Library final {
 public:
  template <typename Fn>
  static Fn GetFunctionAddress(const char* name) {
    using namespace std::string_literals;

    if (!lib_handle_) {
      lib_handle_ = LoadYogiCore();
      CheckVersionCompatibility();
    }

    auto addr = GetProcAddress(lib_handle_, name);
    if (!addr) {
      DieWithErrorMessage("Could not find function "s + name +
                          " in the loaded yogi-core library");
    }

    return reinterpret_cast<Fn>(addr);
  }

 private:
#ifdef _WIN32
  typedef HMODULE LibraryHandle;
  typedef FARPROC ProcAddress;
#else
  typedef void* LibraryHandle;
  typedef void* ProcAddress;
#endif

  static LibraryHandle lib_handle_;

  static LibraryHandle LoadYogiCore() {
    using namespace std::string_literals;

    auto filename = GetFilename();
    auto handle = LoadLibrary(filename);
    if (!handle) {
      DieWithErrorMessage("Could not load "s + filename);
    }

    return handle;
  }

  static void CheckVersionCompatibility() {
    int (*fn)(const char* bindver, char* err, int errsize);
    fn = Library::GetFunctionAddress<decltype(fn)>(
        "YOGI_CheckBindingsCompatibility");

    char err[256] = {0};
    if (fn(YOGI_BINDINGS_VERSION, err, sizeof(err)) != 0) {
      std::cerr << "FATAL: " << err << std::endl;
      std::exit(1);
    }
  }

  static void DieWithErrorMessage(std::string prefix) {
    std::cerr << "FATAL: " << prefix << ": " << GetLastErrorString()
              << std::endl;
    std::exit(1);
  }

  // Methods below are platform-dependent
  static inline const char* GetFilename();
  static inline LibraryHandle LoadLibrary(const char* filename);
  static inline ProcAddress GetProcAddress(LibraryHandle handle,
                                           const char* name);
  static inline std::string GetLastErrorString();
};

_YOGI_WEAK_SYMBOL Library::LibraryHandle Library::lib_handle_;

}  // namespace internal
}  // namespace yogi

#ifdef _WIN32
#include "library_impl_win.h"
#else
#include "library_impl_unix.h"
#endif

#endif  // YOGI_INTERNAL_LIBRARY_H
