"""
Manual pack/unpack for the 7-byte Luos message header.

Layout (from engine/core/inc/struct_luos.h, packed bitfields,
little-endian on both Darwin/arm64 and Linux/x86_64):

    bits  0-3   config       (4)
    bits  4-15  target       (12)
    bits  16-19 target_mode  (4)
    bits  20-31 source       (12)
    bits  32-39 cmd          (8)
    bits  40-55 size         (16)
    total 56 bits = 7 bytes
"""


def pack_header(*, config: int, target: int, target_mode: int,
                source: int, cmd: int, size: int) -> bytes:
    _check(config, 4, "config")
    _check(target, 12, "target")
    _check(target_mode, 4, "target_mode")
    _check(source, 12, "source")
    _check(cmd, 8, "cmd")
    _check(size, 16, "size")
    v = (
        (config & 0xF)
        | ((target & 0xFFF) << 4)
        | ((target_mode & 0xF) << 16)
        | ((source & 0xFFF) << 20)
        | ((cmd & 0xFF) << 32)
        | ((size & 0xFFFF) << 40)
    )
    return v.to_bytes(7, "little")


def unpack_header(raw: bytes) -> dict:
    if len(raw) != 7:
        raise ValueError(f"header must be 7 bytes, got {len(raw)}")
    v = int.from_bytes(raw, "little")
    return dict(
        config=v & 0xF,
        target=(v >> 4) & 0xFFF,
        target_mode=(v >> 16) & 0xF,
        source=(v >> 20) & 0xFFF,
        cmd=(v >> 32) & 0xFF,
        size=(v >> 40) & 0xFFFF,
    )


def _check(val: int, bits: int, name: str) -> None:
    if not (0 <= val < (1 << bits)):
        raise ValueError(
            f"{name}={val} does not fit in {bits} bits (0..{(1 << bits) - 1})"
        )
