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
#include "../../src/utils/bind.h"

#include <memory>

struct Base : public std::enable_shared_from_this<Base> {};

struct Derived : public Base {
  void FunctionWithNoArgs() { called = true; }

  void FunctionWithOneArg(std::string s) {
    EXPECT_EQ(s, "hello");
    called = true;
  }

  void FunctionWithTwoArgs(std::string s, int x) {
    EXPECT_EQ(s, "hello");
    EXPECT_EQ(x, 123);
    called = true;
  }

  bool called = false;
};

class BindTest : public TestFixture {
 protected:
  std::shared_ptr<Derived> obj_ = std::make_shared<Derived>();
};

TEST_F(BindTest, WeakFunctionWithNoArguments) {
  auto fn = utils::BindWeak(&Derived::FunctionWithNoArgs, obj_.get());
  fn();
  EXPECT_TRUE(obj_->called);
}

TEST_F(BindTest, StrongFunctionWithNoArguments) {
  auto fn = utils::BindStrong(&Derived::FunctionWithNoArgs, obj_.get());
  fn();
  EXPECT_TRUE(obj_->called);
}

TEST_F(BindTest, WeakFunctionWithOneArgument) {
  auto fn = utils::BindWeak(&Derived::FunctionWithOneArg, obj_.get());
  fn("hello");
  EXPECT_TRUE(obj_->called);
}

TEST_F(BindTest, StrongFunctionWithOneArgument) {
  auto fn = utils::BindStrong(&Derived::FunctionWithOneArg, obj_.get());
  fn("hello");
  EXPECT_TRUE(obj_->called);
}

TEST_F(BindTest, WeakFunctionWithTwoArguments) {
  auto fn = utils::BindWeak(&Derived::FunctionWithTwoArgs, obj_.get());
  fn("hello", 123);
  EXPECT_TRUE(obj_->called);
}

TEST_F(BindTest, StrongFunctionWithTwoArguments) {
  auto fn = utils::BindStrong(&Derived::FunctionWithTwoArgs, obj_.get());
  fn("hello", 123);
  EXPECT_TRUE(obj_->called);
}

TEST_F(BindTest, WeakExpired) {
  auto fn = utils::BindWeak(&Derived::FunctionWithNoArgs, obj_.get());
  obj_ = {};
  EXPECT_NO_THROW(fn()); // Should not crash
}
