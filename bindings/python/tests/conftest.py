import pytest
import luos_engine as luos


@pytest.fixture(autouse=True)
def _luos_clean_state():
    """Ensure each test starts and ends with the engine stopped and
    services cleared. This matters because the C engine is a process
    singleton."""
    try:
        luos.stop()
    except Exception:
        pass
    yield
    try:
        luos.stop()
    except Exception:
        pass
