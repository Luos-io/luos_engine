from ._ffi import load_dylib
load_dylib()
from ._luos_cffi import ffi, lib
from ._header import unpack_header

_MAX_DATA = 128


class Message:
    """Zero-copy view over a Luos msg_t*. Field access unpacks the header.
    `data` is a memoryview valid only while the C buffer is alive (typically
    only inside the handler that received it; a Message returned by a
    polling read owns its copy)."""

    __slots__ = ("service", "_msg_ptr", "_header_cache")

    def __init__(self, service, msg_ptr):
        self.service = service
        self._msg_ptr = msg_ptr
        self._header_cache = None

    def _header(self) -> dict:
        if self._header_cache is None:
            raw = bytes(ffi.buffer(self._msg_ptr, 7))
            self._header_cache = unpack_header(raw)
        return self._header_cache

    @property
    def cmd(self) -> int: return self._header()["cmd"]
    @property
    def target(self) -> int: return self._header()["target"]
    @property
    def target_mode(self) -> int: return self._header()["target_mode"]
    @property
    def source(self) -> int: return self._header()["source"]

    @property
    def size(self) -> int:
        """The header's size field. For a frame of a send_data transfer it
        is what is still to come, not this frame's data length."""
        return self._header()["size"]

    @property
    def data(self) -> memoryview:
        # A frame never carries more than MAX_DATA_MSG_SIZE bytes, whatever
        # its size field says (a big-data frame counts the rest of the
        # transfer there).
        size = min(self.size, _MAX_DATA)
        if size == 0:
            return memoryview(b"")
        return memoryview(ffi.buffer(
            ffi.cast("char *", self._msg_ptr) + 7, size
        ))

    def bytes(self) -> bytes:
        return bytes(self.data)

    @property
    def timestamp(self) -> float | None:
        """The date a timestamped message was stamped with, in seconds of
        this node's clock (luos.timestamp()), or None when it carries none."""
        if not lib.Luos_IsMsgTimstamped(self._msg_ptr):
            return None
        return lib.Luos_GetMsgTimestamp(self._msg_ptr).raw

    def __repr__(self) -> str:
        return (f"Message(cmd={self.cmd}, target={self.target}, "
                f"source={self.source}, size={self.size})")
