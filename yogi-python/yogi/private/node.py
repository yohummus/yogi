from .terminals import *
from ctypes import *
from struct import *

class ChangeType(Enum):
    REMOVED = 0
    ADDED = 1


class TerminalInfo:
    def __init__(self, type: Terminal, signature: Signature, name: str):
        self._type = type
        self._signature = signature
        self._name = name

    @property
    def type(self):
        return self._type

    @property
    def signature(self):
        return self._signature

    @property
    def name(self):
        return self._name


yogi.YOGI_CreateNode.restype = api_result_handler
yogi.YOGI_CreateNode.argtypes = [c_void_p]

yogi.YOGI_GetKnownTerminals.restype = api_result_handler
yogi.YOGI_GetKnownTerminals.argtypes = [c_void_p, c_void_p, c_uint, POINTER(c_uint)]

yogi.YOGI_AsyncAwaitKnownTerminalsChange.restype = api_result_handler
yogi.YOGI_AsyncAwaitKnownTerminalsChange.argtypes = [c_void_p, c_void_p, c_uint, CFUNCTYPE(None, c_int, c_void_p),
                                                       c_void_p]

yogi.YOGI_CancelAwaitKnownTerminalsChange.restype = api_result_handler
yogi.YOGI_CancelAwaitKnownTerminalsChange.argtypes = [c_void_p]


class Node(Endpoint):
    GET_KNOWN_TERMINALS_INITIAL_BUFFER_SIZE = 1024 * 64
    AWAIT_KNOWN_TERMINALS_CHANGED_BUFFER_SIZE = 1024 * 1

    def __init__(self, scheduler: Scheduler):
        handle = c_void_p()
        yogi.YOGI_CreateNode(byref(handle), scheduler._handle)
        Endpoint.__init__(self, handle, scheduler)

    def get_known_terminals(self) -> List[TerminalInfo]:
        buffer = create_string_buffer(Node.GET_KNOWN_TERMINALS_INITIAL_BUFFER_SIZE)
        num_terminals = c_uint()

        while True:
            try:
                yogi.YOGI_GetKnownTerminals(self._handle, buffer, sizeof(buffer), byref(num_terminals))
                break
            except Failure as failure:
                if failure != BufferTooSmall():
                    raise
                buffer = create_string_buffer(sizeof(buffer) * 2)

        terminals = []
        offset = 0
        info_struct = Struct('=cI')
        for _ in range(num_terminals.value):
            (terminal_type, raw_signature) = info_struct.unpack_from(buffer, offset)
            offset += info_struct.size

            terminal_class = terminal_type_to_class(TerminalType(ord(terminal_type)))
            signature = Signature(raw_signature)
            name = string_at(addressof(buffer) + offset).decode()
            offset += len(name) + 1

            terminals.append(TerminalInfo(terminal_class, signature, name))

        return terminals

    def async_await_known_terminals_change(self, completion_handler: Callable[[Result, TerminalInfo, ChangeType],
                                                                              None]) -> None:
        buffer = create_string_buffer(Node.AWAIT_KNOWN_TERMINALS_CHANGED_BUFFER_SIZE)

        def fn(res):
            if res:
                info_struct = Struct('=ccI')
                (added, terminal_type, raw_signature) = info_struct.unpack_from(buffer)

                change = ChangeType.ADDED if ord(added) == 1 else ChangeType.REMOVED
                terminal_class = terminal_type_to_class(TerminalType(ord(terminal_type)))
                signature = Signature(raw_signature)
                name = string_at(addressof(buffer) + info_struct.size).decode()

                completion_handler(res, TerminalInfo(terminal_class, signature, name), change)
            else:
                completion_handler(res, None, None)

        with WrappedCallback(yogi.YOGI_AsyncAwaitKnownTerminalsChange.argtypes[3], fn) as clb_fn:
            yogi.YOGI_AsyncAwaitKnownTerminalsChange(self._handle, buffer, sizeof(buffer), clb_fn, c_void_p())

    def cancel_await_known_terminals_change(self) -> None:
        yogi.YOGI_CancelAwaitKnownTerminalsChange(self._handle)
