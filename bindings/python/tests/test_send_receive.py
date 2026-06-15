import time
import threading
import luos_engine as luos


def test_two_services_exchange_one_message():
    received = []
    event = threading.Event()

    def on_msg(msg):
        if msg.cmd < luos.FIRST_USER_CMD:
            return  # skip internal detection / routing messages
        received.append((msg.cmd, msg.target, msg.source, msg.bytes()))
        event.set()

    receiver = luos.create_service(type=4, alias="receiver", on_message=on_msg)
    sender = luos.create_service(type=4, alias="sender")
    luos.start()

    peer = sender.find_peer(alias="receiver", timeout=5.0)
    sender.send(cmd=44, target=peer.id, data=b"\x42")

    assert event.wait(timeout=2.0), "receiver did not fire"
    assert len(received) == 1
    cmd, target, source, data = received[0]
    assert cmd == 44
    assert target == peer.id
    assert source == sender.id
    assert data == b"\x42"
    luos.stop()


def test_ten_messages_roundtrip():
    received = []

    def on_msg(m):
        if m.cmd < luos.FIRST_USER_CMD:
            return  # skip internal detection / routing messages
        received.append(m.bytes())

    receiver = luos.create_service(type=4, alias="rx", on_message=on_msg)
    sender = luos.create_service(type=4, alias="tx")
    luos.start()
    peer = sender.find_peer(alias="rx", timeout=5.0)
    for i in range(10):
        sender.send(cmd=44, target=peer.id, data=bytes([i]))
        # The engine message allocator ring has MAX_MSG_NB = 2 * MAX_LOCAL_SERVICE_NUMBER
        # slots (default: 10).  Burst-sending without giving the loop a chance to
        # drain will overflow it.  Sleeping > one loop tick (0.5 ms) is enough.
        time.sleep(0.001)

    deadline = time.monotonic() + 2.0
    while len(received) < 10 and time.monotonic() < deadline:
        time.sleep(0.01)
    luos.stop()

    assert received == [bytes([i]) for i in range(10)]
