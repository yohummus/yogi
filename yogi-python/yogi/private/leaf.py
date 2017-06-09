from .scheduler import *
from ctypes import *


class Endpoint(Object):
    def __init__(self, handle: c_void_p, scheduler: Scheduler):
        Object.__init__(self, handle)
        self._scheduler = scheduler

    @property
    def scheduler(self) -> Scheduler:
        return self._scheduler


yogi.YOGI_CreateLeaf.restype = api_result_handler
yogi.YOGI_CreateLeaf.argtypes = [c_void_p]


class Leaf(Endpoint):
    def __init__(self, scheduler: Scheduler):
        handle = c_void_p()
        yogi.YOGI_CreateLeaf(byref(handle), scheduler._handle)
        Endpoint.__init__(self, handle, scheduler)
