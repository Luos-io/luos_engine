import threading
from dataclasses import dataclass
from typing import Any, Callable

# Pin live Service instances so their cffi trampolines aren't GC'd
# while the C engine holds pointers to them. Keyed by the C
# service_t* pointer (int) returned from Luos_CreateService.
SERVICES: dict[int, "Service"] = {}
LOCK = threading.Lock()

# Set when luos.stop() is called; the loop thread checks it.
SHUTDOWN = threading.Event()

# True between start() and stop(); protects against double-start.
RUNNING = threading.Event()


@dataclass
class LoadedPhy:
    descriptor: Any
    handle: Any
    loop: Callable[[], None]


# Phys loaded via luos.load_phy. Snapshotted at start() and iterated
# by the loop thread. Cleared by stop() so Python-side refs drop, but
# the dylib stays in-process (no unload path — see design spec §Risks).
PHYS: list[LoadedPhy] = []


def register(service_ptr: int, service) -> None:
    with LOCK:
        SERVICES[service_ptr] = service


def clear() -> None:
    with LOCK:
        SERVICES.clear()
        PHYS.clear()
