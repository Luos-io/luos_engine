import atexit
import signal
import threading
import time

from ._ffi import load_dylib
load_dylib()
from ._luos_cffi import lib
from . import _registry

_TICK_S = 0.0005   # 0.5 ms — low latency without 100% CPU
_INIT_DONE = threading.Event()
_LOOP_THREAD: threading.Thread | None = None
_LIFECYCLE_LOCK = threading.Lock()


def init() -> None:
    if _INIT_DONE.is_set():
        return
    lib.Luos_Init()
    _INIT_DONE.set()


def _loop() -> None:
    while not _registry.SHUTDOWN.is_set():
        lib.Luos_Loop()
        time.sleep(_TICK_S)


def start() -> None:
    global _LOOP_THREAD
    with _LIFECYCLE_LOCK:
        if _registry.RUNNING.is_set():
            return
        init()
        _registry.SHUTDOWN.clear()
        _LOOP_THREAD = threading.Thread(
            target=_loop, name="luos-loop", daemon=True
        )
        _LOOP_THREAD.start()
        _registry.RUNNING.set()


def stop() -> None:
    global _LOOP_THREAD
    with _LIFECYCLE_LOCK:
        if not _registry.RUNNING.is_set():
            return
        _registry.SHUTDOWN.set()
        if _LOOP_THREAD is not None:
            _LOOP_THREAD.join(timeout=2.0)
            _LOOP_THREAD = None
        lib.Luos_ServicesClear()
        _registry.clear()
        _registry.RUNNING.clear()
        _INIT_DONE.clear()


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


atexit.register(stop)
