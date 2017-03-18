from enum import Enum
import time


class Timestamp:
    class Precision(Enum):
        SECONDS = 0
        MILLISECONDS = 1
        MICROSECONDS = 2
        NANOSECONDS = 3

    @classmethod
    def now(cls):
        timestamp = Timestamp()
        timestamp._time = time.time()
        timestamp._ns_since_epoch = int(timestamp._time * 1e9)
        return timestamp

    @property
    def ns_since_epoch(self) -> int:
        return self._ns_since_epoch

    @property
    def milliseconds(self) -> int:
        return int(self._ns_since_epoch / 1000000) % 1000

    @property
    def microseconds(self) -> int:
        return int(self._ns_since_epoch / 1000) % 1000

    @property
    def nanoseconds(self) -> int:
        return self._ns_since_epoch % 1000

    def to_string(self, precision: Precision = Precision.MILLISECONDS) -> str:
        s = time.strftime('%d/%m/%Y %T', time.localtime(int(self._time)))

        if precision.value >= self.Precision.MILLISECONDS.value:
            s += '.{:03}'.format(self.milliseconds)
        if precision.value >= self.Precision.MICROSECONDS.value:
            s += '.{:03}'.format(self.microseconds)
        if precision.value >= self.Precision.NANOSECONDS.value:
            s += '.{:03}'.format(self.nanoseconds)

        return s

    def __str__(self):
        return self.to_string()
