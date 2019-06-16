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
#include "../api/errors.h"
#include "../api/enums.h"
#include "context.h"

#include <mutex>
#include <functional>
#include <queue>

namespace objects {

class SignalSet
    : public api::ExposedObjectT<SignalSet, api::ObjectType::kSignalSet> {
 public:
  typedef std::function<void()> CleanupHandler;
  typedef std::function<void(const api::Result& res, api::Signals signal,
                             void* sigarg)>
      AwaitHandler;

  static void RaiseSignal(api::Signals signal, void* sigarg,
                          CleanupHandler cleanup_handler);

  SignalSet(ContextPtr context, api::Signals signals);
  virtual ~SignalSet();

  api::Signals GetSignals() const { return signals_; }
  bool AwaitAsync(AwaitHandler handler);
  bool CancelAwait();

 private:
  struct SignalData {
    api::Signals signal;
    void* sigarg;
    int cnt;
    CleanupHandler cleanup_handler;
  };

  typedef std::shared_ptr<SignalData> SignalDataPtr;

  void OnSignalRaised(const SignalDataPtr& data);
  void DeliverNextSignal();

  const ContextPtr context_;
  const api::Signals signals_;

  std::mutex mutex_;
  AwaitHandler handler_;
  std::queue<SignalDataPtr> queue_;
};

typedef std::shared_ptr<SignalSet> SignalSetPtr;

}  // namespace objects
