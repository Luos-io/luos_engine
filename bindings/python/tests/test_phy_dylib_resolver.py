import ctypes

import pytest

from luos_engine import _ffi


def test_get_phy_handle_returns_cdll_for_preloaded_ws():
    handle = _ffi.get_phy_handle("libws_network")
    assert isinstance(handle, ctypes.CDLL)


def test_get_phy_handle_raises_on_unknown_phy():
    with pytest.raises(_ffi.LuosEngineNotFoundError):
        _ffi.get_phy_handle("libthis_phy_does_not_exist")
