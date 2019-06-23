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

#include "../../../config.h"
#include "../../context.h"

#include <mutex>
#include <memory>
#include <atomic>
#include <vector>

namespace objects {
namespace web {
namespace detail {

class Worker final {
 public:
  typedef std::shared_ptr<std::atomic<int>> LoadCounterPtr;

  Worker() {}
  Worker(ContextWeakPtr context, LoadCounterPtr load_counter);
  Worker(const Worker&) = delete;
  Worker(Worker&& other);
  ~Worker();

  Worker& operator=(Worker&& other);
  Worker& operator=(const Worker&) = delete;

  const ContextWeakPtr& Context() { return context_; }

 private:
  ContextWeakPtr context_;
  LoadCounterPtr load_counter_;
};

class WorkerPool {
 public:
  WorkerPool(ContextPtr fallback_context);

  Worker AcquireWorker();
  void AddWorker(ContextPtr worker_context);

 private:
  using LoadCounterPtr = Worker::LoadCounterPtr;

  const ContextPtr fallback_context_;
  std::mutex mutex_;
  std::vector<std::pair<ContextPtr, LoadCounterPtr>> worker_contexts_;
};

}  // namespace detail
}  // namespace web
}  // namespace objects
