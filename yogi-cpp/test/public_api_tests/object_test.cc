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

#include <regex>

class FakeObject : public yogi::Object {
 public:
  FakeObject() : yogi::Object(nullptr, {}) {}
};

class ObjectTest : public testing::Test {
 protected:
  yogi::ContextPtr object_ = yogi::Context::Create();
};

TEST_F(ObjectTest, Format) {
  std::string s = object_->Format();
  CheckStringMatches(s, "^Context \\[[1-9a-f][0-9a-f]+\\]$");

  s = object_->Format("$T-[$X]");
  CheckStringMatches(s, "^Context-\\[[1-9A-F][0-9A-F]+\\]$");

  FakeObject obj;
  s = obj.Format();
  EXPECT_EQ(s.find("INVALID"), 0u);

  s = obj.Format(nullptr, "MOO");
  EXPECT_EQ(s, "MOO");
}

TEST_F(ObjectTest, ToString) {
  CheckStringMatches(object_->ToString(), "^Context \\[[1-9a-f][0-9a-f]+\\]$");
}
