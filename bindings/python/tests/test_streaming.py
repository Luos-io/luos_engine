"""StreamingChannel over the engine's ring buffer, and Luos_SendStreaming /
Luos_ReceiveStreaming between two services."""
import threading

import pytest
import luos_engine as luos


def test_the_channel_keeps_whole_samples_in_order_and_wraps():
    ch = luos.StreamingChannel(capacity=4, sample_size=2)
    assert ch.available() == 0
    ch.put(b"\x01\x00\x02\x00\x03\x00")
    assert ch.available() == 3
    assert ch.get(2) == b"\x01\x00\x02\x00"
    ch.put(b"\x04\x00\x05\x00")               # wraps around the 4-sample ring
    assert ch.available() == 3
    assert ch.get(3) == b"\x03\x00\x04\x00\x05\x00"
    assert ch.get(1) == b""
    ch.reset()
    assert ch.available() == 0


def test_the_channel_refuses_partial_samples_and_overflow():
    ch = luos.StreamingChannel(capacity=3, sample_size=4)
    with pytest.raises(ValueError):
        ch.put(b"\x01\x02\x03")
    ch.put(bytes(8))                 # two of the three slots: the ring's most
    with pytest.raises(ValueError):
        ch.put(bytes(4))


def test_samples_stream_from_one_service_to_another():
    # 200 one-byte samples: two messages, 128 then 72. receive_streaming
    # says False on the first (more to come) and True on the last.
    rx_channel = luos.StreamingChannel(capacity=256, sample_size=1)
    ends = []
    got = threading.Event()

    def on_msg(msg):
        if msg.cmd >= luos.FIRST_USER_CMD:
            ends.append(msg.service.receive_streaming(msg, rx_channel))
            if ends[-1]:
                got.set()

    rx = luos.create_service(type=4, alias="rx", on_message=on_msg)
    tx = luos.create_service(type=4, alias="tx")
    luos.start()
    peer = tx.find_peer(alias="rx", timeout=5.0)
    tx_channel = luos.StreamingChannel(capacity=256, sample_size=1)
    samples = bytes(range(200))
    tx_channel.put(samples)
    tx.send_streaming(cmd=44, target=peer.id, stream=tx_channel)
    assert got.wait(timeout=2.0)
    luos.stop()
    assert ends == [False, True]
    assert tx_channel.available() == 0
    assert rx_channel.get(200) == samples
