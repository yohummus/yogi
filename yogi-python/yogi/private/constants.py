# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2019 Johannes Bergmann.
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this library. If not, see <http://www.gnu.org/licenses/>.

from .errors import api_result_handler
from .logging import Verbosity
from .library import yogi
from .duration import Duration

from ctypes import c_void_p, c_int, c_char_p, c_longlong, byref


yogi.YOGI_GetConstant.restype = api_result_handler
yogi.YOGI_GetConstant.argtypes = [c_void_p, c_int]


def get_constant(api_id, api_type):
    c = api_type()
    yogi.YOGI_GetConstant(byref(c), api_id)
    return c.value.decode() if api_type is c_char_p else c.value


class MetaConstants(type):
    @property
    def VERSION(self) -> str:
        """Complete Yogi Core version."""
        return get_constant(1, c_char_p)

    @property
    def VERSION_MAJOR(self) -> int:
        """Yogi Core major version number."""
        return get_constant(2, c_int)

    @property
    def VERSION_MINOR(self) -> int:
        """Yogi Core minor version number."""
        return get_constant(3, c_int)

    @property
    def VERSION_PATCH(self) -> int:
        """Yogi Core patch version number."""
        return get_constant(4, c_int)

    @property
    def VERSION_SUFFIX(self) -> str:
        """Yogi Core version suffix."""
        return get_constant(5, c_char_p)

    @property
    def DEFAULT_ADV_INTERFACES(self) -> str:
        """Default network interfaces to use for advertising."""
        return get_constant(6, c_char_p)

    @property
    def DEFAULT_ADV_ADDRESS(self) -> str:
        """Default IP address for advertising."""
        return get_constant(7, c_char_p)

    @property
    def DEFAULT_ADV_PORT(self) -> int:
        """Default UDP port for advertising."""
        return get_constant(8, c_int)

    @property
    def DEFAULT_ADV_INTERVAL(self) -> Duration:
        """Default time between two advertising messages."""
        return Duration.from_nanoseconds(get_constant(9, c_longlong))

    @property
    def DEFAULT_CONNECTION_TIMEOUT(self) -> Duration:
        """Default timeout for connections between two branches."""
        return Duration.from_nanoseconds(get_constant(10, c_longlong))

    @property
    def DEFAULT_LOGGER_VERBOSITY(self) -> Verbosity:
        """Default verbosity for newly created loggers."""
        return Verbosity(get_constant(11, c_int))

    @property
    def DEFAULT_LOG_TIME_FORMAT(self) -> str:
        """Default format of the time string in log entries."""
        return get_constant(12, c_char_p)

    @property
    def DEFAULT_LOG_FORMAT(self) -> str:
        """Default format of a log entry."""
        return get_constant(13, c_char_p)

    @property
    def MAX_MESSAGE_PAYLOAD_SIZE(self) -> int:
        """Maximum size of the payload of a message between branches."""
        return get_constant(14, c_int)

    @property
    def DEFAULT_TIME_FORMAT(self) -> str:
        """Default textual format for timestamps."""
        return get_constant(15, c_char_p)

    @property
    def DEFAULT_INF_DURATION_STRING(self) -> str:
        """Default string to denote an infinite duration."""
        return get_constant(16, c_char_p)

    @property
    def DEFAULT_DURATION_FORMAT(self) -> str:
        """Default textual format for duration strings."""
        return get_constant(17, c_char_p)

    @property
    def DEFAULT_INVALID_HANDLE_STRING(self) -> str:
        """Default string to denote an invalid object handle."""
        return get_constant(18, c_char_p)

    @property
    def DEFAULT_OBJECT_FORMAT(self) -> str:
        """Default textual format for strings describing an object."""
        return get_constant(19, c_char_p)

    @property
    def MIN_TX_QUEUE_SIZE(self) -> int:
        """Minimum size of a send queue for a remote branch."""
        return get_constant(20, c_int)

    @property
    def MAX_TX_QUEUE_SIZE(self) -> int:
        """Maximum size of a send queue for a remote branch."""
        return get_constant(21, c_int)

    @property
    def DEFAULT_TX_QUEUE_SIZE(self) -> int:
        """Default size of a send queue for a remote branch."""
        return get_constant(22, c_int)

    @property
    def MIN_RX_QUEUE_SIZE(self) -> int:
        """Minimum size of a receive queue for a remote branch."""
        return get_constant(23, c_int)

    @property
    def MAX_RX_QUEUE_SIZE(self) -> int:
        """Maximum size of a receive queue for a remote branch."""
        return get_constant(24, c_int)

    @property
    def DEFAULT_RX_QUEUE_SIZE(self) -> int:
        """Default size of a receive queue for a remote branch."""
        return get_constant(25, c_int)

    @property
    def DEFAULT_WEB_PORT(self) -> int:
        """Default port for the web server to listen on for client connections.
        """
        return get_constant(26, c_int)

    @property
    def DEFAULT_WEB_INTERFACES(self) -> str:
        """Default network interfaces to use for the web server."""
        return get_constant(27, c_char_p)

    @property
    def DEFAULT_WEB_TIMEOUT(self) -> Duration:
        """Default timeout for web server connections."""
        return Duration.from_nanoseconds(get_constant(28, c_longlong))

    @property
    def DEFAULT_WEB_CACHE_SIZE(self) -> int:
        """Default size of the web server cache in bytes."""
        return get_constant(29, c_int)

    @property
    def MAX_WEB_CACHE_SIZE(self) -> int:
        """Maximum size of the web server cache in bytes."""
        return get_constant(30, c_int)

    @property
    def DEFAULT_ADMIN_USER(self) -> str:
        """Default user name for the administrator account."""
        return get_constant(31, c_char_p)

    @property
    def DEFAULT_ADMIN_PASSWORD(self) -> str:
        """Default password for the administrator account."""
        return get_constant(32, c_char_p)

    @property
    def DEFAULT_SSL_PRIVATE_KEY(self) -> str:
        """Default private key to use for SSL connections."""
        return get_constant(33, c_char_p)

    @property
    def DEFAULT_SSL_CERTIFICATE_CHAIN(self) -> str:
        """Default certificate chain to use for SSL connections."""
        return get_constant(34, c_char_p)

    @property
    def DEFAULT_SSL_DH_PARAMS(self) -> str:
        """Default DH parameters to use for SSL connections."""
        return get_constant(35, c_char_p)


class Constants(metaclass=MetaConstants):
    """Constants built into the Yogi Core library."""
    pass
