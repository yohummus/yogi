# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2018 Johannes Bergmann.
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this library. If not, see <http://www.gnu.org/licenses/>.

from .object import Object
from .errors import Result, ErrorCode, api_result_handler, \
    false_if_specific_ec_else_raise
from .library import yogi
from .handler import Handler, inc_ref_cnt, dec_ref_cnt
from .context import Context

from enum import IntEnum
from typing import Callable, Any, TypeVar, Union
from ctypes import c_int, c_void_p, POINTER, CFUNCTYPE, byref, py_object
from inspect import signature


class Signals(IntEnum):
    """Signals.

    Attributes:
        NONE No signal.
        INT  Interrupt (e.g. by pressing STRG + C).
        TERM Termination request.
        USR1 User-defined signal 1.
        USR2 User-defined signal 2.
        USR3 User-defined signal 3.
        USR4 User-defined signal 4.
        USR5 User-defined signal 5.
        USR6 User-defined signal 6.
        USR7 User-defined signal 7.
        USR8 User-defined signal 8.
        ALL  All signals.
    """
    NONE = 0
    INT = (1 << 0)
    TERM = (1 << 1)
    USR1 = (1 << 24)
    USR2 = (1 << 25)
    USR3 = (1 << 26)
    USR4 = (1 << 27)
    USR5 = (1 << 28)
    USR6 = (1 << 29)
    USR7 = (1 << 30)
    USR8 = (1 << 31)
    ALL = INT | TERM | USR1 | USR2 | USR3 | USR4 | USR5 | USR6 | USR7 | USR8


yogi.YOGI_RaiseSignal.restype = api_result_handler
yogi.YOGI_RaiseSignal.argtypes = [c_int, py_object,
                                  CFUNCTYPE(None, py_object, c_void_p),
                                  c_void_p]

T = TypeVar('T')


def raise_signal(signal: Signals, sigarg: T = None,
                 fn: Union[Callable[[], Any], Callable[[T], Any]] = None
                 ) -> None:
    """Raises a signal.

    Signals in Yogi are intended to be used similarly to POSIX signals. They
    have to be raised explicitly by the user (e.g. when receiving an actual
    POSIX signal like SIGINT) via this function. A signal will be received by
    all signal sets containing that signal.

    The sigarg parameter can be used to deliver user-defined data to the
    signal handlers.

    The cleanup handler fn will be called once all signal handlers have been
    called. Once fn has been called, the sigarg parameter is not used any more
    and can be cleaned up.

    Note: The cleanup handler fn can get called either from within the
          raise_signal() function or from any context within the program.

    Args:
        signal: The signal to raise (must be a single signal).
        sigarg: User-defined data to pass to the signal handlers.
        fn:     Function to be called once all signal handlers have been
                executed.
    """
    sigarg = py_object(sigarg)
    inc_ref_cnt(sigarg)

    fn_obj = None

    def clb(sigarg, userarg):
        dec_ref_cnt(fn_obj)
        dec_ref_cnt(py_object(sigarg))

        if fn is not None:
            if len(signature(fn).parameters) == 1:
                fn(sigarg)
            else:
                fn()

    wrapped_fn = yogi.YOGI_RaiseSignal.argtypes[2](clb)
    fn_obj = py_object(wrapped_fn)
    inc_ref_cnt(fn_obj)

    try:
        yogi.YOGI_RaiseSignal(signal, sigarg, wrapped_fn, None)
    except Exception:
        dec_ref_cnt(fn_obj)
        dec_ref_cnt(sigarg)
        raise


class PyObjectWrapper(py_object):
    def from_param(self, *args):
        return self


yogi.YOGI_SignalSetCreate.restype = api_result_handler
yogi.YOGI_SignalSetCreate.argtypes = [POINTER(c_void_p), c_void_p, c_int]

yogi.YOGI_SignalSetAwaitSignalAsync.restype = api_result_handler
yogi.YOGI_SignalSetAwaitSignalAsync.argtypes = [c_void_p, CFUNCTYPE(
    None, c_int, c_int, PyObjectWrapper, c_void_p), c_void_p]

yogi.YOGI_SignalSetCancelAwaitSignal.restype = int
yogi.YOGI_SignalSetCancelAwaitSignal.argtypes = [c_void_p]


class SignalSet(Object):
    """Manages a set of signals that the user can wait on.

    Signal sets are used to receive signals raised via raise_signal(). The
    signals are queued until they can be delivered by means of calls to
    await_signal_async().
    """

    def __init__(self, context: Context, signals: Signals):
        """Creates the signal set.

        Args:
            context: The context to use.
            signals: The signals to listen for.
        """
        handle = c_void_p()
        yogi.YOGI_SignalSetCreate(byref(handle), context._handle, signals)
        Object.__init__(self, handle, [context])

    def await_signal_async(self,
                           fn: Union[Callable[[Result, Signals], Any],
                                     Callable[[Result, Signals, Any], Any]]
                           ) -> None:
        """Waits for a signal to be raised.

        The handler fn will be called after one of the signals in the signal
        set is caught.

        Args:
            fn: Handler function to call.
        """
        def wrapped_fn(res, signals, sigarg_obj):
            if len(signature(fn).parameters) == 2:
                fn(res, Signals(signals))
            else:
                sigarg = sigarg_obj.value if sigarg_obj else None
                fn(res, Signals(signals), sigarg)

        with Handler(yogi.YOGI_SignalSetAwaitSignalAsync.argtypes[1],
                     wrapped_fn) as handler:
            yogi.YOGI_SignalSetAwaitSignalAsync(self._handle, handler, None)

    def cancel_await_signal(self) -> bool:
        """Cancels waiting for a signal.

        This causes the handler function registered via await_signal_async() to
        be called with a cancellation error.

        Returns:
            True if the wait operation was cancelled successfully.
        """
        res = yogi.YOGI_SignalSetCancelAwaitSignal(self._handle)
        return false_if_specific_ec_else_raise(res,
                                               ErrorCode.OPERATION_NOT_RUNNING)
