"""Minimal binary-frame relay for Luos ws_network tests.

Binds 127.0.0.1:0, prints `LISTEN 127.0.0.1:<port>` on the first line
of stdout (so the pytest fixture can capture the port), then relays
every received binary frame to every other connected client. No protocol
awareness — pure relay.
"""
import asyncio
import sys
import websockets


async def _relay(websocket, clients):
    clients.add(websocket)
    try:
        async for message in websocket:
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


async def _main():
    clients: set = set()

    async def handler(ws):
        await _relay(ws, clients)

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
