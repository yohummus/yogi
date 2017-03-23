from .configuration import *
from .terminals import *
from .proto import yogi_00000001_pb2
from .proto import yogi_00000004_pb2
from .proto import yogi_000009cd_pb2
from .proto import yogi_0000040d_pb2
import datetime
import threading
import sys


class ProcessInterface:
    _instance = None

    def __init__(self, configuration_or_argv: Optional[Union[Configuration, List[str]]] = sys.argv[1:]):
        cls = type(self)
        if cls._instance:
            raise Exception('ProcessInterface is a singleton and has already been created. Call destroy() on the'
                            ' existing instance before you create a new one.')

        if isinstance(configuration_or_argv, Configuration):
            self._config = configuration_or_argv
        else:
            self._config = Configuration(configuration_or_argv)

        self._scheduler = Scheduler()
        self._leaf = Leaf(self._scheduler)

        self._init_logging()
        self._init_operational()
        self._init_anomalies()

        cls._instance = self

    def destroy(self):
        self._shutdown_logging()
        self._shutdown_operational()
        self._shutdown_anomalies()

        type(self)._instance = None

    def __del__(self):
        if type(self)._instance:
            try:
                self.destroy()
            except Failure:
                pass

    @classproperty
    def instance(cls):
        return cls._instance

    @classproperty
    def config(cls) -> Configuration:
        return cls._instance._config

    @classproperty
    def location(cls) -> Path:
        return cls._instance._config.location

    @classproperty
    def scheduler(cls) -> Scheduler:
        return cls._instance._scheduler

    @classproperty
    def leaf(cls) -> Leaf:
        return cls._instance._leaf

    @classproperty
    def operational(cls) -> bool:
        return cls._instance._operational_state

    def _init_logging(self):
        self._log_terminal = ProducerTerminal(self._config.location / 'Process/Log',
                                              yogi_000009cd_pb2, leaf=self._leaf)

        self._log_verbosity_terminals_lock = threading.Lock()
        self._max_stdout_log_verbosity_terminal = CachedMasterTerminal(
            self._config.location / 'Process/Standard Output Log Verbosity/Max Verbosity', yogi_00000004_pb2,
            leaf=self._leaf)
        self._max_stdout_log_verbosity_terminal.async_receive_message(
            lambda res, msg, cached: self._on_max_logging_verbosity_changed(res, msg, cached, True,
                                                                            self._max_stdout_log_verbosity_terminal))
        self._max_yogi_log_verbosity_terminal = CachedMasterTerminal(
            self._config.location / 'Process/YOGI Log Verbosity/Max Verbosity', yogi_00000004_pb2,
            leaf=self._leaf)
        self._max_yogi_log_verbosity_terminal.async_receive_message(
            lambda res, msg, cached: self._on_max_logging_verbosity_changed(res, msg, cached, False,
                                                                            self._max_yogi_log_verbosity_terminal))
        self._stdout_log_verbosity_terminals = {}
        self._yogi_log_verbosity_terminals = {}

        from .logging import Logger
        Logger.colourised_stdout = self._get_config_child(['logging', 'stdout', 'colourised'], False)

        for logger in Logger.all_loggers:
            self._register_logger(logger)

        child = self._get_config_child(['logging', 'stdout'], None)
        if child:
            Logger.max_stdout_verbosity = self._get_verbosity_from_config(child, 'max-verbosity')
            comp_child = child.get('component-verbosity', {})
            for name, level in comp_child.items():
                logger = Logger(component=name)
                logger.stdout_verbosity = self._get_verbosity_from_config(comp_child, name)

        child = self._get_config_child(['logging', 'yogi'], None)
        if child:
            Logger.max_yogi_verbosity = self._get_verbosity_from_config(child, 'max-verbosity')
            comp_child = child.get('component-verbosity', {})
            for name, level in comp_child.items():
                logger = Logger(component=name)
                logger.yogi_verbosity = self._get_verbosity_from_config(comp_child, name)

    def _get_config_child(self, path, default=None):
        child = self._config.config
        for name in path:
            if name in child:
                child = child[name]
            else:
                return default
        return child

    def _get_verbosity_from_config(self, child, key):
        from .logging import Verbosity
        if key not in child:
            return Verbosity.TRACE
        else:
            return Verbosity[child[key]]

    def _on_max_logging_verbosity_changed(self, res, msg, cached, is_stdout, terminal):
        if not res:
            return

        from .logging import Logger, Verbosity
        try:
            verbosity = Verbosity(msg.value)
        except ValueError:
            verbosity = Verbosity.TRACE

        if not cached:
            if is_stdout:
                Logger.max_stdout_verbosity = verbosity
            else:
                Logger.max_yogi_verbosity = verbosity
            terminal.try_publish(msg)

        fn = lambda res, msg, cached: self._on_max_logging_verbosity_changed(res, msg, cached, is_stdout, terminal)
        terminal.async_receive_message(fn)

    def _on_logging_verbosity_changed(self, res, msg, cached, is_stdout, component, terminal):
        if not res:
            return

        from .logging import Logger, Verbosity
        logger = Logger(component=component)
        try:
            verbosity = Verbosity(msg.value)
        except ValueError:
            verbosity = Verbosity.TRACE

        if not cached:
            if is_stdout:
                logger.stdout_verbosity = verbosity
            else:
                logger.yogi_verbosity = verbosity
            terminal.try_publish(msg)

        fn = lambda res, msg, cached: self._on_logging_verbosity_changed(res, msg, cached, is_stdout, component, terminal)
        with self._log_verbosity_terminals_lock:
            terminal.async_receive_message(fn)

    def _register_logger(self, logger):
        component = logger.component

        stdout_vb_terminal = CachedMasterTerminal(
            self._config.location / 'Process/Standard Output Log Verbosity/Components' / component,
            yogi_00000004_pb2, leaf=self._leaf)
        stdout_vb_terminal.async_receive_message(lambda res, msg, cached: self._on_logging_verbosity_changed(
            res, msg, cached, True, component, stdout_vb_terminal))

        yogi_vb_terminal = CachedMasterTerminal(
            self._config.location / 'Process/YOGI Log Verbosity/Components' / component,
            yogi_00000004_pb2, leaf=self._leaf)
        yogi_vb_terminal.async_receive_message(lambda res, msg, cached: self._on_logging_verbosity_changed(
            res, msg, cached, False, component, yogi_vb_terminal))

        with self._log_verbosity_terminals_lock:
            self._stdout_log_verbosity_terminals[logger.component] = stdout_vb_terminal
            self._yogi_log_verbosity_terminals[logger.component] = yogi_vb_terminal

    def _shutdown_logging(self):
        self._log_terminal.destroy()
        self._max_stdout_log_verbosity_terminal.destroy()
        self._max_yogi_log_verbosity_terminal.destroy()
        for _, terminal in self._stdout_log_verbosity_terminals.items():
            terminal.destroy()
        for _, terminal in self._yogi_log_verbosity_terminals.items():
            terminal.destroy()

    def _init_operational(self):
        self._operational_lock = threading.Lock()
        self._operational_conditions = []
        self._operational_observers = []
        self._operational_state = True
        self._operational_terminal = CachedProducerTerminal(self._config.location / 'Process/Operational',
                                                            yogi_00000001_pb2, leaf=self._leaf)
        self._publish_operational_state(self._operational_state)

    def _shutdown_operational(self):
        for observer in self._operational_observers:
            observer.destroy()

        for oc in self._operational_conditions:
            oc.destroy()

        self._operational_terminal.destroy()

    def _publish_operational_state(self, state):
        msg = self._operational_terminal.make_message(value=state)
        self._operational_terminal.try_publish(msg)

    @classmethod
    def _add_operational_observer(cls, observer):
        self = cls._instance
        with self._operational_lock:
            self._operational_observers.append(observer)
            observer._notify_state(self._operational_state)

    @classmethod
    def _remove_operational_observer(cls, observer):
        self = cls._instance
        if self is None:
            return

        with self._operational_lock:
            if observer in self._operational_observers:
                self._operational_observers.remove(observer)

    @classmethod
    def _notify_operational_condition_change(cls, oc):
        self = cls._instance
        if self is None:
            return

        with self._operational_lock:
            changed = self._update_operational_state()
            if changed:
                from .logging import Logger
                Logger.yogi_logger.log_info("Operational state changed to {} due to a change of the Operational"
                                               " Condition '{}'".format('TRUE' if self._operational_state else 'FALSE',
                                                                        oc.name))

    def _update_operational_state(self):
        operational = True
        for oc in self._operational_conditions:
            if not oc.is_met:
                operational = False
                break

        if operational is self._operational_state:
            return False

        self._operational_state = operational
        self._publish_operational_state(operational)

        for observer in self._operational_observers:
            observer._notify_state(operational)

        return True

    @classmethod
    def _add_operational_condition(cls, oc):
        self = cls._instance
        with self._operational_lock:
            self._operational_conditions.append(oc)

            changed = self._update_operational_state()
            if changed:
                from .logging import Logger
                Logger.yogi_logger.log_info("Operational state changed to {} due to the addition of the Operational"
                                               " Condition '{}'".format('TRUE' if self._operational_state else 'FALSE',
                                                                        oc.name))

    @classmethod
    def _remove_operational_condition(cls, oc):
        self = cls._instance
        if self is None:
            return

        with self._operational_lock:
            if oc not in self._operational_conditions:
                return

            self._operational_conditions.remove(oc)

            changed = self._update_operational_state()
            # TODO: Commented out due to deadlock on Linux (Ticket #4)
            # if changed:
            #     from .logging import Logger
            #     Logger.yogi_logger.log_info("Operational state changed to {} due to the removal of the Operational"
            #                                    " Condition '{}'".format('TRUE' if self._operational_state else 'FALSE',
            #                                                             oc.name))

    def _init_anomalies(self):
        self._running = True
        self._anomalies_cv = threading.Condition()
        self._anomalies_thread = threading.Thread(target=self._anomalies_thread_fn)
        self._active_anomalies = []

        self._errors_terminal = CachedProducerTerminal(self._config.location / 'Process/Errors',
                                                       yogi_0000040d_pb2, leaf=self._leaf)
        self._publish_string_list(self._errors_terminal, [])

        self._warnings_terminal = CachedProducerTerminal(self._config.location / 'Process/Warnings',
                                                         yogi_0000040d_pb2, leaf=self._leaf)
        self._publish_string_list(self._warnings_terminal, [])

        self._anomalies_thread.start()

    def _shutdown_anomalies(self):
        with self._anomalies_cv:
            self._running = False
            self._anomalies_cv.notify()

        if self._anomalies_thread.isAlive():
            self._anomalies_thread.join()

        self._errors_terminal.destroy()
        self._warnings_terminal.destroy()

    def _anomalies_thread_fn(self):
        with self._anomalies_cv:
            while self._running:
                next_expiration = self._update_active_anomalies()
                if next_expiration is None:
                    self._anomalies_cv.wait()
                else:
                    now = datetime.datetime.now()
                    if next_expiration > now:
                        timeout = (next_expiration - now).total_seconds()
                        self._anomalies_cv.wait(timeout)

    def _update_active_anomalies(self):
        now = datetime.datetime.now()
        next_expiration = None

        update_errors = False
        update_warnings = False

        for aa in self._active_anomalies:
            if aa.expiration_time is not None:
                if aa.expiration_time > now:
                    if next_expiration is None:
                        next_expiration = aa.expiration_time
                    else:
                        next_expiration = min(next_expiration, aa.expiration_time)
                else:
                    if aa.is_error:
                        update_errors = True
                    else:
                        update_warnings = True

                    from .logging import Logger
                    type = 'Error' if aa.is_error else 'Warning'
                    Logger.yogi_logger.log_info("{} '{}' expired".format(type, aa.message))

                    self._active_anomalies.remove(aa)

        if update_errors:
            self._publish_anomalies(True)

        if update_warnings:
            self._publish_anomalies(False)

        return next_expiration

    def _publish_anomalies(self, errors):
        lst = [anomaly.message for anomaly in self._active_anomalies if anomaly.is_error is errors]
        msg = self._errors_terminal.make_message(value=lst)
        tm = self._errors_terminal if errors else self._warnings_terminal
        tm.try_publish(msg)

    @classmethod
    def _set_anomaly(cls, anomaly):
        self = cls._instance
        if self is None:
            return

        with self._anomalies_cv:
            if anomaly not in self._active_anomalies:
                from .logging import Logger
                if anomaly.is_error:
                    Logger.yogi_logger.log_error("Error '{}' set".format(anomaly.message))
                else:
                    Logger.yogi_logger.log_warning("Warning '{}' set".format(anomaly.message))

                self._active_anomalies.append(anomaly)
                self._publish_anomalies(anomaly.is_error)
            self._anomalies_cv.notify()

    @classmethod
    def _clear_anomaly(cls, anomaly):
        self = cls._instance
        if self is None:
            return

        with self._anomalies_cv:
            if anomaly in self._active_anomalies:
                self._active_anomalies.remove(anomaly)

                from .logging import Logger
                type = 'Error' if anomaly.is_error else 'Warning'
                Logger.yogi_logger.log_info("{} '{}' cleared".format(type, anomaly.message))

                self._publish_anomalies(anomaly.is_error)
                self._anomalies_cv.notify()

    def _publish_string_list(self, terminal, strings):
        msg = terminal.make_message(value=strings)
        terminal.try_publish(msg)

    @classmethod
    def _publish_log_message(cls, severity, component, message, timestamp, thread_id):
        if not cls._instance:
            return

        json = '{{"severity": "{}", "thread": {}, "component": "{}"}}'.format(severity.name, thread_id, component)

        msg = cls._instance._log_terminal.make_message()
        msg.value.first = message
        msg.value.second = json
        msg.timestamp = timestamp.ns_since_epoch

        cls._instance._log_terminal.try_publish(msg)

    @classmethod
    def _on_new_logger_added(cls, logger):
        if cls._instance:
            cls._instance._register_logger(logger)

    @classmethod
    def _on_max_verbosity_set(cls, verbosity, is_stdout):
        if not cls._instance:
            return

        msg = cls._instance._max_stdout_log_verbosity_terminal.make_message(value=verbosity.value)

        if is_stdout:
            cls._instance._max_stdout_log_verbosity_terminal.try_publish(msg)
        else:
            cls._instance._max_yogi_log_verbosity_terminal.try_publish(msg)

    @classmethod
    def _on_verbosity_set(cls, component, verbosity, is_stdout):
        if not cls._instance:
            return

        msg = cls._instance._max_stdout_log_verbosity_terminal.make_message(value=verbosity.value)

        if is_stdout:
            cls._instance._stdout_log_verbosity_terminals[component].try_publish(msg)
        else:
            cls._instance._yogi_log_verbosity_terminals[component].try_publish(msg)


