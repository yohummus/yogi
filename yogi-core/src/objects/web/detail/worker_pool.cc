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

#include "worker_pool.h"
#include "../../../utils/algorithm.h"
#include "../../../api/errors.h"

#include <algorithm>

namespace objects {
namespace web {
namespace detail {

Worker::Worker(ContextPtr context, bool is_fallback,
               LoadCounterPtr load_counter)
    : context_(context),
      is_fallback_(is_fallback),
      load_counter_(load_counter) {
  if (load_counter_) {
    ++*load_counter;
  }
}

Worker::Worker(Worker&& other) { *this = std::move(other); }

Worker::~Worker() {
  if (load_counter_) {
    --*load_counter_;
  }
}

Worker& Worker::operator=(Worker&& other) {
  context_ = std::move(other.context_);
  is_fallback_ = other.is_fallback_;
  load_counter_ = std::move(other.load_counter_);

  return *this;
}

WorkerPool::WorkerPool(ContextPtr fallback_context)
    : fallback_context_(fallback_context) {}

Worker WorkerPool::AcquireWorker() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (worker_contexts_.empty()) {
    return Worker(fallback_context_, true, {});
  }

  auto it =
      std::min_element(worker_contexts_.begin(), worker_contexts_.end(),
                       [](auto& elem, auto& smallest) {
                         return elem.second->load() < smallest.second->load();
                       });
  YOGI_ASSERT(it != worker_contexts_.end());

  return Worker(it->first, it->first == fallback_context_, it->second);
}

void WorkerPool::AddWorker(ContextPtr worker_context) {
  std::lock_guard<std::mutex> lock(mutex_);
  bool already_added = utils::contains_if(worker_contexts_, [&](auto& elem) {
    return elem.first == worker_context;
  });

  if (already_added) {
    throw api::Error(YOGI_ERR_WORKER_ALREADY_ADDED);
  }

  auto load_counter = std::make_shared<std::atomic<int>>(0);
  worker_contexts_.push_back(std::make_pair(worker_context, load_counter));
}

}  // namespace detail
}  // namespace web
}  // namespace objects
