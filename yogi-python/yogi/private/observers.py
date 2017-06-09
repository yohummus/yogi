from .terminals import *
from .process import ProcessInterface

import threading


class Observer:
    def __init__(self):
        self._terminate = False

    def destroy(self) -> None:
        self._terminate = True
        self.stop()

    def __del__(self):
        if not self._terminate:
            try:
                self.destroy()
            except Failure:
                pass

    def start(self):
        pass

    def stop(self):
        pass


class CallbackId:
    def __init__(self, observer: Observer, id: int):
        self._observer = observer
        self._id = id

    def __eq__(self, other):
        return self._id == other._id and self._observer is other._observer

    def __ne__(self, other):
        return not (self == other)

    def __hash__(self):
        return self._id


class BadCallbackId(Exception):
    def __str__(self):
        return 'Bad Callback ID'


# ======================================================================================================================
# Binding and Subscription Observers
# ======================================================================================================================
class StateObserver(Observer):
    def __init__(self, observable):
        Observer.__init__(self)
        self._observable = observable
        self._lock = threading.RLock()
        self._callbacks = {}
        self._id_counter = 1
        self._callbacks_called_since_start = False

    def _on_state_received(self, res, state):
        if not res:
            return

        with self._lock:
            if self._terminate:
                return

            type(self)._async_await_state_change_fn(self._observable,
                                                    lambda res, state: self._on_state_received(res, state))

            for _, callback in self._callbacks.items():
                callback(state)

            self._callbacks_called_since_start = True


    def _add(self, callback):
        with self._lock:
            callback_id = CallbackId(self, self._id_counter)
            self._id_counter += 1
            while callback_id in self._callbacks:
                callback_id = CallbackId(self, self._id_counter)
                self._id_counter += 1
            self._callbacks[callback_id] = callback
            return callback_id

    def remove(self, callback_id: CallbackId) -> None:
        with self._lock:
            try:
                del self._callbacks[callback_id]
            except KeyError:
                raise BadCallbackId()

    def start(self) -> None:
        with self._lock:
            type(self)._async_get_state_fn(self._observable, lambda res, state: self._on_state_received(res, state))
            self._callbacks_called_since_start = False

    def stop(self) -> None:
        try:
            type(self)._cancel_await_state_change_fn(self._observable)
        except Failure:
            pass

StateObserver.CallbackId = CallbackId


class BindingObserver(StateObserver):
    _async_get_state_fn = BinderMixin.async_get_binding_state
    _async_await_state_change_fn = BinderMixin.async_await_binding_state_change
    _cancel_await_state_change_fn = BinderMixin.cancel_await_binding_state_change

    def __init__(self, binder: BinderMixin):
        StateObserver.__init__(self, binder)

    @property
    def binder(self) -> BinderMixin:
        return self._observable

    def add(self, callback: Callable[[BindingState], None]) -> CallbackId:
        return self._add(callback)


class SubscriptionObserver(StateObserver):
    _async_get_state_fn = SubscribableMixin.async_get_subscription_state
    _async_await_state_change_fn = SubscribableMixin.async_await_subscription_state_change
    _cancel_await_state_change_fn = SubscribableMixin.cancel_await_subscription_state_change

    def __init__(self, subscribable: SubscribableMixin):
        StateObserver.__init__(self, subscribable)

    @property
    def subscribable(self) -> SubscribableMixin:
        return self._observable

    def add(self, callback: Callable[[SubscriptionState], None]) -> CallbackId:
        return self._add(callback)


# ======================================================================================================================
# Message Observers
# ======================================================================================================================
class MessageObserver(Observer):
    def __init__(self, terminal: Terminal):
        Observer.__init__(self)
        self._terminal = terminal
        self._lock = threading.Lock()

        if isinstance(terminal, PublishMessageReceiverMixin):
            self._is_ps_based = True
            self._callbacks = {}
            self._id_counter = 1
            self.add = self._add
            self.remove = self._remove
            self._async_receive_message_fn = terminal.async_receive_message
            self._cancel_receive_message_fn = terminal.cancel_receive_message
        else:
            self._is_ps_based = False
            self._callback = None
            self.set = self._set
            self.clear = self._clear
            if isinstance(terminal, ScatterGatherTerminal):
                self._async_receive_message_fn = terminal.async_receive_scattered_message
                self._cancel_receive_message_fn = terminal.cancel_receive_scattered_message
            else:
                self._async_receive_message_fn = terminal.async_receive_request
                self._cancel_receive_message_fn = terminal.cancel_receive_request

    @property
    def terminal(self) -> Terminal:
        return self._terminal

    def _add(self, callback: Callable[[Any, Optional[bool]], None]) -> CallbackId:
        with self._lock:
            callback_id = CallbackId(self, self._id_counter)
            self._id_counter += 1
            while callback_id in self._callbacks:
                callback_id = CallbackId(self, self._id_counter)
                self._id_counter += 1
            self._callbacks[callback_id] = callback
            return callback_id

    def _remove(self, callback_id: CallbackId) -> None:
        with self._lock:
            try:
                del self._callbacks[callback_id]
            except KeyError:
                raise BadCallbackId()

    def _set(self, callback: Callable[[ScatteredMessage], None]) -> None:
        with self._lock:
            self._callback = callback

    def _clear(self):
        with self._lock:
            self._callback = None

    def _on_message_received(self, res, msg, cached):
        if not res:
            return

        with self._lock:
            if self._terminate:
                return

            self._async_receive_message_fn(lambda res, msg, cached=None: self._on_message_received(res, msg, cached))

            if self._is_ps_based:
                for _, callback in self._callbacks.items():
                    if self._terminal._is_cached:
                        callback(msg, cached)
                    else:
                        callback(msg)
            else:
                if self._callback:
                    self._callback(msg)
                else:
                    msg.ignore()

    def start(self):
        with self._lock:
            self._async_receive_message_fn(lambda res, msg, cached=None: self._on_message_received(res, msg, cached))

    def stop(self):
        try:
            self._cancel_receive_message_fn()
        except Failure:
            pass


# ======================================================================================================================
# Operational Observer
# ======================================================================================================================
class OperationalObserver(Observer):
    def __init__(self):
        Observer.__init__(self)
        self._lock = threading.RLock()
        self._callbacks = {}
        self._id_counter = 1

    def _notify_state(self, state):
        with self._lock:
            for _, callback in self._callbacks.items():
                callback(state)


    def add(self, callback: Callable[[bool], None]) -> CallbackId:
        with self._lock:
            callback_id = CallbackId(self, self._id_counter)
            self._id_counter += 1
            while callback_id in self._callbacks:
                callback_id = CallbackId(self, self._id_counter)
                self._id_counter += 1
            self._callbacks[callback_id] = callback
            return callback_id

    def remove(self, callback_id: CallbackId) -> None:
        with self._lock:
            try:
                del self._callbacks[callback_id]
            except KeyError:
                raise BadCallbackId()

    def start(self) -> None:
        with self._lock:
            ProcessInterface._add_operational_observer(self)

    def stop(self) -> None:
        with self._lock:
            ProcessInterface._remove_operational_observer(self)

OperationalObserver.CallbackId = CallbackId
