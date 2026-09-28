import ctypes
from luos_engine._ffi import resolve_lib_path


def test_dylib_loads_and_cmult_returns_expected():
    lib = ctypes.CDLL(str(resolve_lib_path()))
    lib.cmult.restype = ctypes.c_float
    lib.cmult.argtypes = [ctypes.c_int, ctypes.c_float]
    result = lib.cmult(3, 0.5)
    assert abs(result - 1.5) < 1e-6
