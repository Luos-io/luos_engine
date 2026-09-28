"""Acceptance: two Python processes exchange >=10 IO_STATE toggles over
a local ws_network broker. Assertions are handler-level via a
multiprocessing.Queue — no stdout parsing."""
import multiprocessing
import time

from . import _ws_worker


def _drain_states(q, expected_count, deadline):
    states = []
    errors = []
    while time.monotonic() < deadline:
        try:
            msg = q.get(timeout=0.5)
        except Exception:
            continue
        tag = msg[0]
        if tag == "state":
            states.append(msg[1])
            if len(states) >= expected_count:
                return states, errors
        elif tag == "error":
            errors.append(msg)
    return states, errors


def test_blinker_over_ws(broker):
    ctx = multiprocessing.get_context("spawn")
    q = ctx.Queue()
    led     = ctx.Process(target=_ws_worker.run, args=("led",     broker, q, 10))
    blinker = ctx.Process(target=_ws_worker.run, args=("blinker", broker, q, 10))
    led.start()
    blinker.start()
    try:
        states, errors = _drain_states(q, expected_count=10, deadline=time.monotonic() + 60.0)
        assert not errors, f"worker errors: {errors}"
        assert len(states) >= 10, f"only got {len(states)} states"
        # Alternating 0/1 sequence.
        assert all(a != b for a, b in zip(states, states[1:])), states
    finally:
        for p in (blinker, led):
            p.join(timeout=10.0)
            if p.is_alive():
                p.terminate()
                p.join(timeout=2.0)
            if p.is_alive():
                p.kill()
                p.join(timeout=1.0)
