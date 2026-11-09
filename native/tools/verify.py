"""Inspect the real HH3 binary, generate previews and package locally on E:."""
import hashlib
import json
from pathlib import Path
import re
import shutil
import zipfile
from PIL import Image
from elf_layout import APP_START, APP_END, inspect_elf, startup_hooks

ROOT = Path(__file__).resolve().parents[1]
PROJECT = ROOT.parent
VERSION = "2.0.0-alpha.3"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def sources():
    checksum = hashlib.sha256()
    files = [ROOT / "Makefile", ROOT / "build.ps1", *ROOT.glob("src/*.*"),
             *ROOT.glob("tests/*.cpp"), *ROOT.glob("tools/*.py")]
    for path in sorted(files):
        checksum.update(path.relative_to(ROOT).as_posix().encode("ascii") + b"\0")
        checksum.update(path.read_bytes())
    return checksum.hexdigest()


def main():
    host_log = (ROOT / "dist/host-tests.log").read_text(encoding="utf-8-sig")
    sdk_log = (ROOT / "dist/sdk-file-tests.log").read_text(encoding="utf-8-sig")
    calc_log = (ROOT / "dist/calculator-tests.log").read_text(encoding="utf-8-sig")
    checks = re.search(r"PASS: (\d+) checks; 1864 records; 514 chapters; 39 media; 3 exact rasters; 20 image switches; 840 browse entries; 167 calculators", host_log)
    calc_checks = re.search(r"PASS CALC: (\d+) checks; 163 fixed formulas; 4 fit workflows; (\d+) formula cases; (\d+) fits", calc_log)
    assert checks and calc_checks and "PASS SDK:" in sdk_log, "Successful native test reports required"
    image = inspect_elf((ROOT / "dist/physik.hh3").read_bytes())
    assert image["notes"]["NAME"] == "Physik LK Native"
    assert image["notes"]["VERSION"] == VERSION
    hooks = startup_hooks((ROOT / "dist/physik.elf").read_bytes(), image)
    footprint = max(s["vaddr"] + s["memory_bytes"] for s in image["loads"]) - APP_START
    reference = json.loads((ROOT / "build/reference/reference.json").read_text())
    assert int(calc_checks[2]) == reference["calculator_cases"]
    assert reference["calculator_functions"] == 163 and reference["fit_functions"] == 4
    for name, expected in reference["reference_files"].items():
        assert digest(PROJECT / name) == expected, "Original file changed: " + name
    for path in (ROOT / "build/preview").glob("*.ppm"):
        with Image.open(path) as preview:
            preview.save(path.with_suffix(".png"))
    folder = ROOT / "dist/classpad"
    folder.mkdir(exist_ok=True)
    shutil.copyfile(ROOT / "dist/physik.hh3", folder / "physik.hh3")
    for name in ("wissen.txt", "bilder.dat"):
        shutil.copyfile(PROJECT / name, folder / name)
    names = ("physik.hh3", "wissen.txt", "bilder.dat")
    assert {p.name for p in folder.iterdir()} == set(names)
    output = ROOT / "dist/physik_native_2_0_0_alpha_3.zip"
    with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as archive:
        for name in names:
            archive.write(folder / name, name)
    with zipfile.ZipFile(output) as archive:
        assert archive.testzip() is None
        assert set(archive.namelist()) == set(names)
        for name in names:
            assert archive.read(name) == (folder / name).read_bytes()
    report = dict(version=VERSION, stage="native full-feature test candidate", hardware_tested=False,
                  predecessor_device_feedback="alpha.2: files open, fast complete images and navigation confirmed by user; no numerical timing",
                  source_sha256=sources(),
                  tests=dict(host_checks=int(checks[1]), indexed_records=1864, chapters=514, media=39,
                             pixel_equal_rasters=3, image_switches=20, sdk_io="passed",
                             browse_entries=840, calculator_forms=167, calculator_checks=int(calc_checks[1]),
                             fixed_equations=163, fit_workflows=4, formula_cases=int(calc_checks[2]), fit_cases=int(calc_checks[3]),
                             sanitizers=["AddressSanitizer", "UndefinedBehaviorSanitizer", "LeakSanitizer"]),
                  format="ELF32 big-endian SH4a-nofpu", entry_point=hex(image["entry"]),
                  program_bytes=footprint, program_limit=APP_END - APP_START, startup_hooks=hooks,
                  files={name: digest(folder / name) for name in names},
                  python_reference=reference["reference_files"],
                  sdk_commit="1e483e395391d50d9bd2d21cee4beecc7589fb1c",
                  toolchain_image="ghcr.io/classpaddev/hollyhock-3@sha256:4381a460f8b19411552ac30f2aba73da38d3aba535412a66c69f8528fe94b49a")
    (ROOT / "dist/manifest.json").write_text(json.dumps(report, indent=2) + "\n", encoding="ascii")
    (ROOT / "dist/SHA256SUMS.txt").write_text(digest(output) + "  " + output.name + "\n", encoding="ascii")
    print(f"PASS: native HH3 {footprint}/{APP_END-APP_START} program bytes, startup hooks, three-file package")
    print(output)


if __name__ == "__main__":
    main()
