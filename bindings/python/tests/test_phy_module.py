import pytest
from luos_engine import phy
from luos_engine._luos_cffi import lib


def test_ws_network_descriptor_fields():
    d = phy.ws_network
    assert d.name == "ws_network"
    assert d.dylib_basename == "libws_network"
    assert d.init_symbol == "Ws_Init"
    assert d.loop_symbol == "Ws_Loop"
    assert d.configure is not None


def test_configure_ws_rejects_unknown_kwargs():
    with pytest.raises(TypeError) as exc:
        phy._configure_ws(lib, {"brokre": "ws://x"})
    assert "brokre" in str(exc.value)


def test_configure_ws_accepts_no_kwargs():
    # No broker kwarg = no-op, must not touch the lib.
    phy._configure_ws(lib, {})


def test_configure_ws_rejects_overlong_url():
    with pytest.raises(ValueError):
        phy._configure_ws(lib, {"broker": "ws://" + "x" * 128})
