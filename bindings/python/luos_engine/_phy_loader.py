"""Resolve a phy dylib: prefer a local build, then the download cache,
then fetch the matching asset from the engine's GitHub Release."""
import hashlib
import json
import os
import urllib.request
from pathlib import Path

import platformdirs

from ._ffi import LuosEngineNotFoundError, local_lib_dirs
from ._platform import dylib_ext, platform_key
from ._version import ENGINE_VERSION

GITHUB_REPO = "Luos-io/luos_engine"
_RELEASE_BASE = f"https://github.com/{GITHUB_REPO}/releases/download"


class PhyDownloadError(LuosEngineNotFoundError):
    pass


def cache_dir() -> Path:
    root = os.environ.get("LUOS_ENGINE_CACHE_DIR")
    base = Path(root) if root else Path(platformdirs.user_cache_dir("luos_engine"))
    return base / ENGINE_VERSION / platform_key()


def _fetch_bytes(url: str) -> bytes:
    if not url.startswith("https://"):
        raise PhyDownloadError(f"refusing non-https url: {url}")
    try:
        with urllib.request.urlopen(url) as resp:  # noqa: S310 (https enforced)
            return resp.read()
    except OSError as e:
        raise PhyDownloadError(f"failed to download {url}: {e}") from e


def _fetch_json(url: str) -> dict:
    return json.loads(_fetch_bytes(url).decode("utf-8"))


def _download(asset_name: str, dest: Path) -> Path:
    base = f"{_RELEASE_BASE}/{ENGINE_VERSION}"
    manifest = _fetch_json(f"{base}/manifest-{platform_key()}.json")
    expected = manifest.get(asset_name)
    if expected is None:
        raise PhyDownloadError(
            f"{asset_name} is not published for engine {ENGINE_VERSION}; "
            f"available assets: {sorted(manifest)}"
        )
    data = _fetch_bytes(f"{base}/{asset_name}")
    actual = hashlib.sha256(data).hexdigest()
    if actual != expected:
        raise PhyDownloadError(
            f"checksum mismatch for {asset_name}: "
            f"expected {expected}, got {actual}"
        )
    dest.parent.mkdir(parents=True, exist_ok=True)
    tmp = dest.with_name(dest.name + ".tmp")
    tmp.write_bytes(data)
    try:
        os.replace(tmp, dest)  # atomic
    except OSError:
        tmp.unlink(missing_ok=True)
        raise
    return dest


def ensure_phy_dylib(basename: str) -> Path:
    """Return a filesystem path to `basename` (e.g. 'libws_network') for the
    current platform, fetching and caching it if necessary."""
    ext = dylib_ext()
    filename = basename + ext

    for d in local_lib_dirs():
        candidate = d / filename
        if candidate.is_file():
            return candidate

    cached = cache_dir() / filename
    if cached.is_file():
        return cached

    if os.environ.get("LUOS_ENGINE_NO_DOWNLOAD"):
        raise PhyDownloadError(
            f"{filename} not found locally and downloads are disabled "
            f"(LUOS_ENGINE_NO_DOWNLOAD set). Build it with "
            f"`~/.platformio/penv/bin/pio run -e native_lib` or unset the "
            f"variable to download from the {ENGINE_VERSION} release."
        )

    if ENGINE_VERSION.endswith("+dev"):
        raise PhyDownloadError(
            f"{filename} not found locally and this is a dev build "
            f"(ENGINE_VERSION={ENGINE_VERSION!r}, no matching release). "
            f"Build it with `~/.platformio/penv/bin/pio run -e native_lib`."
        )

    asset_name = f"{basename}-{platform_key()}{ext}"
    return _download(asset_name, cached)
