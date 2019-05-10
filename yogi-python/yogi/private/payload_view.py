# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2018 Johannes Bergmann.
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

from .json_view import JsonView
from .msgpack_view import MsgpackView

from typing import Union, Optional
from enum import IntEnum


class EncodingType(IntEnum):
    """Possible data/payload encoding types.

    Attributes:
        JSON    Data is encoded as JSON.
        MSGPACK Data is encoded as MessagePack.
    """
    JSON = 0
    MSGPACK = 1


class PayloadView:
    """Helper class for passing and converting different types of user payload.
    """

    def __init__(self, buf_or_view: Union[bytearray, JsonView, MsgpackView],
                 size: Optional[int] = None,
                 encoding: Optional[EncodingType] = None):
        """Constructs a view from a buffer, a JsonView or a MsgpackView.

        Args:
            buf_or_view: Byte array, JsonView or MsgpackView.
            size:        Data size in the bytes array.
            encoding:    Encoding of the data in the bytes array.
        """
        if isinstance(buf_or_view, JsonView):
            assert size is None
            assert encoding is None
            self._memview = buf_or_view.data
            self._size = buf_or_view.size
            self._encoding = EncodingType.JSON
        elif isinstance(buf_or_view, MsgpackView):
            assert size is None
            assert encoding is None
            self._memview = buf_or_view.data
            self._size = buf_or_view.size
            self._encoding = EncodingType.MSGPACK
        elif isinstance(buf_or_view, bytearray):
            assert size is not None
            assert encoding is not None
            self._memview = memoryview(buf_or_view)[:size]
            self._size = size
            self._encoding = encoding
        else:
            raise TypeError(
                'Invalid type ({}) passed to constructor.'.format(
                    type(buf_or_view)))

    @property
    def data(self) -> memoryview:
        """Buffer holding the payload data."""
        return self._memview

    @property
    def size(self) -> int:
        """Size of the payload data in bytes."""
        return self._size

    @property
    def encoding(self) -> EncodingType:
        """Encoding of the payload data."""
        return self._encoding

    def __len__(self) -> int:  # pylint: disable=invalid-length-returned
        return self.size

    def __eq__(self, rhs: Union['PayloadView', JsonView, MsgpackView]) -> bool:
        if isinstance(rhs, JsonView):
            rhs_enc = EncodingType.JSON
        elif isinstance(rhs, MsgpackView):
            rhs_enc = EncodingType.MSGPACK
        else:
            rhs_enc = rhs.encoding

        return self._encoding == rhs_enc and self._memview == rhs._memview

    def __ne__(self, rhs: Union['PayloadView', JsonView, MsgpackView]) -> bool:
        return not (self == rhs)
