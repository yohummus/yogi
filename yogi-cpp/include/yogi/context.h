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

#ifndef YOGI_CONTEXT_H
#define YOGI_CONTEXT_H

//! \file
//!
//! Contexts (schedulers for the execution of asynchronous operations).

#include "object.h"
#include "duration.h"
#include "internal/duration_conversion.h"

#include <chrono>
#include <functional>
#include <memory>

namespace yogi {

_YOGI_DEFINE_API_FN(int, YOGI_ContextCreate, (void** context))

_YOGI_DEFINE_API_FN(int, YOGI_ContextPoll, (void* context, int* count))

_YOGI_DEFINE_API_FN(int, YOGI_ContextPollOne, (void* context, int* count))

_YOGI_DEFINE_API_FN(int, YOGI_ContextRun,
                    (void* context, int* count, long long duration))

_YOGI_DEFINE_API_FN(int, YOGI_ContextRunOne,
                    (void* context, int* count, long long duration))

_YOGI_DEFINE_API_FN(int, YOGI_ContextRunInBackground, (void* context))

_YOGI_DEFINE_API_FN(int, YOGI_ContextStop, (void* context))

_YOGI_DEFINE_API_FN(int, YOGI_ContextWaitForRunning,
                    (void* context, long long duration))

_YOGI_DEFINE_API_FN(int, YOGI_ContextWaitForStopped,
                    (void* context, long long duration))

_YOGI_DEFINE_API_FN(int, YOGI_ContextPost,
                    (void* context, void (*fn)(void* userarg), void* userarg))

class Context;

/// Shared pointer to a context.
using ContextPtr = std::shared_ptr<Context>;

////////////////////////////////////////////////////////////////////////////////
/// Scheduler for the execution of asynchronous operations.
///
/// Once an asynchronous operation finishes, the corresponding handler function
/// is added to the context's event queue and executed through once of the
/// Poll... or Run... functions.
////////////////////////////////////////////////////////////////////////////////
class Context : public ObjectT<Context> {
 public:
  using HandlerFn = std::function<void()>;

  /// Creates the context
  ///
  /// \returns The created context.
  static ContextPtr Create() { return ContextPtr(new Context()); }

  /// Runs the context's event processing loop to execute ready handlers.
  ///
  /// This function runs handlers (internal and user-supplied such as functions
  /// registered through Post()) that are ready to run, without blocking, until
  /// the Stop() function has been called or there are no more ready handlers.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \returns Number of executed handlers.
  int Poll() {
    int count;
    int res = internal::YOGI_ContextPoll(GetHandle(), &count);
    internal::CheckErrorCode(res);
    return count;
  }

  /// Runs the context's event processing loop to execute at most one ready
  /// handler.
  ///
  /// This function runs at most one handler (internal and user-supplied such as
  /// functions registered through Post()) that are ready to run, without
  /// blocking.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \returns Number of executed handlers.
  int PollOne() {
    int count;
    int res = internal::YOGI_ContextPollOne(GetHandle(), &count);
    internal::CheckErrorCode(res);
    return count;
  }

  /// Runs the context's event processing loop for the specified duration.
  ///
  /// This function blocks while running the context's event processing loop and
  /// calling dispatched handlers (internal and user-supplied such as functions
  /// registered through Post()) for the specified duration unless Stop() is
  /// called within that time.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \param duration Duration.
  ///
  /// \returns Number of executed handlers.
  int Run(const Duration& duration) {
    int count;
    int res = internal::YOGI_ContextRun(GetHandle(), &count,
                                        internal::ToCoreDuration(duration));
    internal::CheckErrorCode(res);
    return count;
  }

  /// Runs the context's event processing loop.
  ///
  /// This function blocks while running the context's event processing loop and
  /// calling dispatched handlers (internal and user-supplied such as functions
  /// registered through Post()) until Stop() is called.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \returns Number of executed handlers.
  int Run() { return Run(Duration::kInfinity); }

