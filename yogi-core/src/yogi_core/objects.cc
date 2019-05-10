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

#include "macros.h"
#include "helpers.h"
#include "../api/constants.h"
#include "../api/object.h"
#include "../objects/context.h"

#include <boost/algorithm/string.hpp>

YOGI_API int YOGI_FormatObject(void* obj, char* str, int strsize,
                               const char* objfmt, const char* nullstr) {
  CHECK_PARAM(str != nullptr);
  CHECK_PARAM(strsize > 0);

  try {
    std::string s;
    if (obj == nullptr) {
      s = nullstr ? nullstr : api::kDefaultInvalidHandleString;
    } else {
      s = objfmt ? objfmt : api::kDefaultObjectFormat;

      auto object = api::ObjectRegister::Get(obj);
      boost::replace_all(s, "$T", object->TypeName());

      char buf[24];
      sprintf(buf, "%llx", reinterpret_cast<unsigned long long>(obj));
      boost::replace_all(s, "$x", buf);
      sprintf(buf, "%llX", reinterpret_cast<unsigned long long>(obj));
      boost::replace_all(s, "$X", buf);
    }

    if (!CopyStringToUserBuffer(s, str, strsize)) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_Destroy(void* object) {
  CHECK_PARAM(object != nullptr);

  try {
    api::ObjectRegister::Destroy(object);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_DestroyAll() {
  try {
    for (auto& ctx : api::ObjectRegister::GetAll<objects::Context>()) {
      ctx->Stop();
      ctx->WaitForStopped(std::chrono::nanoseconds::max());
    }

    api::ObjectRegister::DestroyAll();
  }
  CATCH_AND_RETURN;
}
