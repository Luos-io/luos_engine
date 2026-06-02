import pytest
from luos_engine import phy
from luos_engine._ffi import get_phy_handle


@pytest.fixture
def ws_handle():
    return get_phy_handle("libws_network")


def test_ws_network_descriptor_fields():
    d = phy.ws_network
    assert d.name == "ws_network"
    assert d.dylib_basename == "libws_network"
    assert d.init_symbol == "Ws_Init"
    assert d.loop_symbol == "Ws_Loop"
    assert d.configure is not None


def test_configure_ws_rejects_unknown_kwargs(ws_handle):
    with pytest.raises(TypeError) as exc:
        phy._configure_ws(ws_handle, {"brokre": "ws://x"})
    assert "brokre" in str(exc.value)


def test_configure_ws_accepts_no_kwargs(ws_handle):
    phy._configure_ws(ws_handle, {})


def test_configure_ws_rejects_overlong_url(ws_handle):
    with pytest.raises(ValueError):
        phy._configure_ws(ws_handle, {"broker": "ws://" + "x" * 128})
