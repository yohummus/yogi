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

class SignalSetTest : public TestFixture {
 protected:
  virtual void SetUp() override { context_ = CreateContext(); }

  void* CreateSignalSet(int signals) {
    void* sigset = nullptr;
    int res = YOGI_SignalSetCreate(&sigset, context_, signals);
    EXPECT_OK(res);
    EXPECT_NE(sigset, nullptr);
    return sigset;
  }

  void* context_;
};

TEST_F(SignalSetTest, AwaitAsync) {
  void* sigset_1 = CreateSignalSet(YOGI_SIG_INT);
  void* sigset_2 = CreateSignalSet(YOGI_SIG_INT | YOGI_SIG_USR1);
  void* sigset_3 = CreateSignalSet(YOGI_SIG_USR2);

  bool called_1 = false;
  int res = YOGI_SignalSetAwaitSignalAsync(
      sigset_1,
      [](int res, int sig, void* sigarg, void* userarg) {
        EXPECT_OK(res);
        EXPECT_EQ(sig, YOGI_SIG_INT);
        EXPECT_EQ(*static_cast<int*>(sigarg), 123);
        *static_cast<bool*>(userarg) = true;
      },
      &called_1);
  EXPECT_OK(res);

  bool called_2 = false;
  res = YOGI_SignalSetAwaitSignalAsync(
      sigset_2,
      [](int res, int sig, void* sigarg, void* userarg) {
        EXPECT_OK(res);
        EXPECT_EQ(sig, YOGI_SIG_INT);
        EXPECT_EQ(*static_cast<int*>(sigarg), 123);
        *static_cast<bool*>(userarg) = true;
      },
      &called_2);
  EXPECT_OK(res);

  bool called_3 = false;
  res = YOGI_SignalSetAwaitSignalAsync(sigset_3,
                                       [](int res, int, void*, void* userarg) {
                                         EXPECT_ERR(res, YOGI_ERR_CANCELED);
                                         *static_cast<bool*>(userarg) = true;
                                       },
                                       &called_3);
  EXPECT_OK(res);

  int sigarg = 123;
  res = YOGI_RaiseSignal(YOGI_SIG_INT, &sigarg, nullptr, nullptr);
  EXPECT_OK(res);

  YOGI_ContextRunOne(context_, nullptr, -1);
  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called_1);
  EXPECT_TRUE(called_2);
  EXPECT_FALSE(called_3);

  int n = -1;
  YOGI_ContextPollOne(context_, &n);
  EXPECT_EQ(n, 0);
}

TEST_F(SignalSetTest, CleanupHandler) {
  void* sigset_1 = CreateSignalSet(YOGI_SIG_INT);
  void* sigset_2 = CreateSignalSet(YOGI_SIG_INT);

  int sigarg = 456;
  std::vector<bool> calls;
  int res = YOGI_RaiseSignal(
      YOGI_SIG_INT, &sigarg,
      [](void* sigarg, void* userarg) {
        EXPECT_EQ(*static_cast<int*>(sigarg), 456);
        static_cast<std::vector<bool>*>(userarg)->push_back(true);
      },
      &calls);
  EXPECT_OK(res);
  EXPECT_TRUE(calls.empty());

  res = YOGI_SignalSetAwaitSignalAsync(
      sigset_1,
      [](int, int, void*, void* userarg) {
        static_cast<std::vector<bool>*>(userarg)->push_back(false);
      },
      &calls);
  EXPECT_OK(res);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_EQ(calls, std::vector<bool>({false}));

  res = YOGI_SignalSetAwaitSignalAsync(
      sigset_2,
      [](int, int, void*, void* userarg) {
        static_cast<std::vector<bool>*>(userarg)->push_back(false);
      },
      &calls);
  EXPECT_OK(res);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_EQ(calls, std::vector<bool>({false, false, true}));
}

TEST_F(SignalSetTest, CleanupHandlerNoSets) {
  CreateSignalSet(YOGI_SIG_INT);
  CreateSignalSet(YOGI_SIG_INT);

  int sigarg = 123;
  bool called = false;
  int res = YOGI_RaiseSignal(YOGI_SIG_TERM, &sigarg,
                             [](void* sigarg, void* userarg) {
                               EXPECT_EQ(*static_cast<int*>(sigarg), 123);
                               *static_cast<bool*>(userarg) = true;
                             },
                             &called);
  EXPECT_OK(res);
  EXPECT_TRUE(called);
}

