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
VERSION = "2.0.0-beta.6"
BASELINE_ZIP = PROJECT / "native-backups/2026-11-21_before_navigation_beta4/physik_hh3_native_v2_0_0_beta_3_test.zip"
BASELINE_SHA256 = "ae5b07e4dc1cf215e30da836fb3c4caa524328e9b4941586bbe856df8aba4164"
PREVIOUS = PROJECT / "native-backups/2026-11-22_before_alphabet_beta5"
PREVIOUS_SHA256 = "c452c307f295e208fd451c3c7e6f125d2bd120bca6c712c8b8302dd7a267aaf8"
PREDECESSOR = PROJECT / "native-backups/2026-11-22_before_menu_cleanup_beta6"
PREDECESSOR_SHA256 = "da5f07c8a2a1d10c2a670ec544ef00db3ef42e7a09ce1c8fbe5d746a5325a750"
VISIBLE_MODES = (0, 2, 3)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def sources():
    checksum = hashlib.sha256()
    files = [ROOT / "Makefile", ROOT / "build.ps1", *ROOT.glob("src/*.*"),
             *ROOT.glob("tests/*.cpp"), *ROOT.glob("tests/*.hpp"), *ROOT.glob("tests/*.py"),
             *ROOT.glob("tools/*.py"), *ROOT.glob("tools/*.cpp"), *ROOT.glob("sources/*.json")]
    for path in sorted(files):
        checksum.update(path.relative_to(ROOT).as_posix().encode("ascii") + b"\0")
        checksum.update(path.read_bytes())
    return checksum.hexdigest()


