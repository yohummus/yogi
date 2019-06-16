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

#include "macros.h"
#include "helpers.h"
#include "../api/constants.h"
#include "../licenses/licenses.h"

#include <regex>
using namespace std::string_literals;

YOGI_API const char* YOGI_GetVersion() { return api::kVersion; }

YOGI_API int YOGI_CheckBindingsCompatibility(const char* bindver, char* err,
                                             int errsize) {
  CHECK_PARAM(bindver != nullptr);
  try {
    static const std::regex re("([0-9]+)\\.([0-9]+)\\.[0-9]+[^ \\s]*");
    const std::string bindver_str = bindver;

    std::smatch m;
    if (!std::regex_match(bindver_str, m, re) || m.size() != 3) {
      throw api::DescriptiveError(YOGI_ERR_INVALID_PARAM)
          << "The supplied version number \"" << bindver_str
          << "\" does not have a valid format. It must have the format "
             "<major>.<minor>.<patch>[<suffix>].";
    }

    int major = std::stoi(m[1]);
    int minor = std::stoi(m[2]);
    if (major != api::kVersionMajor || minor < api::kVersionMinor) {
      throw api::DescriptiveError(YOGI_ERR_INCOMPATIBLE_VERSION)
          << "The bindings version \"" << bindver_str
          << "\" is incompatible with the Yogi Core library version \""
          << api::kVersion << "\". A core library with a major version of "
          << major << " and a minor version of at least " << minor
          << " is required.";
    }
  }
  CATCH_DESCRIPTIVE_AND_RETURN(err, errsize);
}

YOGI_API const char* YOGI_GetLicense() {
  return licenses::kYogiLicense.c_str();
}

YOGI_API const char* YOGI_Get3rdPartyLicenses() {
  return licenses::k3rdPartyLicenses.c_str();
}

YOGI_API const char* YOGI_GetErrorString(int res) {
  return api::Result(res).GetDescription();
}

YOGI_API int YOGI_GetConstant(void* dest, int constant) {
  CHECK_PARAM(dest != nullptr);

  try {
    api::GetConstant(dest, constant);
  }
  CATCH_AND_RETURN;
}
