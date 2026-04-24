import os
import subprocess
import sys
import time
from pathlib import Path

import pytest
import luos_engine as luos


@pytest.fixture(autouse=True)
def _luos_clean_state():
    """Ensure each test starts and ends with the engine stopped and
    services/phys cleared. The C engine is a process singleton."""
    try:
        luos.stop()
    except Exception:
        pass
    yield
    try:
        luos.stop()
    except Exception:
        pass


@pytest.fixture(scope="session")
def broker():
    """Session-scoped WS broker. Yields the ws:// URL; tears the broker
    down after the last test that uses it."""
    script = Path(__file__).parent / "_broker.py"
    proc = subprocess.Popen(
        [sys.executable, "-u", str(script)],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        env={**os.environ, "PYTHONUNBUFFERED": "1"},
    )
    try:
        deadline = time.monotonic() + 5.0
        url = None
        while time.monotonic() < deadline:
            line = proc.stdout.readline().decode(errors="replace")
            if not line:
                time.sleep(0.05)
                continue
            if line.startswith("LISTEN "):
                hostport = line.strip().split(" ", 1)[1]
                url = f"ws://{hostport}"
                break
        if url is None:
            proc.terminate()
            raise RuntimeError("broker did not print LISTEN line within 5s")
        yield url
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=2.0)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait(timeout=1.0)
