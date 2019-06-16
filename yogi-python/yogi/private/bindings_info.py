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


class MetaBindingsInfo(type):
    @property
    def VERSION(self) -> str:
        """Whole version string of the bindings."""
        return '{}.{}.{}{}'.format(self.VERSION_MAJOR, self.VERSION_MINOR,
                                   self.VERSION_PATCH, self.VERSION_SUFFIX)

    @property
    def VERSION_MAJOR(self) -> int:
        """Major version of the bindings."""
        return 0

    @property
    def VERSION_MINOR(self) -> int:
        """Minor version of the bindings."""
        return 0

    @property
    def VERSION_PATCH(self) -> int:
        """Patch version of the bindings."""
        return 3

    @property
    def VERSION_SUFFIX(self) -> str:
        """Version suffix of the bindings."""
        return '-alpha'


class BindingsInfo(metaclass=MetaBindingsInfo):
    """Information about the bindings."""
    pass
