"""LED service over ws_network. Run a broker first (e.g.
`python bindings/python/tests/_broker.py`), then `python led_ws.py`."""
import os
import luos_engine as luos

BROKER = os.environ.get("LUOS_WS_BROKER", "ws://127.0.0.1:8000")


def on_msg(msg):
    if msg.cmd < luos.FIRST_USER_CMD:
        return
    if msg.cmd == luos.Cmd.IO_STATE:
        state = "ON " if msg.data[0] else "off"
        print(f"[led] {state}", flush=True)


def main():
    luos.load_phy(luos.phy.ws_network, broker=BROKER)
    luos.create_service(
        type=luos.Type.STATE, alias="led", on_message=on_msg,
    )
    luos.run()


if __name__ == "__main__":
    main()
