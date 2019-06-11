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

class WebServerTest : public TestFixture {
 protected:
  void* context_ = CreateContext();
};

TEST_F(WebServerTest, ConstructWithoutBranch) {
  char err[256];
  void* server;
  int res = YOGI_WebServerCreate(&server, context_, nullptr, nullptr, nullptr,
                                 err, sizeof(err));
  EXPECT_OK(res) << err;
  EXPECT_NE(server, nullptr);
}
