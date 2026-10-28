"""PC-only loading, drawing and file-system doubles for the frozen baseline."""
import ast
from collections import Counter
import io
from pathlib import Path
import sys
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import gint


def load_app(path=None):
    path = Path(path or ROOT / "physchem_final.py")
    tree = ast.parse(path.read_text(encoding="ascii"))
    last = tree.body.pop()
    assert isinstance(last, ast.Expr) and isinstance(last.value, ast.Call)
    assert last.value.func.id == "run"
    app = {"__name__": "app_under_test", "__file__": str(ROOT / "physchem_final.py")}
    exec(compile(tree, str(path), "exec"), app)
    return app


def no_drawing():
    return {key: (lambda *args: None) for key in
            ("dclear", "drect", "dline", "dtext_opt", "dupdate", "dsubimage", "cleareventflips")}


def event(key=None, x=0, y=0):
    return gint.Event(gint.KEYEV_DOWN if key else gint.KEYEV_TOUCH_UP, key, x, y)


class FileSystem:
    def __init__(self, seek="ok", short=None, contents=None):
        self.data = {name: (ROOT / name).read_bytes() for name in ("wissen.txt", "bilder.dat")}
        if contents:
            self.data.update(contents)
        self.seek_mode, self.short = seek, short
        self.counts, self.handles, self.requests = Counter(), [], []

    def open(self, path, mode="r"):
        name = str(path).replace("\\", "/").rsplit("/", 1)[-1]
        if self.data.get(name) is None:
            raise OSError("missing")
        self.counts["open"] += 1
        owner = self
        class Reader(io.BytesIO):
            def read(self, size=-1):
                owner.counts["read"] += 1
                data = super().read(size)
                owner.counts["bytes"] += len(data)
                # Simulate the old port's strict text read even in rb mode.
                return data.decode("utf-8")
            def readinto(self, target):
                owner.counts["readinto"] += 1
                owner.requests.append(len(target))
                if owner.short:
                    target = memoryview(target)[:owner.short]
                count = super().readinto(target)
                owner.counts["bytes"] += count
                return count
            def seek(self, offset, whence=0):
                owner.counts["seek"] += 1
                if owner.seek_mode == "missing":
                    raise AttributeError("seek")
                if owner.seek_mode == "raises":
                    super().seek(100)
                    raise OSError("seek failed after moving")
                if owner.seek_mode == "lies":
                    return offset
                return super().seek(offset, whence)
            def tell(self):
                if owner.seek_mode == "no_tell":
                    raise AttributeError("tell")
                return super().tell()
        handle = Reader(self.data[name])
        self.handles.append(handle)
        return handle

    def attach(self, app):
        def collect(): self.counts["gc"] += 1
        def mem_free():
            self.counts["mem_free"] += 1
            return 8 * 1024 * 1024
        app.update(open=self.open, gc=SimpleNamespace(collect=collect, mem_free=mem_free))
        return self


class Canvas:
    def __init__(self):
        from PIL import ImageFont
        font = Path("C:/Windows/Fonts/consola.ttf")
        self.font = ImageFont.truetype(str(font), 13) if font.exists() else ImageFont.load_default()
        self.image = None

    def clear(self, color):
        from PIL import Image, ImageDraw
        self.image = Image.new("RGB", (320, 528), gint._color(color))
        self.draw = ImageDraw.Draw(self.image)

    def rect(self, x1, y1, x2, y2, color):
        self.draw.rectangle((x1, y1, x2, y2), fill=gint._color(color))

    def line(self, x1, y1, x2, y2, color):
        self.draw.line((x1, y1, x2, y2), fill=gint._color(color))

    def text(self, x, y, fg, bg, halign, valign, text, size):
        anchor = ("l", "m", "r")[halign] + ("t", "m", "b")[valign]
        self.draw.text((x, y), str(text), fill=gint._color(fg), font=self.font, anchor=anchor)

    def subraster(self, x, y, image, left, top, width, height):
        from PIL import Image
        self.image.paste(Image.frombytes("RGB", (width, height),
                        gint.image_rgb_bytes(image, left, top, width, height)), (x, y))

    def attach(self, app):
        app.update(dclear=self.clear, drect=self.rect, dline=self.line,
                   dtext_opt=self.text, dsubimage=self.subraster, dupdate=lambda: None)
