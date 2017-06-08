from .signature import *
from .leaf import *
from .path import Path
from typing import *


# ======================================================================================================================
# Common classes and enums
# ======================================================================================================================
class TerminalType(Enum):
    DEAF_MUTE = 0
    PUBLISH_SUBSCRIBE = 1
    SCATTER_GATHER = 2
    CACHED_PUBLISH_SUBSCRIBE = 3
    PRODUCER = 4
    CONSUMER = 5
    CACHED_PRODUCER = 6
    CACHED_CONSUMER = 7
    MASTER = 8
    SLAVE = 9
    CACHED_MASTER = 10
    CACHED_SLAVE = 11
    SERVICE = 12
    CLIENT = 13

def terminal_type_to_class(type: TerminalType):
    if type is TerminalType.DEAF_MUTE:
        return DeafMuteTerminal
    elif type is TerminalType.PUBLISH_SUBSCRIBE:
        return PublishSubscribeTerminal
    elif type is TerminalType.SCATTER_GATHER:
        return ScatterGatherTerminal
    elif type is TerminalType.CACHED_PUBLISH_SUBSCRIBE:
        return CachedPublishSubscribeTerminal
    elif type is TerminalType.PRODUCER:
        return ProducerTerminal
    elif type is TerminalType.CONSUMER:
        return ConsumerTerminal
    elif type is TerminalType.CACHED_PRODUCER:
        return CachedProducerTerminal
    elif type is TerminalType.CACHED_CONSUMER:
        return CachedConsumerTerminal
    elif type is TerminalType.MASTER:
        return MasterTerminal
    elif type is TerminalType.SLAVE:
        return SlaveTerminal
    elif type is TerminalType.CACHED_MASTER:
        return CachedMasterTerminal
    elif type is TerminalType.CACHED_SLAVE:
        return CachedSlaveTerminal
    elif type is TerminalType.SERVICE:
        return ServiceTerminal
    elif type is TerminalType.CLIENT:
        return ClientTerminal
    else:
        raise Exception('Invalid terminal type')


class GatherFlags:
    NO_FLAGS = 0
    FINISHED = 1 << 0
    IGNORED = 1 << 1
    DEAF = 1 << 2
    BINDING_DESTROYED = 1 << 3
    CONNECTION_LOST = 1 << 4


yogi.YOGI_CreateTerminal.restype = api_result_handler
yogi.YOGI_CreateTerminal.argtypes = [POINTER(c_void_p), c_void_p, c_int, c_char_p, c_uint]


class Terminal(Object):
    RECEIVE_MESSAGE_BUFFER_SIZE = 1024 * 64

    def __init__(self, type: TerminalType, name_or_path: Union[Path, str], signature: Union[Signature, int],
                 *, leaf: Optional[Leaf] = None):
        if leaf is None:
            from .process import ProcessInterface
            if ProcessInterface._instance is None:
                raise Exception('No ProcessInterface object has been created.')
            self._leaf =  ProcessInterface.leaf
            path = Path(str(name_or_path))
            if not path.is_absolute:
                path = ProcessInterface.location / path
            self._name = str(path)
        else:
            self._leaf = leaf
            self._name = str(name_or_path)

        self._proto_module = None

        if isinstance(signature, Signature):
            self._signature = signature
        else:
            self._signature = Signature(signature);

        handle = c_void_p()
        yogi.YOGI_CreateTerminal(byref(handle), self._leaf._handle, type.value, self._name.encode('utf-8'),
                                    self._signature.raw)
        Object.__init__(self, handle)

    @property
    def leaf(self) -> Leaf:
        return self._leaf

    @property
    def name(self):
        return self._name

    @property
    def signature(self):
        return self._signature


class ProtoTerminal(Terminal):
    def __init__(self, type: TerminalType, name_or_path: Union[Path, str], proto_module: Any, *,
                 leaf: Optional[Leaf] = None):
        signature = Signature(proto_module.PublishMessage.SIGNATURE)
        Terminal.__init__(self, type, name_or_path, signature, leaf=leaf)
        self._proto_module = proto_module


class PrimitiveTerminal(Terminal):
    def __init__(self, *args, **kwargs):
        Terminal.__init__(self, *args, **kwargs)


class PrimitiveProtoTerminal(ProtoTerminal):
    def __init__(self, *args, **kwargs):
        ProtoTerminal.__init__(self, *args, **kwargs)


class ConvenienceTerminal(Terminal):
    def __init__(self, *args, **kwargs):
        Terminal.__init__(self, *args, **kwargs)


class ConvenienceProtoTerminal(ProtoTerminal):
    def __init__(self, *args, **kwargs):
        ProtoTerminal.__init__(self, *args, **kwargs)


# ======================================================================================================================
# Scatter-Gather-related classes
# ======================================================================================================================
class Operation:
    def __init__(self, terminal, operation_id=None):
        self._terminal = terminal
        self._operation_id = operation_id

    @property
    def terminal(self):
        return self._terminal

    @property
    def operation_id(self):
        return self._operation_id

    def __bool__(self):
        return self._operation_id is not None

    def cancel(self):
        self._terminal._cancel_scatter_gather_api_fn(self._terminal._handle, self._operation_id)
        self._operation_id = None


