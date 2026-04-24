"""Worker entrypoint used by test_network_blinker for its two Python
processes (led + blinker). Must be importable at the module level because
`multiprocessing.get_context('spawn')` re-imports it in each child.
"""
import time
import traceback

import luos_engine as luos


def _led(broker_url, q, toggle_count):
    received = []

    def on_msg(msg):
        if msg.cmd < luos.FIRST_USER_CMD:
            return
        if msg.cmd != luos.Cmd.IO_STATE:
            return
        received.append(int(msg.data[0]))
        q.put(("state", int(msg.data[0])))
        if len(received) >= toggle_count:
            q.put(("done", "led"))

    luos.load_phy(luos.phy.ws_network, broker=broker_url)
    luos.create_service(
        type=luos.Type.STATE, alias="led", on_message=on_msg,
    )
    luos.start()
    q.put(("ready", "led"))
    # Wait until we've received enough toggles or hit the deadline.
    deadline = time.monotonic() + 60.0
    while len(received) < toggle_count and time.monotonic() < deadline:
        time.sleep(0.05)
    luos.stop()


def _blinker(broker_url, q, toggle_count):
    luos.load_phy(luos.phy.ws_network, broker=broker_url)
    svc = luos.create_service(type=luos.Type.STATE, alias="blinker")
    luos.start()
    q.put(("ready", "blinker"))
    peer = svc.find_peer(alias="led", timeout=30.0)
    q.put(("peer", peer.id))
    state = 0
    for _ in range(toggle_count):
        state ^= 1
        svc.send(
            cmd=luos.Cmd.IO_STATE,
            target=peer.id,
            data=bytes([state]),
        )
        time.sleep(0.02)
    q.put(("done", "blinker"))
    # Keep alive briefly so led finishes receiving.
    time.sleep(2.0)
    luos.stop()


def run(role, broker_url, q, toggle_count=10):
    try:
        if role == "led":
            _led(broker_url, q, toggle_count)
        elif role == "blinker":
            _blinker(broker_url, q, toggle_count)
        else:
            raise ValueError(f"unknown role {role!r}")
    except Exception:
        q.put(("error", role, traceback.format_exc()))
