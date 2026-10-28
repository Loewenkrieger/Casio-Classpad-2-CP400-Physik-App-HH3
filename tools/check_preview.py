"""Real Tk smoke check: complete image switches and main-menu exit."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tests"))
from support import event, gint, load_app
from build_hh3 import build


def check():
    diagnostic = build(diagnostics=True) / "classpad/physchem_final.py"
    for path, exit_index in ((ROOT / "physchem_final.py", 4), (diagnostic, 5)):
        app = load_app(path)
        try:
            for i in range(20):
                app["render_media_page"](("electron_tube", "rings", "spectrum")[i % 3], i, 20)
                assert len(gint._images) in (0, 3), "Unexpected retained raster sections"
            keys = iter([[event(gint.KEY_DOWN)] for _ in range(exit_index)] + [[event(gint.KEY_EXE)]])
            app["poll_events"] = lambda: next(keys)
            app["run"]()
        finally:
            if gint._root is not None:
                gint._root.destroy()
            gint._root = gint._canvas = None
            gint._images.clear()
            gint._events.clear()
            gint._should_close = False
    print("Tk: both HH3 candidates, 20 complete image switches each, bounded image retention and main-menu exit: OK")


if __name__ == "__main__":
    check()
