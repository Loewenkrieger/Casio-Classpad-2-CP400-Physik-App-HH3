"""Reference-only navigation tests; run with Python -B, without a native build."""
import copy
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile
import unittest

sys.dont_write_bytecode = True
NATIVE = Path(__file__).resolve().parents[1]
PROJECT = NATIVE.parent
TEMP = NATIVE / "build/tmp"
os.environ.update(TEMP=str(TEMP), TMP=str(TEMP), PYTHONPYCACHEPREFIX=str(TEMP))
tempfile.tempdir = str(TEMP)
spec = importlib.util.spec_from_file_location("navigation_generator", NATIVE / "tools/generate_navigation.py")
nav = importlib.util.module_from_spec(spec)
spec.loader.exec_module(nav)


def fnv(data):
    value = 0x811c9dc5
    for byte in data:
        value = (value ^ byte) * 0x1000193 & 0xffffffff
    return value


def decode(raw):
    """Independent parser for the actual fixed-width bytes, including padding."""
    def value(position):
        tag = raw[position:position + 1]
        end = raw.index(b":", position + 1)
        number = int(raw[position + 1:end])
        position = end + 1
        if tag == b"i":
            return number, position
        if tag == b"s":
            if number < 0 or position + number > len(raw):
                raise ValueError("Truncated string")
            return raw[position:position + number].decode("ascii"), position + number
        if tag != b"t" or not 0 <= number <= 32:
            raise ValueError("Invalid record")
        items = []
        for _ in range(number):
            item, position = value(position)
            items.append(item)
        return tuple(items), position
    result, end = value(0)
    if raw[end:].strip(b" "):
        raise ValueError("Trailing record data")
    return result


class NavigationDataTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        TEMP.mkdir(parents=True, exist_ok=True)
        cls.scratch = tempfile.TemporaryDirectory(prefix="navigation-tests-", dir=TEMP)
        cls.template = Path(cls.scratch.name) / "template"
        (cls.template / "package").mkdir(parents=True)
        for name in ("catalog-manifest.json", "knowledge-manifest.json"):
            shutil.copyfile(NATIVE / "build" / name, cls.template / name)
        for name in ("wissen.txt", "bilder.dat"):
            shutil.copyfile(NATIVE / "build/package" / name, cls.template / "package" / name)
        cls.source = nav.read_json(nav.SOURCE)
        cls.knowledge = nav.read_json(NATIVE / "sources/knowledge.json")
        cls.titles, cls.bodies, cls.chapters, cls.legacy = nav.load_legacy()
        cls.catalog = nav.read_json(cls.template / "catalog-manifest.json")
        cls.prefix = (cls.template / "package/wissen.txt").read_bytes()[:cls.catalog["total_bytes"]]
        cls.protected = [PROJECT / "wissen.txt", PROJECT / "bilder.dat", PROJECT / "physchem_final.py",
                         NATIVE / "sources/knowledge.json"]
        for directory in (NATIVE / "dist", NATIVE / "classpad"):
            if directory.exists():
                cls.protected.extend(p for p in directory.rglob("*") if p.is_file())
        cls.frozen_hashes = {p: hashlib.sha256(p.read_bytes()).hexdigest() for p in cls.protected}

    @classmethod
    def tearDownClass(cls):
        try:
            for path, digest in cls.frozen_hashes.items():
                if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
                    raise AssertionError(f"Protected input/release changed: {path}")
        finally:
            cls.scratch.cleanup()

    def setUp(self):
        self.fixture = Path(tempfile.mkdtemp(prefix="case-", dir=self.scratch.name))
        self.build = self.fixture / "build"
        shutil.copytree(self.template, self.build)
        self.header = self.fixture / "navigation_index.inc"
        self.source_path = self.fixture / "navigation.json"
        self.source_path.write_bytes(nav.canonical(self.source))
        self.manifest = nav.generate(self.source_path, self.build, self.header)
        self.data_path = self.build / "package/wissen.txt"
        self.data = self.data_path.read_bytes()

    def assert_raw_tree(self, data):
        manifest = self.manifest
        count, directory = manifest["node_count"], manifest["directory_start"]
        self.assertLessEqual(count, 2048)
        self.assertEqual(manifest["roots"][:3], [0, 1, 2])
        self.assertEqual(len(manifest["roots"]), 4)
        self.assertGreater(manifest["roots"][3], 2)
        self.assertEqual(data[manifest["offset"]:directory],
                         f"PCNAV4 {manifest['identity']}\n".encode("ascii"))
        self.assertEqual(directory, manifest["offset"] + len(b"PCNAV4 ") + 24 + 1)
        cursor = directory + count * 96
        rows = []
        for i in range(count):
            start, size, width, crc, metadata_crc = decode(data[directory + i * 96:directory + (i + 1) * 96])
            node = manifest["nodes"][i]
            self.assertEqual(i, node["id"])
            self.assertEqual((start, size, width, crc, metadata_crc),
                tuple(node[key] for key in ("start", "count", "width", "checksum", "metadata_checksum")))
            self.assertEqual(metadata_crc, fnv(struct.pack(">IIII", start, size, width, crc)))
            self.assertEqual(start, cursor)
            self.assertTrue(1 <= size <= 256)
            self.assertTrue(1 <= width <= 512)
            self.assertLessEqual(size * width, 131072)
            block = data[start:start + size * width]
            self.assertEqual(len(block), size * width)
            self.assertEqual(crc, fnv(block))
            children = [decode(block[j * width:(j + 1) * width]) for j in range(size)]
            self.assertEqual(width, max(len(nav.encoded(child)) for child in children))
            self.assertEqual(children, [(r["label"], r["kind"], r["target"]) for r in node["children"]])
            self.assertEqual(len({child[0] for child in children}), size)
            self.assertEqual(len({child[1:] for child in children}), size)
            self.assertEqual(len({kind == 0 for _, kind, _ in children}), 1)
            for title, kind, target in children:
                self.assertTrue(0 < len(title) <= 96 and all(32 <= ord(c) < 127 for c in title))
                self.assertIn(kind, (0, 1, 2))
                self.assertTrue(i < target < count if kind == 0 else
                                0 <= target < (514 if kind == 1 else 32))
            rows.append(children)
            cursor += size * width
        self.assertEqual(cursor, len(data))
        self.assertEqual(manifest["total_bytes"], len(data))
        return rows

    def test_explicit_complete_stable_source(self):
        nav.validate_source(self.source, self.knowledge)
        refs = {f"legacy:{i}" for i in range(514)} | {f"native:{r['id']}" for r in self.knowledge}
        self.assertEqual(refs, set(self.source["articles"]))
        self.assertEqual(len(refs), 546)
        self.assertEqual(self.manifest["unique"], 546)
        self.assertEqual(set(self.manifest["unique_refs"]), refs)
        self.assertEqual(self.manifest["native_ids"], [r["id"] for r in self.knowledge])
        # Mappings contain stable IDs, no physical byte offsets and no copied bodies.
        for mapping in self.source["articles"].values():
            self.assertLessEqual(set(mapping), {"topics", "group", "quantities", "label"})

    def test_raw_directory_blocks_checksums_and_padding(self):
        self.assert_raw_tree(self.data)

    def test_root_order_depth_homogeneity_and_reachability(self):
        rows = self.assert_raw_tree(self.data)
        seen = set()
        for index, root in enumerate(self.manifest["roots"]):
            if index < 3:
                self.assertEqual([title for title, _, _ in rows[root]], list(nav.ROOT_LABELS))
                self.assertEqual([target for _, _, target in rows[root]], list(range(3 + root * 15, 18 + root * 15)))
            stack = [(root, 0)]
            while stack:
                node, depth = stack.pop()
                self.assertNotIn(node, seen)
                seen.add(node)
                children = rows[node]
                self.assertLessEqual(depth, 2)
                self.assertEqual(children[0][1] == 0, depth < 2)
                if depth < 2:
                    stack.extend((target, depth + 1) for _, _, target in children)
        self.assertEqual(seen, set(range(self.manifest["node_count"])))
        self.assertTrue(any({c[1] for c in children} == {1, 2} for children in rows))

    def test_exact_coverage_and_mode_stats_from_raw_leaves(self):
        rows = self.assert_raw_tree(self.data)
        for index, mode in enumerate(nav.SEARCH_MODES):
            root = self.manifest["roots"][index]
            leaves = [child for _, _, topic in rows[root] for _, _, group in rows[topic] for child in rows[group]]
            legacy = {target for _, kind, target in leaves if kind == 1}
            native = {target for _, kind, target in leaves if kind == 2}
            self.assertEqual(legacy, set(range(146)) if index == 0 else set(range(146, 514)))
            self.assertEqual(native, set(range(32)))
            stats = dict(links=len(leaves), legacy_unique=len(legacy), native_unique=32,
                         unique=len(legacy) + 32)
            self.assertEqual(self.manifest["mode_stats"][index], stats)
            refs = {f"legacy:{target}" if kind == 1 else f"native:{self.knowledge[target]['id']}"
                    for _, kind, target in leaves}
            self.assertEqual(set(self.manifest["modes"][mode]["unique_refs"]), refs)
            self.assertEqual(self.manifest["modes"][mode]["links"], len(leaves))
        self.assertEqual([s["unique"] for s in self.manifest["mode_stats"]], [178, 400, 400, 400])
        formulas = {f"legacy:{i}" for i in range(146, 514)}
        self.assertEqual(formulas - set(self.legacy["ByQuantity"]["unique_refs"]),
                         {f"legacy:{i}" for i in range(476, 514)})
        self.assertEqual(formulas - set(self.legacy["ByTopic"]["unique_refs"]),
                         {f"legacy:{i}" for i in (162, 238, 280, 281, 322, 352, 403)})

    def test_entire_catalog_body_media_prefix_unchanged(self):
        self.assertEqual(self.data[:len(self.prefix)], self.prefix)
        self.assertEqual(self.manifest["offset"], self.catalog["total_bytes"])
        self.assertEqual(self.manifest["prefix_sha256"], hashlib.sha256(self.prefix).hexdigest())
        for i, ((start, length), body) in enumerate(zip(self.chapters, self.bodies)):
            self.assertEqual(self.data[start:start + length], body.encode("ascii"))
            self.assertEqual(self.manifest["legacy_bodies_sha256"][i], hashlib.sha256(body.encode("ascii")).hexdigest())
        for entry, native in zip(nav.read_json(self.build / "knowledge-manifest.json")["entries"], self.knowledge):
            body = (native["title"] + "\n\n" + native["text"] + "\n").encode("ascii")
            self.assertEqual(self.data[entry["start"]:entry["start"] + entry["length"]], body)
        self.assertEqual((self.build / "package/bilder.dat").read_bytes(), (PROJECT / "bilder.dat").read_bytes())
        self.assertEqual(self.knowledge, nav.read_json(NATIVE / "sources/knowledge.json"))

    def test_header_exact_contract(self):
        header = self.header.read_text(encoding="ascii")
        self.assertIn(f"inline constexpr uint32_t navigationOffset={len(self.prefix)};", header)
        self.assertIn('inline constexpr const char* navigationMarker="PCNAV4 ' + self.manifest["identity"] + '\\n";', header)
        self.assertIn(f"inline constexpr Table navigationDirectory{{0,{self.manifest['node_count']},"
                      f"{self.manifest['directory_start']},96}};", header)
        self.assertIn(f"inline constexpr uint32_t navigationAlphabetRoot={self.manifest['roots'][3]};", header)
        self.assertEqual(len(re.findall(r"inline constexpr", header)), 4)

    def test_alphabet_letters_quantities_and_exact_per_quantity_references(self):
        rows = self.assert_raw_tree(self.data)
        root = self.manifest["roots"][3]
        letters = [title for title, _, _ in rows[root]]
        quantities = self.source["quantities"]
        self.assertEqual(len(quantities), 142)
        self.assertEqual(letters, sorted({name[0].upper() for name in quantities.values()}))
        self.assertEqual(self.manifest["alphabet_letters"], letters)
        self.assertEqual(self.manifest["alphabet_quantities"], 142)
        ids = {name: key for key, name in quantities.items()}
        native = {record["id"]: i for i, record in enumerate(self.knowledge)}
        seen = set()
        for initial, _, branch in rows[root]:
            names = [title for title, _, _ in rows[branch]]
            self.assertEqual(names, sorted(names, key=str.casefold))
            for name, _, node in rows[branch]:
                self.assertEqual(name[0].upper(), initial)
                self.assertNotIn(name, seen)
                seen.add(name)
                quantity = ids[name]
                expected = set()
                for ref, mapping in self.source["articles"].items():
                    if quantity in mapping["quantities"]:
                        kind, key = ref.split(":", 1)
                        expected.add((1, int(key)) if kind == "legacy" else (2, native[key]))
                actual = {(kind, target) for _, kind, target in rows[node]}
                self.assertEqual(actual, expected, name)
                self.assertEqual(len(rows[node]), len(actual), "No copies for cross-topic links")
        self.assertEqual(seen, set(quantities.values()))
        for initial, key in (("D", "Delta_f"), ("O", "omega"), ("Q", "Q_charge"), ("Q", "Q_quality"),
                             ("R", "rho_mass"), ("R", "rho_el"), ("A", "a"), ("A", "A_s")):
            self.assertEqual(quantities[key][0].upper(), initial)

    def test_teacher_trees_match_frozen_beta4_structure(self):
        baseline = PROJECT / "native-backups/2026-11-22_before_alphabet_beta5/navigation-manifest.json"
        previous = json.loads(baseline.read_text(encoding="ascii"))
        old = previous["nodes"]
        self.assertEqual(self.manifest["roots"][3], len(old))
        for before, after in zip(old, self.manifest["nodes"]):
            self.assertEqual({k: before[k] for k in ("id", "title", "mode", "topic", "group", "children")},
                             {k: after[k] for k in ("id", "title", "mode", "topic", "group", "children")})
        self.assertEqual(self.manifest["mode_stats"][:3], previous["mode_stats"])

    def test_all_source_folders_and_file_metadata_are_exact(self):
        root = Path(self.source["teacher_root"])
        for topic in self.source["topics"]:
            self.assertTrue(Path(topic["source_path"]).is_dir(), topic)
            self.assertEqual(Path(topic["source_path"]).parent, root)
        for group in self.source["groups"].values():
            for name in group["source_files"]:
                self.assertTrue((root / name).is_file(), name)
        self.assertTrue(any("07-Elektromagnetische-Induktion/" in p for p in self.source["groups"]["ac"]["source_files"]))
        self.assertTrue(any("09-Schwingungen/" in p for p in self.source["groups"]["ac"]["source_files"]))
        self.assertIn("12-Grundlagen-Atomphysik/AB_Potenzialtopf.pdf", self.source["groups"]["well"]["source_files"])

    def test_crucial_teacher_and_body_assignments(self):
        articles = self.source["articles"]
        self.assertEqual(articles["legacy:115"]["group"], "photoelectric")
        self.assertEqual(articles["legacy:119"]["group"], "compton")
        self.assertEqual(articles["legacy:219"]["quantities"], ["I_eff"])
        self.assertTrue(articles["legacy:219"]["label"].startswith("I_eff - "))
        self.assertIn("I_eff = U_eff / Z", self.bodies[219])
        for i in (22, 23, 24, 25, 26, 27, 28, 29, 106, 107, 108, 109, 110):
            self.assertEqual(articles[f"legacy:{i}"]["topics"][:2], ["07", "09"])
        for i in (134, 135, 136, 321, 453, 454, 455, 456, 458, 461, 472):
            self.assertEqual(articles[f"legacy:{i}"]["topics"][0], "12")
            self.assertIn("11", articles[f"legacy:{i}"]["topics"])
        for i in (156, 202, 398, 385, 386, 387, 368, 369):
            self.assertEqual(articles[f"legacy:{i}"]["topics"][0], "06")
            self.assertIn("05", articles[f"legacy:{i}"]["topics"])
        for i in (147, 148, 149, 330, 331, 388):
            self.assertEqual(articles[f"legacy:{i}"]["topics"][0], "other")
        for i in (150, 249, 260, 263, 273, 363, 367, 391, 392):
            self.assertEqual(articles[f"legacy:{i}"]["topics"][0], "04")
        for i in (334, 420, 439, 440, 446, 447, 448, 449, 450, 451):
            self.assertEqual(articles[f"legacy:{i}"]["topics"][:2], ["10", "11"])
        for i in (197, 204, 319, 320, 357, 395, 396, 399, 400, 503, 504, 505, 506, 507, 508):
            self.assertIn("08", articles[f"legacy:{i}"]["topics"])
        for i in range(476, 514):
            self.assertEqual(articles[f"legacy:{i}"]["topics"][0], "09")
        for mapping in articles.values():
            if "abi" in mapping["topics"]:
                self.assertTrue(set(mapping["topics"]) & {"10", "11", "12"})
        for ref in ("legacy:9", "legacy:10", "legacy:36", "legacy:38", "legacy:40", "native:crossfield"):
            self.assertIn("other", articles[ref]["topics"])

    def test_all_38_recovered_quantity_assignments(self):
        expected = ["omega0", "f0", "T", "L", "C", "Q_charge", "I", "I0", "U_C", "Q_charge",
                    "E_cap", "E_mag", "E_ges", "I0", "U0", "P_R", "Q_quality", "Delta_f", "T", "f",
                    "omega", "x", "v", "v_max", "a", "a_max", "F_D", "T", "D_spring", "m", "T", "l",
                    "g", "E_ges", "E_sp", "E_kin", "A_s", "phase"]
        self.assertEqual(len(expected), 38)
        for i, quantity in enumerate(expected, 476):
            self.assertEqual(self.source["articles"][f"legacy:{i}"]["quantities"], [quantity])

    def test_disambiguated_quantities_and_native_legacy_merging(self):
        a, quantities = self.source["articles"], self.source["quantities"]
        for name in quantities.values():
            self.assertIn(" - ", name)
            self.assertFalse(name.startswith("Neu"))
        for i in (261, 262, 263, 481, 485):
            self.assertEqual(a[f"legacy:{i}"]["quantities"], ["Q_charge"])
        self.assertEqual(a["legacy:492"]["quantities"], ["Q_quality"])
        self.assertEqual(a["legacy:391"]["quantities"], ["rho_el"])
        self.assertEqual(a["legacy:393"]["quantities"], ["rho_probability"])
        self.assertIn("rho_mass", a["native:millikan"]["quantities"])
        self.assertEqual(a["legacy:355"]["quantities"], ["h_height"])
        self.assertEqual(a["legacy:474"]["quantities"], ["h_P"])
        self.assertIn("h_P", a["native:planckfit"]["quantities"])
        self.assertIn("Q_charge", a["native:capacitor"]["quantities"])
        self.assertIn("I0", a["legacy:483"]["quantities"])
        self.assertEqual(a["legacy:491"]["label"], "P_R - Verlustleistung im RLC-Kreis")
        self.assertEqual(a["legacy:491"]["quantities"], ["P_R"])
        self.assertIn("R - Verlustleistung", self.bodies[491])
        self.assertIn("P_R = I_eff^2 * R", self.bodies[491])
        # The same label for two different old chapter IDs is visibly qualified.
        nodes = self.manifest["nodes"]
        leaves = [n["children"] for n in nodes if n["mode"] == "ByQuantity" and n["topic"] == "09" and n["group"] == "T"]
        self.assertEqual(len(leaves), 1)
        titles = {r["target"]: r["label"] for r in leaves[0] if r["kind"] == 1}
        self.assertNotEqual(titles[280], titles[503])
        self.assertIn("legacy:280", titles[280])

    def test_idempotent_bytes_header_mtime_and_canonical_identity(self):
        outputs = (self.header, self.data_path, self.build / "navigation-manifest.json")
        original = {path: path.read_bytes() for path in outputs}
        for path in outputs:
            os.utime(path, ns=(1600000000000000000, 1600000000000000000))
        times = {path: path.stat().st_mtime_ns for path in outputs}
        reordered = copy.deepcopy(self.source)
        reordered["articles"] = dict(reversed(list(reordered["articles"].items())))
        self.source_path.write_text(json.dumps(reordered, ensure_ascii=True), encoding="ascii")
        self.assertEqual(nav.generate(self.source_path, self.build, self.header), self.manifest)
        for path in outputs:
            self.assertEqual(path.read_bytes(), original[path])
            self.assertEqual(path.stat().st_mtime_ns, times[path])
        self.data_path.write_bytes(self.data + b"stale-navigation-tail")
        nav.generate(self.source_path, self.build, self.header)
        self.assertEqual(self.data_path.read_bytes(), self.data)

    def test_invalid_source_cases_fail_before_writing(self):
        mutations = {
            "missing legacy": lambda s: s["articles"].pop("legacy:403"),
            "missing native": lambda s: s["articles"].pop("native:motion"),
            "unknown stable ID": lambda s: s["articles"].update({"legacy:514": s["articles"]["legacy:513"]}),
            "missing quantities": lambda s: s["articles"]["legacy:491"].update(quantities=[]),
            "unknown quantity": lambda s: s["articles"]["native:motion"].update(quantities=["unknown"]),
            "unknown group": lambda s: s["articles"]["legacy:0"].update(group="unknown"),
            "empty topics": lambda s: s["articles"]["legacy:0"].update(topics=[]),
            "duplicate topics": lambda s: s["articles"]["legacy:0"].update(topics=["other", "other"]),
            "unknown topic": lambda s: s["articles"]["legacy:0"].update(topics=["13"]),
            "wrong order": lambda s: s["topics"].reverse(),
            "long label": lambda s: s["articles"]["legacy:491"].update(label="x" * 97),
            "non ASCII": lambda s: s["articles"]["legacy:491"].update(label="P\u00e4"),
            "control character": lambda s: s["articles"]["legacy:491"].update(label="P\nR"),
            "quantity no clear name": lambda s: s["quantities"].update(P_R="P_R"),
            "unmerged prefix": lambda s: s["articles"]["legacy:491"].update(label="Neu: Verlustleistung"),
        }
        outputs = (self.header, self.data_path, self.build / "navigation-manifest.json")
        before = {path: path.read_bytes() for path in outputs}
        for name, mutation in mutations.items():
            with self.subTest(name=name):
                source = copy.deepcopy(self.source)
                mutation(source)
                self.source_path.write_bytes(nav.canonical(source))
                with self.assertRaises(ValueError):
                    nav.generate(self.source_path, self.build, self.header)
                self.assertEqual({path: path.read_bytes() for path in outputs}, before)
        self.source_path.write_text('{"version":4,"version":4}', encoding="ascii")
        with self.assertRaisesRegex(ValueError, "Duplicate JSON key"):
            nav.read_json(self.source_path)

    def test_incomplete_mismatched_or_corrupt_staging_rejected(self):
        original_catalog = nav.read_json(self.build / "catalog-manifest.json")
        supplement_path = self.build / "knowledge-manifest.json"
        original_supplement = supplement_path.read_bytes()
        old_header = self.header.read_bytes()
        native_start = nav.read_json(supplement_path)["entries"][0]["start"]
        for position in (10, native_start, original_catalog["offset"], original_catalog["categories"][0]["start"]):
            with self.subTest(corrupt_position=position):
                data = bytearray(self.data)
                data[position] ^= 1
                self.data_path.write_bytes(data)
                with self.assertRaises(ValueError):
                    nav.generate(self.source_path, self.build, self.header)
                self.assertEqual(self.header.read_bytes(), old_header)
        self.data_path.write_bytes(self.prefix[:-1])
        with self.assertRaisesRegex(ValueError, "truncated"):
            nav.generate(self.source_path, self.build, self.header)
        self.data_path.write_bytes(self.data)
        supplement = json.loads(original_supplement)
        supplement["entries"][0]["id"] = "wrong"
        supplement_path.write_bytes(nav.canonical(supplement))
        with self.assertRaisesRegex(ValueError, "ID/body mismatch"):
            nav.generate(self.source_path, self.build, self.header)
        supplement_path.write_bytes(original_supplement)
        catalog_path = self.build / "catalog-manifest.json"
        for field, value in (("total_bytes", len(self.data) + 1), ("total_bytes", len(self.prefix) - 1)):
            catalog = copy.deepcopy(original_catalog)
            catalog[field] = value
            catalog_path.write_bytes(nav.canonical(catalog))
            with self.assertRaises(ValueError):
                nav.generate(self.source_path, self.build, self.header)

    def test_raw_checks_detect_child_and_directory_corruption(self):
        for offset in (self.manifest["nodes"][0]["start"], self.manifest["directory_start"] + 6):
            data = bytearray(self.data)
            data[offset] ^= 1
            with self.subTest(offset=offset), self.assertRaises((AssertionError, ValueError)):
                self.assert_raw_tree(data)

    def test_output_paths_cannot_overwrite_sources_or_releases(self):
        for header in (PROJECT / "wissen.txt", PROJECT / "bilder.dat", nav.SOURCE,
                       NATIVE / "dist/classpad/wissen.txt", self.source_path,
                       self.build / "catalog-manifest.json", self.build / "package/bilder.dat"):
            with self.subTest(header=header), self.assertRaises(ValueError):
                nav.generate(self.source_path, self.build, header)
        with self.assertRaisesRegex(ValueError, "native/build"):
            nav.generate(self.source_path, NATIVE / "dist/classpad", self.header)


if __name__ == "__main__":
    unittest.main()
