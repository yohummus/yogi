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

#include "macros.h"
#include "helpers.h"
#include "../objects/signal_set.h"

YOGI_API int YOGI_RaiseSignal(int signal, void* sigarg,
                              void (*fn)(void* sigarg, void* userarg),
                              void* userarg) {
  using Signals = api::Signals;

  CHECK_FLAGS(signal, Signals::kAllSignals);
  CHECK_PARAM(IsExactlyOneBitSet(signal));

  try {
    objects::SignalSet::CleanupHandler handler;
    if (fn) {
      handler = [fn, sigarg, userarg] { fn(sigarg, userarg); };
    } else {
      handler = [] {};
    }

    objects::SignalSet::RaiseSignal(ConvertFlags(signal, Signals::kNoSignal),
                                    sigarg, handler);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_SignalSetCreate(void** sigset, void* context, int signals) {
  using Signals = api::Signals;

  CHECK_PARAM(sigset != nullptr);
  CHECK_PARAM(context != nullptr);
  CHECK_FLAGS(signals, Signals::kAllSignals);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto set = objects::SignalSet::Create(
        ctx, ConvertFlags(signals, Signals::kNoSignal));
    *sigset = api::ObjectRegister::Register(set);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_SignalSetAwaitSignalAsync(
    void* sigset, void (*fn)(int res, int sig, void* sigarg, void* userarg),
    void* userarg) {
  CHECK_PARAM(sigset != nullptr);
  CHECK_PARAM(fn != nullptr);

  try {
    auto set = api::ObjectRegister::Get<objects::SignalSet>(sigset);
    set->AwaitAsync([=](auto& res, auto signal, void* sigarg) {
      fn(res.GetValue(), signal, sigarg, userarg);
    });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_SignalSetCancelAwaitSignal(void* sigset) {
  CHECK_PARAM(sigset != nullptr);

  try {
    auto set = api::ObjectRegister::Get<objects::SignalSet>(sigset);
    if (!set->CancelAwait()) {
      return YOGI_ERR_OPERATION_NOT_RUNNING;
    }
  }
  CATCH_AND_RETURN;
}