class ScatteredMessage:
    def __init__(self, terminal, operation_id, buffer, bytes_written):
        self._terminal = terminal
        self._operation_id = operation_id

        data = bytes(bytearray((c_byte * bytes_written).from_buffer(buffer)))
        if self._terminal._proto_module:
            self._msg = self._terminal._proto_module.ScatterMessage()
            self._msg.ParseFromString(data)
        else:
            self._msg = data

    @property
    def terminal(self):
        return self._terminal

    @property
    def message(self):
        return self._msg

    @property
    def operation_id(self):
        return self._operation_id

    def respond(self, msg):
        if self._terminal._proto_module:
            data = msg.SerializeToString()
        else:
            data = msg

        buffer = create_string_buffer(data)
        self._terminal._respond_to_scattered_message_api_fn(self._terminal._handle, self._operation_id, buffer,
                                                            sizeof(buffer) - 1)
        self._operation_id = None

    def ignore(self):
        self._terminal._ignore_scattered_message_api_fn(self._terminal._handle, self._operation_id)
        self._operation_id = None


class GatheredMessage:
    def __init__(self, terminal, operation_id, flags, buffer, bytes_written):
        self._terminal = terminal
        self._operation_id = operation_id
        self._flags = flags

        data = bytes(bytearray((c_byte * bytes_written).from_buffer(buffer)))
        if self._terminal._proto_module:
            self._msg = self._terminal._proto_module.GatherMessage()
            self._msg.ParseFromString(data)
        else:
            self._msg = data

    @property
    def terminal(self):
        return self._terminal

    @property
    def flags(self):
        return self._flags

    @property
    def message(self):
        return self._msg

    @property
    def operation_id(self):
        return self._operation_id


# ======================================================================================================================
# Mixins
# ======================================================================================================================
class BindingState(Enum):
    RELEASED = 0
    ESTABLISHED = 1


yogi.YOGI_GetBindingState.restype = api_result_handler
yogi.YOGI_GetBindingState.argtypes = [c_void_p, POINTER(c_int)]

yogi.YOGI_AsyncGetBindingState.restype = api_result_handler
yogi.YOGI_AsyncGetBindingState.argtypes = [c_void_p, CFUNCTYPE(None, c_int, c_int, c_void_p), c_void_p]

yogi.YOGI_AsyncAwaitBindingStateChange.restype = api_result_handler
yogi.YOGI_AsyncAwaitBindingStateChange.argtypes = [c_void_p, CFUNCTYPE(None, c_int, c_int, c_void_p), c_void_p]

yogi.YOGI_CancelAwaitBindingStateChange.restype = api_result_handler
yogi.YOGI_CancelAwaitBindingStateChange.argtypes = [c_void_p]


class BinderMixin(object):
    def get_binding_state(self) -> BindingState:
        state = c_int()
        yogi.YOGI_GetBindingState(self._handle, byref(state))
        return BindingState.RELEASED if state.value == 0 else BindingState.ESTABLISHED

    def async_get_binding_state(self, completion_handler: Callable[[Result, BindingState], None]) -> None:
        def fn(result, state):
            completion_handler(result, BindingState.RELEASED if state == 0 else BindingState.ESTABLISHED)
        with WrappedCallback(yogi.YOGI_AsyncGetBindingState.argtypes[1], fn) as clb_fn:
            yogi.YOGI_AsyncGetBindingState(self._handle, clb_fn, c_void_p())

    def async_await_binding_state_change(self, completion_handler: Callable[[Failure, BindingState], None]) -> None:
        def fn(result, state):
            completion_handler(result, BindingState.RELEASED if state == 0 else BindingState.ESTABLISHED)
        with WrappedCallback(yogi.YOGI_AsyncAwaitBindingStateChange.argtypes[1], fn) as clb_fn:
            yogi.YOGI_AsyncAwaitBindingStateChange(self._handle, clb_fn, c_void_p())

    def cancel_await_binding_state_change(self) -> None:
        yogi.YOGI_CancelAwaitBindingStateChange(self._handle)


class SubscriptionState(Enum):
    UNSUBSCRIBED = 0
    SUBSCRIBED = 1


yogi.YOGI_GetSubscriptionState.restype = api_result_handler
yogi.YOGI_GetSubscriptionState.argtypes = [c_void_p, POINTER(c_int)]

yogi.YOGI_AsyncGetSubscriptionState.restype = api_result_handler
yogi.YOGI_AsyncGetSubscriptionState.argtypes = [c_void_p, CFUNCTYPE(None, c_int, c_int, c_void_p), c_void_p]

yogi.YOGI_AsyncAwaitSubscriptionStateChange.restype = api_result_handler
yogi.YOGI_AsyncAwaitSubscriptionStateChange.argtypes = [c_void_p, CFUNCTYPE(None, c_int, c_int, c_void_p), c_void_p]

yogi.YOGI_CancelAwaitSubscriptionStateChange.restype = api_result_handler
yogi.YOGI_CancelAwaitSubscriptionStateChange.argtypes = [c_void_p]


