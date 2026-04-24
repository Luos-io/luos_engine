"""IntEnum wrappers over the generated int constants."""
from enum import IntEnum
from . import _enum_values as _raw


def _collect(cls) -> dict[str, int]:
    return {
        attr: getattr(cls, attr)
        for attr in dir(cls)
        if not attr.startswith("_") and isinstance(getattr(cls, attr), int)
    }


# IntEnum's functional API; duplicate values become aliases automatically.
Type = IntEnum("Type", _collect(_raw._TypeRaw))
Cmd = IntEnum("Cmd", _collect(_raw._CmdRaw))
TargetMode = IntEnum("TargetMode", _collect(_raw._TargetModeRaw))