TEST_F(SignalSetTest, CleanupHandlerSetDestruction) {
  void* sigset = CreateSignalSet(YOGI_SIG_INT);

  int sigarg = 123;
  bool called = false;
  int res = YOGI_RaiseSignal(YOGI_SIG_INT, &sigarg,
                             [](void* sigarg, void* userarg) {
                               EXPECT_EQ(*static_cast<int*>(sigarg), 123);
                               *static_cast<bool*>(userarg) = true;
                             },
                             &called);
  EXPECT_OK(res);
  EXPECT_FALSE(called);

  YOGI_Destroy(sigset);
  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called);
}

TEST_F(SignalSetTest, Queueing) {
  void* sigset = CreateSignalSet(YOGI_SIG_INT);

  int sigarg_1 = 123;
  int res = YOGI_RaiseSignal(YOGI_SIG_INT, &sigarg_1, nullptr, nullptr);
  EXPECT_OK(res);

  int sigarg_2 = 456;
  res = YOGI_RaiseSignal(YOGI_SIG_TERM, &sigarg_2, nullptr, nullptr);
  EXPECT_OK(res);

  int sigarg_3 = 789;
  res = YOGI_RaiseSignal(YOGI_SIG_INT, &sigarg_3, nullptr, nullptr);
  EXPECT_OK(res);

  std::vector<int> sigargs;
  res = YOGI_SignalSetAwaitSignalAsync(
      sigset,
      [](int res, int sig, void* sigarg, void* userarg) {
        EXPECT_OK(res);
        EXPECT_EQ(sig, YOGI_SIG_INT);
        static_cast<std::vector<int>*>(userarg)->push_back(
            *static_cast<int*>(sigarg));
      },
      &sigargs);
  EXPECT_OK(res);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_EQ(sigargs, std::vector<int>({123}));

  res = YOGI_SignalSetAwaitSignalAsync(
      sigset,
      [](int res, int sig, void* sigarg, void* userarg) {
        EXPECT_OK(res);
        EXPECT_EQ(sig, YOGI_SIG_INT);
        static_cast<std::vector<int>*>(userarg)->push_back(
            *static_cast<int*>(sigarg));
      },
      &sigargs);
  EXPECT_OK(res);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_EQ(sigargs, std::vector<int>({123, 789}));

  int n = -1;
  YOGI_ContextPollOne(context_, &n);
  EXPECT_EQ(n, 0);
}

TEST_F(SignalSetTest, CancelAwait) {
  void* sigset = CreateSignalSet(YOGI_SIG_INT);

  int res = YOGI_SignalSetCancelAwaitSignal(sigset);
  EXPECT_ERR(res, YOGI_ERR_OPERATION_NOT_RUNNING);

  bool called = false;
  YOGI_SignalSetAwaitSignalAsync(
      sigset,
      [](int res, int sig, void* sigarg, void* userarg) {
        EXPECT_ERR(res, YOGI_ERR_CANCELED);
        EXPECT_EQ(sig, YOGI_SIG_NONE);
        EXPECT_EQ(sigarg, nullptr);
        *static_cast<bool*>(userarg) = true;
      },
      &called);

  res = YOGI_SignalSetCancelAwaitSignal(sigset);
  EXPECT_OK(res);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called);
}

TEST_F(SignalSetTest, OverrideAwait) {
  void* sigset = CreateSignalSet(YOGI_SIG_INT);

  bool called_1 = false;
  YOGI_SignalSetAwaitSignalAsync(
      sigset,
      [](int res, int sig, void* sigarg, void* userarg) {
        EXPECT_ERR(res, YOGI_ERR_CANCELED);
        EXPECT_EQ(sig, YOGI_SIG_NONE);
        EXPECT_EQ(sigarg, nullptr);
        *static_cast<bool*>(userarg) = true;
      },
      &called_1);

  bool called_2 = false;
  int res = YOGI_SignalSetAwaitSignalAsync(sigset,
                                           [](int, int, void*, void* userarg) {
                                             *static_cast<bool*>(userarg) =
                                                 true;
                                           },
                                           &called_2);
  EXPECT_OK(res);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called_1);
  EXPECT_FALSE(called_2);
}

TEST_F(SignalSetTest, AwaitOnDestruction) {
  void* sigset = CreateSignalSet(YOGI_SIG_INT);

  bool called = false;
  YOGI_SignalSetAwaitSignalAsync(
      sigset,
      [](int res, int sig, void* sigarg, void* userarg) {
        EXPECT_ERR(res, YOGI_ERR_CANCELED);
        EXPECT_EQ(sig, YOGI_SIG_NONE);
        EXPECT_EQ(sigarg, nullptr);
        *static_cast<bool*>(userarg) = true;
      },
      &called);

  YOGI_Destroy(sigset);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called);
}
