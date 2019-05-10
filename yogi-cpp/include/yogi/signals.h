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

#ifndef YOGI_SIGNALS_H
#define YOGI_SIGNALS_H

//! \file
//!
//! Signal handling.

#include "duration.h"
#include "object.h"
#include "context.h"
#include "internal/flags.h"

#include <memory>
#include <functional>

namespace yogi {

_YOGI_DEFINE_API_FN(int, YOGI_RaiseSignal,
                    (int signal, void* sigarg,
                     void (*fn)(void* sigarg, void* userarg), void* userarg))

_YOGI_DEFINE_API_FN(int, YOGI_SignalSetCreate,
                    (void** sigset, void* context, int signals))

_YOGI_DEFINE_API_FN(int, YOGI_SignalSetAwaitSignalAsync,
                    (void* sigset,
                     void (*fn)(int res, int sig, void* sigarg, void* userarg),
                     void* userarg))

_YOGI_DEFINE_API_FN(int, YOGI_SignalSetCancelAwaitSignal, (void* sigset))

/// \addtogroup enums
/// @{

////////////////////////////////////////////////////////////////////////////////
/// Signals.
////////////////////////////////////////////////////////////////////////////////
enum class Signals {
  /// No signal.
  kNone = 0,

  /// Interrupt (e.g. by receiving SIGINT or pressing STRG + C).
  kInt = (1 << 0),

  /// Termination request (e.g. by receiving SIGTERM).
  kTerm = (1 << 1),

  /// User-defined signal 1.
  kUsr1 = (1 << 24),

  /// User-defined signal 2.
  kUsr2 = (1 << 25),

  /// User-defined signal 3.
  kUsr3 = (1 << 26),

  /// User-defined signal 4.
  kUsr4 = (1 << 27),

  /// User-defined signal 5.
  kUsr5 = (1 << 28),

  /// User-defined signal 6.
  kUsr6 = (1 << 29),

  /// User-defined signal 7.
  kUsr7 = (1 << 30),

  /// User-defined signal 8.
  kUsr8 = (1 << 31),

  /// All flags.
  kAll = kInt | kTerm | kUsr1 | kUsr2 | kUsr3 | kUsr4 | kUsr5 | kUsr6 | kUsr7 |
         kUsr8
};

_YOGI_DEFINE_FLAG_OPERATORS(Signals)

template <>
inline std::string ToString<Signals>(const Signals& signals) {
  switch (signals) {
    _YOGI_TO_STRING_ENUM_CASE(Signals, kNone)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kInt)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kTerm)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kUsr1)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kUsr2)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kUsr3)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kUsr4)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kUsr5)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kUsr6)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kUsr7)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kUsr8)
    _YOGI_TO_STRING_ENUM_CASE(Signals, kAll)
  }

  std::string s;
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kInt)
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kTerm)
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kUsr1)
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kUsr2)
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kUsr3)
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kUsr4)
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kUsr5)
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kUsr6)
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kUsr7)
  _YOGI_TO_STRING_FLAG_APPENDER(signals, Signals, kUsr8)
  return s.substr(3);
}

/// @} enums

namespace internal {

class SigargWrapper {
 public:
  virtual ~SigargWrapper() {}
};

template <typename T>
class SigargWrapperT : public SigargWrapper {
 public:
  SigargWrapperT(T&& data) : data_(std::forward<T>(data)) {}

  T& GetData() { return data_; }

 private:
  T data_;
};

}  // namespace internal

/// Handler function to be called once the signal raised via RaiseSignal() has
/// been delivered to all signal sets.
using RaiseSignalFinishedFn = std::function<void()>;

/// \addtogroup freefn
/// @{

/// Raises a signal.
///
/// Signals in Yogi are intended to be used similarly to POSIX signals. They
/// have to be raised explicitly by the user (e.g. when receiving an actual
/// POSIX signal like SIGINT) via this function. A signal will be received by
/// all signal sets containing that signal.
///
/// The cleanup handler \p fn will be called once all signal handlers have been
/// called.
///
/// \note
///   The cleanup handler \p fn can get called either from within the
///   RaiseSignal() function or from any context within the program.
///
/// \param signal The signal to raise.
/// \param fn     Function to call after all signal sets have been notified.
inline void RaiseSignal(Signals signal, RaiseSignalFinishedFn fn = {}) {
  struct CallbackData {
    RaiseSignalFinishedFn fn;
  };

  auto data = std::make_unique<CallbackData>();
  data->fn = fn;

  int res = internal::YOGI_RaiseSignal(
      static_cast<int>(signal), nullptr,
      [](void*, void* userarg) {
        auto data =
            std::unique_ptr<CallbackData>(static_cast<CallbackData*>(userarg));

        if (data->fn) {
          data->fn();
        }
      },
      data.get());

  internal::CheckErrorCode(res);
  data.release();
}

