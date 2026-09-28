import hashlib
import pytest

from luos_engine import _phy_loader
from luos_engine._ffi import LuosEngineNotFoundError


@pytest.fixture(autouse=True)
def _isolate_cache(tmp_path, monkeypatch):
    """Force the cache into tmp and disable any local-build discovery so
    tests exercise the cache/download paths deterministically. Pin a
    non-dev engine version so the +dev download guard doesn't fire (the
    dev-build guard is covered by its own test)."""
    monkeypatch.setenv("LUOS_ENGINE_CACHE_DIR", str(tmp_path / "cache"))
    monkeypatch.delenv("LUOS_ENGINE_LIB_DIR", raising=False)
    monkeypatch.delenv("LUOS_ENGINE_NO_DOWNLOAD", raising=False)
    monkeypatch.setattr(_phy_loader, "ENGINE_VERSION", "9.9.9")
    # No local build dirs.
    monkeypatch.setattr(_phy_loader, "local_lib_dirs", lambda: iter(()))


def test_resolves_local_build_first(tmp_path, monkeypatch):
    """A phy present in a local build dir wins over cache/download."""
    local = tmp_path / "build"
    local.mkdir()
    ext = _phy_loader.dylib_ext()
    target = local / ("libws_network" + ext)
    target.write_bytes(b"local")
    monkeypatch.setattr(_phy_loader, "local_lib_dirs", lambda: iter([local]))

    def _boom(*a, **k):
        raise AssertionError("must not download when local build exists")

    monkeypatch.setattr(_phy_loader, "_download", _boom)
    assert _phy_loader.ensure_phy_dylib("libws_network") == target


def test_downloads_and_caches_with_checksum(monkeypatch):
    ext = _phy_loader.dylib_ext()
    asset = f"libws_network-{_phy_loader.platform_key()}{ext}"
    payload = b"fake-dylib-bytes"
    digest = hashlib.sha256(payload).hexdigest()

    monkeypatch.setattr(
        _phy_loader, "_fetch_json", lambda url: {asset: digest}
    )
    monkeypatch.setattr(_phy_loader, "_fetch_bytes", lambda url: payload)

    path = _phy_loader.ensure_phy_dylib("libws_network")
    assert path.is_file()
    assert path.read_bytes() == payload
    # Second call hits the cache; downloader must not run again.
    monkeypatch.setattr(
        _phy_loader, "_fetch_bytes",
        lambda url: (_ for _ in ()).throw(AssertionError("re-downloaded")),
    )
    assert _phy_loader.ensure_phy_dylib("libws_network") == path


def test_checksum_mismatch_raises_and_caches_nothing(monkeypatch):
    ext = _phy_loader.dylib_ext()
    asset = f"libws_network-{_phy_loader.platform_key()}{ext}"
    monkeypatch.setattr(
        _phy_loader, "_fetch_json", lambda url: {asset: "deadbeef"}
    )
    monkeypatch.setattr(_phy_loader, "_fetch_bytes", lambda url: b"bytes")
    with pytest.raises(_phy_loader.PhyDownloadError):
        _phy_loader.ensure_phy_dylib("libws_network")
    assert not (_phy_loader.cache_dir() / ("libws_network" + ext)).exists()


def test_asset_not_in_manifest_raises(monkeypatch):
    monkeypatch.setattr(_phy_loader, "_fetch_json", lambda url: {})
    monkeypatch.setattr(_phy_loader, "_fetch_bytes", lambda url: b"x")
    with pytest.raises(_phy_loader.PhyDownloadError):
        _phy_loader.ensure_phy_dylib("libws_network")


def test_no_download_env_raises_when_missing(monkeypatch):
    monkeypatch.setenv("LUOS_ENGINE_NO_DOWNLOAD", "1")
    with pytest.raises(LuosEngineNotFoundError):
        _phy_loader.ensure_phy_dylib("libws_network")


def test_dev_build_refuses_download(monkeypatch):
    """A +dev build has no matching release; fail with a clear message
    instead of 404-ing on a non-existent release asset."""
    monkeypatch.setattr(_phy_loader, "ENGINE_VERSION", "0.0.0+dev")

    def _boom(*a, **k):
        raise AssertionError("must not attempt a download for a dev build")

    monkeypatch.setattr(_phy_loader, "_fetch_json", _boom)
    with pytest.raises(_phy_loader.PhyDownloadError) as exc:
        _phy_loader.ensure_phy_dylib("libws_network")
    assert "dev build" in str(exc.value)
