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

#include "context.h"
#include "../api/errors.h"

#include <boost/asio/post.hpp>
#include <signal.h>

YOGI_DEFINE_INTERNAL_LOGGER("Context")

namespace objects {

Context::Context() : ioc_(1), work_(ioc_), signals_(ioc_), running_(false) {
  SetLoggingPrefix(*this);
}

Context::~Context() {
  Stop();
  WaitForStopped(std::chrono::nanoseconds::max());
}

int Context::Poll() {
  return RunImpl([&] { return ioc_.poll(); });
}

int Context::PollOne() {
  return RunImpl([&] { return ioc_.poll_one(); });
}

int Context::Run(std::chrono::nanoseconds dur) {
  return RunImpl([&] {
    if (dur == dur.max()) {
      return ioc_.run();
    } else {
      return ioc_.run_for(dur);
    }
  });
}

int Context::RunOne(std::chrono::nanoseconds dur) {
  return RunImpl([&] {
    if (dur == dur.max()) {
      return ioc_.run_one();
    } else {
      return ioc_.run_one_for(dur);
    }
  });
}

void Context::RunInBackground() {
  SetRunningFlagAndReset();
  thread_ = std::thread([&] {
    try {
      ioc_.run();
    } catch (const std::exception& e) {
      LOG_FAT("Exception caught in context background thread: " << e.what());
    } catch (...) {
      LOG_FAT("Unknown Exception caught in context background thread");
    }

    ClearRunningFlag();
  });
}

void Context::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  ioc_.stop();
}

bool Context::WaitForRunning(std::chrono::nanoseconds timeout) {
  std::unique_lock<std::mutex> lock(mutex_);
  bool timed_out = false;
  if (timeout == timeout.max()) {
    cv_.wait(lock, [&] { return running_; });
  } else {
    timed_out = !cv_.wait_for(lock, timeout, [&] { return running_; });
  }

  return !timed_out;
}

bool Context::WaitForStopped(std::chrono::nanoseconds timeout) {
  std::unique_lock<std::mutex> lock(mutex_);
  bool timed_out = false;
  if (timeout == timeout.max()) {
    cv_.wait(lock, [&] { return !running_; });
  } else {
    timed_out = !cv_.wait_for(lock, timeout, [&] { return !running_; });
  }

  if (!timed_out && thread_.joinable()) {
    thread_.join();
  }

  return !timed_out;
}

void Context::Post(std::function<void()> fn) { boost::asio::post(ioc_, fn); }

void Context::AwaitSignal(api::Signals sigs, SignalHandler signal_handler) {
  std::lock_guard<std::mutex> lock(mutex_);

  boost::system::error_code ec;
  signals_.cancel(ec);
  if (ec) {
    LOG_ERR("Could not cancel wait for signal operation: " << ec.message());
    throw api::Error(YOGI_ERR_UNKNOWN);
  }

  signals_.clear(ec);
  if (ec) {
    LOG_ERR("Could not clear signal set: " << ec.message());
    throw api::Error(YOGI_ERR_UNKNOWN);
  }

  if (sigs & api::kSigInt) {
    signals_.add(SIGINT, ec);
  }

  if (!ec && sigs & api::kSigTerm) {
    signals_.add(SIGTERM, ec);
  }

  if (ec) {
    LOG_ERR("Could not add SIGINT to signal set: " << ec.message());
    throw api::Error(YOGI_ERR_UNKNOWN);
  }

  signals_.async_wait([=](auto& ec, int sig_num) {
    auto res = api::kSuccess;
    auto sig = sigs;

    if (!ec) {
      switch (sig_num) {
        case SIGINT:
          sig = api::kSigInt;
          break;

        case SIGTERM:
          sig = api::kSigTerm;
          break;

        default:
          YOGI_NEVER_REACHED;
      }
    } else if (ec == boost::asio::error::operation_aborted) {
      res = api::Error(YOGI_ERR_CANCELED);
    } else {
      LOG_ERR("Could not wait for signal set: " << ec.message());
      res = api::Error(YOGI_ERR_UNKNOWN);
    }

    signal_handler(res, sig);
  });
}

void Context::CancelAwaitSignal() { signals_.cancel(); }

void Context::SetRunningFlagAndReset() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (running_) {
    throw api::Error(YOGI_ERR_BUSY);
  }

  running_ = true;
  ioc_.restart();
  cv_.notify_all();
}

void Context::ClearRunningFlag() {
  std::lock_guard<std::mutex> lock(mutex_);
  YOGI_ASSERT(running_ == true);
  running_ = false;
  cv_.notify_all();
}

template <typename Fn>
int Context::RunImpl(Fn fn) {
  SetRunningFlagAndReset();
  int cnt = static_cast<int>(fn());
  ClearRunningFlag();
  return cnt;
}

}  // namespace objects
