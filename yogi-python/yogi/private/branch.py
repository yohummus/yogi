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

from .object import Object
from .errors import Result, FailureException, Success, ErrorCode, \
    api_result_handler, error_code_to_result, \
    run_with_discriptive_failure_awareness, false_if_specific_ec_else_raise
from .library import yogi
from .handler import Handler
from .context import Context
from .timestamp import Timestamp
from .json_view import JsonView
from .configuration import Configuration
from .msgpack_view import MsgpackView
from .payload_view import PayloadView, EncodingType
from .operation_id import OperationId
from .constants import Constants

import json
import inspect
from enum import IntEnum
from uuid import UUID
from typing import Callable, Any, Optional, Dict, Union
from ctypes import c_char, c_int, c_longlong, c_void_p, c_char_p, CFUNCTYPE, \
    POINTER, byref, create_string_buffer, sizeof


class BranchEvents(IntEnum):
    """Events that can be observed on a branch.

    Attributes:
        NONE              No events.
        BRANCH_DISCOVERED A new branch has been discovered.
        BRANCH_QUERIED    Querying a new branch for information finished.
        CONNECT_FINISHED  Connecting to a branch finished.
        CONNECTION_LOST   The connection to a branch was lost.
        ALL               Combination of all flags.
    """
    NONE = 0
    BRANCH_DISCOVERED = (1 << 0)
    BRANCH_QUERIED = (1 << 1)
    CONNECT_FINISHED = (1 << 2)
    CONNECTION_LOST = (1 << 3)
    ALL = BRANCH_DISCOVERED | BRANCH_QUERIED | CONNECT_FINISHED \
        | CONNECTION_LOST


def convert_info_fields(info):
    if info["advertising_interval"] == -1:
        info["advertising_interval"] = float("inf")

    if info["timeout"] == -1:
        info["timeout"] = float("inf")

    info["uuid"] = UUID(info["uuid"])
    info["start_time"] = Timestamp.parse(info["start_time"])


class BranchInfo:
    """Information about a branch."""

    def __init__(self, info_string: str):
        self._info = json.loads(info_string)
        convert_info_fields(self._info)

    def __str__(self) -> str:
        return str(self._info)

    @property
    def uuid(self) -> UUID:
        """UUID of the branch."""
        return self._info["uuid"]

    @property
    def name(self) -> str:
        """Name of the branch."""
        return self._info["name"]

    @property
    def description(self) -> str:
        """Description of the branch."""
        return self._info["description"]

    @property
    def network_name(self) -> str:
        """Name of the network."""
        return self._info["network_name"]

    @property
    def path(self) -> str:
        """Path of the branch."""
        return self._info["path"]

    @property
    def hostname(self) -> str:
        """The machine's hostname."""
        return self._info["hostname"]

    @property
    def pid(self) -> int:
        """ID of the process."""
        return self._info["pid"]

    @property
    def advertising_interval(self) -> float:
        """Advertising interval."""
        return self._info["advertising_interval"]

    @property
    def tcp_server_address(self) -> str:
        """Address of the TCP server for incoming connections."""
        return self._info["tcp_server_address"]

    @property
    def tcp_server_port(self) -> int:
        """Listening port of the TCP server for incoming connections."""
        return self._info["tcp_server_port"]

    @property
    def start_time(self) -> Timestamp:
        """Time when the branch was started."""
        return self._info["start_time"]

    @property
    def timeout(self) -> float:
        """Connection timeout."""
        return self._info["timeout"]

    @property
    def ghost_mode(self) -> bool:
        """True if the branch is in ghost mode."""
        return self._info["ghost_mode"]

    @property
    def tx_queue_size(self) -> int:
        """Size of the send queue for remote branches."""
        return self._info["tx_queue_size"]

    @property
    def rx_queue_size(self) -> int:
        """Size of the receive queue for remote branches."""
        return self._info["rx_queue_size"]


class RemoteBranchInfo(BranchInfo):
    """Information about a remote branch."""

    def __init__(self, info_string: str):
        BranchInfo.__init__(self, info_string)


