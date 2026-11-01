"""Production packaging checks; these tests are never part of device ZIPs."""
import hashlib
import json
import tempfile
import unittest
from unittest.mock import patch
import zipfile

from support import ROOT, load_app
from build_hh3 import DEVICE_FILES, build, diagnostic_source, production_source


class ReleaseTests(unittest.TestCase):
    def setUp(self):
        self.parent = ROOT / "release-ready"
        self.parent.mkdir(exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(dir=self.parent)
        self.addCleanup(self.temporary.cleanup)
        self.output = self.parent / self.temporary.name

    def test_production_packages_match_tested_app_without_diagnostics(self):
        output = build(self.output, version="1.0.0")
        source = (ROOT / "physchem_final.py").read_text(encoding="ascii")
        production_source(source)
        for folder in ("classpad", "pc-preview"):
            for name in DEVICE_FILES:
                self.assertEqual((output / folder / name).read_bytes(), (ROOT / name).read_bytes())
        names = {"physik_hh3_classpad_v1_0_0.zip": ("classpad", set(DEVICE_FILES)),
                 "physik_hh3_pc_v1_0_0.zip": ("pc-preview", set(DEVICE_FILES) | {"gint.py", "README.md", "start_pc_preview.bat"})}
        for name, (folder, entries) in names.items():
            self.assertEqual({p.name for p in (output / folder).iterdir()}, entries)
            with zipfile.ZipFile(output / name) as archive:
                self.assertEqual(set(archive.namelist()), entries)
                self.assertEqual(len(archive.namelist()), len(entries))
                self.assertIsNone(archive.testzip())
                for entry in archive.namelist():
                    self.assertEqual(archive.read(entry), (output / folder / entry).read_bytes())
                    if entry.endswith(".py"):
                        compile(archive.read(entry).decode("ascii"), entry, "exec")
        manifest = json.loads((output / "manifest.json").read_text())
        self.assertEqual(manifest["profile"], "production")
        self.assertEqual(manifest["version"], "1.0.0")
        self.assertFalse(manifest["diagnostics"])
        self.assertFalse(manifest["hardware_tested"])
        for name in DEVICE_FILES:
            self.assertEqual(manifest["files"][name], hashlib.sha256((ROOT / name).read_bytes()).hexdigest())
        checksums = (output / "SHA256SUMS.txt").read_text().splitlines()
        self.assertEqual(len(checksums), 3)
        for line in checksums:
            digest, name = line.split("  ")
            self.assertEqual(digest, hashlib.sha256((output / name).read_bytes()).hexdigest())
        app = load_app(output / "classpad/physchem_final.py")
        self.assertFalse(any(key.startswith(("legacy_", "hh3_", "HH3_", "probe_")) for key in app))
        menus = []
        with patch.dict(app, select_list=lambda title, rows, back: menus.append((title, rows)) or len(rows) - 1):
            app["run"]()
        self.assertEqual(menus[0][0], "Physik LK")
        self.assertEqual([row[0] for row in menus[0][1]],
                         ["Theorie", "Formeln nach Groessen", "Formeln nach Themen", "Physik-Rechner", "Beenden"])

    def test_production_rejects_embedded_or_hidden_diagnostics(self):
        source = (ROOT / "physchem_final.py").read_text(encoding="ascii")
        for extra in ("\ndef legacy_hidden(): pass\n", "\nif False:\n    hh3_test()\n",
                      "\nlabel = 'Grafiktest'\n", "\nimport hh3_diagnostics\n", "\neval('1')\n"):
            with self.subTest(extra=extra), self.assertRaises(ValueError):
                production_source(source + extra)
        with self.assertRaises(ValueError):
            production_source(diagnostic_source(source))
        with patch("build_hh3.production_source", side_effect=ValueError("diagnostics")):
            with self.assertRaises(ValueError):
                build(self.output, version="1.0.0")

    def test_production_version_and_output_guards(self):
        for version in ("", "../1.0.0", "1.0", "01.0.0", "1.0.0-test", 1):
            with self.subTest(version=version), self.assertRaises(ValueError):
                build(self.output, version=version)
        for output in (ROOT, self.parent, ROOT.parent, ROOT / "testbuild/hh3_preview"):
            with self.subTest(output=output), self.assertRaises(ValueError):
                build(output, version="1.0.0")
        with self.assertRaises(ValueError):
            build(self.output, diagnostics=True, version="1.0.0")
        build(self.output, version="1.0.0")
        with self.assertRaises(ValueError):
            build(self.output, version="1.0.1")
        with self.assertRaises(ValueError):
            build(self.output)

    def test_production_rejects_extra_package_files_or_directories(self):
        build(self.output, version="1.0.0")
        with tempfile.TemporaryDirectory(dir=self.output / "classpad"):
            with self.assertRaises(ValueError):
                build(self.output, version="1.0.0")
        extra = self.output / "pc-preview/diagnostics.py"
        extra.write_text("raise RuntimeError('not for release')\n", encoding="ascii")
        with self.assertRaises(ValueError):
            build(self.output, version="1.0.0")


if __name__ == "__main__":
    unittest.main()
