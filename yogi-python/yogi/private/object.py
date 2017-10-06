from .helpers import *
from ctypes import *


yogi.YOGI_Destroy.restype = api_result_handler
yogi.YOGI_Destroy.argtypes = [c_void_p]


class Object:
    def __init__(self, handle: c_void_p):
        self._handle = handle

    def destroy(self) -> None:
        yogi.YOGI_Destroy(self._handle)
        self._handle = None

    def __del__(self):
        if self._handle is not None:
            try:
                self.destroy()
            except Failure:
                pass

    def __str__(self):
        handle_str = '{:#010x}'.format(self._handle.value) if self._handle.value else 'INVALID'
        return '{} [{}]'.format(self.__class__.__name__, handle_str)
