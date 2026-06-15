import os
import socket
import subprocess
import sys
import time

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


def _pick_free_port() -> int:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(("127.0.0.1", 0))
    port = s.getsockname()[1]
    s.close()
    return port


@pytest.fixture(scope="session")
def broker():
    """Session-scoped WS broker spawned via Pyluos's ws_broker tool —
    the canonical Luos WebSocket broker. Yields the ws:// URL; tears
    the broker down after the last test that uses it."""
    port = _pick_free_port()
    proc = subprocess.Popen(
        [sys.executable, "-u", "-m", "pyluos.tools.ws_broker",
         "--ip", "127.0.0.1", "--port", str(port)],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        env={**os.environ, "PYTHONUNBUFFERED": "1"},
    )
    try:
        deadline = time.monotonic() + 5.0
        ready = False
        while time.monotonic() < deadline:
            line = proc.stdout.readline().decode(errors="replace")
            if not line:
                time.sleep(0.05)
                continue
            if "opened on" in line:
                ready = True
                break
        if not ready:
            proc.terminate()
            raise RuntimeError("pyluos ws_broker did not start within 5s")
        yield f"ws://127.0.0.1:{port}"
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=2.0)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait(timeout=1.0)
