import platform
from ctypes import *
import atexit


# ======================================================================================================================
# Load the shared library
# ======================================================================================================================
library_filename = None
if platform.system() == 'Windows':
    library_filename = "yogi_core.dll"
elif platform.system() == 'Linux':
    library_filename = "libyogi_core.so"
else:
    raise Exception(platform.system() + ' is not supported')

try:
    yogi = cdll.LoadLibrary(library_filename)
except Exception as e:
    raise Exception('ERROR: Could not load {}: {}. Make sure the library is in your library search path.'
                    .format(library_filename, e))


# ======================================================================================================================
# Initialise the library and clean up on exit
# ======================================================================================================================
yogi.YOGI_Initialise.restype = c_int
yogi.YOGI_Initialise.argtypes = []

res = yogi.YOGI_Initialise()
if res < 0:
    from .result import Failure
    raise Failure(res)

yogi.YOGI_Shutdown.restype = c_int
yogi.YOGI_Shutdown.argtypes = []

def exit_handler():
    if yogi:
        res = yogi.YOGI_Shutdown()
        if res < 0:
            from .result import Failure
            print('ERROR: Could not shutdown YOGI: {}'.format(Failure(res)))

atexit.register(exit_handler)
