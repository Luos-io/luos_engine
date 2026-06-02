import atexit
import signal
import threading
import time

from ._ffi import load_dylib, get_phy_handle
load_dylib()
from ._luos_cffi import lib
from . import _registry
from ._registry import LoadedPhy
from ._service import LuosError

_TICK_S = 0.0005   # 0.5 ms — low latency without 100% CPU
_INIT_DONE = threading.Event()
_LOOP_THREAD: threading.Thread | None = None
_LIFECYCLE_LOCK = threading.Lock()

# Phys whose init symbol has been invoked in this process. Persists
# across stop()/start() cycles because each phy's C init (e.g. Ws_Init)
# allocates process-wide resources that have no teardown path — Mongoose
# spawns a pthread, etc. Calling init twice would leak. See design spec
# §Risks for the load-once-per-process rationale.
_INITIALIZED_PHYS: set[str] = set()


def init() -> None:
    if _INIT_DONE.is_set():
        return
    lib.Luos_Init()
    _INIT_DONE.set()


def _loop(phy_ticks: tuple) -> None:
    while not _registry.SHUTDOWN.is_set():
        lib.Luos_Loop()
        for tick in phy_ticks:
            tick()
        time.sleep(_TICK_S)


def start() -> None:
    global _LOOP_THREAD
    with _LIFECYCLE_LOCK:
        if _registry.RUNNING.is_set():
            return
        init()
        _registry.SHUTDOWN.clear()
        phy_ticks = tuple(p.loop for p in _registry.PHYS)
        _LOOP_THREAD = threading.Thread(
            target=_loop, name="luos-loop", daemon=True, args=(phy_ticks,),
        )
        _LOOP_THREAD.start()
        _registry.RUNNING.set()


def stop() -> None:
    global _LOOP_THREAD
    with _LIFECYCLE_LOCK:
        if _registry.RUNNING.is_set():
            _registry.SHUTDOWN.set()
            if _LOOP_THREAD is not None:
                _LOOP_THREAD.join(timeout=2.0)
                _LOOP_THREAD = None
            lib.Luos_ServicesClear()
            _registry.RUNNING.clear()
            _INIT_DONE.clear()
        _registry.clear()


def run() -> None:
    start()
    try:
        signal.pause()
    except KeyboardInterrupt:
        pass
    finally:
        stop()


def is_detected() -> bool:
    return bool(lib.Luos_IsDetected())


def load_phy(descriptor, **kwargs) -> None:
    """Load and initialise a network phy. Must be called before start().

    The phy dylib is resolved and loaded lazily on first use via
    get_phy_handle. This function validates kwargs via
    descriptor.configure, then calls the phy's init symbol (once per
    process — subsequent calls skip init but still register the phy
    for the loop thread to tick).

    Luos_Init (via init()) must run before any phy init symbol so that
    LuosIO_Init → Phy_Init reserves slot 0 for the internal luos_phy
    before external phys claim it via Phy_Create.
    """
    with _LIFECYCLE_LOCK:
        if _registry.RUNNING.is_set():
            raise LuosError("load_phy must be called before start()")
        for existing in _registry.PHYS:
            if existing.descriptor.name == descriptor.name:
                return  # idempotent within a stop/start cycle
        handle = get_phy_handle(descriptor.dylib_basename)
        if descriptor.configure is not None:
            descriptor.configure(handle, dict(kwargs))
        elif kwargs:
            raise TypeError(
                f"load_phy({descriptor.name}): descriptor takes no kwargs, "
                f"got {sorted(kwargs)}"
            )
        if descriptor.name not in _INITIALIZED_PHYS:
            # Ensure Luos_Init has run so that Phy_Init places the internal
            # luos_phy at slot 0 before the external phy claims a slot.
            init()
            getattr(handle, descriptor.init_symbol)()
            _INITIALIZED_PHYS.add(descriptor.name)
        loop_callable = getattr(handle, descriptor.loop_symbol)
        _registry.PHYS.append(LoadedPhy(
            descriptor=descriptor,
            handle=handle,
            loop=loop_callable,
        ))


atexit.register(stop)
