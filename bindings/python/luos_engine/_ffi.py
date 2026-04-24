import ctypes
import os
import threading
from pathlib import Path

_LIB_NAMES = ("libluos_engine.dylib", "libluos_engine.so", "libluos_engine.dll")


class LuosEngineNotFoundError(RuntimeError):
    pass


def _find_in_dir(d: Path):
    for name in _LIB_NAMES:
        p = d / name
        if p.is_file():
            return p
    return None


def resolve_lib_path() -> Path:
    env_lib = os.environ.get("LUOS_ENGINE_LIB")
    if env_lib:
        p = Path(env_lib)
        if not p.is_file():
            raise LuosEngineNotFoundError(
                f"LUOS_ENGINE_LIB={env_lib} does not exist"
            )
        return p

    env_dir = os.environ.get("LUOS_ENGINE_LIB_DIR")
    if env_dir:
        found = _find_in_dir(Path(env_dir))
        if found:
            return found
        raise LuosEngineNotFoundError(
            f"No libluos_engine.* in LUOS_ENGINE_LIB_DIR={env_dir}"
        )

    here = Path(__file__).resolve().parent
    for ancestor in (here, *here.parents):
        candidate = ancestor / ".pio" / "build" / "native_lib"
        if candidate.is_dir():
            found = _find_in_dir(candidate)
            if found:
                return found

    raise LuosEngineNotFoundError(
        "libluos_engine not found. Build it with "
        "`~/.platformio/penv/bin/pio run -e native_lib` or set "
        "LUOS_ENGINE_LIB / LUOS_ENGINE_LIB_DIR."
    )


_LIB_HANDLE = None
_LIB_LOCK = threading.Lock()


def load_dylib():
    """Pre-load libluos_engine with RTLD_GLOBAL so the cffi extension's
    unresolved symbols bind to its exports."""
    global _LIB_HANDLE
    if _LIB_HANDLE is None:
        with _LIB_LOCK:
            if _LIB_HANDLE is None:
                path = resolve_lib_path()
                _LIB_HANDLE = ctypes.CDLL(str(path), mode=ctypes.RTLD_GLOBAL)
    return _LIB_HANDLE
