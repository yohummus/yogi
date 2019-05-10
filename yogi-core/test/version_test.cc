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

TEST(VersionTest, Macros) {
	std::ostringstream ss;
	ss << YOGI_HDR_VERSION_MAJOR << '.' << YOGI_HDR_VERSION_MINOR << '.' << YOGI_HDR_VERSION_PATCH;

	auto version = ss.str();
	EXPECT_EQ(version, YOGI_HDR_VERSION);
}

TEST(VersionTest, GetVersion) {
  EXPECT_STREQ(YOGI_HDR_VERSION, YOGI_GetVersion());
}
