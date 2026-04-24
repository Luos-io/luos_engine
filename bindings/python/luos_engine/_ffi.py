import ctypes
import os
import threading
from pathlib import Path

_LIB_NAMES = ("libluos_engine.dylib", "libluos_engine.so", "libluos_engine.dll")

# Phy dylibs that must be pre-loaded so the cffi extension's unresolved
# Ws_* symbols resolve at dlopen on macOS (flat namespace, eager binding).
# Listed by basename; probed against platform extensions at load time.
_PHY_DYLIB_BASENAMES: tuple[str, ...] = ("libws_network",)
_PHY_DYLIB_EXTENSIONS: tuple[str, ...] = (".dylib", ".so")


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
_PHY_HANDLES: dict[str, ctypes.CDLL] = {}


def load_dylib():
    """Pre-load libluos_engine and any co-located phy dylibs with
    RTLD_GLOBAL. The phy preload is required because cffi's extension
    declares phy symbols unconditionally in its cdef, and macOS resolves
    them eagerly at dlopen — deferring the preload to luos.load_phy would
    fail at import time. Phy activation (Ws_Init etc.) still runs only
    when the user calls luos.load_phy."""
    global _LIB_HANDLE
    if _LIB_HANDLE is None:
        with _LIB_LOCK:
            if _LIB_HANDLE is None:
                path = resolve_lib_path()
                _LIB_HANDLE = ctypes.CDLL(str(path), mode=ctypes.RTLD_GLOBAL)
                lib_dir = path.parent
                for basename in _PHY_DYLIB_BASENAMES:
                    for ext in _PHY_DYLIB_EXTENSIONS:
                        phy_path = lib_dir / (basename + ext)
                        if phy_path.is_file():
                            _PHY_HANDLES[basename] = ctypes.CDLL(
                                str(phy_path), mode=ctypes.RTLD_GLOBAL
                            )
                            break
    return _LIB_HANDLE


def get_phy_handle(basename: str) -> ctypes.CDLL:
    """Return the cached CDLL for a preloaded phy dylib. Raises
    LuosEngineNotFoundError if the phy was not co-located with the
    engine dylib at import time (i.e., the phy dylib didn't exist
    or couldn't be loaded)."""
    handle = _PHY_HANDLES.get(basename)
    if handle is None:
        raise LuosEngineNotFoundError(
            f"{basename}.{{dylib,so}} was not preloaded. Build it with "
            "`~/.platformio/penv/bin/pio run -e native_lib` and ensure "
            "it lives beside libluos_engine."
        )
    return handle
