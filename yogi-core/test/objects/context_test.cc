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

#include <atomic>
#include <thread>

class ContextTest : public TestFixture {
 protected:
  virtual void SetUp() override { context_ = CreateContext(); }

  void* context_;
};

TEST_F(ContextTest, CreateAndDestroy) {
  // Context gets created in SetUp()
  int res = YOGI_Destroy(context_);
  EXPECT_OK(res);
}

TEST_F(ContextTest, Poll) {
  int res = YOGI_ContextPoll(context_, nullptr);
  EXPECT_OK(res);

  int cnt = -1;
  res = YOGI_ContextPoll(context_, &cnt);
  EXPECT_OK(res);
  EXPECT_EQ(cnt, 0);

  res = YOGI_ContextPost(context_, [](void*) {}, nullptr);
  EXPECT_OK(res);
  res = YOGI_ContextPost(context_, [](void*) {}, nullptr);
  EXPECT_OK(res);

  res = YOGI_ContextPoll(context_, &cnt);
  EXPECT_OK(res);
  EXPECT_EQ(cnt, 2);
}

TEST_F(ContextTest, PollOne) {
  int res = YOGI_ContextPollOne(context_, nullptr);
  EXPECT_OK(res);

  int cnt = -1;
  res = YOGI_ContextPollOne(context_, &cnt);
  EXPECT_OK(res);
  EXPECT_EQ(cnt, 0);

  YOGI_ContextPost(context_, [](void*) {}, nullptr);
  YOGI_ContextPost(context_, [](void*) {}, nullptr);

  res = YOGI_ContextPollOne(context_, &cnt);
  EXPECT_OK(res);
  EXPECT_EQ(cnt, 1);
}

TEST_F(ContextTest, Run) {
  int res;
  std::thread th([&] { res = YOGI_ContextRun(context_, nullptr, -1); });
  YOGI_ContextWaitForRunning(context_, -1);

  YOGI_ContextStop(context_);
  th.join();
  EXPECT_OK(res);

  int count = -1;
  th = std::thread([&] { res = YOGI_ContextRun(context_, &count, -1); });
  YOGI_ContextWaitForRunning(context_, -1);

  std::atomic<int> n(0);
  YOGI_ContextPost(context_,
                   [](void* n_) { ++*static_cast<std::atomic<int>*>(n_); }, &n);
  YOGI_ContextPost(context_,
                   [](void* n_) { ++*static_cast<std::atomic<int>*>(n_); }, &n);

  while (n < 2)
    ;

  YOGI_ContextStop(context_);
  th.join();
  EXPECT_OK(res);
  EXPECT_EQ(count, 2);
  EXPECT_EQ(n, 2);
}

TEST_F(ContextTest, RunOne) {
  int res;
  std::thread th([&] { res = YOGI_ContextRunOne(context_, nullptr, -1); });
  YOGI_ContextWaitForRunning(context_, -1);

  YOGI_ContextStop(context_);
  th.join();
  EXPECT_OK(res);

  int count = -1;
  th = std::thread([&] { res = YOGI_ContextRunOne(context_, &count, -1); });
  YOGI_ContextWaitForRunning(context_, -1);

  std::atomic<int> n(0);
  YOGI_ContextPost(context_,
                   [](void* n_) { ++*static_cast<std::atomic<int>*>(n_); }, &n);
  YOGI_ContextPost(context_,
                   [](void* n_) { ++*static_cast<std::atomic<int>*>(n_); }, &n);

  while (n < 1)
    ;

  YOGI_ContextStop(context_);
  th.join();
  EXPECT_OK(res);
  EXPECT_EQ(count, 1);
  EXPECT_EQ(n, 1);
}

TEST_F(ContextTest, RunFor) {
  int res = YOGI_ContextRun(context_, nullptr, 0);
  EXPECT_OK(res);

  int n = 0;
  YOGI_ContextPost(context_, [](void* n_) { ++*static_cast<int*>(n_); }, &n);
  YOGI_ContextPost(context_, [](void* n_) { ++*static_cast<int*>(n_); }, &n);

  int count = -1;
  auto start_time = std::chrono::steady_clock::now();
  res = YOGI_ContextRun(context_, &count, 5000000);
  auto dur = std::chrono::steady_clock::now() - start_time;

  EXPECT_OK(res);
  EXPECT_GE(dur, 5ms);
  EXPECT_LT(dur, 5ms + kTimingMargin);
  EXPECT_EQ(count, 2);
  EXPECT_EQ(n, 2);
}

TEST_F(ContextTest, RunOneFor) {
  int res = YOGI_ContextRunOne(context_, nullptr, 0);
  EXPECT_OK(res);

  int n = 0;
  YOGI_ContextPost(context_, [](void* n_) { ++*static_cast<int*>(n_); }, &n);

  int count = -1;
  auto start_time = std::chrono::steady_clock::now();
  res = YOGI_ContextRunOne(context_, &count, 5000000);
  auto dur = std::chrono::steady_clock::now() - start_time;

  EXPECT_OK(res);
  EXPECT_LT(dur, 5ms + kTimingMargin);
  EXPECT_EQ(count, 1);
  EXPECT_EQ(n, 1);

  start_time = std::chrono::steady_clock::now();
  res = YOGI_ContextRunOne(context_, &count, 5000000);
  dur = std::chrono::steady_clock::now() - start_time;

  EXPECT_OK(res);
  EXPECT_GE(dur, 5ms);
  EXPECT_LT(dur, 5ms + kTimingMargin);
  EXPECT_EQ(count, 0);
  EXPECT_EQ(n, 1);
}

TEST_F(ContextTest, RunInBackground) {
  std::atomic<int> n(0);
  YOGI_ContextPost(context_,
                   [](void* n_) { ++*static_cast<std::atomic<int>*>(n_); }, &n);

  int res = YOGI_ContextRunInBackground(context_);
  EXPECT_OK(res);

  YOGI_ContextPost(context_,
                   [](void* n_) { ++*static_cast<std::atomic<int>*>(n_); }, &n);

  while (n != 2)
    ;
}

TEST_F(ContextTest, WaitForStopped) {
  YOGI_ContextRunInBackground(context_);

  int res = YOGI_ContextWaitForStopped(context_, 0);
  EXPECT_ERR(res, YOGI_ERR_TIMEOUT);

  auto start_time = std::chrono::steady_clock::now();
  res = YOGI_ContextWaitForStopped(context_, 1000000);
  auto dur = std::chrono::steady_clock::now() - start_time;
  EXPECT_ERR(res, YOGI_ERR_TIMEOUT);
  EXPECT_GE(dur, 1ms);
  EXPECT_LT(dur, 1ms + kTimingMargin);

  YOGI_ContextStop(context_);
  res = YOGI_ContextWaitForStopped(context_, 1000000000);
  EXPECT_OK(res);
}

TEST_F(ContextTest, ExceptionInBackgroundThread) {
  int res = YOGI_ContextRunInBackground(context_);
  EXPECT_OK(res);

  YOGI_ContextPost(context_,
                   [](void*) { throw std::runtime_error("My exception"); },
                   nullptr);

  // Background thread should stop and print an error message
  res = YOGI_ContextWaitForStopped(context_, 1000000);
  EXPECT_OK(res);
}

TEST_F(ContextTest, Post) {
  int res = YOGI_ContextPost(context_, [](void*) {}, nullptr);
  EXPECT_OK(res);
}
