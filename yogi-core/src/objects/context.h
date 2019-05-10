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

#pragma once

#include "../config.h"
#include "../api/object.h"
#include "../api/enums.h"
#include "logger.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace objects {

class Context : public api::ExposedObjectT<Context, api::ObjectType::kContext> {
 public:
  typedef std::function<void(const api::Result&, api::Signals)> SignalHandler;

  Context();
  virtual ~Context();

  boost::asio::io_context& IoContext() { return ioc_; }

  int Poll();
  int PollOne();
  int Run(std::chrono::nanoseconds dur);
  int RunOne(std::chrono::nanoseconds dur);
  void RunInBackground();
  void Stop();
  bool WaitForRunning(std::chrono::nanoseconds timeout);
  bool WaitForStopped(std::chrono::nanoseconds timeout);
  void Post(std::function<void()> fn);
  void AwaitSignal(api::Signals signals, SignalHandler signal_handler);
  void CancelAwaitSignal();

 private:
  void SetRunningFlagAndReset();
  void ClearRunningFlag();

  template <typename Fn>
  int RunImpl(Fn fn);

  static const LoggerPtr logger_;

  boost::asio::io_context ioc_;
  boost::asio::io_context::work work_;
  boost::asio::signal_set signals_;
  bool running_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread thread_;
};

typedef std::shared_ptr<Context> ContextPtr;

}  // namespace objects
