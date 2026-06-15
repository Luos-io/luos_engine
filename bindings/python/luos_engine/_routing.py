import time
from dataclasses import dataclass

from ._ffi import load_dylib
load_dylib()
from ._luos_cffi import ffi, lib


@dataclass(frozen=True)
class RemoteService:
    id: int
    type: int
    alias: str


class PeerNotFound(RuntimeError):
    pass


_RTB_SERVICE_MODE = 1


def routing_table() -> list[RemoteService]:
    """All SERVICE-mode entries in the routing table."""
    result = ffi.new("search_result_t *")
    lib.RTFilter_Reset(result)
    return _collect(result)


def find_services(*, alias: str | None = None,
                  type: int | None = None) -> list[RemoteService]:
    result = ffi.new("search_result_t *")
    lib.RTFilter_Reset(result)
    if alias is not None:
        lib.RTFilter_Alias(result, alias.encode("ascii"))
    if type is not None:
        lib.RTFilter_Type(result, int(type))
    return _collect(result)


def _collect(result) -> list[RemoteService]:
    out = []
    for i in range(result.result_nbr):
        entry = result.result_table[i]
        if lib.rtb_mode(entry) != _RTB_SERVICE_MODE:
            continue
        alias_buf = ffi.new("char[16]")
        lib.rtb_service_alias(entry, alias_buf)
        alias = ffi.string(alias_buf, 16).decode("ascii", errors="replace")
        out.append(RemoteService(
            id=lib.rtb_service_id(entry),
            type=lib.rtb_service_type(entry),
            alias=alias,
        ))
    return out


def wait_for_peer(*, alias: str | None = None, type: int | None = None,
                  timeout: float = 5.0) -> RemoteService:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        hits = find_services(alias=alias, type=type)
        if len(hits) == 1:
            return hits[0]
        if len(hits) > 1:
            raise ValueError(
                f"{len(hits)} peers match alias={alias!r} type={type!r}; "
                "narrow the filter"
            )
        time.sleep(0.02)
    raise PeerNotFound(f"no peer matching alias={alias!r} type={type!r} "
                       f"within {timeout}s")