def main():
    host_log = (ROOT / "dist/host-tests.log").read_text(encoding="utf-8-sig")
    sdk_log = (ROOT / "dist/sdk-file-tests.log").read_text(encoding="utf-8-sig")
    calc_log = (ROOT / "dist/calculator-tests.log").read_text(encoding="utf-8-sig")
    kin_log = (ROOT / "dist/kinematics-tests.log").read_text(encoding="utf-8-sig")
    checks = re.search(r"PASS: (\d+) checks; 1864 records; 514 chapters; 39 media; 3 exact rasters; 20 image switches; (\d+) browse entries; 167 calculators; 7 native kinematics forms", host_log)
    calc_checks = re.search(r"PASS CALC: (\d+) checks; 163 fixed formulas; 4 fit workflows; (\d+) formula cases; (\d+) fits", calc_log)
    kin_checks = re.search(r"PASS KINEMATICS: (\d+) checks; 7 masks; 4 original scenarios", kin_log)
    assert checks and calc_checks and kin_checks and "PASS SDK:" in sdk_log, "Successful native test reports required"
    suites = {}
    for suite in ("mechanics", "fields", "optics", "analysis", "simulation", "extensions", "navigation"):
        log = (ROOT / f"dist/{suite}-tests.log").read_text(encoding="utf-8-sig")
        match = re.search(r"PASS(?: [A-Z_]+)?: (\d+) checks", log, re.IGNORECASE)
        assert match and "FAIL" not in log, "Successful extension test required: " + suite
        suites[suite] = int(match[1])
    extension_log = (ROOT / "dist/extensions-tests.log").read_text(encoding="utf-8-sig")
    counts = re.search(r"; (\d+) categories; (\d+) workflows; (\d+) articles; (\d+) links", extension_log)
    assert counts, "Extension inventory required"
    categories, workflows, articles, links = map(int, counts.groups())
    navigation = json.loads((ROOT / "build/navigation-manifest.json").read_text())
    assert navigation["roots"][:3] == [0, 1, 2] and len(navigation["roots"]) == 4
    assert len(navigation["mode_stats"]) == 4
    assert navigation["alphabet_quantities"] == 142
    assert navigation["alphabet_letters"] == sorted(set(navigation["alphabet_letters"]))
    stored_links = sum(mode["links"] for mode in navigation["mode_stats"])
    browse_links = sum(navigation["mode_stats"][mode]["links"] for mode in VISIBLE_MODES)
    assert int(checks[2]) == browse_links
    for mode, stats in enumerate(navigation["mode_stats"]):
        assert stats["legacy_unique"] == (146 if mode == 0 else 368)
        assert stats["native_unique"] == 32
        assert stats["unique"] == (178 if mode == 0 else 400)
    navigation_log = (ROOT / "dist/navigation-tests.log").read_text(encoding="utf-8-sig")
    nav_counts = re.search(r"; (\d+) nodes; 546 chapters; 178 theory; 400 formulas per search; (\d+) links", navigation_log)
    assert nav_counts and int(nav_counts[1]) == navigation["node_count"] and int(nav_counts[2]) == stored_links
    assert f"; {browse_links} public links;" in navigation_log
    data_log = (ROOT / "dist/navigation-data-tests.log").read_text(encoding="utf-8-sig")
    assert re.search(r"\bOK\b", data_log) and "FAILED" not in data_log
    catalog = json.loads((ROOT / "build/native-catalog.json").read_text())
    assert len(catalog) == workflows and len({c["id"] for c in catalog}) == workflows
    knowledge = json.loads((ROOT / "build/knowledge-manifest.json").read_text())
    assert len(knowledge["entries"]) == articles
    compact_suites = {}
    for suite in ("host", "mechanics", "fields", "optics", "analysis", "simulation", "extensions", "catalog", "navigation"):
        log = (ROOT / f"dist/compact-{suite}-tests.log").read_text(encoding="utf-8-sig")
        match = re.search(r"PASS(?: [A-Z_]+)?: (\d+) checks", log, re.IGNORECASE)
        assert match and "FAIL" not in log, "Data-backed device path must pass: " + suite
        compact_suites[suite] = int(match[1])
    catalog_manifest = json.loads((ROOT / "build/catalog-manifest.json").read_text())
    assert catalog_manifest["workflows"] == workflows and len(catalog_manifest["categories"]) == categories
    image = inspect_elf((ROOT / "dist/physik.hh3").read_bytes())
    assert image["notes"]["NAME"] == "Physik LK Native"
    assert image["notes"]["VERSION"] == VERSION
    hooks = startup_hooks((ROOT / "dist/physik.elf").read_bytes(), image)
    footprint = max(s["vaddr"] + s["memory_bytes"] for s in image["loads"]) - APP_START
    assert APP_END - APP_START == 337920 and footprint <= 337920, "Original HH3 program region must not grow"
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
        shutil.copyfile(ROOT / "build/package" / name, folder / name)
    legacy = (PROJECT / "wissen.txt").read_bytes()
    combined = (folder / "wissen.txt").read_bytes()
    assert combined[:len(legacy)] == legacy
    assert combined[len(legacy):].startswith(("KNEXT3 " + knowledge["identity"] + "\n").encode("ascii"))
    assert navigation["offset"] == catalog_manifest["total_bytes"]
    assert len(combined) == navigation["total_bytes"]
    assert combined[catalog_manifest["offset"]:].startswith(("PCCATALOG3 " + catalog_manifest["identity"] + "\n").encode("ascii"))
    assert combined[navigation["offset"]:].startswith(("PCNAV4 " + navigation["identity"] + "\n").encode("ascii"))
    assert digest(folder / "bilder.dat") == reference["reference_files"]["bilder.dat"]
    assert digest(BASELINE_ZIP) == BASELINE_SHA256, "Frozen Beta.3 baseline changed"
    with zipfile.ZipFile(BASELINE_ZIP) as baseline:
        previous = baseline.read("wissen.txt")
        assert len(previous) == navigation["offset"]
        assert combined[:len(previous)] == previous, "Beta.3 knowledge, formulas or calculator metadata changed"
        assert baseline.read("bilder.dat") == (folder / "bilder.dat").read_bytes(), "Beta.3 media changed"
    previous_zip = PREVIOUS / "physik_hh3_native_v2_0_0_beta_4_test.zip"
    assert digest(previous_zip) == PREVIOUS_SHA256, "Frozen Beta.4 baseline changed"
    previous_navigation = json.loads((PREVIOUS / "navigation-manifest.json").read_text())
    assert navigation["roots"][3] == previous_navigation["node_count"]
    assert navigation["mode_stats"][:3] == previous_navigation["mode_stats"]
    with zipfile.ZipFile(previous_zip) as baseline:
        beta4_data = baseline.read("wissen.txt")
        assert beta4_data[:navigation["offset"]] == combined[:navigation["offset"]], "Beta.4 article/calculator data changed"
        assert baseline.read("bilder.dat") == (folder / "bilder.dat").read_bytes(), "Beta.4 media changed"
        for old, new in zip(previous_navigation["nodes"], navigation["nodes"]):
            for key in ("id", "title", "mode", "topic", "group", "children", "count", "width"):
                assert old[key] == new[key], "Existing teacher navigation changed: " + key
            size = old["count"] * old["width"]
            assert beta4_data[old["start"]:old["start"]+size] == combined[new["start"]:new["start"]+size]
    predecessor_zip = PREDECESSOR / "physik_hh3_native_v2_0_0_beta_5_test.zip"
    assert digest(predecessor_zip) == PREDECESSOR_SHA256, "Frozen Beta.5 baseline changed"
    with zipfile.ZipFile(predecessor_zip) as baseline:
        assert baseline.read("wissen.txt") == combined, "Beta.5 knowledge or navigation data changed"
        assert baseline.read("bilder.dat") == (folder / "bilder.dat").read_bytes(), "Beta.5 media changed"
    assert navigation == json.loads((PREDECESSOR / "navigation-manifest.json").read_text()), "Beta.5 search assignments changed"
    names = ("physik.hh3", "wissen.txt", "bilder.dat")
    assert {p.name for p in folder.iterdir()} == set(names)
    output = ROOT / "dist/physik_hh3_native_v2_0_0_beta_6_test.zip"
    with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as archive:
        for name in names:
            archive.write(folder / name, name)
    with zipfile.ZipFile(output) as archive:
        assert archive.testzip() is None
        assert set(archive.namelist()) == set(names)
        for name in names:
            assert archive.read(name) == (folder / name).read_bytes()
    report = dict(version=VERSION, stage="local test candidate; simplified home with topic and alphabet formula searches", hardware_tested=False,
                  home_menu=["Theorie", "Formeln nach Themen", "Formeln A-Z", "Physik-Rechner", "Beenden"],
                  visible_search_modes=[navigation["search_modes"][mode] for mode in VISIBLE_MODES],
                  predecessor_device_feedback="alpha.2: files open, fast complete images and navigation confirmed by user; no numerical timing",
                  source_sha256=sources(),
                  tests=dict(host_checks=int(checks[1]), indexed_records=1864, chapters=514+articles, legacy_chapters=514, media=39,
                             pixel_equal_rasters=3, image_switches=20, sdk_io="passed",
                             browse_entries=browse_links, calculator_categories=18+categories, calculator_forms=174+workflows, legacy_calculator_forms=167,
                             calculator_checks=int(calc_checks[1]), kinematics_checks=int(kin_checks[1]), kinematics_forms=7,
                             kinematics_original_scenarios=4, fixed_equations=170, legacy_fixed_equations=163,
                             fit_workflows=4, formula_cases=int(calc_checks[2]), fit_cases=int(calc_checks[3]),
                             extension_workflows=workflows, extension_suites=suites, supplemental_articles=articles,
                             data_catalog_suites=compact_suites, exact_data_workflows=workflows,
                             navigation_nodes=navigation["node_count"], navigation_modes=navigation["mode_stats"],
                             stored_navigation_entries=stored_links,
                             alphabet_quantities=142, alphabet_letters=len(navigation["alphabet_letters"]),
                             alphabet_links=navigation["mode_stats"][3]["links"],
                             sanitizers=["AddressSanitizer", "UndefinedBehaviorSanitizer", "LeakSanitizer"]),
                  format="ELF32 big-endian SH4a-nofpu", entry_point=hex(image["entry"]),
                  program_bytes=footprint, program_limit=APP_END - APP_START, startup_hooks=hooks,
                  files={name: digest(folder / name) for name in names},
                  python_reference=reference["reference_files"],
                  knowledge_extension=knowledge,
                  native_catalog=catalog_manifest,
                  navigation=navigation,
                  baseline=dict(version="2.0.0-beta.5", zip_sha256=PREDECESSOR_SHA256,
                                unchanged_knowledge_bytes=len(combined), unchanged_media=True,
                                unchanged_navigation_nodes=navigation["node_count"],
                                frozen_beta3_zip_sha256=BASELINE_SHA256),
                  sdk_commit="1e483e395391d50d9bd2d21cee4beecc7589fb1c",
                  toolchain_image="ghcr.io/classpaddev/hollyhock-3@sha256:4381a460f8b19411552ac30f2aba73da38d3aba535412a66c69f8528fe94b49a")
    (ROOT / "dist/manifest.json").write_text(json.dumps(report, indent=2) + "\n", encoding="ascii")
    (ROOT / "dist/SHA256SUMS.txt").write_text(
        digest(output) + "  " + output.name + "\n" +
        digest(ROOT / "dist/manifest.json") + "  manifest.json\n", encoding="ascii")
    lines = ["# Neue Native Funktionen", "", "Lokaler Testbuild beta.6; keine vollstaendige Hardwarefreigabe.", "",
             "| Kategorie | Funktion | Kennung |", "| --- | --- | --- |"]
    lines.extend(f'| {c["category"]} | {c["title"]} | `{c["id"]}` |' for c in catalog)
    (ROOT / "dist/FUNKTIONEN.md").write_text("\n".join(lines) + "\n", encoding="ascii")
    print(f"PASS: native HH3 {footprint}/{APP_END-APP_START} program bytes, startup hooks, three-file package")
    print(output)


if __name__ == "__main__":
    main()
