from .terminals import *


yogi.YOGI_CreateBinding.restype = api_result_handler
yogi.YOGI_CreateBinding.argtypes = [POINTER(c_void_p), c_void_p, c_char_p]


class Binding(BinderMixin, Object):
    def __init__(self, terminal: PrimitiveTerminal, targets: str):
        self._terminal = terminal
        self._targets = targets

        handle = c_void_p()
        yogi.YOGI_CreateBinding(byref(handle), self._terminal._handle, self._targets.encode('utf-8'))
        Object.__init__(self, handle)

    @property
    def terminal(self) -> PrimitiveTerminal:
        return self._terminal

    @property
    def targets(self) -> str:
        return self._targets