/// @} freefn

/// Handler function to be called once the signal raised via RaiseSignal() has
/// been delivered to all signal sets.
///
/// \tparam Sigarg Type of the \p sigarg signal argument passed to
///                RaiseSignalWithArg().
///
/// \param sigarg Signal argument passed to RaiseSignalWithArg(); will be
///               nullptr if no signal argument was specified when raising the
///               signal.
template <typename Sigarg>
using RaiseSignalFinishedFnT = std::function<void(Sigarg* sigarg)>;

/// \addtogroup freefn
/// @{

/// Raises a signal.
///
/// Signals in Yogi are intended to be used similarly to POSIX signals. They
/// have to be raised explicitly by the user (e.g. when receiving an actual
/// POSIX signal like SIGINT) via this function. A signal will be received by
/// all signal sets containing that signal.
///
/// The \p sigarg parameter can be used to deliver user-defined data to the
/// signal handlers.
///
/// The cleanup handler \p fn will be called once all signal handlers have been
/// called.
///
/// \note
///   The cleanup handler \p fn can get called either from within the
///   RaiseSignal() function or from any context within the program.
///
/// \tparam Sigarg Type of the \p sigarg signal argument.
///
/// \param signal The signal to raise.
/// \param sigarg Signal argument to pass to the signal handlers.
/// \param fn     Function to call after all signal sets have been notified.
template <typename Sigarg>
inline void RaiseSignalWithArg(Signals signal, Sigarg&& sigarg,
                               RaiseSignalFinishedFnT<Sigarg> fn = {}) {
  struct CallbackData {
    internal::SigargWrapperT<Sigarg> sigarg_wrapper;
    RaiseSignalFinishedFnT<Sigarg> fn;

    CallbackData(Sigarg&& sigarg)
        : sigarg_wrapper(std::forward<Sigarg>(sigarg)) {}
  };

  auto data = std::make_unique<CallbackData>(std::forward<Sigarg>(sigarg));
  data->fn = fn;

  internal::SigargWrapper* sigarg_wrapper_p = &data->sigarg_wrapper;

  int res = internal::YOGI_RaiseSignal(
      static_cast<int>(signal), sigarg_wrapper_p,
      [](void* sigarg, void* userarg) {
        auto data =
            std::unique_ptr<CallbackData>(static_cast<CallbackData*>(userarg));

        if (data->fn) {
          auto sigarg_wrapper_p = static_cast<internal::SigargWrapper*>(sigarg);
          auto sigarg_wrapper_ptr =
              static_cast<internal::SigargWrapperT<Sigarg>*>(sigarg_wrapper_p);

          data->fn(&sigarg_wrapper_ptr->GetData());
        }
      },
      data.get());

  internal::CheckErrorCode(res);
  data.release();
}

/// @} freefn

class SignalSet;

/// Shared pointer to a signal set.
using SignalSetPtr = std::shared_ptr<SignalSet>;

////////////////////////////////////////////////////////////////////////////////
/// A set of signals to receive when raised via RaiseSignal() and
/// RaiseSignalWithArg().
///
/// \note
///   The signals are queued until they can be delivered by means of calls to
///   AwaitSignal().
////////////////////////////////////////////////////////////////////////////////
class SignalSet : public ObjectT<SignalSet> {
 public:
  /// Handler function used for the AwaitSignal() function.
  ///
  /// \param res    %Result of the wait operation.
  /// \param signal The raised signal.
  using SignalHandlerFn =
      std::function<void(const Result& res, Signals signal)>;