class SubscribableMixin(object):
    def get_subscription_state(self) -> SubscriptionState:
        state = c_int()
        yogi.YOGI_GetSubscriptionState(self._handle, byref(state))
        return SubscriptionState.UNSUBSCRIBED if state.value == 0 else SubscriptionState.SUBSCRIBED

    def async_get_subscription_state(self, completion_handler: Callable[[Result, SubscriptionState], None]) -> None:
        def fn(result, state):
            completion_handler(result, SubscriptionState.UNSUBSCRIBED if state == 0 else SubscriptionState.SUBSCRIBED)
        with WrappedCallback(yogi.YOGI_AsyncGetSubscriptionState.argtypes[1], fn) as clb_fn:
            yogi.YOGI_AsyncGetSubscriptionState(self._handle, clb_fn, c_void_p())

    def async_await_subscription_state_change(self, completion_handler: Callable[[Failure, SubscriptionState],
                                                                                 None]) -> None:
        def fn(result, state):
            completion_handler(result, SubscriptionState.UNSUBSCRIBED if state == 0 else SubscriptionState.SUBSCRIBED)
        with WrappedCallback(yogi.YOGI_AsyncAwaitSubscriptionStateChange.argtypes[1], fn) as clb_fn:
            yogi.YOGI_AsyncAwaitSubscriptionStateChange(self._handle, clb_fn, c_void_p())

    def cancel_await_subscription_state_change(self) -> None:
        yogi.YOGI_CancelAwaitSubscriptionStateChange(self._handle)


class PublishMixin:
    def make_message(self, **kwargs) -> Any:
        return self._get_send_msg_class()(**kwargs)

    def publish(self, msg):
        if self._proto_module:
            if not isinstance(msg, self._get_send_msg_class()):
                raise Exception('Invalid parameter: {} is not of type {}'
                                .format(type(msg), self._get_send_msg_class()))
            data = msg.SerializeToString()
        else:
            data = msg

        buffer = create_string_buffer(data)
        self._publish_api_fn(self._handle, buffer, sizeof(buffer) - 1)

    def try_publish(self, msg) -> bool:
        try:
            self.publish(msg)
            return True
        except Failure:
            return False


class PublishMessageReceiverMixin:
    def async_receive_message(self, completion_handler):
        buffer = create_string_buffer(Terminal.RECEIVE_MESSAGE_BUFFER_SIZE)

        def fn(res, bytes_written, cached=None):
            if res:
                data = bytes(bytearray((c_byte * bytes_written).from_buffer(buffer)))
                if self._proto_module:
                    msg = self._get_recv_msg_class()()
                    msg.ParseFromString(data)
                else:
                    msg = data
            else:
                msg = None

            if self._is_cached:
                completion_handler(res, msg, bool(cached))
            else:
                completion_handler(res, msg)

        with WrappedCallback(self._async_receive_message_api_fn.argtypes[3], fn) as clb_fn:
            self._async_receive_message_api_fn(self._handle, buffer, sizeof(buffer), clb_fn, c_void_p())

    def cancel_receive_message(self):
        self._cancel_receive_message_api_fn(self._handle)


class CacheMixin:
    def get_cached_message(self) -> Any:
        buffer = create_string_buffer(Terminal.RECEIVE_MESSAGE_BUFFER_SIZE)
        bytes_written = c_uint()
        self._get_cached_message_api_fn(self._handle, buffer, sizeof(buffer), byref(bytes_written))

        data = bytes(bytearray((c_byte * bytes_written.value).from_buffer(buffer)))
        if self._proto_module:
            msg = self._get_recv_msg_class()()
            msg.ParseFromString(data)
        else:
            msg = data

        return msg


class ServiceMixin:
    def _async_receive_scattered_message(self, completion_handler):
        buffer = create_string_buffer(Terminal.RECEIVE_MESSAGE_BUFFER_SIZE)

        def fn(res, operation_id, bytes_written):
            msg = ScatteredMessage(self, operation_id, buffer, bytes_written) if res else None
            completion_handler(res, msg)

        with WrappedCallback(self._async_receive_scattered_message_api_fn.argtypes[3], fn) as clb_fn:
            self._async_receive_scattered_message_api_fn(self._handle, buffer, sizeof(buffer), clb_fn, c_void_p())

    def _cancel_receive_scattered_message(self):
        self._cancel_receive_scattered_message_api_fn(self._handle, c_void_p())


class ClientMixin:
    def _async_scatter_gather(self, msg, completion_handler: Callable[[Result, Any], ControlFlow]) -> Operation:
        if self._proto_module:
            data = msg.SerializeToString()
        else:
            data = msg

        scatter_buffer = create_string_buffer(data)
        gather_buffer = create_string_buffer(Terminal.RECEIVE_MESSAGE_BUFFER_SIZE)

        def fn(res, operation_id, flags, bytes_written):
            msg = GatheredMessage(self, operation_id, flags, gather_buffer, bytes_written) if res else None
            ret = completion_handler(res, msg)
            return ControlFlow.STOP.value if ret is None else ret.value

        with WrappedCallback(self._async_scatter_gather_api_fn.argtypes[5], fn) as clb_fn:
            res = self._async_scatter_gather_api_fn(self._handle, scatter_buffer, sizeof(scatter_buffer) - 1,
                                                    gather_buffer, sizeof(gather_buffer), clb_fn, c_void_p())

        return Operation(self, res.value)


# ======================================================================================================================
# Deaf-Mute Terminals
# ======================================================================================================================
class DeafMuteTerminal(PrimitiveProtoTerminal):
    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        PrimitiveProtoTerminal.__init__(self, TerminalType.DEAF_MUTE, name, proto_module, leaf=leaf)