  /// Runs the context's event processing loop for the specified duration to
  /// execute at most one handler.
  ///
  /// This function blocks while running the context's event processing loop and
  /// calling dispatched handlers (internal and user-supplied such as functions
  /// registered through Post()) for the specified duration until a single
  /// handler function has been executed, unless Stop() is called within that
  /// time.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \param duration Duration.
  ///
  /// \returns Number of executed handlers.
  int RunOne(const Duration& duration) {
    int count;
    int res = internal::YOGI_ContextRunOne(GetHandle(), &count,
                                           internal::ToCoreDuration(duration));
    internal::CheckErrorCode(res);
    return count;
  }

  /// Runs the context's event processing loop to execute at most one handler.
  ///
  /// This function blocks while running the context's event processing loop and
  /// calling dispatched handlers (internal and user-supplied such as functions
  /// registered through Post()) until a single handler function has been
  /// executed, unless Stop() is called.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \returns Number of executed handlers.
  int RunOne() { return RunOne(Duration::kInfinity); }

  /// Starts an internal thread for running the context's event processing loop.
  ///
  /// This function starts a threads that runs the context's event processing
  /// loop in the background. It relieves the user from having to start a thread
  /// and calling the appropriate Run... or Poll... functions themself. The
  /// thread can be stopped using Stop().
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  void RunInBackground() {
    int res = internal::YOGI_ContextRunInBackground(GetHandle());
    internal::CheckErrorCode(res);
  }

  /// Stops the context's event processing loop.
  ///
  /// This function signals the context to stop running its event processing
  /// loop. This causes Run... functions to return as soon as possible and it
  /// terminates the thread started via RunInBackground().
  void Stop() {
    int res = internal::YOGI_ContextStop(GetHandle());
    internal::CheckErrorCode(res);
  }

  /// Blocks until the context's event processing loop is being run or until the
  /// specified timeout is reached.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \param duration Maximum time to wait.
  ///
  /// \returns True if the context's event processing loop is running within the
  ///          specified duration and false otherwise.
  bool WaitForRunning(const Duration& duration) {
    int res = internal::YOGI_ContextWaitForRunning(
        GetHandle(), internal::ToCoreDuration(duration));
    if (res == static_cast<int>(ErrorCode::kTimeout)) return false;
    internal::CheckErrorCode(res);
    return true;
  }

  /// Blocks until the context's event processing loop is being run.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  void WaitForRunning() { WaitForRunning(Duration::kInfinity); }

  /// Blocks until no thread is running the context's event processing
  /// loop or until the specified timeout is reached.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \param duration Maximum time to wait.
  ///
  /// \returns True if the context's event processing loop is not running within
  ///          the specified duration and false otherwise.
  bool WaitForStopped(const Duration& duration) {
    int res = internal::YOGI_ContextWaitForStopped(
        GetHandle(), internal::ToCoreDuration(duration));
    if (res == static_cast<int>(ErrorCode::kTimeout)) return false;
    internal::CheckErrorCode(res);
    return true;
  }

  /// Blocks until no thread is running the context's event processing loop.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  void WaitForStopped() { WaitForStopped(Duration::kInfinity); }

  /// Adds the given function to the context's event processing queue to
  /// be executed and returns immediately.
  ///
  /// The handler \p fn will only be executed after this function returns and
  /// only by a thread running the context's event processing loop.
  ///
  /// \param fn Function to call from within the context.
  void Post(HandlerFn fn) {
    auto fn_ptr = new HandlerFn(fn);
    static auto wrapper = [](void* userarg) {
      std::unique_ptr<HandlerFn> fn_ptr(static_cast<HandlerFn*>(userarg));
      (*fn_ptr)();
    };

    int res = internal::YOGI_ContextPost(GetHandle(), wrapper, fn_ptr);
    internal::CheckErrorCode(res);
  }

 private:
  Context()
      : ObjectT(internal::CallApiCreate(internal::YOGI_ContextCreate), {}) {}
};

}  // namespace yogi

#endif  // YOGI_CONTEXT_H
