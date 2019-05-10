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

#include "../common.h"

#include <yogi_core.h>

class SignalsTest : public testing::Test {
 protected:
  yogi::ContextPtr context_ = yogi::Context::Create();
};

TEST_F(SignalsTest, SignalsEnum) {
  // clang-format off
  CHECK_ENUM_ELEMENT(Signals, kNone, YOGI_SIG_NONE);
  CHECK_ENUM_ELEMENT(Signals, kInt,  YOGI_SIG_INT);
  CHECK_ENUM_ELEMENT(Signals, kTerm, YOGI_SIG_TERM);
  CHECK_ENUM_ELEMENT(Signals, kUsr1, YOGI_SIG_USR1);
  CHECK_ENUM_ELEMENT(Signals, kUsr2, YOGI_SIG_USR2);
  CHECK_ENUM_ELEMENT(Signals, kUsr3, YOGI_SIG_USR3);
  CHECK_ENUM_ELEMENT(Signals, kUsr4, YOGI_SIG_USR4);
  CHECK_ENUM_ELEMENT(Signals, kUsr5, YOGI_SIG_USR5);
  CHECK_ENUM_ELEMENT(Signals, kUsr6, YOGI_SIG_USR6);
  CHECK_ENUM_ELEMENT(Signals, kUsr7, YOGI_SIG_USR7);
  CHECK_ENUM_ELEMENT(Signals, kUsr8, YOGI_SIG_USR8);
  CHECK_ENUM_ELEMENT(Signals, kAll,  YOGI_SIG_ALL);
  // clang-format on

  auto signals = yogi::Signals::kNone;
  EXPECT_EQ(yogi::ToString(signals), "kNone");
  signals = signals | yogi::Signals::kInt;
  EXPECT_EQ(yogi::ToString(signals), "kInt");
  signals = signals | yogi::Signals::kTerm;
  EXPECT_EQ(yogi::ToString(signals), "kInt | kTerm");
  signals = signals | yogi::Signals::kUsr1;
  EXPECT_EQ(yogi::ToString(signals), "kInt | kTerm | kUsr1");
  signals = signals | yogi::Signals::kUsr2;
  EXPECT_EQ(yogi::ToString(signals), "kInt | kTerm | kUsr1 | kUsr2");
  signals = signals | yogi::Signals::kUsr3;
  EXPECT_EQ(yogi::ToString(signals), "kInt | kTerm | kUsr1 | kUsr2 | kUsr3");
  signals = signals | yogi::Signals::kUsr4;
  EXPECT_EQ(yogi::ToString(signals),
            "kInt | kTerm | kUsr1 | kUsr2 | kUsr3 | kUsr4");
  signals = signals | yogi::Signals::kUsr5;
  EXPECT_EQ(yogi::ToString(signals),
            "kInt | kTerm | kUsr1 | kUsr2 | kUsr3 | kUsr4 | kUsr5");
  signals = signals | yogi::Signals::kUsr6;
  EXPECT_EQ(yogi::ToString(signals),
            "kInt | kTerm | kUsr1 | kUsr2 | kUsr3 | kUsr4 | kUsr5 | kUsr6");
  signals = signals | yogi::Signals::kUsr7;
  EXPECT_EQ(
      yogi::ToString(signals),
      "kInt | kTerm | kUsr1 | kUsr2 | kUsr3 | kUsr4 | kUsr5 | kUsr6 | kUsr7");
  signals = signals | yogi::Signals::kUsr8;
  EXPECT_EQ(yogi::ToString(signals), "kAll");
}

TEST_F(SignalsTest, RaiseSignalCleanupHandler) {
  bool called = false;
  yogi::RaiseSignal(yogi::Signals::kUsr1, [&] { called = true; });
  EXPECT_TRUE(called);

  called = false;
  yogi::RaiseSignalWithArg<std::string>(yogi::Signals::kUsr2, "Hello",
                                        [&](std::string* sigarg) {
                                          ASSERT_NE(sigarg, nullptr);
                                          EXPECT_EQ(*sigarg, "Hello");
                                          called = true;
                                        });
  EXPECT_TRUE(called);
}

TEST_F(SignalsTest, AwaitSignal) {
  auto sigset = yogi::SignalSet::Create(
      context_, yogi::Signals::kTerm | yogi::Signals::kUsr5);

  bool called = false;
  sigset->AwaitSignal([&](auto& res, auto signal) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kOk);
    EXPECT_EQ(signal, yogi::Signals::kTerm);
    called = true;
  });

  yogi::RaiseSignalWithArg<std::string>(yogi::Signals::kTerm, "123");
  context_->Poll();
  EXPECT_TRUE(called);

  called = false;
  sigset->AwaitSignal<std::string>([&](auto& res, auto signal, auto* sigarg) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kOk);
    EXPECT_EQ(signal, yogi::Signals::kTerm);
    EXPECT_EQ(sigarg, nullptr);
    called = true;
  });

  yogi::RaiseSignal(yogi::Signals::kTerm);
  context_->Poll();
  EXPECT_TRUE(called);

  called = false;
  sigset->AwaitSignal<std::string>([&](auto& res, auto signal, auto* sigarg) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kOk);
    EXPECT_EQ(signal, yogi::Signals::kTerm);
    ASSERT_NE(sigarg, nullptr);
    EXPECT_EQ(*sigarg, "Hello");
    called = true;
  });

  yogi::RaiseSignalWithArg<std::string>(yogi::Signals::kTerm, "Hello");
  context_->Poll();
  EXPECT_TRUE(called);
}

TEST_F(SignalsTest, CancelAwaitSignal) {
  auto sigset = yogi::SignalSet::Create(context_, yogi::Signals::kTerm);

  EXPECT_FALSE(sigset->CancelAwaitSignal());

  bool called = false;
  sigset->AwaitSignal<std::string>([&](auto& res, auto signal, auto* sigarg) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Failure&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kCanceled);
    EXPECT_EQ(signal, yogi::Signals::kNone);
    EXPECT_EQ(sigarg, nullptr);
    called = true;
  });

  EXPECT_TRUE(sigset->CancelAwaitSignal());
  context_->Poll();
  EXPECT_TRUE(called);

  called = false;
  sigset->AwaitSignal([&](auto& res, auto signal) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Failure&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kCanceled);
    EXPECT_EQ(signal, yogi::Signals::kNone);
    called = true;
  });

  sigset->CancelAwaitSignal();
  context_->Poll();
  EXPECT_TRUE(called);
}