class RawDeafMuteTerminal(PrimitiveTerminal):
    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        PrimitiveTerminal.__init__(self, TerminalType.DEAF_MUTE, name, signature, leaf=leaf)


# ======================================================================================================================
# Publish-Subscribe Terminals
# ======================================================================================================================
yogi.YOGI_PS_Publish.restype = api_result_handler
yogi.YOGI_PS_Publish.argtypes = [c_void_p, c_void_p, c_uint]

yogi.YOGI_PS_AsyncReceiveMessage.restype = api_result_handler
yogi.YOGI_PS_AsyncReceiveMessage.argtypes = [c_void_p, c_void_p, c_uint, CFUNCTYPE(None, c_int, c_uint, c_void_p),
                                               c_void_p]

yogi.YOGI_PS_CancelReceiveMessage.restype = api_result_handler
yogi.YOGI_PS_CancelReceiveMessage.argtypes = [c_void_p]


class PublishSubscribeTerminal(PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, PrimitiveProtoTerminal):
    _publish_api_fn = yogi.YOGI_PS_Publish
    _async_receive_message_api_fn = yogi.YOGI_PS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_PS_CancelReceiveMessage

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        PrimitiveProtoTerminal.__init__(self, TerminalType.PUBLISH_SUBSCRIBE, name, proto_module, leaf=leaf)
        self._is_cached = False

    def _get_send_msg_class(self):
        return self._proto_module.PublishMessage

    def _get_recv_msg_class(self):
        return self._proto_module.PublishMessage


class RawPublishSubscribeTerminal(PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, PrimitiveTerminal):
    _publish_api_fn = yogi.YOGI_PS_Publish
    _async_receive_message_api_fn = yogi.YOGI_PS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_PS_CancelReceiveMessage

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        PrimitiveTerminal.__init__(self, TerminalType.PUBLISH_SUBSCRIBE, name, signature, leaf=leaf)
        self._is_cached = False


# ======================================================================================================================
# Cached Publish-Subscribe Terminals
# ======================================================================================================================
yogi.YOGI_CPS_Publish.restype = api_result_handler
yogi.YOGI_CPS_Publish.argtypes = [c_void_p, c_void_p, c_uint]

yogi.YOGI_CPS_GetCachedMessage.restype = api_result_handler
yogi.YOGI_CPS_GetCachedMessage.argtypes = [c_void_p, c_void_p, c_uint, POINTER(c_uint)]

yogi.YOGI_CPS_AsyncReceiveMessage.restype = api_result_handler
yogi.YOGI_CPS_AsyncReceiveMessage.argtypes = [c_void_p, c_void_p, c_uint,
                                                CFUNCTYPE(None, c_int, c_uint, c_int, c_void_p), c_void_p]

yogi.YOGI_CPS_CancelReceiveMessage.restype = api_result_handler
yogi.YOGI_CPS_CancelReceiveMessage.argtypes = [c_void_p]


class CachedPublishSubscribeTerminal(CacheMixin, PublishMixin, PublishMessageReceiverMixin, SubscribableMixin,
                                     PrimitiveProtoTerminal):
    _get_cached_message_api_fn = yogi.YOGI_CPS_GetCachedMessage
    _publish_api_fn = yogi.YOGI_CPS_Publish
    _async_receive_message_api_fn = yogi.YOGI_CPS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_CPS_CancelReceiveMessage

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        PrimitiveProtoTerminal.__init__(self, TerminalType.CACHED_PUBLISH_SUBSCRIBE, name, proto_module, leaf=leaf)
        self._is_cached = True

    def _get_send_msg_class(self):
        return self._proto_module.PublishMessage

    def _get_recv_msg_class(self):
        return self._proto_module.PublishMessage


class RawCachedPublishSubscribeTerminal(CacheMixin, PublishMixin, PublishMessageReceiverMixin, SubscribableMixin,
                                     PrimitiveTerminal):
    _get_cached_message_api_fn = yogi.YOGI_CPS_GetCachedMessage
    _publish_api_fn = yogi.YOGI_CPS_Publish
    _async_receive_message_api_fn = yogi.YOGI_CPS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_CPS_CancelReceiveMessage

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        PrimitiveTerminal.__init__(self, TerminalType.CACHED_PUBLISH_SUBSCRIBE, name, signature, leaf=leaf)
        self._is_cached = True


# ======================================================================================================================
# Scatter-Gather Terminals
# ======================================================================================================================
yogi.YOGI_SG_AsyncScatterGather.restype = api_result_handler
yogi.YOGI_SG_AsyncScatterGather.argtypes = [c_void_p, c_void_p, c_uint, c_void_p, c_uint,
                                              CFUNCTYPE(c_int, c_int, c_int, c_int, c_uint, c_void_p), c_void_p]

yogi.YOGI_SG_CancelScatterGather.restype = api_result_handler
yogi.YOGI_SG_CancelScatterGather.argtypes = [c_void_p, c_int]

yogi.YOGI_SG_AsyncReceiveScatteredMessage.restype = api_result_handler
yogi.YOGI_SG_AsyncReceiveScatteredMessage.argtypes = [c_void_p, c_void_p, c_uint,
                                                        CFUNCTYPE(None, c_int, c_int, c_uint, c_void_p), c_void_p]

