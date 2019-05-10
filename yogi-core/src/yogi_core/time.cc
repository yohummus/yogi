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
#include "../utils/timestamp.h"

#include <boost/algorithm/string.hpp>

YOGI_API int YOGI_GetCurrentTime(long long* timestamp) {
  CHECK_PARAM(timestamp != nullptr);

  try {
    auto now = utils::Timestamp::Now();
    *timestamp = now.NanosecondsSinceEpoch().count();
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_FormatTime(long long timestamp, char* str, int strsize,
                             const char* timefmt) {
  CHECK_PARAM(str != nullptr);
  CHECK_PARAM(strsize > 0);

  try {
    auto t = utils::Timestamp(std::chrono::nanoseconds(timestamp));
    auto s = t.ToFormattedString(timefmt ? timefmt : api::kDefaultTimeFormat);
    if (!CopyStringToUserBuffer(s, str, strsize)) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ParseTime(long long* timestamp, const char* str,
                            const char* timefmt) {
  CHECK_PARAM(timestamp != nullptr);
  CHECK_PARAM(str != nullptr);

  try {
    auto t = utils::Timestamp::Parse(
        str, timefmt ? timefmt : api::kDefaultTimeFormat);
    *timestamp = t.NanosecondsSinceEpoch().count();
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_FormatDuration(long long dur, int neg, char* str, int strsize,
                                 const char* durfmt, const char* infstr) {
  CHECK_PARAM(dur >= -1);
  CHECK_PARAM(neg == YOGI_TRUE || neg == YOGI_FALSE);
  CHECK_PARAM(str != nullptr);
  CHECK_PARAM(strsize > 0);

  try {
    std::string s;
    if (dur == -1) {
      s = infstr ? infstr : api::kDefaultInfiniteDurationString;

      boost::replace_all(s, "%-", neg == YOGI_TRUE ? "-" : "");
      boost::replace_all(s, "%+", neg == YOGI_TRUE ? "-" : "+");
    } else {
      s = durfmt ? durfmt : api::kDefaultDurationFormat;

      boost::replace_all(s, "%-", neg == YOGI_TRUE ? "-" : "");
      boost::replace_all(s, "%+", neg == YOGI_TRUE ? "-" : "+");

      char tmp[32];
      auto days = dur / 86'400'000'000'000;
      sprintf(tmp, "%i", static_cast<int>(days));
      boost::replace_all(s, "%d", tmp);
      boost::replace_all(s, "%D", days > 0 ? tmp : "");
      boost::replace_all(s, "%T", "%H:%M:%S");
      sprintf(tmp, "%02i", static_cast<int>((dur / 3'600'000'000'000) % 24));
      boost::replace_all(s, "%H", tmp);
      sprintf(tmp, "%02i", static_cast<int>((dur / 60'000'000'000) % 60));
      boost::replace_all(s, "%M", tmp);
      sprintf(tmp, "%02i", static_cast<int>((dur / 1'000'000'000) % 60));
      boost::replace_all(s, "%S", tmp);
      sprintf(tmp, "%03i", static_cast<int>((dur / 1'000'000) % 1000));
      boost::replace_all(s, "%3", tmp);
      sprintf(tmp, "%03i", static_cast<int>((dur / 1'000) % 1000));
      boost::replace_all(s, "%6", tmp);
      sprintf(tmp, "%03i", static_cast<int>(dur % 1000));
      boost::replace_all(s, "%9", tmp);
    }

    if (!CopyStringToUserBuffer(s, str, strsize)) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}
