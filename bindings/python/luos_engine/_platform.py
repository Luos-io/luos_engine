"""Map the running interpreter's platform to the keys used for phy dylib
asset names and cache paths."""
import platform
import sys

_OS_MAP = {"darwin": "macos", "win32": "windows"}
_EXT_MAP = {"macos": ".dylib", "linux": ".so", "windows": ".dll"}
_ARCH_MAP = {
    "x86_64": "x86_64",
    "amd64": "x86_64",
    "arm64": "arm64",
    "aarch64": "arm64",
}


def _os_name() -> str:
    if sys.platform.startswith("linux"):
        return "linux"
    name = _OS_MAP.get(sys.platform)
    if name is None:
        raise RuntimeError(f"unsupported platform: {sys.platform}")
    return name


def dylib_ext() -> str:
    return _EXT_MAP[_os_name()]


def platform_key() -> str:
    os_name = _os_name()
    arch = _ARCH_MAP.get(platform.machine().lower())
    if arch is None:
        raise RuntimeError(
            f"unsupported architecture: {platform.machine()!r}"
        )
    return f"{os_name}-{arch}"
