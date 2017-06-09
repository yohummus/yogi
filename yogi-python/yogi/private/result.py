from .library import yogi
from ctypes import *


yogi.YOGI_GetErrorString.restype = c_char_p
yogi.YOGI_GetErrorString.argtypes = [c_int]


class Result:
    def __init__(self, value: int):
        self._value = value

    @property
    def value(self):
        return self._value

    def __bool__(self):
        return self._value >= 0

    def __eq__(self, other):
        return self._value == other.value and isinstance(other, Result)

    def __ne__(self, other):
        return not (self == other)

    def __str__(self):
        s = yogi.YOGI_GetErrorString(self._value if self._value < 0 else 0).decode()
        return '[{}] {}'.format(self._value, s)

    def __hash__(self):
        return self._value


class Failure(Exception, Result):
    def __init__(self, value: int):
        assert value < 0
        Result.__init__(self, value)

    def __str__(self):
        return Result.__str__(self)


class Canceled(Failure):
    def __init__(self):
        Failure.__init__(self, -12)


class BufferTooSmall(Failure):
    def __init__(self):
        Failure.__init__(self, -14)


class Timeout(Failure):
    def __init__(self):
        Failure.__init__(self, -27)


class Success(Result):
    def __init__(self, value: int = 0):
        assert value >= 0
        Result.__init__(self, value)


def api_result_handler(result: int) -> Success:
    if result < 0:
        raise Failure(result)
    else:
        return Success(result)


def api_result_to_result(result: int) -> Result:
    if result < 0:
        return Failure(result)
    else:
        return Success(result)
