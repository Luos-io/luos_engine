"""Luos_SendData / Luos_ReceiveData through the binding: a payload bigger
than one frame, split by the engine and reassembled on the other side."""
import threading
import time

import pytest
import luos_engine as luos


def _pair(on_msg):
    receiver = luos.create_service(type=4, alias="rx", on_message=on_msg)
    sender = luos.create_service(type=4, alias="tx")
    luos.start()
    peer = sender.find_peer(alias="rx", timeout=5.0)
    return sender, receiver, peer


@pytest.mark.parametrize("size", [129, 512, 1000])
def test_a_big_payload_arrives_whole(size):
    payload = bytes(i % 251 for i in range(size))
    buffer = bytearray(size)
    frames = []
    done = threading.Event()

    def on_msg(msg):
        if msg.cmd != 44:
            return
        frames.append(msg.size)
        n = msg.service.receive_data(msg, buffer)
        if n:
            done.set()

    sender, _receiver, peer = _pair(on_msg)
    sender.send_data(cmd=44, target=peer.id, data=payload)
    assert done.wait(timeout=2.0), "the transfer never completed"
    luos.stop()
    assert bytes(buffer) == payload
    # The size field of each frame is what was still to come, as the C
    # engine counts it: the whole size first, one frame's worth on the last.
    expected = []
    left = size
    while left > 0:
        expected.append(left)
        left -= 128
    assert frames == expected


def test_receive_data_returns_zero_until_the_last_frame():
    payload = bytes(range(256))
    buffer = bytearray(256)
    returns = []
    done = threading.Event()

    def on_msg(msg):
        if msg.cmd != 44:
            return
        returns.append(msg.service.receive_data(msg, buffer))
        if returns[-1]:
            done.set()

    sender, _receiver, peer = _pair(on_msg)
    sender.send_data(cmd=44, target=peer.id, data=payload)
    assert done.wait(timeout=2.0)
    luos.stop()
    assert returns == [0, 256]


def test_a_frame_of_a_transfer_never_reads_past_one_frame_of_data():
    seen = []
    done = threading.Event()

    def on_msg(msg):
        if msg.cmd != 44:
            return
        seen.append((msg.size, len(msg.data)))
        if msg.size <= 128:
            done.set()

    sender, _receiver, peer = _pair(on_msg)
    sender.send_data(cmd=44, target=peer.id, data=bytes(300))
    assert done.wait(timeout=2.0)
    luos.stop()
    assert seen == [(300, 128), (172, 128), (44, 44)]


def test_a_buffer_too_small_for_the_transfer_is_refused_before_the_engine_sees_it():
    caught = []
    done = threading.Event()

    def on_msg(msg):
        if msg.cmd != 44:
            return
        try:
            msg.service.receive_data(msg, bytearray(10))
        except ValueError as err:
            caught.append(str(err))
        done.set()

    sender, _receiver, peer = _pair(on_msg)
    sender.send_data(cmd=44, target=peer.id, data=bytes(200))
    assert done.wait(timeout=2.0)
    luos.stop()
    assert caught and "200" in caught[0]


def test_send_data_refuses_an_empty_payload():
    sender = luos.create_service(type=4, alias="tx")
    with pytest.raises(ValueError):
        sender.send_data(cmd=44, target=1, data=b"")


def test_send_still_refuses_more_than_one_frame():
    sender = luos.create_service(type=4, alias="tx")
    with pytest.raises(ValueError, match="send_data"):
        sender.send(cmd=44, target=1, data=bytes(129))
