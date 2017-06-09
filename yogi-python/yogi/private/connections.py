from .leaf import *
from ctypes import *
from typing import *


yogi.YOGI_GetConnectionDescription.restype = api_result_handler
yogi.YOGI_GetConnectionDescription.argtypes = [c_void_p, c_char_p, c_uint]

yogi.YOGI_GetRemoteVersion.restype = api_result_handler
yogi.YOGI_GetRemoteVersion.argtypes = [c_void_p, c_char_p, c_uint]

yogi.YOGI_GetRemoteIdentification.restype = api_result_handler
yogi.YOGI_GetRemoteIdentification.argtypes = [c_void_p, c_void_p, c_uint, POINTER(c_uint)]

yogi.YOGI_AssignConnection.restype = api_result_handler
yogi.YOGI_AssignConnection.argtypes = [c_void_p, c_void_p, c_int]

yogi.YOGI_AsyncAwaitConnectionDeath.restype = api_result_handler
yogi.YOGI_AsyncAwaitConnectionDeath.argtypes = [c_void_p, CFUNCTYPE(None, c_int, c_void_p), c_void_p]

yogi.YOGI_CancelAwaitConnectionDeath.restype = api_result_handler
yogi.YOGI_CancelAwaitConnectionDeath.argtypes = [c_void_p]

yogi.YOGI_CreateLocalConnection.restype = api_result_handler
yogi.YOGI_CreateLocalConnection.argtypes = [POINTER(c_void_p), c_void_p, c_void_p]


class Connection(Object):
    STRING_BUFFER_SIZE = 128

    def __init__(self, handle: c_void_p):
        Object.__init__(self, handle)

    @property
    def description(self) -> str:
        buffer = create_string_buffer(self.STRING_BUFFER_SIZE)
        yogi.YOGI_GetConnectionDescription(self._handle, buffer, sizeof(buffer))
        return string_at(addressof(buffer)).decode()

    @property
    def remote_version(self) -> str:
        buffer = create_string_buffer(self.STRING_BUFFER_SIZE)
        yogi.YOGI_GetRemoteVersion(self._handle, buffer, sizeof(buffer))
        return string_at(addressof(buffer)).decode()

    @property
    def remote_identification(self) -> Optional[str]:
        buffer = create_string_buffer(self.STRING_BUFFER_SIZE)
        bytes_written = c_uint()
        yogi.YOGI_GetRemoteIdentification(self._handle, buffer, sizeof(buffer), byref(bytes_written))
        if bytes_written.value == 0:
            return None
        else:
            return string_at(addressof(buffer)).decode()


class LocalConnection(Connection):
    def __init__(self, endpointA: Endpoint, endpointB: Endpoint):
        handle = c_void_p()
        yogi.YOGI_CreateLocalConnection(byref(handle), endpointA._handle, endpointB._handle)
        Connection.__init__(self, handle)


class NonLocalConnection(Connection):
    def __init__(self, handle: c_void_p):
        Connection.__init__(self, handle)

    def assign(self, endpoint: Endpoint, timeout: Optional[float] = None) -> None:
        yogi.YOGI_AssignConnection(self._handle, endpoint._handle, make_api_timeout(timeout))

    def async_await_death(self, completion_handler: Callable[[Failure], None]) -> None:
        with WrappedCallback(yogi.YOGI_AsyncAwaitConnectionDeath.argtypes[1], completion_handler) as clb_fn:
            yogi.YOGI_AsyncAwaitConnectionDeath(self._handle, clb_fn, c_void_p())

    def cancel_await_death(self) -> None:
        yogi.YOGI_CancelAwaitConnectionDeath(self._handle)
