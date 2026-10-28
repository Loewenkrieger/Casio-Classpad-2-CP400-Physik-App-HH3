"""HH3-specific I/O, regression, drawing and packaging acceptance checks."""
import ast
import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zipfile

from support import ROOT, Canvas, FileSystem, event, gint, load_app, no_drawing
from build_hh3 import BASELINE, build


class HH3Tests(unittest.TestCase):
    def setUp(self):
        self.app = load_app()
        self.old = load_app(BASELINE)
        self.fs = FileSystem().attach(self.app)
        FileSystem().attach(self.old)

    def test_frozen_data_and_baseline(self):
        baseline = json.loads((ROOT / "tests/baseline.json").read_text())
        for name, digest in baseline["files"].items():
            path = BASELINE if name.endswith(".py") else ROOT / name
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), digest)

    def test_import_does_not_open_data(self):
        tree = ast.parse((ROOT / "physchem_final.py").read_text(encoding="ascii"))
        tree.body.pop()
        def forbidden(*args): self.fail("Data read during import")
        namespace = {"__name__": "import_check", "__file__": str(ROOT / "physchem_final.py"), "open": forbidden}
        exec(compile(tree, "import_check", "exec"), namespace)
        self.assertEqual(len(namespace["CHAPTERS"]), 514)

    def test_all_tables_and_chapters(self):
        for name in ("MODES", "CALC_CATS", "CHAPTERS", "SECTIONS", "CALCS", "ENTRIES"):
            self.assertEqual(tuple(self.app[name]), tuple(self.old[name]), name)
        for name in ("MEDIA_INDEX", "ARTICLE_MEDIA", "CALC_NOTES"):
            self.assertEqual(dict(self.app[name].items()), dict(self.old[name].items()), name)
        for i in range(514):
            self.assertEqual(self.app["load_chapter"](i), self.old["load_chapter"](i))

    def test_only_io_and_diagnostics_functions_changed(self):
        old = ast.parse(BASELINE.read_text(encoding="ascii"))
        new = ast.parse((ROOT / "physchem_final.py").read_text(encoding="ascii"))
        allowed = {"store_text", "skip_store", "open_store", "DiskTable", "load_chapter",
                   "media_stage", "media_error", "fill_media", "read_media", "show_media"}
        definitions = {node.name: node for node in new.body if isinstance(node, (ast.FunctionDef, ast.ClassDef))}
        for node in old.body:
            if isinstance(node, (ast.FunctionDef, ast.ClassDef)) and node.name not in allowed:
                self.assertEqual(ast.dump(node), ast.dump(definitions[node.name]), node.name)

    def test_all_legacy_calculators(self):
        for row in tuple(self.app["CALCS"])[:82]:
            values = [2.] * len(row[4])
            self.assertEqual(self.app["calc_value"](row[-1], values), self.old["calc_value"](row[-1], values))

    def test_every_image_matches_baseline_pixels(self):
        output = ROOT / "testbuild/hh3_preview/screenshots"
        output.mkdir(parents=True, exist_ok=True)
        actual, expected = Canvas(), Canvas()
        actual.attach(self.app)
        expected.attach(self.old)
        ids = list(self.app["MEDIA_INDEX"])
        self.assertEqual(len(ids), 39)
        self.assertFalse(any(key.startswith(("private_", "probe_")) for key in ids))
        for ident in ids:
            self.assertEqual(self.app["read_media"](ident), self.old["read_media"](ident))
            self.app["render_media_page"](ident, 0, 1)
            self.old["render_media_page"](ident, 0, 1)
            self.assertEqual(actual.image.tobytes(), expected.image.tobytes(), ident)
            actual.image.save(output / (ident + ".png"))

    def test_direct_io_counts_and_no_success_heap_probes(self):
        report = {}
        for ident in ("electron_tube", "rings", "spectrum"):
            values = []
            for app in (self.old, self.app):
                fs = FileSystem().attach(app)
                app["MEDIA_INDEX"].last_key = None
                app["MEDIA_INDEX"].last_value = None
                app.update(no_drawing())
                app["render_media_page"](ident, 0, 1)
                values.append(dict(fs.counts))
                self.assertTrue(all(handle.closed for handle in fs.handles))
            self.assertEqual(values[1].get("mem_free", 0), 0)
            self.assertEqual(values[1]["gc"], 2)
            self.assertEqual(values[1]["seek"], 2)
            self.assertLessEqual(max(fs.requests), 4096)
            self.assertLess(values[1]["readinto"], values[0]["readinto"])
            if ident != "electron_tube":
                self.assertLess(values[1]["bytes"], values[0]["bytes"] / 2)
            report[ident] = {"original": values[0], "hh3": values[1]}
        path = ROOT / "testbuild/hh3_preview/operation-counts.json"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps({"note": "PC operation counts, not device timings", "images": report}, indent=2))

    def test_seek_failures_restart_from_header(self):
        for mode in ("missing", "raises", "lies", "no_tell"):
            app = load_app()
            fs = FileSystem(seek=mode).attach(app)
            with self.subTest(mode=mode):
                self.assertEqual(app["read_media"]("spectrum"), self.old["read_media"]("spectrum"))
                self.assertEqual(app["load_chapter"](513), self.old["load_chapter"](513))
                self.assertTrue(all(handle.closed for handle in fs.handles))
                self.assertLessEqual(max(fs.requests), 4096)
                self.assertEqual(fs.counts["mem_free"], 0)

    def test_short_reads_direct_and_fallback(self):
        for mode in ("ok", "raises"):
            app = load_app()
            fs = FileSystem(seek=mode, short=17).attach(app)
            self.assertEqual(app["read_media"]("rings"), self.old["read_media"]("rings"))
            self.assertEqual(app["load_chapter"](513), self.old["load_chapter"](513))
            self.assertTrue(all(handle.closed for handle in fs.handles))

    def test_invalid_readinto_counts_and_missing_api(self):
        for value in (-1, 3, None, 0, "1"):
            with self.subTest(value=value), self.assertRaises(ValueError):
                self.app["fill_bytes"](lambda target: value, bytearray(2))
        class NoReadInto(io.BytesIO):
            readinto = None
        with patch.dict(self.app, open=lambda *args: NoReadInto((ROOT / "bilder.dat").read_bytes())):
            with self.assertRaisesRegex(ValueError, "readinto"):
                self.app["read_media"]("rings")

    def test_bad_positions_are_rejected(self):
        for start in (-1, 0, 1, 33.5, "34"):
            with self.subTest(start=start), self.assertRaises(ValueError):
                self.app["positioned_store"](1, start)
        self.assertEqual(self.fs.counts["open"], 0)

    def test_media_corruption_and_file_cleanup(self):
        original = (ROOT / "bilder.dat").read_bytes()
        row = self.old["MEDIA_INDEX"]["rings"]
        corrupted = bytearray(original)
        corrupted[row[2] + 50] ^= 1
        for content in (None, b"wrong\n", original[:20], original[:row[2] + 10], bytes(corrupted)):
            app = load_app()
            fs = FileSystem(contents={"bilder.dat": content}).attach(app)
            with self.subTest(size=None if content is None else len(content)):
                with self.assertRaises((ValueError, OSError)):
                    app["read_media"]("rings")
                self.assertTrue(all(handle.closed for handle in fs.handles))
                self.assertTrue(app["load_chapter"](0))
                self.assertEqual(app["calc_value"]("Q_UK", [app["E_CHARGE"]]), 1)

    def test_error_reports_only_current_heap(self):
        self.app["media_stage"]("Bilddaten lesen")
        self.assertEqual(self.fs.counts["mem_free"], 0)
        text = self.app["media_error"](ValueError(), "rings")
        self.assertIn("Bilddaten lesen", text)
        self.assertIn("ValueError", text)
        self.assertIn("RAM bei Fehler", text)
        self.assertIn("keine Zusatzmeldung", text)
        self.assertEqual(self.fs.counts["mem_free"], 1)

    def test_core_bytearray_without_slice_assignment_or_decode(self):
        class CoreBytearray(bytearray):
            def __setitem__(self, key, value):
                if isinstance(key, slice): raise TypeError("slice assignment unavailable")
                return super().__setitem__(key, value)
            def __getattribute__(self, name):
                if name == "decode": raise AttributeError("decode")
                return super().__getattribute__(name)
        canvas = Canvas()
        canvas.attach(self.app)
        with patch.dict(self.app, bytearray=CoreBytearray):
            self.assertEqual(self.app["load_chapter"](513), self.old["load_chapter"](513))
            for ident in ("electron_tube", "rings", "spectrum"):
                self.app["render_media_page"](ident, 0, 1)

    def test_render_failures_do_not_poison_followup(self):
        errors = []
        self.app.update(no_drawing())
        def fail(*args): raise MemoryError()
        with patch.dict(self.app, image_p8_rgb565=fail, show_text=lambda *args: errors.append(args)):
            self.app["show_media"](("rings",))
        self.assertIn("Bildobjekt erzeugen", errors[0][1])
        self.assertTrue(all(handle.closed for handle in self.fs.handles))
        self.app["render_media_page"]("rings", 0, 1)

    def test_native_palette_and_complete_strips(self):
        for ident, row in self.app["MEDIA_INDEX"].items():
            if row[1] not in ("P4", "P8"):
                continue
            calls = []
            def draw(x, y, image, left, top, width, height):
                calls.append((top, height, id(image)))
                self.assertEqual((x, y, left, width), (8, 68 + top, 0, row[4]))
                self.assertEqual(len(gint.image_rgb_bytes(image, left, top, width, height)), width * height * 3)
            with patch.dict(self.app, dsubimage=draw):
                self.app["draw_media"](ident, self.app["read_media"](ident))
            self.assertEqual([(top, height) for top, height, _ in calls],
                             [(top, min(128, row[5] - top)) for top in range(0, row[5], 128)])
            self.assertEqual(len({identity for _, _, identity in calls}), 1)

    def test_main_exit_and_clean_candidate_menu(self):
        menus = []
        with patch.dict(self.app, select_list=lambda title, rows, back: menus.append((title, rows)) or len(rows) - 1):
            self.app["run"]()
        self.assertEqual(menus[0][0], "Physik LK")
        self.assertEqual([row[0] for row in menus[0][1]],
                         ["Theorie", "Formeln nach Groessen", "Formeln nach Themen", "Physik-Rechner", "Beenden"])

    def test_build_archives_and_guards(self):
        output = build(diagnostics=True)
        self.assertEqual({p.name for p in (output / "classpad").iterdir()},
                         {"physchem_final.py", "wissen.txt", "bilder.dat"})
        for folder, name in (("classpad", "physik_hh3_test.zip"), ("pc-preview", "physik_hh3_pc_test.zip")):
            ast.parse("import " + Path(name).stem)
            with zipfile.ZipFile(output / name) as archive:
                self.assertIsNone(archive.testzip())
                for entry in archive.namelist():
                    self.assertEqual(archive.read(entry), (output / folder / entry).read_bytes())
                    if entry.endswith(".py"):
                        compile(archive.read(entry).decode("ascii"), entry, "exec")
        for path in (ROOT, ROOT / "tests", ROOT.parent, ROOT / "testbuild"):
            with self.assertRaises(ValueError): build(path)
        with self.assertRaises(ValueError): build(output, diagnostics=False)
        manifest = json.loads((output / "manifest.json").read_text())
        self.assertFalse(manifest["hardware_tested"])
        source = (output / "classpad/physchem_final.py").read_text(encoding="ascii")
        for node in ast.walk(ast.parse(source)):
            if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
                self.assertNotIn(node.func.id, ("eval", "exec", "compile"))
        with tempfile.TemporaryDirectory(dir=ROOT / "testbuild") as temporary:
            folder = Path(temporary) / "classpad"
            folder.mkdir()
            (folder / "unexpected.pdf").write_bytes(b"not a package file")
            with self.assertRaises(ValueError): build(temporary)

    def test_diagnostic_comparison_and_state_restoration(self):
        output = build(diagnostics=True)
        app = load_app(output / "classpad/physchem_final.py")
        fs = FileSystem().attach(app)
        app.update(no_drawing())
        originals = {key: app[key] for key in ("MEDIA_INDEX", "legacy_MEDIA_INDEX", "gc", "media_stage", "open_store")}
        app["hh3_bind"](True)
        try:
            old = app["hh3_trial"](True, "rings")
            new = app["hh3_trial"](False, "rings")
            self.assertEqual(old[2], (26, 14))
            self.assertEqual(new[2], (2, 0))
            self.assertEqual(len(new[1]), 10)
        finally:
            app["hh3_bind"](False)
        for key, value in originals.items(): self.assertIs(app[key], value)
        self.assertFalse(app["HH3_ACTIVE"])
        self.assertTrue(all(handle.closed for handle in fs.handles))

    def test_diagnostic_flow_cancel_and_twenty_swaps(self):
        app = load_app(build(diagnostics=True) / "classpad/physchem_final.py")
        FileSystem().attach(app)
        app.update(no_drawing())
        output = []
        with patch.dict(app, poll_events=lambda: [], show_text=lambda *args: output.append(args)):
            app["hh3_compare"]()
        self.assertEqual(len(output), 3)
        self.assertTrue(all("Median gesamt" in body for _, body in output))
        index = app["MEDIA_INDEX"]
        with patch.dict(app, poll_events=lambda: [event(gint.KEY_EXIT)], show_text=lambda *args: None):
            app["hh3_compare"]()
        self.assertIs(app["MEDIA_INDEX"], index)
        self.assertFalse(app["HH3_ACTIVE"])
        frames = []
        with patch.dict(app, poll_events=lambda: [], render_media_page=lambda *args: frames.append(args), show_text=lambda *args: None):
            app["hh3_swaps"]()
        self.assertEqual(len(frames), 20)
        choices = iter((4, 5))
        with patch.dict(app, select_list=lambda *args: next(choices), hh3_test=lambda: None):
            app["run"]()

    def test_diagnostic_errors_restore_bindings(self):
        app = load_app(build(diagnostics=True) / "classpad/physchem_final.py")
        FileSystem().attach(app)
        app.update(no_drawing())
        original = app["MEDIA_INDEX"]
        errors = []
        def fail(*args): raise OSError("device test")
        with patch.dict(app, poll_events=lambda: [], legacy_render_media_page=fail,
                        show_text=lambda *args: errors.append(args)):
            app["hh3_compare"]()
        self.assertIs(app["MEDIA_INDEX"], original)
        self.assertFalse(app["HH3_ACTIVE"])
        self.assertEqual(errors[0][0], "HH3-Testfehler")

    def test_diagnostic_menu_handles_missing_directory(self):
        app = load_app(build(diagnostics=True) / "classpad/physchem_final.py")
        FileSystem(contents={"bilder.dat": None}).attach(app)
        app.update(no_drawing())
        choices = iter((0, 2))
        errors = []
        with patch.dict(app, select_list=lambda *args: next(choices),
                        show_text=lambda *args: errors.append(args)):
            app["hh3_test"]()
        self.assertEqual(errors[0][0], "HH3-Testfehler")
        self.assertIn("Verzeichnis", errors[0][1])


if __name__ == "__main__":
    unittest.main()
