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

#include "../../common.h"

class WebServerTest : public TestFixture {
 protected:
  void* context_ = CreateContext();
};

TEST_F(WebServerTest, ConstructWithoutBranch) {
  CreateWebServer(context_, nullptr, nullptr);
}

TEST_F(WebServerTest, WelcomePage) {
  CreateWebServer(context_, nullptr, nullptr);
  RunContextInBackground(context_);
  getchar();
}