class Anomaly:
    def __init__(self, message: str, is_error: bool):
        self._message = message
        self._is_error = is_error
        self._expiration_time = None

    def destroy(self):
        self.clear()

    def __del__(self):
        self.clear()

    @property
    def is_error(self) -> bool:
        return self._is_error

    @property
    def message(self) -> str:
        return self._message

    @property
    def expiration_time(self) -> Optional[datetime.datetime]:
        return self._expiration_time

    def set(self, time: Optional[datetime.timedelta] = None, *, days: int = 0, hours: int = 0,
            minutes: int = 0, seconds: int = 0, milliseconds: int = 0) -> None:
        if time is None and any(x is not 0 for x in [days, hours, minutes, seconds, milliseconds]):
            time = datetime.timedelta(days=days, hours=hours, minutes=minutes, seconds=seconds,
                                      milliseconds=milliseconds)

        if time is not None:
            self._expiration_time = datetime.datetime.now() + time

        ProcessInterface._set_anomaly(self)

    def clear(self) -> None:
        ProcessInterface._clear_anomaly(self)


class Error(Anomaly):
    def __init__(self, message: str):
        Anomaly.__init__(self, message, True)


class Warning(Anomaly):
    def __init__(self, message: str):
        Anomaly.__init__(self, message, False)


