import pytest
import luos_engine as luos
from luos_engine import _registry


def test_load_phy_populates_registry_before_start():
    luos.load_phy(luos.phy.ws_network)
    assert any(p.descriptor.name == "ws_network" for p in _registry.PHYS)


def test_load_phy_idempotent_within_cycle():
    luos.load_phy(luos.phy.ws_network)
    luos.load_phy(luos.phy.ws_network)
    names = [p.descriptor.name for p in _registry.PHYS]
    assert names.count("ws_network") == 1


def test_load_phy_rejects_unknown_kwargs():
    with pytest.raises(TypeError):
        luos.load_phy(luos.phy.ws_network, brokre="ws://x")


def test_load_phy_after_start_is_rejected():
    luos.init()
    luos.start()
    try:
        with pytest.raises(luos.LuosError):
            luos.load_phy(luos.phy.ws_network)
    finally:
        luos.stop()
