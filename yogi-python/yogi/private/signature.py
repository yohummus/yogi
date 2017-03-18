class Signature:
    def __init__(self, raw: int = 0):
        self._raw = raw

    @property
    def raw(self) -> int:
        return self._raw

    def __eq__(self, other):
        if isinstance(other, int):
            return self._raw == other
        else:
            return self._raw == other.raw and isinstance(other, Signature)

    def __ne__(self, other):
        return not (self == other)

    def __str__(self):
        return '{:#010x}'.format(self._raw)[2:]
