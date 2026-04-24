"""Parse C enums from luos_list.h, struct_luos.h, and the macros they
reference (engine_config.h) into a Python module of plain int constants.

Run manually: python -m tools.gen_enums
Or automatically via _build.py at install time.
"""
import re
from pathlib import Path


_ENUM_RE = re.compile(
    r"typedef\s+enum\s*\{(?P<body>.*?)\}\s*(?P<name>\w+)\s*;",
    re.DOTALL,
)
_DEFINE_RE = re.compile(r"#define\s+(\w+)\s+([^\n/]+?)(?://|$)", re.MULTILINE)
_LINE_COMMENT_RE = re.compile(r"//[^\n]*")
_BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)


def _parse_macros(*texts: str) -> dict[str, int]:
    """Pull `#define NAME numeric-literal` entries into a dict."""
    macros: dict[str, int] = {}
    for text in texts:
        for name, value in _DEFINE_RE.findall(text):
            v = value.strip()
            try:
                macros[name] = int(v, 0)
            except ValueError:
                pass  # non-numeric, skip
    return macros


def _eval(expr: str, macros: dict[str, int],
          prior: list[tuple[str, int]]) -> int:
    env = {**macros, **{n: v for n, v in prior}}
    # eval with no builtins — only the names we've collected are reachable.
    return int(eval(expr, {"__builtins__": {}}, env))


def _parse_body(body: str, macros: dict[str, int]) -> list[tuple[str, int]]:
    items: list[tuple[str, int]] = []
    current = 0
    for line in body.split(","):
        line = re.sub(r"/\*.*?\*/", "", line)
        line = re.sub(r"//.*", "", line)
        line = line.strip()
        if not line:
            continue
        m = re.match(r"(\w+)\s*(?:=\s*(.+))?$", line)
        if not m:
            continue
        name, expr = m.group(1), m.group(2)
        if expr is not None:
            current = _eval(expr.strip(), macros, items)
        items.append((name, current))
        current += 1
    return items


def _strip_comments(text: str) -> str:
    """Remove C-style block and line comments."""
    text = _BLOCK_COMMENT_RE.sub("", text)
    text = _LINE_COMMENT_RE.sub("", text)
    return text


def extract_enums(text: str, macros: dict[str, int]
                  ) -> dict[str, list[tuple[str, int]]]:
    clean = _strip_comments(text)
    return {m.group("name"): _parse_body(m.group("body"), macros)
            for m in _ENUM_RE.finditer(clean)}


def render(repo_root: Path) -> str:
    luos_list = (repo_root / "engine/core/inc/luos_list.h").read_text()
    struct_luos = (repo_root / "engine/core/inc/struct_luos.h").read_text()
    engine_config = (repo_root / "engine/engine_config.h").read_text()

    macros = _parse_macros(luos_list, struct_luos, engine_config)
    enums = {**extract_enums(luos_list, macros),
             **extract_enums(struct_luos, macros)}

    want = {
        "luos_type_t": "Type",
        "luos_cmd_t": "Cmd",
        "target_mode_t": "TargetMode",
    }
    lines = ["# Generated from engine/core/inc/*.h and engine/engine_config.h.",
             "# Do not edit by hand — see bindings/python/tools/gen_enums.py.",
             ""]
    for c_name, py_name in want.items():
        if c_name not in enums:
            raise RuntimeError(f"enum {c_name} not found in headers")
        lines.append(f"class _{py_name}Raw:")
        seen = set()
        for name, value in enums[c_name]:
            short = name[:-5] if (py_name == "Type" and name.endswith("_TYPE")) else name
            if short in seen:
                continue
            seen.add(short)
            lines.append(f"    {short} = {value}")
        lines.append("")
    return "\n".join(lines)


def main():
    repo_root = Path(__file__).resolve().parents[3]
    target = Path(__file__).resolve().parent.parent / "luos_engine" / "_enum_values.py"
    target.write_text(render(repo_root))
    print(f"wrote {target}")


if __name__ == "__main__":
    main()
