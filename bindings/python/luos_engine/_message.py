from ._ffi import load_dylib
load_dylib()
from ._luos_cffi import ffi
from ._header import unpack_header


class Message:
    """Zero-copy view over a Luos msg_t*. Field access unpacks the header.
    `data` is a memoryview valid only while the C buffer is alive (typically
    only inside the handler that received it)."""

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
    def size(self) -> int: return self._header()["size"]

    @property
    def data(self) -> memoryview:
        size = self.size
        if size == 0:
            return memoryview(b"")
        return memoryview(ffi.buffer(
            ffi.cast("char *", self._msg_ptr) + 7, size
        ))

    def bytes(self) -> bytes:
        return bytes(self.data)

    def __repr__(self) -> str:
        return (f"Message(cmd={self.cmd}, target={self.target}, "
                f"source={self.source}, size={self.size})")
