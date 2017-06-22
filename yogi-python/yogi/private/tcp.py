from .connections import *
import threading
import weakref


class TcpConnection(NonLocalConnection):
    def __init__(self, handle: c_void_p):
        NonLocalConnection.__init__(self, handle)


yogi.YOGI_CreateTcpClient.restype = api_result_handler
yogi.YOGI_CreateTcpClient.argtypes = [POINTER(c_void_p), c_void_p, c_void_p, c_uint]

yogi.YOGI_AsyncTcpConnect.restype = api_result_handler
yogi.YOGI_AsyncTcpConnect.argtypes = [c_void_p, c_char_p, c_uint, c_int, CFUNCTYPE(None, c_int, c_void_p, c_void_p),
                                        c_void_p]

yogi.YOGI_CancelTcpConnect.restype = api_result_handler
yogi.YOGI_CancelTcpConnect.argtypes = [c_void_p]


class TcpClient(Object):
    def __init__(self, scheduler: Scheduler, identification: Optional[str] = None):
        handle = c_void_p()
        if identification is None:
            yogi.YOGI_CreateTcpClient(byref(handle), scheduler._handle, c_void_p(), 0)
        else:
            buffer = create_string_buffer(identification.encode('utf-8'))
            yogi.YOGI_CreateTcpClient(byref(handle), scheduler._handle, buffer, sizeof(buffer))
        Object.__init__(self, handle)
        self._scheduler = scheduler
        self._identification = identification

    @property
    def scheduler(self) -> Scheduler:
        return self._scheduler

    @property
    def identification(self) -> Optional[str]:
        return self._identification

    def async_connect(self, host: str, port: int, handshake_timeout: Optional[float],
                      completion_handler: Callable[[Result, Optional[TcpConnection]], None]) -> None:
        def fn(res, connection_handle):
            connection = None
            if res:
                connection = TcpConnection(connection_handle)
            completion_handler(res, connection)

        with WrappedCallback(yogi.YOGI_AsyncTcpConnect.argtypes[4], fn) as clb_fn:
            yogi.YOGI_AsyncTcpConnect(self._handle, host.encode('utf-8'), port, make_api_timeout(handshake_timeout),
                                         clb_fn, c_void_p())

    def cancel_connect(self) -> None:
        yogi.YOGI_CancelTcpConnect(self._handle)


yogi.YOGI_CreateTcpServer.restype = api_result_handler
yogi.YOGI_CreateTcpServer.argtypes = [POINTER(c_void_p), c_void_p, c_char_p, c_uint, c_void_p, c_uint]

yogi.YOGI_AsyncTcpAccept.restype = api_result_handler
yogi.YOGI_AsyncTcpAccept.argtypes = [c_void_p, c_int, CFUNCTYPE(None, c_int, c_void_p, c_void_p), c_void_p]

yogi.YOGI_CancelTcpAccept.restype = api_result_handler
yogi.YOGI_CancelTcpAccept.argtypes = [c_void_p]


class TcpServer(Object):
    def __init__(self, scheduler: Scheduler, address: str, port: int, identification: Optional[str] = None):
        handle = c_void_p()
        if identification is None:
            yogi.YOGI_CreateTcpServer(byref(handle), scheduler._handle, address.encode('utf-8'), port, c_void_p(), 0)
        else:
            buffer = create_string_buffer(identification.encode('utf-8'))
            yogi.YOGI_CreateTcpServer(byref(handle), scheduler._handle, address.encode('utf-8'), port, buffer,
                                        sizeof(buffer))
        Object.__init__(self, handle)
        self._scheduler = scheduler
        self._address = address
        self._port = port
        self._identification = identification

    @property
    def scheduler(self) -> Scheduler:
        return self._scheduler

    @property
    def address(self) -> str:
        return self._address

    @property
    def port(self) -> int:
        return self._port

    @property
    def identification(self) -> Optional[str]:
        return self._identification

    def async_accept(self, handshake_timeout: Optional[float],
                     completion_handler: Callable[[Result, Optional[TcpConnection]], None]) -> None:
        def fn(res, connection_handle):
            connection = None
            if res:
                connection = TcpConnection(connection_handle)
            completion_handler(res, connection)

        with WrappedCallback(yogi.YOGI_AsyncTcpAccept.argtypes[2], fn) as clb_fn:
            yogi.YOGI_AsyncTcpAccept(self._handle, make_api_timeout(handshake_timeout), clb_fn, c_void_p())

    def cancel_accept(self) -> None:
        yogi.YOGI_CancelTcpAccept(self._handle)


