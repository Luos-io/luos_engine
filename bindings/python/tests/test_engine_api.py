"""The engine-level calls: version, tx_complete, packages."""
import time

import pytest
import luos_engine as luos


def test_the_engine_reports_a_version():
    major, minor, build = luos.engine_version()
    assert all(isinstance(v, int) for v in (major, minor, build))


def test_the_engine_calls_are_callable_on_an_idle_engine():
    luos.create_service(type=4, alias="idle")
    luos.start()
    time.sleep(0.05)
    assert luos.tx_complete() is True
    assert luos.nbr_available_msg() == 0
    luos.reset_statistic()
    luos.set_irq_state(True)
    luos.stop()


def test_a_package_inits_once_and_loops_on_the_engine_thread():
    inits, loops = [], []
    luos.add_package(lambda: inits.append(1), lambda: loops.append(1))
    luos.start()
    time.sleep(0.05)
    luos.stop()
    assert inits == [1] and len(loops) > 5
    luos.start()
    time.sleep(0.02)
    luos.stop()
    assert inits == [1]
    luos._engine._PACKAGES.clear()
    luos._engine._INITIALIZED_PACKAGES.clear()


def test_a_package_cannot_be_added_while_running():
    luos.start()
    with pytest.raises(luos.LuosError):
        luos.add_package(lambda: None, lambda: None)
    luos.stop()
