import threading

# Pin live Service instances so their cffi trampolines aren't GC'd
# while the C engine holds pointers to them. Keyed by the C
# service_t* pointer (int) returned from Luos_CreateService.
SERVICES: dict[int, "Service"] = {}
LOCK = threading.Lock()

# Set when luos.stop() is called; the loop thread checks it.
SHUTDOWN = threading.Event()

# True between start() and stop(); protects against double-start.
RUNNING = threading.Event()


def register(service_ptr: int, service) -> None:
    with LOCK:
        SERVICES[service_ptr] = service


def get(service_ptr: int):
    with LOCK:
        return SERVICES.get(service_ptr)


def clear() -> None:
    with LOCK:
        SERVICES.clear()