yogi.YOGI_SG_CancelReceiveScatteredMessage.restype = api_result_handler
yogi.YOGI_SG_CancelReceiveScatteredMessage.argtypes = [c_void_p]

yogi.YOGI_SG_RespondToScatteredMessage.restype = api_result_handler
yogi.YOGI_SG_RespondToScatteredMessage.argtypes = [c_void_p, c_int, c_void_p, c_uint]

yogi.YOGI_SG_IgnoreScatteredMessage.restype = api_result_handler
yogi.YOGI_SG_IgnoreScatteredMessage.argtypes = [c_void_p, c_int]


class ScatterGatherTerminal(ServiceMixin, ClientMixin, SubscribableMixin, PrimitiveProtoTerminal):
    _async_scatter_gather_api_fn = yogi.YOGI_SG_AsyncScatterGather
    _cancel_scatter_gather_api_fn = yogi.YOGI_SG_CancelScatterGather
    _async_receive_scattered_message_api_fn = yogi.YOGI_SG_AsyncReceiveScatteredMessage
    _cancel_receive_scattered_message_api_fn = yogi.YOGI_SG_CancelReceiveScatteredMessage
    _respond_to_scattered_message_api_fn = yogi.YOGI_SG_RespondToScatteredMessage
    _ignore_scattered_message_api_fn = yogi.YOGI_SG_IgnoreScatteredMessage

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        PrimitiveProtoTerminal.__init__(self, TerminalType.SCATTER_GATHER, name, proto_module, leaf=leaf)

    def make_scatter_message(self, **kwargs) -> Any:
        return self._proto_module.ScatterMessage(**kwargs)

    def make_gather_message(self, **kwargs) -> Any:
        return self._proto_module.GatherMessage(**kwargs)

    def async_scatter_gather(self, msg, completion_handler: Callable[[Result, Optional[GatheredMessage]],
                                                                     ControlFlow])-> Operation:
        return self._async_scatter_gather(msg, completion_handler)

    def try_async_scatter_gather(self, msg, completion_handler: Callable[[Result, Optional[GatheredMessage]],
                                                                         ControlFlow]) -> Operation:
        try:
            return self.async_scatter_gather(msg, completion_handler)
        except Failure:
            return Operation(self)

    def async_receive_scattered_message(self, completion_handler: Callable[[Result, Optional[ScatteredMessage]], None]):
        self._async_receive_scattered_message(completion_handler)

    def cancel_receive_scattered_message(self):
        self._cancel_receive_scattered_message()

ScatterGatherTerminal.Operation = Operation
ScatterGatherTerminal.ScatteredMessage = ScatteredMessage
ScatterGatherTerminal.GatheredMessage = GatheredMessage


class RawScatterGatherTerminal(ServiceMixin, ClientMixin, SubscribableMixin, PrimitiveTerminal):
    _async_scatter_gather_api_fn = yogi.YOGI_SG_AsyncScatterGather
    _cancel_scatter_gather_api_fn = yogi.YOGI_SG_CancelScatterGather
    _async_receive_scattered_message_api_fn = yogi.YOGI_SG_AsyncReceiveScatteredMessage
    _cancel_receive_scattered_message_api_fn = yogi.YOGI_SG_CancelReceiveScatteredMessage
    _respond_to_scattered_message_api_fn = yogi.YOGI_SG_RespondToScatteredMessage
    _ignore_scattered_message_api_fn = yogi.YOGI_SG_IgnoreScatteredMessage

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        PrimitiveTerminal.__init__(self, TerminalType.SCATTER_GATHER, name, signature, leaf=leaf)

    def async_scatter_gather(self, msg, completion_handler: Callable[[Result, Optional[GatheredMessage]],
                                                                     ControlFlow])-> Operation:
        return self._async_scatter_gather(msg, completion_handler)

    def try_async_scatter_gather(self, msg, completion_handler: Callable[[Result, Optional[GatheredMessage]],
                                                                         ControlFlow]) -> Operation:
        try:
            return self.async_scatter_gather(msg, completion_handler)
        except Failure:
            return Operation(self)

    def async_receive_scattered_message(self, completion_handler: Callable[[Result, Optional[ScatteredMessage]], None]):
        self._async_receive_scattered_message(completion_handler)

    def cancel_receive_scattered_message(self):
        self._cancel_receive_scattered_message()

RawScatterGatherTerminal.Operation = Operation
RawScatterGatherTerminal.ScatteredMessage = ScatteredMessage
RawScatterGatherTerminal.GatheredMessage = GatheredMessage


# ======================================================================================================================
# Producer/Consumer Terminals
# ======================================================================================================================
yogi.YOGI_PC_Publish.restype = yogi.YOGI_PS_Publish.restype
yogi.YOGI_PC_Publish.argtypes = yogi.YOGI_PS_Publish.argtypes

yogi.YOGI_PC_AsyncReceiveMessage.restype = yogi.YOGI_PS_AsyncReceiveMessage.restype
yogi.YOGI_PC_AsyncReceiveMessage.argtypes = yogi.YOGI_PS_AsyncReceiveMessage.argtypes

yogi.YOGI_PC_CancelReceiveMessage.restype = yogi.YOGI_PS_CancelReceiveMessage.restype
yogi.YOGI_PC_CancelReceiveMessage.argtypes = yogi.YOGI_PS_CancelReceiveMessage.argtypes


