from typing import *
import posixpath


class BadPath(Exception):
    def __init__(self, path: str):
        self._path = path

    def __str__(self):
        return 'Invalid path: \'{}\''.format(self._path)


class Path:
    def __init__(self, path: Optional[str] = None):
        self._path = path if path else ''
        if '//' in self._path:
            raise BadPath(self._path)

    def __str__(self):
        return self._path

    def __len__(self):
        return len(self._path)

    def __eq__(self, other):
        return self._path == str(other)

    def __ne__(self, other):
        return not (self == other)

    def __truediv__(self, other):
        if Path(str(other)).is_absolute:
            raise BadPath(str(other))
        return Path(posixpath.join(self._path, str(other)))

    def clear(self) -> None:
        self._path = ''

    @property
    def is_absolute(self) -> bool:
        return not len(self._path) == 0 and self._path[0] == '/'

    @property
    def is_root(self) -> bool:
        return self._path == '/'
