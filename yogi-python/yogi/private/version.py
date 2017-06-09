from .library import yogi
from ctypes import *


yogi.YOGI_GetVersion.restype = c_char_p
yogi.YOGI_GetVersion.argtypes = []


def get_version() -> str:
    return yogi.YOGI_GetVersion().decode()
