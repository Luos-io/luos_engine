from luos_engine._header import pack_header, unpack_header


def test_pack_unpack_roundtrip():
    fields = dict(config=0, target=0xABC, target_mode=2,
                  source=0x123, cmd=44, size=5)
    raw = pack_header(**fields)
    assert len(raw) == 7
    assert unpack_header(raw) == fields


def test_pack_bytes_are_deterministic():
    raw = pack_header(config=0, target=0x001, target_mode=0,
                      source=0x000, cmd=0, size=0)
    assert raw == bytes([0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])


def test_reject_out_of_range():
    import pytest
    with pytest.raises(ValueError):
        pack_header(config=0, target=0x1000, target_mode=0,
                    source=0, cmd=0, size=0)  # target is 12 bits
