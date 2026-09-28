"""streaming_channel_t: a ring buffer of fixed-size samples that
Service.send_streaming drains into messages and Service.receive_streaming
fills from them."""
from ._ffi import load_dylib
load_dylib()
from ._luos_cffi import ffi, lib


class StreamingChannel:
    """Streaming_CreateChannel over a buffer this object owns: `capacity`
    samples of `sample_size` bytes each, of which capacity - 1 can wait at
    once. Sizes everywhere are in samples. Between two services, a burst
    that spans several messages only counts right with 1-byte samples (see
    Service.send_streaming)."""

    def __init__(self, *, capacity: int, sample_size: int):
        if capacity <= 0 or not 0 < sample_size <= 255:
            raise ValueError("capacity must be > 0 and sample_size in 1..255")
        self.sample_size = int(sample_size)
        self.capacity = int(capacity)
        self._ring = ffi.new("uint8_t[]", self.capacity * self.sample_size)
        self._channel = ffi.new("streaming_channel_t *")
        self._channel[0] = lib.Streaming_CreateChannel(
            self._ring, self.capacity, self.sample_size
        )

    def available(self) -> int:
        """Streaming_GetAvailableSampleNB: samples waiting to be read."""
        return lib.Streaming_GetAvailableSampleNB(self._channel)

    def put(self, samples: bytes | bytearray | memoryview) -> int:
        """Streaming_PutSample: append whole samples; returns what is
        available afterwards. The ring tells full from empty by one free
        slot, so it holds capacity - 1 samples at most; more is refused
        here, before the engine asserts on it."""
        raw = bytes(samples)
        n, rem = divmod(len(raw), self.sample_size)
        if rem or n == 0:
            raise ValueError(
                f"{len(raw)} bytes is not a whole number of {self.sample_size}-byte samples"
            )
        if self.available() + n >= self.capacity:
            raise ValueError(
                f"{n} samples do not fit: {self.available()} waiting of {self.capacity}"
            )
        return lib.Streaming_PutSample(self._channel, ffi.from_buffer(raw), n)

    def get(self, n: int) -> bytes:
        """Streaming_GetSample: the oldest `n` samples, or b"" when fewer
        are available."""
        n = int(n)
        if n <= 0:
            raise ValueError("n must be > 0")
        if self.available() < n:
            return b""
        out = ffi.new("uint8_t[]", n * self.sample_size)
        lib.Streaming_GetSample(self._channel, out, n)
        return bytes(ffi.buffer(out, n * self.sample_size))

    def reset(self) -> None:
        lib.Streaming_ResetChannel(self._channel)
