import sys
from pathlib import Path
from cffi import FFI

ffibuilder = FFI()

# Minimal cdef — will expand as tasks land.
ffibuilder.cdef("""
float cmult(int int_param, float float_param);
""")

# Discover repo root and engine include dirs at build time.
_BUILD_DIR = Path(__file__).resolve().parent.parent  # bindings/python/
_REPO_ROOT = _BUILD_DIR.parent.parent

_INCLUDE_DIRS = [
    str(_REPO_ROOT / "engine" / "core" / "inc"),
    str(_REPO_ROOT / "engine"),
    str(_REPO_ROOT / "engine" / "OD"),
    str(_REPO_ROOT / "engine" / "IO" / "inc"),
]

_extra_link_args = []
if sys.platform == "darwin":
    _extra_link_args = ["-undefined", "dynamic_lookup"]
elif sys.platform.startswith("linux"):
    _extra_link_args = ["-Wl,--unresolved-symbols=ignore-in-object-files"]

ffibuilder.set_source(
    "luos_engine._luos_cffi",
    """
    #include "luos_engine.h"
    """,
    include_dirs=_INCLUDE_DIRS,
    extra_link_args=_extra_link_args,
)

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
