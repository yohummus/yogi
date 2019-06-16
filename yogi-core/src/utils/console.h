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

#include <string>
#include <stdio.h>

namespace utils {

enum class ForegroundColour {
  kBlack,
  kBlue,
  kGreen,
  kCyan,
  kRed,
  kMagenta,
  kBrown,
  kGrey,
  kDarkGrey,
  kLightBlue,
  kLightGreen,
  kLightCyan,
  kLightRed,
  kLightMagenta,
  kYellow,
  kWhite,
};

enum class BackgroundColour {
  kBlack   = static_cast<int>(ForegroundColour::kBlack),
  kRed     = static_cast<int>(ForegroundColour::kRed),
  kGreen   = static_cast<int>(ForegroundColour::kGreen),
  kYellow  = static_cast<int>(ForegroundColour::kYellow),
  kBlue    = static_cast<int>(ForegroundColour::kBlue),
  kMagenta = static_cast<int>(ForegroundColour::kMagenta),
  kCyan    = static_cast<int>(ForegroundColour::kCyan),
  kWhite   = static_cast<int>(ForegroundColour::kWhite),
};

void SetConsoleTitle(FILE* stream, const std::string& title);
void SetConsoleColour(FILE* stream, ForegroundColour color);
void SetConsoleColour(FILE* stream, BackgroundColour color);
void ResetConsoleColours(FILE* stream);

}  // namespace utils
