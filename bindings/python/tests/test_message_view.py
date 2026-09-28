from luos_engine._ffi import load_dylib
load_dylib()
from luos_engine._luos_cffi import ffi, lib
from luos_engine._header import pack_header
from luos_engine._message import Message


def test_message_view_reads_header_and_data():
    raw_header = pack_header(config=0, target=5, target_mode=0,
                             source=3, cmd=44, size=3)
    msg = ffi.new("msg_t *")
    ffi.memmove(msg, raw_header, 7)
    # Write 3 data bytes after the header.
    ffi.memmove(ffi.cast("char *", msg) + 7, b"\xAA\xBB\xCC", 3)

    view = Message(service=None, msg_ptr=msg)
    assert view.cmd == 44
    assert view.target == 5
    assert view.source == 3
    assert view.size == 3
    assert bytes(view.data) == b"\xAA\xBB\xCC"
    assert view.bytes() == b"\xAA\xBB\xCC"


def test_message_data_respects_size_field():
    raw_header = pack_header(config=0, target=1, target_mode=0,
                             source=2, cmd=0, size=0)
    msg = ffi.new("msg_t *")
    ffi.memmove(msg, raw_header, 7)
    view = Message(service=None, msg_ptr=msg)
    assert view.size == 0
    assert len(view.data) == 0
