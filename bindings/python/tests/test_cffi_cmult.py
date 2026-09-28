from luos_engine._ffi import load_dylib
load_dylib()
from luos_engine._luos_cffi import lib


def test_cffi_cmult():
    assert abs(lib.cmult(3, 0.5) - 1.5) < 1e-6
