"""Two Python Luos services in one process: `led` (renders ASCII art)
and `blinker` (sends IO_STATE every second)."""
import os
import signal
import sys
import threading
import time

import luos_engine as luos

LED_ON, LED_OFF = 1, 0
_FIRST_USER_CMD = 43  # Luos reserves 0-42 for internal protocol

LED_ON_ART = """                                   \\ | /
         ___________________________.-.________________
        /                        - |***| -            /;
       /    ________________     - |***| -           //
      /    /               /;     [` - ')           //
     /    /     MCU       //       `---'           //
    /    /               //         | |           //
   /    /______________ //                       //
  /     '---------------'                       //
 /                                             //
/_____________________________________________//
`---------------------------------------------'
"""

LED_OFF_ART = """
         ___________________________.-.________________
        /                          |   |              /;
       /    ________________       |   |             //
      /    /               /;     [` - ')           //
     /    /     MCU       //       `---'           //
    /    /               //         | |           //
   /    /______________ //                       //
  /     '---------------'                       //
 /                                             //
/_____________________________________________//
`---------------------------------------------'
"""


def _clear():
    os.system("clear" if os.name != "nt" else "cls")


def on_led_message(msg):
    if msg.cmd < _FIRST_USER_CMD:
        return  # skip internal detection / routing messages
    if msg.cmd != luos.Cmd.IO_STATE:
        return
    state = msg.data[0]
    _clear()
    print("LED service running.\n", flush=True)
    print(LED_ON_ART if state == LED_ON else LED_OFF_ART, flush=True)


def main():
    led = luos.create_service(
        type=luos.Type.STATE, alias="led", on_message=on_led_message
    )
    blinker = luos.create_service(type=luos.Type.STATE, alias="blinker")
    luos.start()

    peer = blinker.find_peer(alias="led", timeout=5.0)
    print(f"blinker: found led at id={peer.id}", flush=True)

    stop_event = threading.Event()
    def handler(*_): stop_event.set()
    signal.signal(signal.SIGINT, handler)

    state = LED_OFF
    try:
        while not stop_event.is_set():
            state ^= 1
            blinker.send(cmd=luos.Cmd.IO_STATE, target=peer.id,
                         data=bytes([state]))
            time.sleep(1.0)
    finally:
        luos.stop()


if __name__ == "__main__":
    main()
