import traceback
import sys
from typing import Callable, Optional

from ._ffi import load_dylib
load_dylib()
from ._luos_cffi import ffi, lib
from ._header import pack_header
from ._message import Message
from . import _registry


class LuosError(RuntimeError):
    pass


class SendError(LuosError):
    pass


_MAX_DATA = 128


class Service:
    """A local Luos service. Owns a cffi callback trampoline pinned in
    the registry so C always has a valid pointer while the Service is live."""

    def __init__(self, on_message: Optional[Callable[[Message], None]] = None,
                 *, type: int, alias: str,
                 revision: tuple[int, int, int] = (0, 0, 0)):
        # Luos_CreateService requires the engine to be initialized.
        from ._engine import init
        init()  # idempotent

        self._handler = on_message
        self._alias = alias
        self._type = int(type)
        self._revision = revision
        self._trampoline = ffi.callback(
            "void(service_t*, const msg_t*)", self._dispatch
        )
        rev = ffi.new("revision_t *", {
            "major": revision[0], "minor": revision[1], "build": revision[2]
        })
        self._handle = lib.Luos_CreateService(
            self._trampoline, self._type, alias.encode("ascii"), rev[0]
        )
        _registry.register(int(ffi.cast("uintptr_t", self._handle)), self)

    @property
    def id(self) -> int:
        return lib.service_id(self._handle)

    @property
    def type(self) -> int:
        return lib.service_type(self._handle)

    @property
    def alias(self) -> str:
        return self._alias

    def on_message(self, fn: Callable[[Message], None]) -> Callable:
        """Decorator or setter for the service handler."""
        self._handler = fn
        return fn

    def _dispatch(self, service_ptr, msg_ptr) -> None:
        if self._handler is None:
            return
        try:
            self._handler(Message(self, msg_ptr))
        except Exception:
            # NEVER let an exception unwind through the cffi boundary.
            traceback.print_exc(file=sys.stderr)

    def send(self, *, cmd: int, target: int,
             data: bytes | bytearray | memoryview | None = None,
             target_mode: int = 0) -> None:
        payload = b"" if data is None else bytes(data)
        if len(payload) > _MAX_DATA:
            raise ValueError(
                f"data is {len(payload)} bytes; MAX_DATA_MSG_SIZE is {_MAX_DATA}"
            )
        header = pack_header(
            config=0,
            target=target,
            target_mode=target_mode,
            source=0,           # engine fills this with our id when sending
            cmd=cmd,
            size=len(payload),
        )
        msg = ffi.new("msg_t *")
        ffi.memmove(msg, header, 7)
        if payload:
            ffi.memmove(ffi.cast("char *", msg) + 7, payload, len(payload))
        rc = lib.Luos_SendMsg(self._handle, msg)
        if rc != lib.SUCCEED:
            raise SendError(
                f"Luos_SendMsg(cmd={cmd}, target={target}, size={len(payload)}) "
                f"returned {rc}"
            )

    def detect(self) -> None:
        lib.Luos_Detect(self._handle)


def create_service(*, type: int, alias: str,
                   revision: tuple[int, int, int] = (0, 0, 0),
                   on_message: Optional[Callable[[Message], None]] = None
                   ) -> Service:
    return Service(on_message, type=type, alias=alias, revision=revision)
