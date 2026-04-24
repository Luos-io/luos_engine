import time
import luos_engine as luos

_FIRST_USER_CMD = 43


def test_blinker_alternates_at_handler_level():
    states = []

    def on_led(m):
        if m.cmd < _FIRST_USER_CMD:
            return
        states.append(m.data[0])

    led = luos.create_service(
        type=luos.Type.STATE, alias="led",
        on_message=on_led,
    )
    blinker = luos.create_service(type=luos.Type.STATE, alias="blinker")
    luos.start()

    peer = blinker.find_peer(alias="led", timeout=5.0)
    for i in range(12):
        blinker.send(cmd=luos.Cmd.IO_STATE, target=peer.id,
                     data=bytes([i % 2]))
        time.sleep(0.02)  # > loop tick to drain message allocator

    deadline = time.monotonic() + 2.0
    while len(states) < 12 and time.monotonic() < deadline:
        time.sleep(0.01)
    luos.stop()

    assert len(states) >= 10, f"got {len(states)} states"
    assert states[:10] == [0, 1, 0, 1, 0, 1, 0, 1, 0, 1]
