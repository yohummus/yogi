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

from typing import Union

import msgpack


class MsgpackView:
    """Helper class for passing different types of MessagePack to functions.
    """

    def __init__(self, msg_pack: Union[bytes, object]):
        """Constructs a view from either a byte array or arbitrary objects.

        Args:
            msg_pack: Serialized MessagePack or an arbitrary object that can be
                      serialized as MessagePack.
        """
        if isinstance(msg_pack, bytes):
            data_bytes = msg_pack
        else:
            data_bytes = msgpack.packb(msg_pack)

        self._memview = memoryview(data_bytes)

    @property
    def data(self) -> memoryview:
        """Serialized MessagePack data."""
        return self._memview

    @property
    def size(self) -> int:
        """Length of the serialized MessagePack data in bytes."""
        return self._memview.nbytes

    def __len__(self) -> int:  # pylint: disable=invalid-length-returned
        return self.size

    def __eq__(self, rhs: 'MsgpackView') -> bool:
        return self._memview == rhs._memview

    def __ne__(self, rhs: 'MsgpackView') -> bool:
        return not (self == rhs)

    def __hash__(self) -> int:
        return hash(self._memview)
