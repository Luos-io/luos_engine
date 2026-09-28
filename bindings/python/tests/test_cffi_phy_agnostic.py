import os
import subprocess
import sys
from pathlib import Path


def test_cffi_imports_without_phy_present(tmp_path):
    """The cffi extension must import even when no phy dylib is loadable.
    We point the loader at a dir containing ONLY the core engine (copied),
    so no libws_network.* is preloadable, and import in a clean subprocess."""
    # Find the locally-built core engine.
    repo = Path(__file__).resolve().parents[3]
    src = repo / ".pio" / "build" / "native_lib"
    core = next(src.glob("libluos_engine.*"))
    isolated = tmp_path / "libonly"
    isolated.mkdir()
    (isolated / core.name).write_bytes(core.read_bytes())  # core only, no phy

    env = {
        **os.environ,
        "LUOS_ENGINE_LIB_DIR": str(isolated),
        "LUOS_ENGINE_NO_DOWNLOAD": "1",
    }
    code = (
        "import luos_engine; "
        "from luos_engine._luos_cffi import lib; "
        "print('OK', hasattr(lib, 'Luos_Init'))"
    )
    result = subprocess.run(
        [sys.executable, "-c", code],
        capture_output=True, text=True, env=env,
        cwd=str(Path(__file__).resolve().parent.parent),
    )
    assert result.returncode == 0, result.stderr
    assert "OK True" in result.stdout
