"""Luos_SendTimestampMsg / Luos_GetMsgTimestamp: a date rides with the
message and is read back on the same clock."""
import threading

import pytest
import luos_engine as luos


def test_a_timestamped_message_carries_its_date_and_a_plain_one_none():
    seen = []
    done = threading.Event()

    def on_msg(msg):
        if msg.cmd >= luos.FIRST_USER_CMD:
            seen.append((msg.bytes(), msg.timestamp))
            if len(seen) == 2:
                done.set()

    rx = luos.create_service(type=4, alias="rx", on_message=on_msg)
    tx = luos.create_service(type=4, alias="tx")
    luos.start()
    peer = tx.find_peer(alias="rx", timeout=5.0)
    stamp = luos.timestamp() - 0.25          # a quarter second ago
    tx.send_timestamped(cmd=44, target=peer.id, data=b"s", timestamp=stamp)
    tx.send(cmd=45, target=peer.id, data=b"p")
    assert done.wait(timeout=2.0)
    luos.stop()
    (data_s, ts), (data_p, none) = seen
    assert data_s == b"s" and data_p == b"p" and none is None
    # The engine re-dates the stamp on reception (latency-corrected), so it
    # lands within the transit of the message, well under a second.
    assert ts == pytest.approx(stamp, abs=0.5)


def test_the_clock_runs_in_seconds():
    a = luos.timestamp()
    import time
    time.sleep(0.05)
    assert 0.03 < luos.timestamp() - a < 0.5


def test_a_timestamped_payload_leaves_room_for_the_stamp():
    tx = luos.create_service(type=4, alias="tx")
    with pytest.raises(ValueError):
        tx.send_timestamped(cmd=44, target=1, data=bytes(121), timestamp=0.0)