  /// Handler function used for the AwaitSignal<Sigarg>() function.
  ///
  /// \tparam Type of the signal argument passed to RaiseSignalWithArg().
  ///
  /// \param res    %Result of the wait operation.
  /// \param signal The raised signal.
  /// \param sigarg Value of the signal argument passed to RaiseSignalWithArg();
  ///               will be nullptr if no signal argument was specified when
  ///               raising the signal.
  template <typename Sigarg>
  using SignalHandlerFnT =
      std::function<void(const Result& res, Signals signal, Sigarg* sigarg)>;

  /// Create a signal set.
  ///
  /// \param context The context to use.
  /// \param signals Signals in the signal set.
  ///
  /// \returns The created signal set.
  static SignalSetPtr Create(ContextPtr context, Signals signals) {
    return SignalSetPtr(new SignalSet(context, signals));
  }

  /// Waits for a signal to be raised.
  ///
  /// The handler \p fn will be called after one of the signals in the set is
  /// caught.
  ///
  /// \param fn Handler function to call.
  void AwaitSignal(SignalHandlerFn fn) {
    struct CallbackData {
      SignalHandlerFn fn;
    };

    auto data = std::make_unique<CallbackData>();
    data->fn = fn;

    int res = internal::YOGI_SignalSetAwaitSignalAsync(
        GetHandle(),
        [](int res, int sig, void*, void* userarg) {
          auto data = std::unique_ptr<CallbackData>(
              static_cast<CallbackData*>(userarg));

          if (data->fn) {
            internal::WithErrorCodeToResult(res, [&](const auto& result) {
              data->fn(result, static_cast<Signals>(sig));
            });
          }
        },
        data.get());

    internal::CheckErrorCode(res);
    data.release();
  }

  /// Waits for a signal to be raised.
  ///
  /// The handler \p fn will be called after one of the signals in the set is
  /// caught. The signal argument will be casted to the given template type T.
  /// If casting to T fails, then an exception will be thrown. In order to
  /// receive any type of signal argument, chose the default Sigarg type for T.
  ///
  /// \tparam Sigarg Type of the signal argument passed to RaiseSignalWithArg().
  ///
  /// \param fn Handler function to call.
  template <typename Sigarg>
  void AwaitSignal(SignalHandlerFnT<Sigarg> fn) {
    struct CallbackData {
      SignalHandlerFnT<Sigarg> fn;
    };

    auto data = std::make_unique<CallbackData>();
    data->fn = fn;

    int res = internal::YOGI_SignalSetAwaitSignalAsync(
        GetHandle(),
        [](int res, int sig, void* sigarg, void* userarg) {
          auto data = std::unique_ptr<CallbackData>(
              static_cast<CallbackData*>(userarg));

          if (data->fn) {
            auto sigarg_wrapper_p =
                static_cast<internal::SigargWrapper*>(sigarg);

            Sigarg* typed_sigarg = nullptr;
            if (sigarg_wrapper_p) {
              // Next line throws if Sigarg is incompatible with the type used
              // for RaiseSignalWithArg()
              dynamic_cast<internal::SigargWrapperT<Sigarg>&>(
                  *sigarg_wrapper_p);

              auto sigarg_wrapper_ptr =
                  static_cast<internal::SigargWrapperT<Sigarg>*>(
                      sigarg_wrapper_p);

              typed_sigarg = &sigarg_wrapper_ptr->GetData();
            }

            internal::WithErrorCodeToResult(res, [&](const auto& result) {
              data->fn(result, static_cast<Signals>(sig), typed_sigarg);
            });
          }
        },
        data.get());

    internal::CheckErrorCode(res);
    data.release();
  }

  /// Cancels waiting for a signal.
  ///
  /// This causes the handler function registered via AwaitSignal() to be
  /// called with a cancellation error.
  ///
  /// \return True if the wait operation was cancelled successfully.
  bool CancelAwaitSignal() {
    int res = internal::YOGI_SignalSetCancelAwaitSignal(GetHandle());
    return internal::FalseIfSpecificErrorElseThrow(
        res, ErrorCode::kOperationNotRunning);
  }

 private:
  SignalSet(ContextPtr context, Signals signals)
      : ObjectT(internal::CallApiCreate(internal::YOGI_SignalSetCreate,
                                        GetForeignHandle(context),
                                        static_cast<int>(signals)),
                {context}) {}
};

}  // namespace yogi

#endif  // YOGI_SIGNALS_H
