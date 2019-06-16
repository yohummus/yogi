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

#pragma once

#include "../config.h"
#include "context.h"

#include <boost/asio/high_resolution_timer.hpp>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace objects {

class Timer : public api::ExposedObjectT<Timer, api::ObjectType::kTimer> {
 public:
  typedef std::function<void(const api::Result&)> HandlerFn;

  Timer(ContextPtr context);

  void StartAsync(std::chrono::nanoseconds timeout, HandlerFn fn);
  bool Cancel();

 private:
  const ContextPtr context_;
  boost::asio::high_resolution_timer timer_;
};

typedef std::shared_ptr<Timer> TimerPtr;

}  // namespace objects
