import os
from pathlib import Path
import pytest


def test_discover_via_walk_up(monkeypatch):
    monkeypatch.delenv("LUOS_ENGINE_LIB", raising=False)
    monkeypatch.delenv("LUOS_ENGINE_LIB_DIR", raising=False)
    from luos_engine._ffi import resolve_lib_path
    p = resolve_lib_path()
    assert p.is_file()
    assert p.name.startswith("libluos_engine.")


def test_env_var_override(tmp_path, monkeypatch):
    fake = tmp_path / "libluos_engine.dylib"
    fake.write_bytes(b"")
    monkeypatch.setenv("LUOS_ENGINE_LIB", str(fake))
    from luos_engine._ffi import resolve_lib_path
    assert resolve_lib_path() == fake


def test_missing_raises(monkeypatch, tmp_path):
    monkeypatch.setenv("LUOS_ENGINE_LIB", str(tmp_path / "nope.dylib"))
    from luos_engine._ffi import resolve_lib_path, LuosEngineNotFoundError
    with pytest.raises(LuosEngineNotFoundError):
        resolve_lib_path()
