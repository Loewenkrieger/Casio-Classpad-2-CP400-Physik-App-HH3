"""Small desktop preview of the ClassPad gint drawing and event API.

This module is only meant for development on a PC. Do not copy it to the
calculator; the ClassPad already provides its own gint module.
"""

from collections import deque

SCREEN_W = 320
SCREEN_H = 528

C_NONE = None
C_BLACK = "#000000"
C_DARK = "#333333"
C_LIGHT = "#dddddd"
C_WHITE = "#ffffff"

DTEXT_LEFT = 0
DTEXT_CENTER = 1
DTEXT_RIGHT = 2
DTEXT_TOP = 0
DTEXT_MIDDLE = 1
DTEXT_BOTTOM = 2

KEYEV_NONE = 0
KEYEV_DOWN = 1
KEYEV_HOLD = 2
KEYEV_TOUCH_DOWN = 3
KEYEV_TOUCH_UP = 4
KEYEV_TOUCH_DRAG = 5

KEY_UP = "up"
KEY_DOWN = "down"
KEY_LEFT = "left"
KEY_RIGHT = "right"
KEY_EXE = "exe"
KEY_DEL = "del"
KEY_EXIT = "exit"

_canvas = None
_root = None
_events = deque()
_pressed_this_frame = set()
_should_close = False
_images = []


class Event:
    def __init__(self, event_type=KEYEV_NONE, key=None, x=0, y=0):
        self.type = event_type
        self.key = key
        self.x = x
        self.y = y


def C_RGB(r, g, b):
    def channel(value):
        return max(0, min(255, int(value * 255 / 31)))

    return "#{:02x}{:02x}{:02x}".format(channel(r), channel(g), channel(b))


