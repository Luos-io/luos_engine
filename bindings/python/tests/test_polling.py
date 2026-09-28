"""A service created without a callback keeps its messages in the engine
until Luos_ReadMsg / Luos_ReadFromService / Luos_ReadFromCmd copy them out."""
import time

import pytest
import luos_engine as luos


def _wait_read(read, timeout=2.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        msg = read()
        if msg is not None:
            return msg
        time.sleep(0.005)
    return None


def _user(read):
    """Detection traffic is queued for a polling service too: skip it."""
    def user_only():
        while True:
            msg = read()
            if msg is None or msg.cmd >= luos.FIRST_USER_CMD:
                return msg
    return user_only


def test_read_msg_returns_what_was_sent_and_then_nothing():
    poller = luos.create_service(type=4, alias="poller", polling=True)
    sender = luos.create_service(type=4, alias="tx")
    luos.start()
    peer = sender.find_peer(alias="poller", timeout=5.0)
    sender.send(cmd=44, target=peer.id, data=b"\x07\x08")
    msg = _wait_read(_user(poller.read_msg))
    assert msg is not None
    assert (msg.cmd, msg.source, msg.bytes()) == (44, sender.id, b"\x07\x08")
    assert _user(poller.read_msg)() is None
    luos.stop()


def test_read_from_cmd_and_from_service_pick_their_message():
    poller = luos.create_service(type=4, alias="poller", polling=True)
    a = luos.create_service(type=4, alias="a")
    b = luos.create_service(type=4, alias="b")
    luos.start()
    peer = a.find_peer(alias="poller", timeout=5.0)
    a.send(cmd=44, target=peer.id, data=b"a")
    time.sleep(0.002)
    b.send(cmd=45, target=peer.id, data=b"b")
    time.sleep(0.05)
    msg = _wait_read(lambda: poller.read_from_cmd(45))
    assert msg is not None and msg.bytes() == b"b" and msg.source == b.id
    msg = _wait_read(_user(lambda: poller.read_from_service(a.id)))
    assert msg is not None and msg.bytes() == b"a" and msg.cmd == 44
    luos.stop()


def test_a_polling_service_takes_no_handler():
    with pytest.raises(ValueError):
        luos.create_service(type=4, alias="p", polling=True,
                            on_message=lambda m: None)
    poller = luos.create_service(type=4, alias="p", polling=True)
    with pytest.raises(luos.LuosError):
        poller.on_message(lambda m: None)
