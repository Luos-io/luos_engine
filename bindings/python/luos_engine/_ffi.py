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


def local_lib_dirs():
    """Yield directories that may hold locally-built luos binaries
    (engine + phys), in priority order. Shared by resolve_lib_path and
    the phy loader so both honour the same dev/build locations."""
    env_dir = os.environ.get("LUOS_ENGINE_LIB_DIR")
    if env_dir:
        yield Path(env_dir)
    here = Path(__file__).resolve().parent
    for ancestor in (here, *here.parents):
        candidate = ancestor / ".pio" / "build" / "native_lib"
        if candidate.is_dir():
            yield candidate


def resolve_lib_path() -> Path:
    env_lib = os.environ.get("LUOS_ENGINE_LIB")
    if env_lib:
        p = Path(env_lib)
        if not p.is_file():
            raise LuosEngineNotFoundError(
                f"LUOS_ENGINE_LIB={env_lib} does not exist"
            )
        return p

    for d in local_lib_dirs():
        found = _find_in_dir(d)
        if found:
            return found

    raise LuosEngineNotFoundError(
        "libluos_engine not found. Build it with "
        "`~/.platformio/penv/bin/pio run -e native_lib` or set "
        "LUOS_ENGINE_LIB / LUOS_ENGINE_LIB_DIR."
    )


_LIB_HANDLE = None
_LIB_LOCK = threading.Lock()
_PHY_HANDLES: dict[str, ctypes.CDLL] = {}


def load_dylib():
    """Load libluos_engine with RTLD_GLOBAL so the cffi extension's engine
    symbols resolve. Phys are loaded lazily by get_phy_handle, not here."""
    global _LIB_HANDLE
    if _LIB_HANDLE is None:
        with _LIB_LOCK:
            if _LIB_HANDLE is None:
                path = resolve_lib_path()
                _LIB_HANDLE = ctypes.CDLL(str(path), mode=ctypes.RTLD_GLOBAL)
    return _LIB_HANDLE


def get_phy_handle(basename: str) -> ctypes.CDLL:
    """Return a cached RTLD_GLOBAL CDLL for the phy `basename`, resolving
    and loading (and downloading if necessary) on first use."""
    handle = _PHY_HANDLES.get(basename)
    if handle is None:
        from ._phy_loader import ensure_phy_dylib
        path = ensure_phy_dylib(basename)
        handle = ctypes.CDLL(str(path), mode=ctypes.RTLD_GLOBAL)
        _PHY_HANDLES[basename] = handle
    return handle
