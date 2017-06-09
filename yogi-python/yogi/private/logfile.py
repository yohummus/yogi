from .result import *
from ctypes import *
from enum import Enum


class Verbosity(Enum):
    TRACE = 5
    DEBUG = 4
    INFO = 3
    WARNING = 2
    ERROR = 1
    FATAL = 0


yogi.YOGI_SetLogFile.restype = api_result_handler
yogi.YOGI_SetLogFile.argtypes = [c_char_p, c_int]


def set_log_file(filename: str, verbosity: Verbosity) -> None:
    yogi.YOGI_SetLogFile(filename.encode('utf-8'), verbosity.value)
