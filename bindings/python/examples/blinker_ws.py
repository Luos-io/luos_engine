"""Blinker service over ws_network. Run `led_ws.py` in another terminal
(and a broker before both), then `python blinker_ws.py`."""
import os
import time
import luos_engine as luos

BROKER = os.environ.get("LUOS_WS_BROKER", "ws://127.0.0.1:8000")


def main():
    luos.load_phy(luos.phy.ws_network, broker=BROKER)
    svc = luos.create_service(type=luos.Type.STATE, alias="blinker")
    luos.start()
    peer = svc.find_peer(alias="led", timeout=30.0)
    print(f"[blinker] found led at id={peer.id}", flush=True)
    try:
        state = 0
        while True:
            state ^= 1
            svc.send(
                cmd=luos.Cmd.IO_STATE,
                target=peer.id,
                data=bytes([state]),
            )
            time.sleep(1.0)
    except KeyboardInterrupt:
        pass
    finally:
        luos.stop()


if __name__ == "__main__":
    main()