def _color(value):
    if isinstance(value, int):
        return "#%02x%02x%02x" % (((value >> 11) & 31)*255//31, ((value >> 5) & 63)*255//63, (value & 31)*255//31)
    return "" if value is C_NONE else value


def _ensure_gui():
    global _canvas, _root
    if _root is not None:
        return

    import tkinter as tk

    _root = tk.Tk()
    _root.title("Casio ClassPad 2 - PC Preview")
    _root.resizable(False, False)
    _canvas = tk.Canvas(
        _root,
        width=SCREEN_W,
        height=SCREEN_H,
        highlightthickness=0,
        bg=C_WHITE,
    )
    _canvas.pack()
    _root.protocol("WM_DELETE_WINDOW", _close)
    _root.bind("<KeyPress>", _key_down)
    _canvas.bind("<ButtonPress-1>", _touch_down)
    _canvas.bind("<B1-Motion>", _touch_drag)
    _canvas.bind("<ButtonRelease-1>", _touch_up)
    _canvas.focus_set()


def _close():
    global _should_close
    _should_close = True


def _key_down(event):
    key_map = {
        "Up": KEY_UP,
        "Down": KEY_DOWN,
        "Left": KEY_LEFT,
        "Right": KEY_RIGHT,
        "Return": KEY_EXE,
        "KP_Enter": KEY_EXE,
        "Delete": KEY_DEL,
        "BackSpace": KEY_DEL,
        "Escape": KEY_EXIT,
    }
    key = key_map.get(event.keysym)
    if key is not None:
        _pressed_this_frame.add(key)
        _events.append(Event(KEYEV_DOWN, key=key))


def _touch_down(event):
    _events.append(Event(KEYEV_TOUCH_DOWN, x=event.x, y=event.y))


def _touch_drag(event):
    _events.append(Event(KEYEV_TOUCH_DRAG, x=event.x, y=event.y))


def _touch_up(event):
    _events.append(Event(KEYEV_TOUCH_UP, x=event.x, y=event.y))


def dclear(color):
    _ensure_gui()
    _canvas.delete("all")
    _images.clear()
    _canvas.configure(bg=_color(color))


def drect(x1, y1, x2, y2, color):
    _ensure_gui()
    _canvas.create_rectangle(x1, y1, x2, y2, fill=_color(color), outline="")


def drect_border(x1, y1, x2, y2, fill, width, border):
    _ensure_gui()
    _canvas.create_rectangle(
        x1,
        y1,
        x2,
        y2,
        fill=_color(fill),
        outline=_color(border),
        width=width,
    )


def dline(x1, y1, x2, y2, color):
    _ensure_gui()
    _canvas.create_line(x1, y1, x2, y2, fill=_color(color))


def dhline(y, color):
    dline(0, y, SCREEN_W, y, color)


def dpoly(points, border, fill):
    _ensure_gui()
    _canvas.create_polygon(points, outline=_color(border), fill=_color(fill))


def dtext(x, y, color, text):
    dtext_opt(x, y, color, C_NONE, DTEXT_LEFT, DTEXT_TOP, text, -1)


def dtext_opt(x, y, color, background, horizontal, vertical, text, size):
    _ensure_gui()
    horizontal_anchor = {
        DTEXT_LEFT: "w",
        DTEXT_CENTER: "center",
        DTEXT_RIGHT: "e",
    }[horizontal]
    anchor = horizontal_anchor
    if vertical == DTEXT_TOP:
        anchor = {"w": "nw", "center": "n", "e": "ne"}[horizontal_anchor]
    elif vertical == DTEXT_BOTTOM:
        anchor = {"w": "sw", "center": "s", "e": "se"}[horizontal_anchor]
    _canvas.create_text(
        x,
        y,
        text=str(text),
        fill=_color(color),
        anchor=anchor,
        font=("Consolas", 10),
    )


def dupdate():
    global _pressed_this_frame
    _ensure_gui()
    if _should_close:
        raise SystemExit
    _pressed_this_frame = set()
    _root.update_idletasks()
    _root.update()


def pollevent():
    if _events:
        return _events.popleft()
    return Event()


def keypressed(key):
    return key in _pressed_this_frame


def clearevents():
    _events.clear()


def cleareventflips():
    pass


class _IndexedImage:
    def __init__(self, width, height, data, palette, bits):
        if not isinstance(width, int) or not isinstance(height, int):
            raise TypeError("Image dimensions must be integers")
        data_view, palette_view = memoryview(data), memoryview(palette)
        # Match PythonExtra's len-based checks before taking byte views.
        self.stride = (width + 1) // 2 if bits == 4 else width
        self.color_count = 16 if bits == 4 else len(palette) // 2
        if width <= 0 or height <= 0 or len(data) < self.stride * height:
            raise ValueError("Invalid image dimensions or pixel buffer")
        if not 1 <= self.color_count <= 256 or len(palette) < 2 * self.color_count:
            raise ValueError("Invalid RGB565 palette byte length")
        self.width, self.height = width, height
        self.data, self.palette, self.bits = data_view.cast("B"), palette_view.cast("B"), bits


def image_p4_rgb565(width, height, data, palette):
    return _IndexedImage(width, height, data, palette, 4)


def image_p8_rgb565(width, height, data, palette):
    return _IndexedImage(width, height, data, palette, 8)


def image_rgb_bytes(img, left=0, top=0, width=None, height=None):
    width = img.width - left if width is None else width
    height = img.height - top if height is None else height
    if min(left, top, width, height) < 0 or left + width > img.width or top + height > img.height:
        raise ValueError("Image region outside pixel buffer")
    result = bytearray(width * height * 3)
    for i in range(width * height):
        row, col = divmod(i, width)
        row, col = row + top, col + left
        if img.bits == 4:
            packed = img.data[row * img.stride + col // 2]
            index = packed >> 4 if col % 2 == 0 else packed & 15
        else:
            index = (img.data[row * img.stride + col] + 128) & 255
        if index >= img.color_count:
            raise ValueError("Pixel index outside palette")
        word = img.palette[2*index] * 256 + img.palette[2*index+1]
        result[i*3:i*3+3] = bytes((((word>>11)&31)*255//31, ((word>>5)&63)*255//63, (word&31)*255//31))
    return bytes(result)


def dimage(x, y, img):
    dsubimage(x, y, img, 0, 0, img.width, img.height)


def dsubimage(x, y, img, left, top, width, height):
    right, bottom = min(img.width, left + width), min(img.height, top + height)
    if left < 0:
        x, left = x - left, 0
    if top < 0:
        y, top = y - top, 0
    width, height = right - left, bottom - top
    if width <= 0 or height <= 0:
        return
    import tkinter as tk
    _ensure_gui()
    ppm = ("P6\n%d %d\n255\n" % (width, height)).encode("ascii") + image_rgb_bytes(img, left, top, width, height)
    rendered = tk.PhotoImage(data=ppm, format="PPM")
    _images.append(rendered)
    _canvas.create_image(x, y, anchor="nw", image=rendered)
