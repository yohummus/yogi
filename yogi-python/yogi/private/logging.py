from .logfile import Verbosity
from .timestamp import *
from .process import *
import platform
from ctypes import *
import threading
from typing import *
import sys


class Verbosities:
    def __init__(self):
        self.stdout = Verbosity.TRACE
        self.yogi = Verbosity.TRACE


if platform.system() == 'Windows':
    class _Coord(Structure):
        _fields_ = [
            ("X", c_short),
            ("Y", c_short)
        ]


    class _SmallRect(Structure):
        _fields_ = [
            ("Left", c_short),
            ("Top", c_short),
            ("Right", c_short),
            ("Bottom", c_short)
        ]


    class _ConsoleScreenBufferInfo(Structure):
        _fields_ = [
            ("dwSize", _Coord),
            ("dwCursorPosition", _Coord),
            ("wAttributes", c_ushort),
            ("srWindow", _SmallRect),
            ("dwMaximumWindowSize", _SmallRect)
        ]


class Logger:
    _colourised_stdout = False
    _maxVerbosities = Verbosities()
    _loggerVerbosities = {}
    _app_logger = None
    _yogi_logger = None
    _lock = threading.Lock()

    if platform.system() == 'Windows':
        _STD_OUTPUT_HANDLE = -11
        _win32_stdout_handle = windll.kernel32.GetStdHandle(_STD_OUTPUT_HANDLE)
        _win32_original_csbi = _ConsoleScreenBufferInfo()
        windll.kernel32.GetConsoleScreenBufferInfo(_win32_stdout_handle, byref(_win32_original_csbi))
        _win32_original_colours = _win32_original_csbi.wAttributes

    @classproperty
    def colourised_stdout(cls) -> bool:
        return cls._colourised_stdout

    @colourised_stdout.setter
    def colourised_stdout(cls, enabled: bool):
        cls._colourised_stdout = enabled

    @classproperty
    def max_stdout_verbosity(cls) -> Verbosity:
        return cls._maxVerbosities.stdout

    @max_stdout_verbosity.setter
    def max_stdout_verbosity(cls, verbosity: Verbosity):
        cls._maxVerbosities.stdout = verbosity
        ProcessInterface._on_max_verbosity_set(verbosity, True)

    @classproperty
    def max_yogi_verbosity(cls) -> Verbosity:
        return cls._maxVerbosities.yogi

    @max_yogi_verbosity.setter
    def max_yogi_verbosity(cls, verbosity: Verbosity):
        cls._maxVerbosities.yogi = verbosity
        ProcessInterface._on_max_verbosity_set(verbosity, False)

    @classproperty
    def app_logger(cls):
        return cls._app_logger

    @classproperty
    def yogi_logger(cls):
        return cls._yogi_logger

    @classproperty
    def all_loggers(cls):
        return [Logger(cmp) for cmp in cls._loggerVerbosities]

    @classmethod
    def _reset_colours(cls):
        if not cls.colourised_stdout or not sys.stdout.isatty():
            return

        if platform.system() == 'Windows':
            windll.kernel32.SetConsoleTextAttribute(cls._win32_stdout_handle, cls._win32_original_colours)
        else:
            print('\033[0m', end='')

    @classmethod
    def _set_colour(cls, severity):
        if not cls.colourised_stdout or not sys.stdout.isatty():
            return

        if platform.system() == 'Windows':
            colour = {
                Verbosity.TRACE:   6,
                Verbosity.DEBUG:   10,
                Verbosity.INFO:    15,
                Verbosity.WARNING: 14,
                Verbosity.ERROR:   12,
                Verbosity.FATAL:   15 | 64
            }[severity]
            windll.kernel32.SetConsoleTextAttribute(cls._win32_stdout_handle, colour)
        else:
            seq = {
                Verbosity.TRACE:   '\033[22;33m',
                Verbosity.DEBUG:   '\033[01;32m',
                Verbosity.INFO:    '\033[01;37m',
                Verbosity.WARNING: '\033[01;33m',
                Verbosity.ERROR:   '\033[01;31m',
                Verbosity.FATAL:   '\033[41m\033[01;37m'
            }[severity]
            print(seq, end='')

    def __init__(self, component: Optional[str] = None):
        self._component = component if component else 'app'
        if self._component not in self._loggerVerbosities:
            self._verbosities = Verbosities()
            self._loggerVerbosities[self._component] = self._verbosities
            ProcessInterface._on_new_logger_added(self)
        else:
            self._verbosities = self._loggerVerbosities[self._component]

    @property
    def component(self) -> str:
        return self._component

    @property
    def stdout_verbosity(self) -> Verbosity:
        return self._verbosities.stdout

    @stdout_verbosity.setter
    def stdout_verbosity(self, verbosity: Verbosity):
        self._verbosities.stdout = verbosity
        ProcessInterface._on_verbosity_set(self._component, verbosity, True)

    @property
    def yogi_verbosity(self) -> Verbosity:
        return self._verbosities.yogi

    @yogi_verbosity.setter
    def yogi_verbosity(self, verbosity: Verbosity):
        self._verbosities.yogi = verbosity
        ProcessInterface._on_verbosity_set(self._component, verbosity, False)

    @property
    def effective_stdout_verbosity(self) -> Verbosity:
        return Verbosity(min(self.max_stdout_verbosity.value, self.stdout_verbosity.value))

    @property
    def effective_yogi_verbosity(self) -> Verbosity:
        return Verbosity(min(self.max_yogi_verbosity.value, self.yogi_verbosity.value))

    @property
    def max_effective_verbosity(self):
        return Verbosity(max(self.effective_stdout_verbosity.value, self.effective_yogi_verbosity.value))

    def log(self, severity: Verbosity, *args):
        timestamp = Timestamp.now()
        thread_id = threading.current_thread().ident
        msg = ''.join([str(arg) for arg in args])

        severity_tag = {
            Verbosity.FATAL:   'FAT',
            Verbosity.ERROR:   'ERR',
            Verbosity.WARNING: 'WRN',
            Verbosity.INFO:    'IFO',
            Verbosity.DEBUG:   'DBG',
            Verbosity.TRACE:   'TRC'
        }[severity]

        if severity.value <= self.effective_stdout_verbosity.value:
            s1 = '{} [T{:05}] '.format(timestamp, thread_id)
            s2 = '{} {}: {}'.format(severity_tag, self._component, msg)

            with self._lock:
                self._reset_colours()
                print(s1, end='')
                sys.stdout.flush()
                self._set_colour(severity)
                print(s2, end='')
                sys.stdout.flush()
                self._reset_colours()
                print('')
                sys.stdout.flush()

        if severity.value <= self.effective_yogi_verbosity.value:
            ProcessInterface._publish_log_message(severity, self._component, msg, timestamp, thread_id)

    def log_trace(self, *args):
        self.log(Verbosity.TRACE, *args)

    def log_debug(self, *args):
        self.log(Verbosity.DEBUG, *args)

    def log_info(self, *args):
        self.log(Verbosity.INFO, *args)

    def log_warning(self, *args):
        self.log(Verbosity.WARNING, *args)

    def log_error(self, *args):
        self.log(Verbosity.ERROR, *args)

    def log_fatal(self, *args):
        self.log(Verbosity.FATAL, *args)


Logger._app_logger = Logger()
Logger._yogi_logger = Logger('yogi')


def log(verbosity: Verbosity, *args):
    Logger.app_logger.log(verbosity, *args)


def log_trace(*args):
    log(Verbosity.TRACE, *args)


def log_debug(*args):
    log(Verbosity.DEBUG, *args)


def log_info(*args):
    log(Verbosity.INFO, *args)


def log_warning(*args):
    log(Verbosity.WARNING, *args)


def log_error(*args):
    log(Verbosity.ERROR, *args)


def log_fatal(*args):
    log(Verbosity.FATAL, *args)
