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

#ifndef YOGI_TIMER_H
#define YOGI_TIMER_H

//! \file
//!
//! Timer implementation.

#include "duration.h"
#include "object.h"
#include "context.h"

#include <memory>
#include <functional>

namespace yogi {

_YOGI_DEFINE_API_FN(int, YOGI_TimerCreate, (void** timer, void* context))

_YOGI_DEFINE_API_FN(int, YOGI_TimerStartAsync,
                    (void* timer, long long duration,
                     void (*fn)(int res, void* userarg), void* userarg))

_YOGI_DEFINE_API_FN(int, YOGI_TimerCancel, (void* timer))

class Timer;

/// Shared pointer to a timer.
using TimerPtr = std::shared_ptr<Timer>;

////////////////////////////////////////////////////////////////////////////////
/// Simple timer implementation.
////////////////////////////////////////////////////////////////////////////////
class Timer : public ObjectT<Timer> {
 public:
  /// Handler function to be called once a timer expires.
  ///
  /// \param res %Result of the wait operation.
  using HandlerFn = std::function<void(const Result& res)>;

  /// Creates a timer.
  ///
  /// \param context The context to use.
  ///
  /// \returns The created timer.
  static TimerPtr Create(ContextPtr context) {
    return TimerPtr(new Timer(context));
  }

  /// Starts the timer.
  ///
  /// If the timer is already running, the timer will be canceled first,
  /// as if Cancel() were called explicitly.
  ///
  /// \param duration Time when the timer expires.
  /// \param fn       Handler function to call after the given time passed.
  void Start(const Duration& duration, HandlerFn fn) {
    struct CallbackData {
      HandlerFn fn;
    };

    auto data = std::make_unique<CallbackData>();
    data->fn = fn;

    int res = internal::YOGI_TimerStartAsync(
        GetHandle(), internal::ToCoreDuration(duration),
        [](int res, void* userarg) {
          auto data = std::unique_ptr<CallbackData>(
              static_cast<CallbackData*>(userarg));

          internal::WithErrorCodeToResult(res, [&](const auto& result) {
            if (data->fn) {
              data->fn(result);
            }
          });
        },
        data.get());

    internal::CheckErrorCode(res);
    data.release();
  }

  /// Cancels the timer.
  ///
  /// Canceling the timer will result in the handler function registered via
  /// Start() to be called with a cancellation error.static Note that if the
  /// handler is already scheduled for execution, it will be called without an
  /// error.
  ///
  /// \returns True if the timer was canceled successfully and false otherwise
  ///          (i.e. the timer has not been started or it already expired).
  bool Cancel() {
    int res = internal::YOGI_TimerCancel(GetHandle());
    if (res == static_cast<int>(ErrorCode::kTimerExpired)) {
      return false;
    }

    internal::CheckErrorCode(res);
    return true;
  }

 private:
  Timer(ContextPtr context)
      : ObjectT(internal::CallApiCreate(internal::YOGI_TimerCreate,
                                        GetForeignHandle(context)),
                {context}) {}
};

}  // namespace yogi

#endif  // YOGI_TIMER_H
