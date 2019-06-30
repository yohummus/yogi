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
#include "../api/object.h"
#include "../objects/context.h"

YOGI_API int YOGI_FormatObject(void* obj, char* str, int strsize,
                               const char* objfmt, const char* nullstr) {
  CHECK_PARAM(str != nullptr);
  CHECK_PARAM(strsize > 0);

  try {
    std::string s;
    if (obj == nullptr) {
      s = nullstr ? nullstr : api::kDefaultInvalidHandleString;
    } else {
      auto object = api::ObjectRegister::Get(obj);
      s = object->Format(objfmt ? objfmt : api::kDefaultObjectFormat);
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
    api::ObjectRegister::DestroyAll();
  }
  CATCH_AND_RETURN;
}
