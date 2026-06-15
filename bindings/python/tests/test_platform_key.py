import pytest
from luos_engine import _platform


def test_dylib_ext_matches_current_platform():
    import sys
    ext = _platform.dylib_ext()
    if sys.platform == "darwin":
        assert ext == ".dylib"
    elif sys.platform.startswith("linux"):
        assert ext == ".so"
    elif sys.platform.startswith("win"):
        assert ext == ".dll"


def test_platform_key_shape():
    key = _platform.platform_key()
    os_part, _, arch_part = key.partition("-")
    assert os_part in {"macos", "linux", "windows"}
    assert arch_part in {"x86_64", "arm64"}


def test_platform_key_normalizes_arch(monkeypatch):
    monkeypatch.setattr(_platform.platform, "machine", lambda: "AMD64")
    monkeypatch.setattr(_platform.sys, "platform", "win32")
    assert _platform.platform_key() == "windows-x86_64"


def test_platform_key_rejects_unknown(monkeypatch):
    monkeypatch.setattr(_platform.platform, "machine", lambda: "sparc")
    monkeypatch.setattr(_platform.sys, "platform", "linux")
    with pytest.raises(RuntimeError):
        _platform.platform_key()
