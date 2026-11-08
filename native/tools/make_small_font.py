"""Generate a readable small DejaVu font; output and font input stay on E:."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
font = ImageFont.truetype(str(ROOT / "build/DejaVuSans.ttf"), 13)
advances, glyphs = [], []
for code in range(32, 127):
    char = chr(code)
    advances.append(max(1, round(font.getlength(char))))
    image = Image.new("L", (16, 16))
    ImageDraw.Draw(image).text((0, -1), char, font=font, fill=255)
    glyphs.append([sum(0x8000 >> x for x in range(16) if image.getpixel((x, y)) >= 100) for y in range(16)])
text = "// Generated from DejaVu Sans, 13 px; see licenses/FONT-LICENSE.txt.\n#pragma once\n#include <cstdint>\nnamespace physik::smallfont {\n"
text += "inline constexpr int height=16;\ninline constexpr uint8_t advance[]={" + ",".join(map(str, advances)) + "};\n"
text += "inline constexpr uint16_t bits[][16]={\n" + "\n".join("  {" + ",".join(hex(word) for word in rows) + "}," for rows in glyphs) + "\n};\n}\n"
(ROOT / "src/small_font.hpp").write_text(text, encoding="ascii", newline="\n")
