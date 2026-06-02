"""Phy descriptor module.

Each descriptor bundles the metadata needed to load a network phy: its
name, dylib basename, init/loop symbol names, and an optional configure
hook that runs after the dylib is preloaded but before the init symbol
is called.
"""
import ctypes
from dataclasses import dataclass
from typing import Any, Callable


@dataclass(frozen=True)
class Phy:
    name: str
    dylib_basename: str
    init_symbol: str
    loop_symbol: str
    configure: Callable[[Any, dict], None] | None = None


def _configure_ws(handle: Any, kwargs: dict) -> None:
    broker = kwargs.pop("broker", None)
    if kwargs:
        raise TypeError(
            f"load_phy(ws_network): unexpected kwargs {sorted(kwargs)}"
        )
    if broker is None:
        return
    if "\x00" in broker:
        raise ValueError("broker url must not contain null bytes")
    try:
        encoded = broker.encode("ascii")
    except UnicodeEncodeError as e:
        raise ValueError(f"broker url must be ASCII: {e}") from None
    if len(encoded) >= 128:
        raise ValueError("broker url must be < 128 bytes")
    fn = handle.Ws_SetBroker
    fn.argtypes = [ctypes.c_char_p]
    fn.restype = None
    fn(encoded)


ws_network = Phy(
    name="ws_network",
    dylib_basename="libws_network",
    init_symbol="Ws_Init",
    loop_symbol="Ws_Loop",
    configure=_configure_ws,
)
