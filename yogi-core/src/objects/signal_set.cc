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

#include "signal_set.h"

#include <algorithm>

namespace objects {

void SignalSet::RaiseSignal(api::Signals signal, void* sigarg,
                            CleanupHandler cleanup_handler) {
  YOGI_ASSERT(signal != api::kNoSignal);

  auto data = std::make_shared<SignalData>();
  data->signal = signal;
  data->sigarg = sigarg;
  data->cleanup_handler = cleanup_handler;

  auto sets = api::ObjectRegister::GetMatching<SignalSet>(
      [&](auto& set) { return set->GetSignals() & signal; });

  data->cnt = static_cast<int>(sets.size());
  if (data->cnt == 0) {
    data->cleanup_handler();
    return;
  }

  for (auto& set : sets) {
    set->OnSignalRaised(data);
  }
}

SignalSet::SignalSet(ContextPtr context, api::Signals signals)
    : context_(context), signals_(signals) {}

bool SignalSet::AwaitAsync(AwaitHandler handler) {
  std::lock_guard<std::mutex> lock(mutex_);

  bool canceled = false;
  if (handler_) {
    canceled = true;
    auto old_handler = handler_;
    context_->Post([old_handler] {
      old_handler(api::Error(YOGI_ERR_CANCELED), api::kNoSignal, nullptr);
    });
  }

  handler_ = handler;

  if (!queue_.empty()) {
    DeliverNextSignal();
  }

  return canceled;
}

SignalSet::~SignalSet() {
  while (!queue_.empty()) {
    auto data = queue_.front();
    queue_.pop();

    --data->cnt;
    if (data->cnt == 0) {
      context_->Post([data] { data->cleanup_handler(); });
    }
  }

  CancelAwait();
}

bool SignalSet::CancelAwait() { return AwaitAsync({}); }

void SignalSet::OnSignalRaised(const SignalDataPtr& data) {
  YOGI_ASSERT(signals_ & data->signal);

  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(data);
  DeliverNextSignal();
}

void SignalSet::DeliverNextSignal() {
  YOGI_ASSERT(!queue_.empty());

  if (!handler_) {
    return;
  }

  AwaitHandler handler;
  std::swap(handler_, handler);

  auto data = queue_.front();
  queue_.pop();

  context_->Post([handler, data] {
    handler(api::kSuccess, data->signal, data->sigarg);
    --data->cnt;
    if (data->cnt == 0) {
      data->cleanup_handler();
    }
  });
}

}  // namespace objects
