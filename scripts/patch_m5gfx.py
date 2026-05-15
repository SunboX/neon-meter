from __future__ import annotations

import json
from pathlib import Path

try:
    Import("env")  # type: ignore[name-defined]
except NameError:
    env = None  # type: ignore[assignment]


def patch_m5gfx_manifest() -> None:
    if env is not None:
        libdeps = Path(env.subst("$PROJECT_LIBDEPS_DIR"))
    else:
        libdeps = Path.cwd() / ".pio" / "libdeps" / "m5stack-cores3"
    manifest = libdeps / "M5GFX" / "library.json"
    if not manifest.exists():
        return

    data = json.loads(manifest.read_text())
    build = data.setdefault("build", {})
    wanted = [
        "+<*>",
        "-<lgfx/Fonts/lvgl/>",
        "-<lgfx/v1/lv_font/>",
    ]
    if build.get("srcFilter") == wanted:
        return

    build["srcFilter"] = wanted
    manifest.write_text(json.dumps(data, indent=2) + "\n")
    print("Patched M5GFX library.json to exclude bundled LVGL font shim sources")


patch_m5gfx_manifest()
