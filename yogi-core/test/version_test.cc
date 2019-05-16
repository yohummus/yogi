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

#include "common.h"

TEST(VersionTest, HeaderVersion) {
  EXPECT_GE(YOGI_HDR_VERSION_MAJOR, 0);
  EXPECT_GE(YOGI_HDR_VERSION_MINOR, 0);
  EXPECT_GE(YOGI_HDR_VERSION_PATCH, 0);

  auto version =
      MakeVersionString(YOGI_HDR_VERSION_MAJOR, YOGI_HDR_VERSION_MINOR,
                        YOGI_HDR_VERSION_PATCH, YOGI_HDR_VERSION_SUFFIX);

  EXPECT_EQ(version, YOGI_HDR_VERSION);
}

TEST(VersionTest, GetVersion) {
  EXPECT_STREQ(YOGI_HDR_VERSION, YOGI_GetVersion());
}

TEST(VersionTest, CheckBindingsCompatibility) {
  char err[128] = {123};

  EXPECT_OK(
      YOGI_CheckBindingsCompatibility(YOGI_HDR_VERSION, err, sizeof(err)));
  EXPECT_STREQ(err, "");

  EXPECT_EQ(
      YOGI_CheckBindingsCompatibility(YOGI_HDR_VERSION " ", err, sizeof(err)),
      YOGI_ERR_INVALID_PARAM);
  EXPECT_NE(std::string(err).find("not have a valid format"),
            std::string::npos);

  auto version =
      MakeVersionString(YOGI_HDR_VERSION_MAJOR, YOGI_HDR_VERSION_MINOR + 1,
                        YOGI_HDR_VERSION_PATCH + 1);
  EXPECT_OK(YOGI_CheckBindingsCompatibility(version.c_str(), err, sizeof(err)));
  EXPECT_STREQ(err, "");

  version = MakeVersionString(YOGI_HDR_VERSION_MAJOR + 1,
                              YOGI_HDR_VERSION_MINOR, YOGI_HDR_VERSION_PATCH);
  EXPECT_EQ(YOGI_CheckBindingsCompatibility(version.c_str(), err, sizeof(err)),
            YOGI_ERR_INCOMPATIBLE_VERSION);
  EXPECT_NE(std::string(err).find("is incompatible with"), std::string::npos);

  if (YOGI_HDR_VERSION_MINOR > 0) {
    version =
        MakeVersionString(YOGI_HDR_VERSION_MAJOR, YOGI_HDR_VERSION_MINOR - 1,
                          YOGI_HDR_VERSION_PATCH);
    EXPECT_EQ(
        YOGI_CheckBindingsCompatibility(version.c_str(), err, sizeof(err)),
        YOGI_ERR_INCOMPATIBLE_VERSION);
    EXPECT_NE(std::string(err).find("is incompatible with"), std::string::npos);
  }
}