class AutoConnectingTcpClient:
    def __init__(self, endpoint: Endpoint, host: str, port: int, timeout: Optional[float] = None,
                 identification: Optional[str] = None):
        # TODO: Allow ProcessInterface and Configuration as ctor parameters
        self._endpoint = endpoint
        self._host = host
        self._port = port
        self._timeout = timeout
        self._identification = identification
        self._connect_observer = None
        self._disconnect_observer = None
        self._client = TcpClient(endpoint.scheduler, identification)
        self._cv = threading.Condition()
        self._reconnectThread = threading.Thread(target=self._reconnect_thread_fn, args=(weakref.ref(self), self._cv),
                                                 name="Reconnect Thread")
        self._reconnectThreadInitialised = False
        self._running = False
        self._connection = None
        self._connected = False

        self._reconnectThread.start()
        with self._cv:
            while not self._reconnectThreadInitialised:
                self._cv.wait()

    @property
    def endpoint(self) -> Endpoint:
        return self._endpoint

    @property
    def host(self) -> str:
        return self._host

    @property
    def port(self) -> int:
        return self._port

    @property
    def timeout(self) -> Optional[float]:
        return self._timeout

    @property
    def identification(self) -> Optional[str]:
        return self._identification

    @property
    def connect_observer(self) -> Callable[[Result, Optional[TcpConnection]], None]:
        with self._cv:
            return self._connect_observer

    @connect_observer.setter
    def connect_observer(self, fn: Callable[[Result, Optional[TcpConnection]], None]):
        with self._cv:
            self._connect_observer = fn

    @property
    def disconnect_observer(self) -> Callable[[Failure], None]:
        with self._cv:
            return self._disconnect_observer

    @disconnect_observer.setter
    def disconnect_observer(self, fn: Callable[[Failure], None]):
        with self._cv:
            self._disconnect_observer = fn

    @property
    def connected(self) -> bool:
        return self._connected

    @classmethod
    def _reconnect_thread_fn(cls, weak_self, cv):
        with cv:
            weak_self()._reconnectThreadInitialised = True
            cv.notify()

            while True:
                cv.wait()
                self = weak_self()
                if not self:
                    break

                if not self._running:
                    return

                if self._connection is not None:
                    self._connection.destroy()
                    self._connection = None

                self = None
                cv.wait(timeout=1.0)
                self = weak_self()
                if not self or not self._running:
                    return

                self._start_connect()

    def _start_connect(self):
        # TODO: logging
        weak_self = weakref.ref(self)
        self._client.async_connect(self._host, self._port, self._timeout,
                                   lambda res, conn: weak_self()._on_connect_completed(weak_self, res, conn))

    @classmethod
    def _on_connect_completed(cls, weak_self, res, connection):
        self = weak_self()
        if not self or res == Canceled():
            return

        with self._cv:
            if not self._running:
                return

            if res == Success():
                try:
                    connection.assign(self._endpoint, self._timeout)
                    connection.async_await_death(lambda err: cls._on_connection_died(weak_self, err))
                    self._connection = connection

                    # TODO: Logging

                    self._connected = True
                    if self._connect_observer:
                        self._connect_observer(res, connection)

                    return
                except Failure as err:
                    res = err
                    connection.destroy()

            # TODO: Logging

            if self._connect_observer:
                self._connect_observer(res, None)

            self._cv.notify()

    @classmethod
    def _on_connection_died(cls, weak_self, err):
        self = weak_self()
        if not self or err == Canceled():
            return

        with self._cv:
            if not self._running:
                return

            # TODO: Logging

            self._connected = False
            if self._disconnect_observer:
                self._disconnect_observer(err)

            self._cv.notify()

    def start(self):
        with self._cv:
            if self._running:
                raise Exception('Already started')
            if not self._host or not self._port or self._port > 65535:
                raise Exception('Invalid target')

            self._start_connect()
            self._running = True

    def try_start(self) -> bool:
        try:
            self.start()
            return True
        except Failure:
            return False

    def destroy(self) -> None:
        with self._cv:
            self._running = False
            self._cv.notify()

        if self._reconnectThread.isAlive():
            self._reconnectThread.join()

        self._client.destroy()
        self._client = None

        if self._connection:
            self._connection.destroy()
            self._connection = None

    def __del__(self):
        if self._client is not None:
            try:
                self.destroy()
            except Failure:
                pass

    def __str__(self):
        if self.host and self.port:
            return '{} connecting to {}:{}'.format(self.__class__.__name__, self.host, self.port)
        else:
            return '{} (disabled)'.format(self.__class__.__name__)
