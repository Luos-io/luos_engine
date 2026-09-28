import time
import traceback
import sys
from typing import Callable, Optional

from ._ffi import load_dylib
load_dylib()
from ._luos_cffi import ffi, lib
from ._header import pack_header
from ._message import Message
from ._streaming import StreamingChannel
from . import _registry
from . import _routing


class LuosError(RuntimeError):
    pass


class SendError(LuosError):
    pass


_MAX_DATA = 128


class Service:
    """A local Luos service. Owns a cffi callback trampoline pinned in
    the registry so C always has a valid pointer while the Service is live.

    With `polling=True` the service is created without a callback, as a C
    service with a NULL SERVICE_CB: the engine keeps its messages until
    read_msg / read_from_service / read_from_cmd copies them out."""

    def __init__(self, on_message: Optional[Callable[[Message], None]] = None,
                 *, type: int, alias: str,
                 revision: tuple[int, int, int] = (0, 0, 0),
                 polling: bool = False):
        # Luos_CreateService requires the engine to be initialized.
        from ._engine import init
        init()  # idempotent

        if polling and on_message is not None:
            raise ValueError("a polling service has no on_message handler")
        self._handler = on_message
        self._alias = alias
        self._type = int(type)
        self._revision = revision
        self._polling = polling
        self._trampoline = ffi.NULL if polling else ffi.callback(
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
        if self._polling:
            raise LuosError("a polling service has no handler; use read_msg")
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

    # --- sending ---------------------------------------------------------

    @staticmethod
    def _new_msg(*, cmd: int, target: int, target_mode: int,
                 payload: bytes = b""):
        """A msg_t with its header filled and `payload` (at most one frame)
        copied in. The engine fills `source` with our id when sending."""
        header = pack_header(
            config=0,
            target=target,
            target_mode=target_mode,
            source=0,
            cmd=cmd,
            size=len(payload),
        )
        msg = ffi.new("msg_t *")
        ffi.memmove(msg, header, 7)
        if payload:
            ffi.memmove(ffi.cast("char *", msg) + 7, payload, len(payload))
        return msg

    def send(self, *, cmd: int, target: int,
             data: bytes | bytearray | memoryview | None = None,
             target_mode: int = 0) -> None:
        payload = b"" if data is None else bytes(data)
        if len(payload) > _MAX_DATA:
            raise ValueError(
                f"data is {len(payload)} bytes; MAX_DATA_MSG_SIZE is {_MAX_DATA}"
                " -- send_data splits a bigger payload for you"
            )
        msg = self._new_msg(cmd=cmd, target=target, target_mode=target_mode,
                            payload=payload)
        rc = lib.Luos_SendMsg(self._handle, msg)
        if rc != lib.SUCCEED:
            raise SendError(
                f"Luos_SendMsg(cmd={cmd}, target={target}, size={len(payload)}) "
                f"returned {rc}"
            )

    def send_data(self, *, cmd: int, target: int,
                  data: bytes | bytearray | memoryview,
                  target_mode: int = 0) -> None:
        """Luos_SendData: a payload of any size (up to 65535 bytes). The
        engine splits it into MAX_DATA_MSG_SIZE frames whose `size` field
        counts what is still to come, so the receiver reassembles it with
        receive_data. Blocks until every frame is queued; the loop must be
        running (luos.start()) or the engine's 500 ms timeout asserts."""
        payload = bytes(data)
        if not 0 < len(payload) <= 0xFFFF:
            raise ValueError(f"send_data takes 1..65535 bytes, got {len(payload)}")
        msg = self._new_msg(cmd=cmd, target=target, target_mode=target_mode)
        buf = ffi.from_buffer(payload)
        lib.Luos_SendData(self._handle, msg, buf, len(payload))

    def receive_data(self, msg: Message, buffer: bytearray) -> int:
        """Luos_ReceiveData: feed one frame of a send_data transfer into
        `buffer`, which must hold the whole payload (the first frame's
        msg.size). Returns 0 while frames are still to come, the total size
        once the last one landed, or a negative value when a frame went
        missing (the session is then reset)."""
        if len(buffer) < msg.size:
            raise ValueError(
                f"buffer holds {len(buffer)} bytes, the transfer needs {msg.size}"
            )
        return lib.Luos_ReceiveData(self._handle, msg._msg_ptr,
                                    ffi.from_buffer(buffer))

    def send_timestamped(self, *, cmd: int, target: int,
                         data: bytes | bytearray | memoryview | None = None,
                         timestamp: float, target_mode: int = 0) -> None:
        """Luos_SendTimestampMsg: `timestamp` is a luos.timestamp() date in
        seconds; the receiver reads it back as msg.timestamp, corrected for
        the transit. The stamp rides after the data, so a payload is at most
        MAX_DATA_MSG_SIZE - 8 bytes."""
        payload = b"" if data is None else bytes(data)
        if len(payload) > _MAX_DATA - 8:
            raise ValueError(
                f"a timestamped payload is at most {_MAX_DATA - 8} bytes, got {len(payload)}"
            )
        msg = self._new_msg(cmd=cmd, target=target, target_mode=target_mode,
                            payload=payload)
        stamp = ffi.new("time_luos_t *", {"raw": float(timestamp)})
        rc = lib.Luos_SendTimestampMsg(self._handle, msg, stamp[0])
        if rc != lib.SUCCEED:
            raise SendError(
                f"Luos_SendTimestampMsg(cmd={cmd}, target={target}) returned {rc}"
            )

    def send_streaming(self, *, cmd: int, target: int,
                       stream: StreamingChannel, max_size: int | None = None,
                       target_mode: int = 0) -> None:
        """Luos_SendStreaming / Luos_SendStreamingSize: drain the channel's
        available samples into as many messages as needed, `max_size` samples
        at most when given. The engine's size field counts samples left
        while its receiver reads it as bytes, so only 1-byte samples arrive
        whole across a burst of several messages."""
        msg = self._new_msg(cmd=cmd, target=target, target_mode=target_mode)
        if max_size is None:
            lib.Luos_SendStreaming(self._handle, msg, stream._channel)
        else:
            lib.Luos_SendStreamingSize(self._handle, msg, stream._channel,
                                       int(max_size))

    def receive_streaming(self, msg: Message, stream: StreamingChannel) -> bool:
        """Luos_ReceiveStreaming: put a streaming message's samples into the
        channel. True once the last message of the burst landed, False while
        more are to come (what the C returns as SUCCEED / FAILED)."""
        rc = lib.Luos_ReceiveStreaming(self._handle, msg._msg_ptr,
                                       stream._channel)
        return rc == lib.SUCCEED

    # --- polling reception -----------------------------------------------

    def _read(self, fn, *args) -> Message | None:
        msg = ffi.new("msg_t *")
        if fn(self._handle, *args, msg) != lib.SUCCEED:
            return None
        # The copy is ours: the Message keeps the cdata alive.
        return Message(self, msg)

    def read_msg(self) -> Message | None:
        """Luos_ReadMsg: the oldest message waiting for this polling
        service, or None."""
        return self._read(lib.Luos_ReadMsg)

    def read_from_service(self, service_id: int) -> Message | None:
        """Luos_ReadFromService: the oldest waiting message from that
        service id, or None."""
        return self._read(lib.Luos_ReadFromService, int(service_id))

    def read_from_cmd(self, cmd: int) -> Message | None:
        """Luos_ReadFromCmd: the oldest waiting message with that command,
        or None."""
        return self._read(lib.Luos_ReadFromCmd, int(cmd))

    # --- pub/sub, alias, detection -----------------------------------------

    def subscribe(self, topic: int) -> None:
        """Luos_Subscribe: receive what is sent to `topic` with
        TargetMode.TOPIC. Topics are 0..MAX_LOCAL_TOPIC_NUMBER-1 (19)."""
        if lib.Luos_Subscribe(self._handle, int(topic)) != lib.SUCCEED:
            raise LuosError(f"Luos_Subscribe({topic}) failed: already subscribed "
                            "or no topic slot left")

    def unsubscribe(self, topic: int) -> None:
        if lib.Luos_Unsubscribe(self._handle, int(topic)) != lib.SUCCEED:
            raise LuosError(f"Luos_Unsubscribe({topic}) failed: not subscribed")

    def update_alias(self, alias: str) -> None:
        """Luos_UpdateAlias: rename the service (at most 15 ASCII chars)."""
        encoded = alias.encode("ascii")
        if lib.Luos_UpdateAlias(self._handle, encoded, len(encoded)) != lib.SUCCEED:
            raise LuosError(f"Luos_UpdateAlias({alias!r}) failed")
        self._alias = alias

    def detect(self) -> None:
        lib.Luos_Detect(self._handle)

    def find_peer(self, *, alias: str | None = None,
                  type: int | None = None,
                  timeout: float = 5.0):
        """Blocking: retries detection until the peer appears or timeout."""
        deadline = time.monotonic() + timeout
        while True:
            self.detect()
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break
            try:
                return _routing.wait_for_peer(
                    alias=alias, type=type,
                    timeout=min(1.0, remaining),
                )
            except _routing.PeerNotFound:
                if time.monotonic() >= deadline:
                    break
        raise _routing.PeerNotFound(
            f"no peer matching alias={alias!r} type={type!r} within {timeout}s"
        )


def create_service(*, type: int, alias: str,
                   revision: tuple[int, int, int] = (0, 0, 0),
                   on_message: Optional[Callable[[Message], None]] = None,
                   polling: bool = False
                   ) -> Service:
    return Service(on_message, type=type, alias=alias, revision=revision,
                   polling=polling)
