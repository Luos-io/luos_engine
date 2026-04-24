"""Public API for the Luos engine Python binding."""

from ._ffi import load_dylib, LuosEngineNotFoundError
load_dylib()

from ._luos_cffi import ffi, lib  # low-level access
from ._engine import init, start, stop, run, is_detected, load_phy
from . import phy
from ._service import Service, create_service, LuosError, SendError
from ._message import Message
from ._routing import RemoteService, PeerNotFound, routing_table, find_services, wait_for_peer
from ._enums import Cmd, Type, TargetMode

# Module-level C-style aliases (for porting ergonomics):
IO_STATE = Cmd.IO_STATE
STATE_TYPE = Type.STATE
BROADCAST = TargetMode.BROADCAST
SERVICEID = TargetMode.SERVICEID

# First user-definable command value. Luos reserves cmd 0-42 for internal
# protocol messages (detection, routing, bootloader). Handlers should
# usually skip cmd < FIRST_USER_CMD.
FIRST_USER_CMD = 43

__all__ = [
    "ffi", "lib",
    "init", "start", "stop", "run", "is_detected",
    "load_phy", "phy",
    "Service", "create_service", "Message",
    "RemoteService", "PeerNotFound",
    "routing_table", "find_services", "wait_for_peer",
    "LuosError", "SendError", "LuosEngineNotFoundError",
    "Cmd", "Type", "TargetMode",
    "IO_STATE", "STATE_TYPE", "BROADCAST", "SERVICEID",
    "FIRST_USER_CMD",
]