class LocalBranchInfo(BranchInfo):
    """Information about a local branch."""

    def __init__(self, info_string: str):
        BranchInfo.__init__(self, info_string)

    @property
    def advertising_address(self) -> str:
        """Advertising IP address."""
        return self._info["advertising_address"]

    @property
    def advertising_port(self) -> int:
        """Advertising port."""
        return self._info["advertising_port"]


class BranchEventInfo:
    """Information associated with a branch event."""

    def __init__(self, info_string: str):
        self._info = json.loads(info_string)
        convert_info_fields(self._info)

    def __str__(self) -> str:
        return str(self._info)

    @property
    def uuid(self) -> UUID:
        """UUID of the branch."""
        return self._info["uuid"]


class BranchDiscoveredEventInfo(BranchEventInfo):
    """Information associated with the BRANCH_DISCOVERED event."""

    def __init__(self, info_string: str):
        BranchEventInfo.__init__(self, info_string)

    @property
    def tcp_server_address(self) -> str:
        """Address of the TCP server for incoming connections."""
        return self._info["tcp_server_address"]

    @property
    def tcp_server_port(self) -> int:
        """Listening port of the TCP server for incoming connections."""
        return self._info["tcp_server_port"]


class BranchQueriedEventInfo(BranchEventInfo, RemoteBranchInfo):
    """Information associated with the BRANCH_QUERIED event."""

    def __init__(self, info_string: str):
        BranchEventInfo.__init__(self, info_string)
        # No need to call RemoteBranchInfo's constructor here


class ConnectFinishedEventInfo(BranchEventInfo):
    """Information associated with the CONNECT_FINISHED event."""

    def __init__(self, info_string: str):
        BranchEventInfo.__init__(self, info_string)


class ConnectionLostEventInfo(BranchEventInfo):
    """Information associated with the CONNECTION_LOST event."""

    def __init__(self, info_string: str):
        BranchEventInfo.__init__(self, info_string)


yogi.YOGI_BranchCreate.restype = int
yogi.YOGI_BranchCreate.argtypes = [
    POINTER(c_void_p), c_void_p, c_void_p, c_char_p, c_char_p, c_int]

yogi.YOGI_BranchGetInfo.restype = api_result_handler
yogi.YOGI_BranchGetInfo.argtypes = [c_void_p, c_void_p, c_char_p, c_int]

yogi.YOGI_BranchGetConnectedBranches.restype = api_result_handler
yogi.YOGI_BranchGetConnectedBranches.argtypes = [
    c_void_p, c_void_p, c_char_p, c_int, CFUNCTYPE(None, c_int, c_void_p),
    c_void_p]

yogi.YOGI_BranchAwaitEventAsync.restype = api_result_handler
yogi.YOGI_BranchAwaitEventAsync.argtypes = [
    c_void_p, c_int, c_void_p, c_char_p, c_int,
    CFUNCTYPE(None, c_int, c_int, c_int, c_void_p), c_void_p]

yogi.YOGI_BranchCancelAwaitEvent.restype = int
yogi.YOGI_BranchCancelAwaitEvent.argtypes = [c_void_p]

yogi.YOGI_BranchSendBroadcast.restype = int
yogi.YOGI_BranchSendBroadcast.argtypes = [
    c_void_p, c_int, c_void_p, c_int, c_int]

yogi.YOGI_BranchSendBroadcastAsync.restype = api_result_handler
yogi.YOGI_BranchSendBroadcastAsync.argtypes = [
    c_void_p, c_int, c_void_p, c_int, c_int,
    CFUNCTYPE(None, c_int, c_int, c_void_p), c_void_p]

yogi.YOGI_BranchCancelSendBroadcast.restype = int
yogi.YOGI_BranchCancelSendBroadcast.argtypes = [c_void_p, c_int]

yogi.YOGI_BranchReceiveBroadcastAsync.restype = api_result_handler
yogi.YOGI_BranchReceiveBroadcastAsync.argtypes = [
    c_void_p, c_void_p, c_int, c_void_p, c_int,
    CFUNCTYPE(None, c_int, c_int, c_void_p), c_void_p]

yogi.YOGI_BranchCancelReceiveBroadcast.restype = int
yogi.YOGI_BranchCancelReceiveBroadcast.argtypes = [c_void_p]


