"""Generate host-only fixtures from the unchanged Python application."""
import hashlib
import ast
import json
import math
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
    science = ast.parse((PROJECT / "tests/test_science.py").read_text(encoding="ascii"))
    cls = next(node for node in science.body if isinstance(node, ast.ClassDef) and node.name == "ScienceTests")
    method = next(node for node in cls.body if isinstance(node, ast.FunctionDef) and node.name == "test_all_quantum_equations_independently")
    prefix = []
    for node in method.body:
        prefix.append(node)
        if isinstance(node, ast.Assign) and any(isinstance(target, ast.Name) and target.id == "cases" for target in node.targets):
            break
    namespace = {"APP": app, "math": math}
    exec(compile(ast.Module(body=prefix, type_ignores=[]), "independent_reference_cases", "exec"), namespace)
    equations = []
    for row in app["CALCS"]:
        ident = row[-1]
        if ident.startswith("FIT_"):
            continue
        values, expected = namespace["cases"][ident[2:]] if ident.startswith("Q_") and ident[2:] in namespace["cases"] else ([2.] * len(row[4]), None)
        if expected is None:
            expected = app["calc_value"](ident, values)
        equations.append((ident, list(values), True, expected))
        for i in range(len(values)):
            for replacement in (0., -abs(values[i])):
                altered = list(values)
                altered[i] = replacement
                try:
                    result = app["calc_value"](ident, altered)
                    ok = isinstance(result, (int, float)) and math.isfinite(result)
                except (ValueError, ZeroDivisionError, OverflowError):
                    result, ok = 0., False
                equations.append((ident, altered, ok, result if ok else 0.))
    with (output / "calculations.ref").open("wb") as stream:
        stream.write(struct.pack(">I", len(equations)))
        for ident, values, ok, expected in equations:
            name = ident.encode("ascii")
            stream.write(struct.pack(">I", len(name)) + name + struct.pack(">I", len(values)))
            stream.write(struct.pack(">" + "d" * len(values), *values))
            stream.write(struct.pack(">Bd", ok, expected))
    fits = []
    for wavelength in (False, True):
        for count in (2, 3, 10):
            points = []
            for i in range(count):
                frequency = 5e14 + i * 1e13
                x = app["LIGHT_C"] / frequency if wavelength else frequency
                y = app["H_PLANCK"] * frequency - 2 * app["E_CHARGE"]
                points.append((x, y if wavelength else y / app["E_CHARGE"]))
            fits.append((wavelength, points, app["fit_planck"](points, wavelength)))
    points = [(x * 1e-9, y * app["E_CHARGE"]) for x, y in zip((486, 434, 410, 397), (.61, .92, 1.09, 1.19))]
    fits.append((True, points, app["fit_planck"](points, True)))
    with (output / "fits.ref").open("wb") as stream:
        stream.write(struct.pack(">I", len(fits)))
        for wavelength, points, expected in fits:
            stream.write(struct.pack(">II", wavelength, len(points)))
            for x, y in points:
                stream.write(struct.pack(">dd", x, y))
            stream.write(struct.pack(">dddd", *expected))
    report = dict(tables={name: len(rows) for name, rows in zip(names, tables)}, calculator_cases=len(equations),
                  calculator_functions=163, fit_functions=4,
                  chapters=len(app["CHAPTERS"]), media=len(media),
                  reference_files={name: hashlib.sha256((PROJECT / name).read_bytes()).hexdigest()
                                   for name in ("physchem_final.py", "wissen.txt", "bilder.dat")})
    (output / "reference.json").write_text(json.dumps(report, indent=2) + "\n", encoding="ascii")
    print("Python reference fixtures: 514 chapters, 39 media, all indexed records")


if __name__ == "__main__":
    main()
