import ctypes
import pytest

from luos_engine import _ffi


def test_get_phy_handle_loads_ws_from_local_build():
    """With a local .pio build present, get_phy_handle resolves and
    dlopens the ws phy lazily."""
    handle = _ffi.get_phy_handle("libws_network")
    assert isinstance(handle, ctypes.CDLL)


def test_get_phy_handle_unknown_phy_offline_raises(monkeypatch):
    """An unknown phy with downloads disabled raises (subclass of
    LuosEngineNotFoundError)."""
    monkeypatch.setenv("LUOS_ENGINE_NO_DOWNLOAD", "1")
    with pytest.raises(_ffi.LuosEngineNotFoundError):
        _ffi.get_phy_handle("libthis_phy_does_not_exist")
