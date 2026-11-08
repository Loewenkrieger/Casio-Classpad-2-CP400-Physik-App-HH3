"""Generate host-only fixtures from the unchanged Python application."""
import hashlib
import json
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
PROJECT = ROOT.parent
sys.path.insert(0, str(PROJECT / "tests"))
from support import load_app, gint


def encoded(value):
    if isinstance(value, int):
        return ("i" + str(value) + ":").encode("ascii")
    if isinstance(value, str):
        raw = value.encode("ascii")
        return ("s" + str(len(raw)) + ":").encode("ascii") + raw
    return ("t" + str(len(value)) + ":").encode("ascii") + b"".join(encoded(v) for v in value)


def write_sequence(path, values):
    values = list(values)
    with path.open("wb") as stream:
        stream.write(struct.pack(">I", len(values)))
        for value in values:
            stream.write(struct.pack(">I", len(value)))
            stream.write(value)


def main():
    output = ROOT / "build/reference"
    (output / "media").mkdir(parents=True, exist_ok=True)
    baseline = json.loads((PROJECT / "tests/baseline.json").read_text())
    for name in ("wissen.txt", "bilder.dat"):
        assert hashlib.sha256((PROJECT / name).read_bytes()).hexdigest() == baseline["files"][name]
    app = load_app()
    names = ("MODES", "CALC_CATS", "CHAPTERS", "SECTIONS", "CALCS", "CALC_NOTES", "ARTICLE_MEDIA", "ENTRIES", "MEDIA_INDEX")
    tables = []
    for name in names:
        table = app[name]
        if hasattr(table, "table"):
            table = table.table
        tables.append(tuple(table))
    write_sequence(output / "records.ref", (encoded(row) for table in tables for row in table))
    write_sequence(output / "chapters.ref", (app["load_chapter"](i).encode("ascii") for i in range(len(app["CHAPTERS"]))))
    media = []
    for ident, row in app["MEDIA_INDEX"].items():
        data = bytes(app["read_media"](ident))
        (output / "media" / (ident + ".bin")).write_bytes(data)
        media.append(ident)
        if row[1] in ("P4", "P8"):
            count = 16 if row[1] == "P4" else 256
            constructor = gint.image_p4_rgb565 if count == 16 else gint.image_p8_rgb565
            image = constructor(row[4], row[5], data[2 * count:], data[:2 * count])
            (output / "media" / (ident + ".rgb")).write_bytes(gint.image_rgb_bytes(image))
    (output / "media.txt").write_text("\n".join(media) + "\n", encoding="ascii", newline="\n")
    report = dict(tables={name: len(rows) for name, rows in zip(names, tables)},
                  chapters=len(app["CHAPTERS"]), media=len(media),
                  reference_files={name: hashlib.sha256((PROJECT / name).read_bytes()).hexdigest()
                                   for name in ("physchem_final.py", "wissen.txt", "bilder.dat")})
    (output / "reference.json").write_text(json.dumps(report, indent=2) + "\n", encoding="ascii")
    print("Python reference fixtures: 514 chapters, 39 media, all indexed records")


if __name__ == "__main__":
    main()
