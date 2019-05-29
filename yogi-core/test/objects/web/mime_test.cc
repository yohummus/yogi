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

#include "../../common.h"
#include "../../../src/objects/detail/web/mime.h"
using objects::detail::web::FileExtensionToMimeType;

TEST(MimeTest, WellKnownFileExtensions) {
  EXPECT_EQ(FileExtensionToMimeType("txt"), "text/plain");
  EXPECT_EQ(FileExtensionToMimeType("hTmL"), "text/html");
  EXPECT_EQ(FileExtensionToMimeType("WAV"), "audio/x-wav");
}

TEST(MimeTest, UnknownFileExtensions) {
  EXPECT_EQ(FileExtensionToMimeType("blabliblup"), "application/octet-stream");
}