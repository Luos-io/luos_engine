"""Luos ws_network-aware broker for tests.

Binds 127.0.0.1:0, prints `LISTEN 127.0.0.1:<port>` on the first line
of stdout (so the pytest fixture can capture the port).

Protocol (single-byte control frames defined in ws_network.c):
  PING (0x00) — sender wants to detect peers.
      Broker replies OK (0x02) to the sender if there are undetected peers,
      NOK (0x03) otherwise, and relays the PING to one undetected peer.
  END  (0x01) — branch done; marks this client as topology-detected and
      relays the END to the master (the last PING sender).
  OK   (0x02) / NOK (0x03) — broker → client only; never relay.
  Multi-byte frames — normal Luos messages; relay to all other clients.
      A START_DETECTION broadcast (Luos cmd byte == 3, size == 0) resets
      the per-session topology-detected set.

Topology tracking rationale: in a WS star topology every connected client
is always reachable, so a naive relay would return OK to every PING even
for already-assigned nodes.  We track which clients have sent END (meaning
their topology branch is closed) and exclude them from PING forwarding,
letting the master see NOK when all branches are exhausted.
"""
import asyncio
import sys
import websockets

PING = bytes([0])
END  = bytes([1])
OK   = bytes([2])
NOK  = bytes([3])

_START_DETECTION_CMD = 3   # reserved_luos_cmd_t value


def _is_start_detection(msg: bytes) -> bool:
    """Return True if this Luos message is a START_DETECTION broadcast."""
    # header is 7 bytes packed: [config:4|target:12][target_mode:4|source:12][cmd][size_lo][size_hi]
    return len(msg) == 7 and msg[4] == _START_DETECTION_CMD and msg[5] == 0 and msg[6] == 0


async def _handle(websocket, clients, detected):
    clients.add(websocket)
    try:
        async for message in websocket:
            if not isinstance(message, (bytes, bytearray)):
                message = message.encode()
            if len(message) == 1 and message[0] == 0:  # PING
                # Only relay to peers that haven't yet sent END for this detection
                available = [p for p in clients if p is not websocket and p not in detected]
                if available:
                    await websocket.send(OK)
                    for peer in available:
                        try:
                            await peer.send(PING)
                        except websockets.ConnectionClosed:
                            pass
                else:
                    await websocket.send(NOK)
            elif len(message) == 1 and message[0] == 1:  # END
                # This client has finished its branch — mark as detected.
                detected.add(websocket)
                # Relay END to all other clients (master needs to receive it).
                for peer in [p for p in clients if p is not websocket]:
                    try:
                        await peer.send(END)
                    except websockets.ConnectionClosed:
                        pass
            elif len(message) == 1 and message[0] in (2, 3):  # OK / NOK
                # Broker-generated; never relay.
                pass
            else:
                # Normal Luos message — relay to all other clients.
                if _is_start_detection(message):
                    # New detection round: reset topology state.
                    detected.clear()
                dead = []
                for peer in clients:
                    if peer is websocket:
                        continue
                    try:
                        await peer.send(message)
                    except websockets.ConnectionClosed:
                        dead.append(peer)
                for d in dead:
                    clients.discard(d)
    finally:
        clients.discard(websocket)
        detected.discard(websocket)


async def _main():
    clients: set = set()
    detected: set = set()   # clients that have sent END in the current detection round

    async def handler(ws):
        await _handle(ws, clients, detected)

    async with websockets.serve(handler, "127.0.0.1", 0) as server:
        port = server.sockets[0].getsockname()[1]
        sys.stdout.write(f"LISTEN 127.0.0.1:{port}\n")
        sys.stdout.flush()
        await asyncio.Future()   # run forever


if __name__ == "__main__":
    try:
        asyncio.run(_main())
    except KeyboardInterrupt:
        pass