class ProducerTerminal(PublishMixin, SubscribableMixin, ConvenienceProtoTerminal):
    _publish_api_fn = yogi.YOGI_PC_Publish

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.PRODUCER, name, proto_module, leaf=leaf)
        self._is_cached = False

    def _get_send_msg_class(self):
        return self._proto_module.PublishMessage


class RawProducerTerminal(PublishMixin, SubscribableMixin, ConvenienceTerminal):
    _publish_api_fn = yogi.YOGI_PC_Publish

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.PRODUCER, name, signature, leaf=leaf)
        self._is_cached = False


class ConsumerTerminal(PublishMessageReceiverMixin, BinderMixin, ConvenienceProtoTerminal):
    _async_receive_message_api_fn = yogi.YOGI_PC_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_PC_CancelReceiveMessage

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.CONSUMER, name, proto_module, leaf=leaf)
        self._is_cached = False

    def _get_recv_msg_class(self):
        return self._proto_module.PublishMessage


class RawConsumerTerminal(PublishMessageReceiverMixin, BinderMixin, ConvenienceTerminal):
    _async_receive_message_api_fn = yogi.YOGI_PC_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_PC_CancelReceiveMessage

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.CONSUMER, name, signature, leaf=leaf)
        self._is_cached = False


# ======================================================================================================================
# Cached Producer/Consumer Terminals
# ======================================================================================================================
yogi.YOGI_CPC_Publish.restype = yogi.YOGI_CPS_Publish.restype
yogi.YOGI_CPC_Publish.argtypes = yogi.YOGI_CPS_Publish.argtypes

yogi.YOGI_CPC_GetCachedMessage.restype = yogi.YOGI_CPS_GetCachedMessage.restype
yogi.YOGI_CPC_GetCachedMessage.argtypes = yogi.YOGI_CPS_GetCachedMessage.argtypes

yogi.YOGI_CPC_AsyncReceiveMessage.restype = yogi.YOGI_CPS_AsyncReceiveMessage.restype
yogi.YOGI_CPC_AsyncReceiveMessage.argtypes = yogi.YOGI_CPS_AsyncReceiveMessage.argtypes

yogi.YOGI_CPC_CancelReceiveMessage.restype = yogi.YOGI_CPS_CancelReceiveMessage.restype
yogi.YOGI_CPC_CancelReceiveMessage.argtypes = yogi.YOGI_CPS_CancelReceiveMessage.argtypes


class CachedProducerTerminal(PublishMixin, SubscribableMixin, ConvenienceProtoTerminal):
    _publish_api_fn = yogi.YOGI_CPC_Publish

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.CACHED_PRODUCER, name, proto_module, leaf=leaf)
        self._is_cached = True

    def _get_send_msg_class(self):
        return self._proto_module.PublishMessage


class RawCachedProducerTerminal(PublishMixin, SubscribableMixin, ConvenienceTerminal):
    _publish_api_fn = yogi.YOGI_CPC_Publish

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.CACHED_PRODUCER, name, signature, leaf=leaf)
        self._is_cached = True


class CachedConsumerTerminal(CacheMixin, PublishMessageReceiverMixin, BinderMixin, ConvenienceProtoTerminal):
    _get_cached_message_api_fn = yogi.YOGI_CPC_GetCachedMessage
    _async_receive_message_api_fn = yogi.YOGI_CPC_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_CPC_CancelReceiveMessage

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.CACHED_CONSUMER, name, proto_module, leaf=leaf)
        self._is_cached = True

    def _get_recv_msg_class(self):
        return self._proto_module.PublishMessage


class RawCachedConsumerTerminal(CacheMixin, PublishMessageReceiverMixin, BinderMixin, ConvenienceTerminal):
    _get_cached_message_api_fn = yogi.YOGI_CPC_GetCachedMessage
    _async_receive_message_api_fn = yogi.YOGI_CPC_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_CPC_CancelReceiveMessage

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.CACHED_CONSUMER, name, signature, leaf=leaf)
        self._is_cached = True


# ======================================================================================================================
# Master/Slave Terminals
# ======================================================================================================================
yogi.YOGI_MS_Publish.restype = yogi.YOGI_PS_Publish.restype
yogi.YOGI_MS_Publish.argtypes = yogi.YOGI_PS_Publish.argtypes

yogi.YOGI_MS_AsyncReceiveMessage.restype = yogi.YOGI_PS_AsyncReceiveMessage.restype
yogi.YOGI_MS_AsyncReceiveMessage.argtypes = yogi.YOGI_PS_AsyncReceiveMessage.argtypes

yogi.YOGI_MS_CancelReceiveMessage.restype = yogi.YOGI_PS_CancelReceiveMessage.restype
yogi.YOGI_MS_CancelReceiveMessage.argtypes = yogi.YOGI_PS_CancelReceiveMessage.argtypes


class MasterTerminal(PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, BinderMixin, ConvenienceProtoTerminal):
    _publish_api_fn = yogi.YOGI_MS_Publish
    _async_receive_message_api_fn = yogi.YOGI_MS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_MS_CancelReceiveMessage

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.MASTER, name, proto_module, leaf=leaf)
        self._is_cached = False

    def _get_send_msg_class(self):
        return self._proto_module.MasterMessage

    def _get_recv_msg_class(self):
        return self._proto_module.SlaveMessage


