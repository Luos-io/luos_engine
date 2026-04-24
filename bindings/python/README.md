# luos_engine — Python binding

Python binding for the [Luos engine](https://github.com/Luos-io/luos_engine), designed to feel familiar to C-Luos users.

## Install (development)

```bash
# 1. Build the shared library
~/.platformio/penv/bin/pio run -e native_lib

# 2. Install the binding in a venv
python3 -m venv .venv && source .venv/bin/activate
pip install -e bindings/python
```

## Quick start

```python
import luos_engine as luos

led = luos.create_service(
    type=luos.Type.STATE, alias="led",
    on_message=lambda msg: print("LED state:", msg.data[0]),
)
blinker = luos.create_service(type=luos.Type.STATE, alias="blinker")

luos.start()
peer = blinker.find_peer(alias="led")
blinker.send(cmd=luos.Cmd.IO_STATE, target=peer.id, data=b"\x01")
luos.stop()
```

## Porting from C

| C                                                         | Python                                                                                |
|-----------------------------------------------------------|---------------------------------------------------------------------------------------|
| `Luos_CreateService(cb, STATE_TYPE, "led", rev)`          | `luos.create_service(type=luos.Type.STATE, alias="led", on_message=cb)`               |
| `msg->header.cmd == IO_STATE`                             | `msg.cmd == luos.Cmd.IO_STATE`                                                        |
| `msg->data[0]`                                            | `msg.data[0]` (memoryview, valid only inside the handler)                             |
| `Luos_SendMsg(svc, &m)`                                   | `svc.send(cmd=..., target=..., data=b"...")`                                          |
| `Luos_Detect(svc)` + poll routing table                   | `svc.find_peer(alias="led", timeout=5.0)`                                             |
| `while(1) { Luos_Loop(); }`                               | `luos.start()` / `luos.stop()` (background loop thread)                               |

## Environment variables

- `LUOS_ENGINE_LIB` — absolute path to `libluos_engine.{dylib,so,dll}`.
- `LUOS_ENGINE_LIB_DIR` — directory containing it.
- If neither is set, the package walks up from its install location looking for `.pio/build/native_lib/libluos_engine.*`. Covers the `pip install -e` dev flow.

## Running

```bash
pytest bindings/python/tests/              # 17 tests green
python bindings/python/examples/blinker.py # ASCII LED toggles every second
```

## Protocol notes

- **Reserved commands.** Luos reserves `cmd` values 0–42 for internal protocol messages (detection, routing, bootloader, assert, etc.). User-defined commands start at 43. Handlers that only care about application traffic should ignore messages with `cmd < 43`. See `engine/engine_config.h:LUOS_LAST_RESERVED_CMD`.
- **Message allocator pressure.** The engine's message ring holds `2 * MAX_LOCAL_SERVICE_NUMBER` slots (10 by default). Burst-sending more messages than the loop can drain raises a C assertion. In practice, a short sleep between back-to-back sends (> 1 ms) is enough. The blinker example sends once per second, which is well under the limit.
- **Handlers run on the loop thread.** A slow Python handler (file I/O, network) will block `Luos_Loop()` and starve detection/routing. Offload heavy work to your own thread from inside the handler.

## Limitations (Phase 1)

- In-process only. Network phys (`ws_network`, `serial_network`, `robus_network`) are Phase 2 — they require exposing the network libs as separately loadable dylibs.
- No streaming, big-data, pub/sub, or timestamped-send wrappers yet. The raw C symbols are reachable via `luos.lib.*` for power users.
- Tested on macOS (Darwin/arm64). Linux is expected to work via `-Wl,--unresolved-symbols=ignore-in-object-files` but not yet validated in CI. Windows is out of scope for Phase 1 (`signal.pause()` in `luos.run()` is POSIX-only).

## Design

- Spec: [docs/superpowers/specs/2026-04-23-python-binding-design.md](../../docs/superpowers/specs/2026-04-23-python-binding-design.md)
- Plan: [docs/superpowers/plans/2026-04-23-python-binding.md](../../docs/superpowers/plans/2026-04-23-python-binding.md)