class OperationalCondition:
    def __init__(self, name: str):
        self._name = name
        self._is_set = False
        ProcessInterface._add_operational_condition(self)

    def destroy(self) -> None:
        ProcessInterface._remove_operational_condition(self)

    def __del__(self):
        try:
            self.destroy()
        except Failure:
            pass

    @property
    def name(self) -> str:
        return self._name

    @property
    def is_met(self) -> bool:
        return self._is_set

    def _set(self):
        if not self._is_set:
            self._is_set = True
            ProcessInterface._notify_operational_condition_change(self)

            from .logging import Logger
            Logger.yogi_logger.log_debug("Operational Condition '{}' set".format(self.name))

    def _clear(self):
        if self._is_set:
            self._is_set = False
            ProcessInterface._notify_operational_condition_change(self)

            from .logging import Logger
            Logger.yogi_logger.log_debug("Operational Condition '{}' cleared".format(self.name))


class ManualOperationalCondition(OperationalCondition):
    def set(self) -> None:
        self._set()

    def clear(self) -> None:
        self._clear()


class Dependency(OperationalCondition):
    def __init__(self, name: str, bindings_and_terminals: List[Union[BinderMixin, SubscribableMixin]]):
        OperationalCondition.__init__(self, name)
        self._lock = threading.Lock()
        self._non_ready_observers = 0
        self._observers = []
        self._oc_lock = threading.Lock()

        from .observers import BindingObserver, SubscriptionObserver

        try:
            for obj in bindings_and_terminals:
                if isinstance(obj, BinderMixin):
                    observer = BindingObserver(obj)
                elif isinstance(obj, SubscribableMixin):
                    observer = SubscriptionObserver(obj)
                else:
                    raise Exception('Dependency only accepts Binders and Subscribables. Class {} is neither.'
                                    .format(type(obj)))

                self._observers.append(observer)
                observer.add(lambda state: self._on_state_changed(observer, state))
                observer.start()
        except:
            self.destroy()
            raise

    def destroy(self) -> None:
        for observer in self._observers:
            observer.destroy()
        self._observers = []
        super(Dependency, self).destroy()

    def _on_state_changed(self, observer, state):
        nro = 0
        with self._lock:
            if state in [BindingState.RELEASED, SubscriptionState.UNSUBSCRIBED]:
                self._non_ready_observers += 1
            else:
                if observer._callbacks_called_since_start:
                    self._non_ready_observers -= 1

            self._oc_lock.acquire()
            nro = self._non_ready_observers

        try:
            if nro:
                self._clear()
            else:
                self._set()
        finally:
            self._oc_lock.release()


class ProcessDependency(Dependency):
    def __init__(self, process_path: Union[str, Path],
                 bindings_and_terminals: List[Union[BinderMixin, SubscribableMixin]]):

        if isinstance(process_path, str):
            process_path = Path(process_path)

        Dependency.__init__(self, 'Terminals for process {} available'.format(process_path), bindings_and_terminals)
        self._poc = ManualOperationalCondition('Process {} operational'.format(process_path))
        self._operational_terminal = CachedConsumerTerminal(process_path / 'Process/Operational', yogi_00000001_pb2)

        from .observers import MessageObserver
        self._operational_observer = MessageObserver(self._operational_terminal)
        self._operational_observer.add(lambda msg, cached: self._update_poc(msg))

        try:
            msg = self._operational_terminal.get_cached_message()
            self._update_poc(msg)
        except Failure:
            pass

        self._operational_observer.start()

    def destroy(self) -> None:
        self._operational_observer.destroy()
        self._operational_terminal.destroy()
        self._poc.destroy()
        super(ProcessDependency, self).destroy()

    def _update_poc(self, msg):
        if msg.value:
            self._poc.set()
        else:
            self._poc.clear()