class RawMasterTerminal(PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, BinderMixin, ConvenienceTerminal):
    _publish_api_fn = yogi.YOGI_MS_Publish
    _async_receive_message_api_fn = yogi.YOGI_MS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_MS_CancelReceiveMessage

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.MASTER, name, signature, leaf=leaf)
        self._is_cached = False


class SlaveTerminal(PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, BinderMixin, ConvenienceProtoTerminal):
    _publish_api_fn = yogi.YOGI_MS_Publish
    _async_receive_message_api_fn = yogi.YOGI_MS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_MS_CancelReceiveMessage

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.SLAVE, name, proto_module, leaf=leaf)
        self._is_cached = False

    def _get_send_msg_class(self):
        return self._proto_module.SlaveMessage

    def _get_recv_msg_class(self):
        return self._proto_module.MasterMessage


class RawSlaveTerminal(PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, BinderMixin, ConvenienceTerminal):
    _publish_api_fn = yogi.YOGI_MS_Publish
    _async_receive_message_api_fn = yogi.YOGI_MS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_MS_CancelReceiveMessage

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.SLAVE, name, signature, leaf=leaf)
        self._is_cached = False


# ======================================================================================================================
# Cached Master/Slave Terminals
# ======================================================================================================================
yogi.YOGI_CMS_Publish.restype = yogi.YOGI_CPS_Publish.restype
yogi.YOGI_CMS_Publish.argtypes = yogi.YOGI_CPS_Publish.argtypes

yogi.YOGI_CMS_GetCachedMessage.restype = yogi.YOGI_CPS_GetCachedMessage.restype
yogi.YOGI_CMS_GetCachedMessage.argtypes = yogi.YOGI_CPS_GetCachedMessage.argtypes

yogi.YOGI_CMS_AsyncReceiveMessage.restype = yogi.YOGI_CPS_AsyncReceiveMessage.restype
yogi.YOGI_CMS_AsyncReceiveMessage.argtypes = yogi.YOGI_CPS_AsyncReceiveMessage.argtypes

yogi.YOGI_CMS_CancelReceiveMessage.restype = yogi.YOGI_CPS_CancelReceiveMessage.restype
yogi.YOGI_CMS_CancelReceiveMessage.argtypes = yogi.YOGI_CPS_CancelReceiveMessage.argtypes


class CachedMasterTerminal(CacheMixin, PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, BinderMixin,
                           ConvenienceProtoTerminal):
    _get_cached_message_api_fn = yogi.YOGI_CMS_GetCachedMessage
    _publish_api_fn = yogi.YOGI_CMS_Publish
    _async_receive_message_api_fn = yogi.YOGI_CMS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_CMS_CancelReceiveMessage

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.CACHED_MASTER, name, proto_module, leaf=leaf)
        self._is_cached = True

    def _get_send_msg_class(self):
        return self._proto_module.MasterMessage

    def _get_recv_msg_class(self):
        return self._proto_module.SlaveMessage


class RawCachedMasterTerminal(CacheMixin, PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, BinderMixin,
                              ConvenienceTerminal):
    _get_cached_message_api_fn = yogi.YOGI_CMS_GetCachedMessage
    _publish_api_fn = yogi.YOGI_CMS_Publish
    _async_receive_message_api_fn = yogi.YOGI_CMS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_CMS_CancelReceiveMessage

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.CACHED_MASTER, name, signature, leaf=leaf)
        self._is_cached = True


class CachedSlaveTerminal(CacheMixin, PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, BinderMixin,
                          ConvenienceProtoTerminal):
    _get_cached_message_api_fn = yogi.YOGI_CMS_GetCachedMessage
    _publish_api_fn = yogi.YOGI_CMS_Publish
    _async_receive_message_api_fn = yogi.YOGI_CMS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_CMS_CancelReceiveMessage

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.CACHED_SLAVE, name, proto_module, leaf=leaf)
        self._is_cached = True

    def _get_send_msg_class(self):
        return self._proto_module.SlaveMessage

    def _get_recv_msg_class(self):
        return self._proto_module.MasterMessage


class RawCachedSlaveTerminal(CacheMixin, PublishMixin, PublishMessageReceiverMixin, SubscribableMixin, BinderMixin,
                             ConvenienceTerminal):
    _get_cached_message_api_fn = yogi.YOGI_CMS_GetCachedMessage
    _publish_api_fn = yogi.YOGI_CMS_Publish
    _async_receive_message_api_fn = yogi.YOGI_CMS_AsyncReceiveMessage
    _cancel_receive_message_api_fn = yogi.YOGI_CMS_CancelReceiveMessage

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.CACHED_SLAVE, name, signature, leaf=leaf)
        self._is_cached = True


# ======================================================================================================================
# Service/Client Terminals
# ======================================================================================================================
yogi.YOGI_SC_AsyncRequest.restype = yogi.YOGI_SG_AsyncScatterGather.restype
yogi.YOGI_SC_AsyncRequest.argtypes = yogi.YOGI_SG_AsyncScatterGather.argtypes

yogi.YOGI_SC_CancelRequest.restype = yogi.YOGI_SG_CancelScatterGather.restype
yogi.YOGI_SC_CancelRequest.argtypes = yogi.YOGI_SG_CancelScatterGather.argtypes

