"""Luos_Subscribe / Luos_Unsubscribe: a message sent to a topic reaches the
subscribed services."""
import threading

import pytest
import luos_engine as luos


def test_a_subscriber_receives_the_topic_and_no_longer_after_unsubscribing():
    got = []
    event = threading.Event()

    def on_msg(msg):
        if msg.cmd >= luos.FIRST_USER_CMD:
            got.append(msg.bytes())
            event.set()

    sub = luos.create_service(type=4, alias="sub", on_message=on_msg)
    pub = luos.create_service(type=4, alias="pub")
    sub.subscribe(3)
    luos.start()
    pub.find_peer(alias="sub", timeout=5.0)
    pub.send(cmd=44, target=3, target_mode=luos.TargetMode.TOPIC, data=b"t3")
    assert event.wait(timeout=2.0)
    assert got == [b"t3"]

    event.clear()
    sub.unsubscribe(3)
    pub.send(cmd=44, target=3, target_mode=luos.TargetMode.TOPIC, data=b"t3")
    assert not event.wait(timeout=0.3)
    luos.stop()
    assert got == [b"t3"]


def test_subscribing_twice_or_unsubscribing_a_stranger_is_an_error():
    svc = luos.create_service(type=4, alias="s")
    svc.subscribe(5)
    with pytest.raises(luos.LuosError):
        svc.subscribe(5)
    with pytest.raises(luos.LuosError):
        svc.unsubscribe(6)