class Branch(Object):
    """Entry point into a Yogi network.

    A branch represents an entry point into a YOGI network. It advertises
    itself via IP broadcasts/multicasts with its unique ID and information
    required for establishing a connection. If a branch detects other branches
    on the network, it connects to them via TCP to retrieve further
    information such as their name, description and network name. If the
    network names match, two branches attempt to authenticate with each other
    by securely comparing passwords. Once authentication succeeds and there is
    no other known branch with the same path then the branches can actively
    communicate as part of the Yogi network.

    Note: Even though the authentication process via passwords is done in a
          secure manner, any further communication is done in plain text.
    """

    def __init__(self, context: Context,
                 config: Union[Configuration, JsonView, str, object] = None,
                 section: str = None):
        """Creates the branch.

        The branch is configured via the config parameter. The supplied
        configuration must have the following structure:

            {
              "name":                   "Fan Controller",
              "description":            "Controls a fan via PWM",
              "path":                   "/Cooling System/Fan Controller",
              "network_name":           "Hardware Control",
              "network_password":       "secret",
              "advertising_interfaces": ["localhost"],
              "advertising_address":    "ff02::8000:2439",
              "advertising_port":       13531,
              "advertising_interval":   1.0,
              "timeout":                3.0,
              "ghost_mode":             false,
              "tx_queue_size":          1000000,
              "rx_queue_size":          100000
            }

        All of the properties are optional and if unspecified (or set to
        null), their respective default values will be used. The properties
        have the following meaning:
         - name: Name of the branch (default: PID@hostname).
         - description: Description of the branch.
         - path: Path of the branch in the network (default: /name where name
           is the name of the branch). Must start with a slash.
         - network_name: Name of the network to join (default: the machine's
           hostname).
         - network_password: Password for the network (default: no password).
         - advertising_interfaces: Network interfaces to use for advertising
           and for branch connections. Valid strings are Unix device names
           ("eth0", "en5", "wlan0"), adapter names on Windows ("Ethernet",
           "VMware Network Adapter WMnet1") or MAC addresses
           ("11:22:33:44:55:66"). Furthermore, the special strings "localhost"
           and "all" can be used to denote loopback and all available
           interfaces respectively.
         - advertising_address: Multicast address to use for advertising, e.g.
           239.255.0.1 for IPv4 or ff02::8000:1234 for IPv6.
         - advertising_port: Port to use for advertising.
         - advertising_interval: Time between advertising messages. Must be at
           least 1 ms.
         - timeout: Amount of time of inactivity before a connection is
           considered to be broken. Must be at least 1 ms.
         - ghost_mode: Set to true to activate ghost mode (default: false).
         - tx_queue_size: Size of the send queues for remote branches.
         - rx_queue_size: Size of the receive queues for remote branches.

        Advertising and establishing connections can be limited to certain
        network interfaces via the _interface_ property. The default is to use
        all available interfaces.

        Setting the ghost_mode property to true prevents the branch from
        actively participating in the Yogi network, i.e. the branch will not
        advertise itself and it will not authenticate in order to join a
        network. However, the branch will temporarily connect to other
        branches in order to obtain more detailed information such as name,
        description, network name and so on. This is useful for obtaining
        information about active branches without actually becoming part of
        the Yogi network.

        Attention:
          The _tx_queue_size_ and _rx_queue_size_ properties affect every
          branch connection and can therefore consume a large amount of memory.
          For example, in a network of 10 branches where these properties are
          set to 1 MB, the resulting memory used for the queues would be
          10 x 2 x 1 MB = 20 MB for each of the 10 branches. This value grows
          with the number of branches squared.

        Args:
            context: The context to use.
            config:  Branch properties as configuration or JSON.
            section: Section in config to use instead of the root section.
                     Syntax is JSON pointer (RFC 6901).
        """
        if not isinstance(config, Configuration):
            if not isinstance(config, JsonView):
                config = JsonView(config)

            config = Configuration(json=config)

        if section:
            section = section.encode()

        handle = c_void_p()
        run_with_discriptive_failure_awareness(
            lambda err: yogi.YOGI_BranchCreate(byref(handle),
                                               context._handle,
                                               config._handle,
                                               section,
                                               err, sizeof(err)))

        Object.__init__(self, handle, [context])
        self._info = self.__get_info()

    @property
    def info(self) -> LocalBranchInfo:
        """Information about the local branch.

        Returns:
            Object containing information about the branch.
        """
        return self._info

    @property
    def uuid(self) -> UUID:
        """UUID of the branch."""
        return self._info.uuid

    @property
    def name(self) -> str:
        """Name of the branch."""
        return self._info.name

    @property
    def description(self) -> str:
        """Description of the branch."""
        return self._info.description

    @property
    def network_name(self) -> str:
        """Name of the network."""
        return self._info.network_name

    @property
    def path(self) -> str:
        """Path of the branch."""
        return self._info.path

    @property
    def hostname(self) -> str:
        """The machine's hostname."""
        return self._info.hostname

    @property
    def pid(self) -> int:
        """ID of the process."""
        return self._info.pid

    @property
    def advertising_address(self) -> str:
        """Advertising IP address."""
        return self._info.advertising_address

    @property
    def advertising_port(self) -> int:
        """Advertising port."""
        return self._info.advertising_port

    @property
    def advertising_interval(self) -> float:
        """Advertising interval."""
        return self._info.advertising_interval

    @property
    def tcp_server_address(self) -> str:
        """Address of the TCP server for incoming connections."""
        return self._info.tcp_server_address

    @property
    def tcp_server_port(self) -> int:
        """Listening port of the TCP server for incoming connections."""
        return self._info.tcp_server_port

    @property
    def start_time(self) -> Timestamp:
        """Time when the branch was started."""
        return self._info.start_time

    @property
    def timeout(self) -> float:
        """Connection timeout."""
        return self._info.timeout

    @property
    def ghost_mode(self) -> bool:
        """True if the branch is in ghost mode."""
        return self._info.ghost_mode

    @property
    def tx_queue_size(self) -> int:
        """Size of the send queue for remote branches."""
        return self._info.tx_queue_size

    @property
    def rx_queue_size(self) -> int:
        """Size of the receive queue for remote branches."""
        return self._info.rx_queue_size

    def get_connected_branches(self) -> Dict[str, RemoteBranchInfo]:
        """Retrieves information about all connected remote branches.

        Returns:
            Dictionary mapping the UUID of each connected remote branch to an
            object containing detailed information.
        """
        s = create_string_buffer(1024)
        strings = []

        def append_string(res, userarg):
            strings.append(s.value.decode("utf-8"))

        c_append_string = yogi.YOGI_BranchGetConnectedBranches.argtypes[4](
            append_string)

        while True:
            try:
                yogi.YOGI_BranchGetConnectedBranches(
                    self._handle, None, s, sizeof(s), c_append_string, None)
                break
            except FailureException as e:
                if e.failure.error_code is ErrorCode.BUFFER_TOO_SMALL:
                    strings = []
                    s = create_string_buffer(sizeof(s) * 2)
                else:
                    raise

        branches = {}
        for string in strings:
            info = RemoteBranchInfo(string)
            branches[info.uuid] = info

        return branches

    def await_event_async(self, events: BranchEvents,
                          fn: Callable[[Result, BranchEvents, Result,
                                        Optional[BranchEventInfo]], Any],
                          buffer_size: int = 1024) -> None:
        """Waits for a branch event to occur.

        This function will register the handler fn to be executed once one of
        the given branch events occurs. The handler's parameters are, from
        left to right, the result associated with the wait operation, the
        event that occurred, the result associated with the event, and a
        dictionary containing event details.

        If this function is called while a previous wait operation is still
        active then the previous operation will be canceled, i.e. the handler
        fn for the previous operation will be called with a cancellation
        error.

        If successful, the event information passed to the handler function fn
        contains at least the UUID of the remote branch.

        In case that the internal buffer for reading the event information is
        too small, fn will be called with the corresponding error and the
        event information is lost. You can set the size of this buffer via the
        bufferSize parameter.

        Args:
            events:      Events to observe.
            fn:          Handler function to call.
            buffer_size: Size of the internal buffer for reading the event
                         information.
        """
        s = create_string_buffer(buffer_size)

        def wrapped_fn(res, event, evres):
            info = None
            if res == Success():
                string = s.value.decode("utf-8")
                if event == BranchEvents.BRANCH_DISCOVERED:
                    info = BranchDiscoveredEventInfo(string)
                elif event == BranchEvents.BRANCH_QUERIED:
                    info = BranchQueriedEventInfo(string)
                elif event == BranchEvents.CONNECT_FINISHED:
                    info = ConnectFinishedEventInfo(string)
                elif event == BranchEvents.CONNECTION_LOST:
                    info = ConnectionLostEventInfo(string)
                else:
                    info = BranchEventInfo(string)

            branch_events = BranchEvents(event)
            ev_res = error_code_to_result(evres)

            if len(inspect.signature(fn).parameters) == 3:
                fn(res, branch_events, ev_res)
            else:
                fn(res, branch_events, ev_res, info)

        with Handler(yogi.YOGI_BranchAwaitEventAsync.argtypes[5], wrapped_fn
                     ) as handler:
            yogi.YOGI_BranchAwaitEventAsync(self._handle, events, None, s,
                                            sizeof(s), handler, None)

    def cancel_await_event(self) -> bool:
        """Cancels waiting for a branch event.

        Calling this function will cause the handler registerd via
        await_event_async() to be called with a cancellation error.

        Returns:
            True if the wait operation was cancelled successfully.
        """
        res = yogi.YOGI_BranchCancelAwaitEvent(self._handle)
        return false_if_specific_ec_else_raise(res,
                                               ErrorCode.OPERATION_NOT_RUNNING)

    def send_broadcast(self, payload: Union[PayloadView, JsonView,
                                            MsgpackView],
                       *, block: bool = True) -> bool:
        """Sends a broadcast message to all connected branches.

        Broadcast messages contain arbitrary data encoded as JSON or
        MessagePack. As opposed to sending messages via terminals, broadcast
        messages don't have to comply with a defined schema for the payload;
        any data that can be encoded is valid. This implies that validating the
        data is entirely up to the user code.

        Setting the block parameter to False will cause the function to skip
        sending the message to branches that have a full send queue. If at
        least one branch was skipped, the function will return False. If the
        parameter is set to True instead, the function will block until the
        message has been put into the send queues of all connected branches.

        Attention: Calling this function from within a handler function
                   executed through the branch's context with block set to True
                   will cause a dead-lock if any send queue is full!

        Args:
            payload: Payload to send.
            block:   Block until message has been put into all send buffers.

        Returns:
            True if the message was successfully put into all send buffers.
        """
        if not isinstance(payload, PayloadView):
            payload = PayloadView(payload)

        res = yogi.YOGI_BranchSendBroadcast(self._handle, payload.encoding,
                                            payload.data.obj, payload.size,
                                            1 if block else 0)
        return false_if_specific_ec_else_raise(res, ErrorCode.TX_QUEUE_FULL)

    def send_broadcast_async(self, payload: Union[PayloadView, JsonView,
                                                  MsgpackView],
                             fn: Callable[[Result, OperationId], Any],
                             *, retry: bool = True) -> OperationId:
        """Sends a broadcast message to all connected branches.

        Broadcast messages contain arbitrary data encoded as JSON or
        MessagePack. As opposed to sending messages via terminals, broadcast
        messages don't have to comply with a defined schema for the payload;
        any data that can be encoded is valid. This implies that validating the
        data is entirely up to the user code.

        The handler function fn will be called once the message has been put
        into the send queues of all connected branches.

        The function returns an ID which uniquely identifies this send
        operation until fn has been called. It can be used in a subsequent
        cancel_send_broadcast() call to abort the operation.

        Args:
            payload: Payload to send.
            fn:      Handler to call once the operation finishes.
            retry:   Retry sending the message if a send queue is full.

        Returns:
            ID of the send operation.
        """
        if not isinstance(payload, PayloadView):
            payload = PayloadView(payload)

        def wrapped_fn(res, oid):
            fn(res, OperationId(oid))

        with Handler(yogi.YOGI_BranchSendBroadcastAsync.argtypes[5], wrapped_fn
                     ) as handler:
            res = yogi.YOGI_BranchSendBroadcastAsync(self._handle,
                                                     payload.encoding,
                                                     payload.data.obj,
                                                     payload.size,
                                                     1 if retry else 0,
                                                     handler, None)

        return OperationId(res.value)

    def cancel_send_broadcast(self, oid: OperationId) -> bool:
        """Cancels a send broadcast operation.

        Calling this function will cause the send operation with the specified
        operation ID to be canceled, resulting in the handler function
        registered via the send_broadcast_async() call that returned the same
        operation ID to be called with the Canceled error.

        Note: If the send operation has already been carried out but the
              handler function has not been called yet, then cancelling the
              operation will fail and False will be returned.

        Args:
            oid: ID of the send operation.

        Returns:
            True if the operation has been canceled successfully.
        """
        res = yogi.YOGI_BranchCancelSendBroadcast(self._handle, oid.value)
        return false_if_specific_ec_else_raise(res,
                                               ErrorCode.INVALID_OPERATION_ID)

    def receive_broadcast_async(self, fn: Callable[[Result, UUID, PayloadView,
                                                    Optional[bytearray]], Any],
                                *,
                                encoding: EncodingType = EncodingType.MSGPACK,
                                buffer: Optional[bytearray] = None
                                ) -> None:
        """Receives a broadcast message from any of the connected branches.

        Broadcast messages contain arbitrary data encoded as JSON or
        MessagePack. As opposed to sending messages via terminals, broadcast
        messages don't have to comply with a defined schema for the payload;
        any data that can be encoded is valid. This implies that validating the
        data is entirely up to the user code.

        This function will register fn to be called once a broadcast message
        has been received. The payload will be encoded as per encoding.

        Attention: If a custom buffer is specified and the received payload
                   does not fit into it then fn will be called with the
                   BUFFER_TOO_SMALL error and the buffer will contain as
                   much received data as possible. In this case, the payload
                   view passed to fn will be invalid.

        If this function is called while a previous receive operation is still
        active then the previous operation will be canceled with the CANCELED
        error.

        Attention: Broadcast messages do not get queued, i.e. if a branch is
                   not actively receiving broadcast messages then they will be
                   discarded. To ensure that no messages get missed, call
                   receive_broadcast_async() again from within the handler fn.

        Args:
            fn:       Handler to call for the received broadcast message.
            encoding: Encoding to use for the received payload.
            buffer:   Custom buffer to use for receiving the payload.
        """
        if buffer is None:
            buffer = bytearray(Constants.MAX_MESSAGE_PAYLOAD_SIZE)

        uuid_buffer = create_string_buffer(16)
        buffer_ptr = (c_char * len(buffer)).from_buffer(buffer)

        def wrapped_fn(res, size):
            uuid = UUID(bytes=uuid_buffer.raw)
            payload = PayloadView(buffer, size, encoding)

            if len(inspect.signature(fn).parameters) == 3:
                fn(res, uuid, payload)
            else:
                fn(res, uuid, payload, buffer)

        with Handler(yogi.YOGI_BranchReceiveBroadcastAsync.argtypes[5],
                     wrapped_fn) as handler:
            yogi.YOGI_BranchReceiveBroadcastAsync(self._handle, uuid_buffer,
                                                  encoding, buffer_ptr,
                                                  len(buffer), handler, None)

    def cancel_receive_broadcast(self) -> bool:
        """Cancels a receive broadcast operation.

        Calling this function will cause the handler registered via
        receive_broadcast_async() to be called with the CANCELED error.

        Note: If the receive handler has already been scheduled for execution
              this function will return False.

        Returns:
            True if the operation has been canceled successfully.
        """
        res = yogi.YOGI_BranchCancelReceiveBroadcast(self._handle)
        return false_if_specific_ec_else_raise(res,
                                               ErrorCode.OPERATION_NOT_RUNNING)

    def __get_info(self) -> LocalBranchInfo:
        s = create_string_buffer(1024)
        while True:
            try:
                yogi.YOGI_BranchGetInfo(self._handle, None, s, sizeof(s))
                break
            except FailureException as e:
                if e.failure.error_code is ErrorCode.BUFFER_TOO_SMALL:
                    s = create_string_buffer(sizeof(s) * 2)
                else:
                    raise

        return LocalBranchInfo(s.value.decode("utf-8"))
