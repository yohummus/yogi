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
from .errors import Result, FailureException, ErrorCode, api_result_handler
from .library import yogi
from .handler import Handler
from .context import Context
from .duration import Duration, duration_to_api_duration

from typing import Callable, Any
from ctypes import c_int, c_longlong, c_void_p, CFUNCTYPE, POINTER, byref


yogi.YOGI_TimerCreate.restype = api_result_handler
yogi.YOGI_TimerCreate.argtypes = [POINTER(c_void_p), c_void_p]

yogi.YOGI_TimerStartAsync.restype = api_result_handler
yogi.YOGI_TimerStartAsync.argtypes = [
    c_void_p, c_longlong, CFUNCTYPE(None, c_int, c_void_p), c_void_p]

yogi.YOGI_TimerCancel.restype = api_result_handler
yogi.YOGI_TimerCancel.argtypes = [c_void_p]


class Timer(Object):
    """Simple timer implementation."""

    def __init__(self, context: Context):
        """Creates the timer.

        Args:
            context: The context to use.
        """
        handle = c_void_p()
        yogi.YOGI_TimerCreate(byref(handle), context._handle)
        Object.__init__(self, handle, [context])

    def start_async(self, duration: Duration,
                    fn: Callable[[Result], Any]) -> None:
        """Starts the timer.

        If the timer is already running, the timer will be canceled first, as
        if cancel() were called explicitly.

        Args:
            duration: Time when the timer expires.
            fn:       Handler function to call after the given time passed.
        """
        dur = duration_to_api_duration(duration)
        with Handler(yogi.YOGI_TimerStartAsync.argtypes[2], fn) as handler:
            yogi.YOGI_TimerStartAsync(self._handle, dur, handler, None)

    def cancel(self) -> bool:
        """Cancels the given timer.

        Canceling a timer will result in the handler function registered via
        start_async() to be called with a cancellation error. Note that if the
        handler is already scheduled for execution, it will be called without
        an error.

        Returns:
            True if the timer was canceled successfully and False otherwise
            (i.e. the timer has not been started or it already expired).
        """
        try:
            yogi.YOGI_TimerCancel(self._handle)
        except FailureException as e:
            if e.failure.error_code is ErrorCode.TIMER_EXPIRED:
                return False
            else:
                raise

        return True
