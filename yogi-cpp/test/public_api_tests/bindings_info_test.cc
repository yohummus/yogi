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

#include "../common.h"

#include <yogi_core.h>

TEST(BindingsInfoTest, Version) {
  EXPECT_STREQ(YOGI_BINDINGS_VERSION, YOGI_HDR_VERSION);
  EXPECT_EQ(yogi::bindings_info::kVersion, YOGI_HDR_VERSION);
}

TEST(BindingsInfoTest, VersionMajor) {
  EXPECT_EQ(YOGI_BINDINGS_VERSION_MAJOR, YOGI_HDR_VERSION_MAJOR);
  EXPECT_EQ(yogi::bindings_info::kVersionMajor, YOGI_HDR_VERSION_MAJOR);
}

TEST(BindingsInfoTest, VersionMinor) {
  EXPECT_EQ(YOGI_BINDINGS_VERSION_MINOR, YOGI_HDR_VERSION_MINOR);
  EXPECT_EQ(yogi::bindings_info::kVersionMinor, YOGI_HDR_VERSION_MINOR);
}

TEST(BindingsInfoTest, VersionPatch) {
  EXPECT_EQ(YOGI_BINDINGS_VERSION_PATCH, YOGI_HDR_VERSION_PATCH);
  EXPECT_EQ(yogi::bindings_info::kVersionPatch, YOGI_HDR_VERSION_PATCH);
}

TEST(BindingsInfoTest, VersionSuffix) {
  EXPECT_STREQ(YOGI_BINDINGS_VERSION_SUFFIX, YOGI_HDR_VERSION_SUFFIX);
  EXPECT_EQ(yogi::bindings_info::kVersionSuffix, YOGI_HDR_VERSION_SUFFIX);
}