yogi.YOGI_SC_AsyncReceiveRequest.restype = yogi.YOGI_SG_AsyncReceiveScatteredMessage.restype
yogi.YOGI_SC_AsyncReceiveRequest.argtypes = yogi.YOGI_SG_AsyncReceiveScatteredMessage.argtypes

yogi.YOGI_SC_CancelReceiveRequest.restype = yogi.YOGI_SG_CancelReceiveScatteredMessage.restype
yogi.YOGI_SC_CancelReceiveRequest.argtypes = yogi.YOGI_SG_CancelReceiveScatteredMessage.argtypes

yogi.YOGI_SC_RespondToRequest.restype = yogi.YOGI_SG_RespondToScatteredMessage.restype
yogi.YOGI_SC_RespondToRequest.argtypes = yogi.YOGI_SG_RespondToScatteredMessage.argtypes

yogi.YOGI_SC_IgnoreRequest.restype = yogi.YOGI_SG_IgnoreScatteredMessage.restype
yogi.YOGI_SC_IgnoreRequest.argtypes = yogi.YOGI_SG_IgnoreScatteredMessage.argtypes


class ServiceTerminal(ServiceMixin, BinderMixin, ConvenienceProtoTerminal):
    _async_receive_scattered_message_api_fn = yogi.YOGI_SC_AsyncReceiveRequest
    _cancel_receive_scattered_message_api_fn = yogi.YOGI_SC_CancelReceiveRequest
    _respond_to_scattered_message_api_fn = yogi.YOGI_SC_RespondToRequest
    _ignore_scattered_message_api_fn = yogi.YOGI_SC_IgnoreRequest

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.SERVICE, name, proto_module, leaf=leaf)

    def make_request_message(self, **kwargs) -> Any:
        return self._proto_module.ScatterMessage(**kwargs)

    def make_response_message(self, **kwargs) -> Any:
        return self._proto_module.GatherMessage(**kwargs)

    def async_receive_request(self, completion_handler: Callable[[Result, Optional[ScatteredMessage]], None]):
        self._async_receive_scattered_message(completion_handler)

    def cancel_receive_request(self):
        self._cancel_receive_scattered_message()

ServiceTerminal.Request = ScatteredMessage


class RawServiceTerminal(ServiceMixin, BinderMixin, ConvenienceTerminal):
    _async_receive_scattered_message_api_fn = yogi.YOGI_SC_AsyncReceiveRequest
    _cancel_receive_scattered_message_api_fn = yogi.YOGI_SC_CancelReceiveRequest
    _respond_to_scattered_message_api_fn = yogi.YOGI_SC_RespondToRequest
    _ignore_scattered_message_api_fn = yogi.YOGI_SC_IgnoreRequest

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.SERVICE, name, signature, leaf=leaf)

    def async_receive_request(self, completion_handler: Callable[[Result, Optional[ScatteredMessage]], None]):
        self._async_receive_scattered_message(completion_handler)

    def cancel_receive_request(self):
        self._cancel_receive_scattered_message()

RawServiceTerminal.Request = ScatteredMessage


class ClientTerminal(ClientMixin, SubscribableMixin, ConvenienceProtoTerminal):
    _async_scatter_gather_api_fn = yogi.YOGI_SC_AsyncRequest
    _cancel_scatter_gather_api_fn = yogi.YOGI_SC_CancelRequest

    def __init__(self, name: str, proto_module: Any, *, leaf: Optional[Leaf] = None):
        ConvenienceProtoTerminal.__init__(self, TerminalType.CLIENT, name, proto_module, leaf=leaf)

    def make_request_message(self, **kwargs) -> Any:
        return self._proto_module.ScatterMessage(**kwargs)

    def make_response_message(self, **kwargs) -> Any:
        return self._proto_module.GatherMessage(**kwargs)

    def async_request(self, msg, completion_handler: Callable[[Result, Optional[GatheredMessage]],
                                                              ControlFlow])-> Operation:
        return self._async_scatter_gather(msg, completion_handler)

    def try_async_request(self, msg, completion_handler: Callable[[Result, Optional[GatheredMessage]],
                                                                  ControlFlow]) -> Operation:
        try:
            return self.async_request(msg, completion_handler)
        except Failure:
            return Operation(self)

ScatterGatherTerminal.Operation = Operation
ScatterGatherTerminal.Response = GatheredMessage


class RawClientTerminal(ClientMixin, SubscribableMixin, ConvenienceTerminal):
    _async_scatter_gather_api_fn = yogi.YOGI_SC_AsyncRequest
    _cancel_scatter_gather_api_fn = yogi.YOGI_SC_CancelRequest

    def __init__(self, name: str, signature: Union[Signature, int], *, leaf: Optional[Leaf] = None):
        ConvenienceTerminal.__init__(self, TerminalType.CLIENT, name, signature, leaf=leaf)

    def async_request(self, msg, completion_handler: Callable[[Result, Optional[GatheredMessage]],
                                                              ControlFlow])-> Operation:
        return self._async_scatter_gather(msg, completion_handler)

    def try_async_request(self, msg, completion_handler: Callable[[Result, Optional[GatheredMessage]],
                                                                  ControlFlow]) -> Operation:
        try:
            return self.async_request(msg, completion_handler)
        except Failure:
            return Operation(self)

RawScatterGatherTerminal.Operation = Operation
RawScatterGatherTerminal.Response = GatheredMessage
