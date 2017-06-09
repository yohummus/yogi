from .object import *
from ctypes import *


yogi.YOGI_CreateScheduler.restype = api_result_handler
yogi.YOGI_CreateScheduler.argtypes = [POINTER(c_void_p)]

yogi.YOGI_SetSchedulerThreadPoolSize.restype = api_result_handler
yogi.YOGI_SetSchedulerThreadPoolSize.argtypes = [c_void_p, c_uint]


class Scheduler(Object):
    def __init__(self):
        handle = c_void_p()
        yogi.YOGI_CreateScheduler(byref(handle))
        Object.__init__(self, handle)

    def set_thread_pool_size(self, n: int) -> None:
        yogi.YOGI_SetSchedulerThreadPoolSize(self._handle, n)
