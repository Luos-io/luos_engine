from luos_engine._ffi import load_dylib
load_dylib()
from luos_engine._luos_cffi import ffi, lib
from luos_engine._header import pack_header


def test_python_pack_matches_c_bitfield_layout():
    raw = pack_header(
        config=0, target=0xABC, target_mode=2,
        source=0x123, cmd=44, size=5,
    )
    buf = ffi.from_buffer("uint8_t[7]", raw)
    assert lib.peek_target(buf) == 0xABC
    assert lib.peek_source(buf) == 0x123
    assert lib.peek_cmd(buf) == 44
    assert lib.peek_size(buf) == 5
    assert lib.peek_target_mode(buf) == 2
    assert lib.peek_config(buf) == 0


def test_every_field_position_individually():
    # Flip one field at a time; all other C peeks must read 0.
    cases = [
        ("target", 0xFFF, lib.peek_target),
        ("source", 0xFFF, lib.peek_source),
        ("cmd", 0xFF, lib.peek_cmd),
        ("size", 0xFFFF, lib.peek_size),
        ("target_mode", 0xF, lib.peek_target_mode),
        ("config", 0xF, lib.peek_config),
    ]
    for field, value, peek in cases:
        kwargs = dict(config=0, target=0, target_mode=0,
                      source=0, cmd=0, size=0)
        kwargs[field] = value
        raw = pack_header(**kwargs)
        buf = ffi.from_buffer("uint8_t[7]", raw)
        assert peek(buf) == value, f"{field} roundtrip failed"
