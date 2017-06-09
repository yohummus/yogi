from .result import *
from enum import Enum
from ctypes import *


class ControlFlow(Enum):
    CONTINUE = 0
    STOP = 1


_inc_ref_cnt = pythonapi.Py_IncRef
_inc_ref_cnt.argtypes = [py_object]
_inc_ref_cnt.restype = None

_dec_ref_cnt = pythonapi.Py_DecRef
_dec_ref_cnt.argtypes = [py_object]
_dec_ref_cnt.restype = None


class WrappedCallback:
    def __init__(self, c_function_type, fn):
        self._fn_obj = None

        def clb(res, *args):
            if res < 0:
                ret = fn(Failure(res), *args[:-1])
            else:
                ret = fn(Success(res), *args[:-1])
            if ret is None or ret == ControlFlow.STOP:
                _dec_ref_cnt(self._fn_obj)
            return ret

        self._wrapped_fn = c_function_type(clb)
        self._fn_obj = py_object(self._wrapped_fn)

    def __enter__(self):
        _inc_ref_cnt(self._fn_obj)
        return self._wrapped_fn

    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_type is not None:
            _dec_ref_cnt(self._fn_obj)


def make_api_timeout(timeout):
    if timeout is None:
        return -1
    else:
        return int(timeout * 1000)


class ClassProperty:
    def __init__(self, getter, setter=None):
        self._getter = getter
        self._setter = setter

    def __get__(self, instance, cls=None):
        if cls is None:
            cls = type(instance)
        return self._getter.__get__(instance, cls)()

    def __set__(self, instance, value):
        if not self._setter:
            raise AttributeError("can't set attribute")
        cls = type(instance)
        return self._setter.__get__(instance, cls)(value)

    def setter(self, setter):
        if not isinstance(setter, (classmethod, staticmethod)):
            setter = classmethod(setter)
        self._setter = setter
        return self


def classproperty(getter):
    if not isinstance(getter, (classmethod, staticmethod)):
        getter = classmethod(getter)
    return ClassProperty(getter)