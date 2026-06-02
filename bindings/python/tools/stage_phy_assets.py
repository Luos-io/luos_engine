"""Copy built phy dylibs into an output dir, renamed by platform key, and
write a per-platform manifest of sha256 checksums. Run per-OS in CI; each
run writes its own manifest-<key>.json so matrix jobs never clobber each
other's manifest.

Usage: python stage_phy_assets.py <build_dir> <out_dir>
"""
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from luos_engine._platform import dylib_ext, platform_key  # noqa: E402


def main(build_dir: str, out_dir: str) -> None:
    build = Path(build_dir)
    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    key = platform_key()
    ext = dylib_ext()

    manifest = {}
    for src in build.glob(f"lib*_network{ext}"):
        basename = src.name[: -len(ext)]  # libws_network
        asset = f"{basename}-{key}{ext}"
        data = src.read_bytes()
        (out / asset).write_bytes(data)
        manifest[asset] = hashlib.sha256(data).hexdigest()
        print(f"staged {asset}")

    (out / f"manifest-{key}.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True)
    )


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
