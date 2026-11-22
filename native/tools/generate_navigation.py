"""Append the teacher-aligned, reference-only PCNAV4 tree after the catalog.

Run with Python -B after generate_catalog.py. --build-dir and --header allow
isolated verification without touching the staged package or generated headers.
"""
import argparse
from collections import Counter, deque
import hashlib
import json
from pathlib import Path
import struct
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
PROJECT = ROOT.parent
SOURCE = ROOT / "sources/navigation.json"
MODES = ("Theory", "ByQuantity", "ByTopic")
SEARCH_MODES = (*MODES, "Alphabet")
MODE_TITLES = ("Theorie", "Formeln nach Groessen", "Formeln nach Themen")
ROOT_LABELS = (
    "00 Kinematik", "01 Erhaltungssaetze", "02 Kraefte", "03 Bewegung / Wurf",
    "04 Elektrische Felder", "05 Magnetisches Feld", "06 Teilchen in Feldern",
    "07 Elektromagn. Induktion", "08 Modellbildung / Simulation", "09 Schwingungen",
    "10 Wellen", "11 Quantenphysik", "12 Grundlagen Atomphysik",
    "Abiturvorbereitung", "Sonstiges",
)
FROZEN = {
    "wissen.txt": "4ef9d73f2eb2d33dc50973122d48ce86596234a250d90277452cde915a6b17fb",
    "bilder.dat": "8fd93e7e62d7e61d85decd1273639dedf607a8bfcb52240ac8ed530ea0a529a3",
}
DIRECTORY_WIDTH = 96


def require(condition, message):
    if not condition:
        raise ValueError(message)


def canonical(value):
    return json.dumps(value, sort_keys=True, ensure_ascii=True,
                      separators=(",", ":")).encode("ascii")


def checksum(data):
    result = 2166136261
    for byte in data:
        result = ((result ^ byte) * 16777619) & 0xffffffff
    return result


def encoded(value):
    if type(value) is int:
        return f"i{value}:".encode("ascii")
    if isinstance(value, str):
        raw = value.encode("ascii")
        return f"s{len(raw)}:".encode("ascii") + raw
    require(isinstance(value, tuple), "Only integer/string/tuple records are supported")
    return f"t{len(value)}:".encode("ascii") + b"".join(map(encoded, value))


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, f"Duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path):
    return json.loads(Path(path).read_text(encoding="ascii"), object_pairs_hook=unique_object)


def label(value):
    require(isinstance(value, str) and 0 < len(value) <= 96 and
            all(32 <= ord(c) < 127 for c in value), f"Invalid navigation label: {value!r}")
    require(not value.lower().startswith("neu"), f"Unmerged navigation label: {value}")
    return value


def load_legacy():
    for name, digest in FROZEN.items():
        require(hashlib.sha256((PROJECT / name).read_bytes()).hexdigest() == digest,
                f"Frozen source changed: {name}")
    sys.path.insert(0, str(PROJECT / "tests"))
    from support import load_app
    app = load_app()  # support removes run() from the AST; no GUI is executed.
    chapters = list(app["CHAPTERS"])
    require(len(chapters) == 514, "Expected 514 legacy chapter IDs")
    entries, sections, modes = (list(app[key]) for key in ("ENTRIES", "SECTIONS", "MODES"))
    titles = {target: title for title, target in entries[:146]}
    bodies = [app["load_chapter"](i) for i in range(514)]
    for i in range(146, 514):
        lines = bodies[i].splitlines()
        require(len(lines) > 4 and lines[0] == "Groesse:", f"Malformed legacy formula {i}")
        titles[i] = lines[1]
    coverage = {}
    for mode, (_, start, count) in zip(MODES, modes):
        refs = [f"legacy:{target}" for _, first, size in sections[start:start + count]
                for _, target in entries[first:first + size]]
        coverage[mode] = dict(links=len(refs), unique=len(set(refs)),
                              unique_refs=sorted(set(refs)))
    require([(v["links"], v["unique"]) for v in coverage.values()] ==
            [(146, 146), (330, 330), (364, 361)], "Legacy browser coverage changed")
    return titles, bodies, chapters, coverage


def validate_source(source, knowledge):
    require(source.get("version") == 4, "Expected navigation source version 4")
    require(source.get("frozen") == FROZEN, "Frozen source identities do not match")
    topics = source["topics"]
    require([t["label"] for t in topics] == list(ROOT_LABELS), "Teacher topic order/labels changed")
    topic_ids = [t["id"] for t in topics]
    require(len(set(topic_ids)) == 15, "Duplicate topic IDs")
    for topic in topics:
        label(topic["label"])
        require(bool(topic.get("source_path")), "Missing exact teacher folder path")
    for group in source["groups"].values():
        label(group["label"])
        require(isinstance(group.get("source_files"), list), "Missing group source metadata")
    for quantity in source["quantities"].values():
        label(quantity)
        require(" - " in quantity, f"Quantity needs symbol and clear name: {quantity}")
    require(len(set(source["quantities"].values())) == len(source["quantities"]),
            "Different quantities must have distinct labels")
    require(len(knowledge) == 32 and len({r["id"] for r in knowledge}) == 32,
            "Expected 32 unique native article IDs")
    expected = {f"legacy:{i}" for i in range(514)} | {f"native:{r['id']}" for r in knowledge}
    mappings = source["articles"]
    require(set(mappings) == expected,
            f"Explicit article mapping mismatch; missing={sorted(expected - set(mappings))}; "
            f"unknown={sorted(set(mappings) - expected)}")
    used_quantities, used_groups = set(), set()
    for ref, mapping in mappings.items():
        links, quantities = mapping["topics"], mapping["quantities"]
        require(isinstance(links, list) and links and len(links) == len(set(links)) and
                set(links) <= set(topic_ids), f"Invalid topics for {ref}")
        require(mapping["group"] in source["groups"], f"Unknown group for {ref}")
        require(isinstance(quantities, list) and len(quantities) == len(set(quantities)) and
                set(quantities) <= set(source["quantities"]), f"Invalid quantities for {ref}")
        is_formula = ref.startswith("native:") or int(ref.split(":")[1]) >= 146
        require(bool(quantities) == is_formula, f"Missing/unexpected formula quantities for {ref}")
        if "label" in mapping:
            label(mapping["label"])
        used_quantities.update(quantities)
        used_groups.add(mapping["group"])
    require(used_quantities == set(source["quantities"]), "Unused quantity definitions")
    require(used_groups == set(source["groups"]), "Unused subgroup definitions")


def article_rows(leaves):
    names = Counter(leaf["label"] for leaf in leaves.values())
    for ref, leaf in leaves.items():
        if names[leaf["label"]] > 1:
            suffix = f" [{ref}]"
            leaf["label"] = label(leaf["label"][:96 - len(suffix)] + suffix)
    return sorted(leaves.values(), key=lambda r: (r["label"].casefold(), r["kind"], r["target"]))


def alphabet_tree(source, titles, native):
    buckets = {quantity: {} for quantity in source["quantities"]}
    for ref, mapping in source["articles"].items():
        kind = 1 if ref.startswith("legacy:") else 2
        key = ref.split(":", 1)[1]
        target = int(key) if kind == 1 else native[key][0]
        title = mapping.get("label", titles[target] if kind == 1 else native[key][1]["title"])
        for quantity in mapping["quantities"]:
            buckets[quantity][ref] = dict(label=label(title), kind=kind, target=target)
    letters = {}
    for quantity, name in sorted(source["quantities"].items(), key=lambda item: (item[1].casefold(), item[0])):
        initial = name[0].upper()
        require("A" <= initial <= "Z", f"Quantity needs an alphabetic initial: {name}")
        require(bool(buckets[quantity]), f"Empty alphabet quantity: {quantity}")
        letters.setdefault(initial, []).append(dict(title=name, mode="Alphabet", topic=initial,
            group=quantity, children=article_rows(buckets[quantity])))
    return dict(title="Formeln A-Z", mode="Alphabet", topic=None, group=None, children=[
        dict(title=initial, mode="Alphabet", topic=initial, group=None, children=groups)
        for initial, groups in sorted(letters.items())])


def build_tree(source, titles, knowledge):
    native = {r["id"]: (i, r) for i, r in enumerate(knowledge)}
    roots = [dict(title=title, mode=mode, topic=None, group=None, children=[])
             for title, mode in zip(MODE_TITLES, MODES)]
    for root in roots:
        for topic in source["topics"]:
            buckets = {}
            for ref, mapping in source["articles"].items():
                kind = 1 if ref.startswith("legacy:") else 2
                key = ref.split(":", 1)[1]
                target = int(key) if kind == 1 else native[key][0]
                theory = kind == 1 and target < 146
                if topic["id"] not in mapping["topics"] or (kind == 1 and
                        theory != (root["mode"] == "Theory")):
                    continue
                title = mapping.get("label", titles[target] if kind == 1 else native[key][1]["title"])
                groups = mapping["quantities"] if root["mode"] == "ByQuantity" else [mapping["group"]]
                for group in groups:
                    buckets.setdefault(group, {})[ref] = dict(label=label(title), kind=kind, target=target)
            require(bool(buckets), f"Empty topic: {root['mode']}/{topic['id']}")
            topic_node = dict(title=topic["label"], mode=root["mode"], topic=topic["id"],
                              group=None, children=[])
            for group, leaves in buckets.items():
                group_label = (source["quantities"][group] if root["mode"] == "ByQuantity"
                               else source["groups"][group]["label"])
                children = article_rows(leaves)
                topic_node["children"].append(dict(title=group_label, mode=root["mode"],
                    topic=topic["id"], group=group, children=children))
            topic_node["children"].sort(key=lambda n: (n["title"].casefold(), n["group"]))
            root["children"].append(topic_node)

    # Assign all roots first, then visit breadth first. Menu edges always point forward.
    nodes, queue = [], deque(roots)
    # Append the new tree after the existing trees, keeping their node IDs stable.
    for pending in (queue, deque([alphabet_tree(source, titles, native)])):
        while pending:
            node = pending.popleft()
            node["id"] = len(nodes)
            nodes.append(node)
            pending.extend(child for child in node["children"] if "children" in child)
    require(3 <= len(nodes) <= 2048, "Navigation exceeds node budget")
    for node in nodes:
        children = [dict(label=child["title"], kind=0, target=child["id"])
                    if "children" in child else child for child in node["children"]]
        node["children"] = children
        require(1 <= len(children) <= 256, f"Child count out of range: {node['title']}")
        require(len({c["label"] for c in children}) == len(children), "Duplicate sibling labels")
        require(len({(c["kind"], c["target"]) for c in children}) == len(children), "Duplicate sibling references")
        require(all((c["kind"] == 0) == (children[0]["kind"] == 0) for c in children), "Mixed menu/article children")
        for child in children:
            label(child["label"])
            if child["kind"] == 0:
                require(node["id"] < child["target"] < len(nodes), "Non-forward menu edge")
    return nodes


def catalog_prefix(build_dir, knowledge):
    catalog = read_json(build_dir / "catalog-manifest.json")
    supplement = read_json(build_dir / "knowledge-manifest.json")
    data = (build_dir / "package/wissen.txt").read_bytes()
    total = catalog["total_bytes"]
    require(type(total) is int and 0 < total <= len(data), "Catalog package truncated")
    base = (PROJECT / "wissen.txt").read_bytes()
    require(data[:len(base)] == base, "Frozen legacy prefix changed in package")
    require(supplement["base_bytes"] == len(base) and supplement["base_sha256"] == FROZEN["wissen.txt"],
            "Knowledge manifest has wrong frozen prefix")
    identity = hashlib.sha256(json.dumps(knowledge, sort_keys=True, ensure_ascii=True).encode("ascii")).hexdigest()[:24]
    require(supplement["identity"] == identity, "Knowledge manifest does not match native sources")
    marker = f"KNEXT3 {identity}\n".encode("ascii")
    require(data[len(base):len(base) + len(marker)] == marker, "Native knowledge marker mismatch")
    cursor = len(base) + len(marker)
    require(len(supplement["entries"]) == len(knowledge), "Native manifest entry count mismatch")
    for record, entry in zip(knowledge, supplement["entries"]):
        body = (record["title"] + "\n\n" + record["text"] + "\n").encode("ascii")
        require(entry == dict(id=record["id"], start=cursor, length=len(body), checksum=checksum(body)),
                f"Native manifest ID/body mismatch: {record['id']}")
        require(data[cursor:cursor + len(body)] == body, f"Native body changed: {record['id']}")
        cursor += len(body)
    require(cursor == supplement["total_bytes"] == catalog["offset"], "Catalog must follow knowledge")
    marker = f"PCCATALOG3 {catalog['identity']}\n".encode("ascii")
    require(data[cursor:cursor + len(marker)] == marker, "Catalog marker mismatch")
    cursor += len(marker)
    require(bool(catalog["categories"]), "Empty catalog")
    for entry in catalog["categories"]:
        count, width = entry["count"], entry["width"]
        require(type(count) is int and type(width) is int and count > 0 and
                0 < width <= 4096 and count * width <= 131072, "Invalid catalog block")
        end = cursor + count * width
        require(entry["start"] == cursor and end <= total and
                checksum(data[cursor:end]) == entry["checksum"], "Catalog block/prefix mismatch")
        cursor = end
    require(cursor == total, "Catalog total_bytes does not end at final block")
    media = build_dir / "package/bilder.dat"
    require(media.exists() and hashlib.sha256(media.read_bytes()).hexdigest() == FROZEN["bilder.dat"],
            "Package media changed or missing")
    return data[:total], catalog


def write_changed(path, data):
    path = Path(path)
    if not path.exists() or path.read_bytes() != data:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)


def generate(source_path=SOURCE, build_dir=ROOT / "build", header=ROOT / "src/navigation_index.inc"):
    build_dir, header = Path(build_dir).resolve(), Path(header).resolve()
    package = build_dir / "package/wissen.txt"
    require(build_dir.is_relative_to(ROOT / "build"), "Build output must stay under native/build")
    require(header == ROOT / "src/navigation_index.inc" or
            header.is_relative_to(ROOT / "build"), "Header output must not overwrite source/release files")
    outputs = (package.resolve(), header, (build_dir / "navigation-manifest.json").resolve())
    inputs = {Path(source_path).resolve(), ROOT / "sources/knowledge.json",
              build_dir / "catalog-manifest.json", build_dir / "knowledge-manifest.json",
              *(PROJECT / name for name in (*FROZEN, "physchem_final.py"))}
    require(len(set(outputs)) == 3 and not set(outputs) & inputs, "Output overlaps an input file")
    require(all(path.is_relative_to(ROOT) for path in outputs), "Output escapes native directory")
    require(header.parent != package.parent, "Header must not overwrite package data")
    source, knowledge = read_json(source_path), read_json(ROOT / "sources/knowledge.json")
    validate_source(source, knowledge)
    titles, bodies, chapters, old_coverage = load_legacy()
    prefix, catalog = catalog_prefix(build_dir, knowledge)
    nodes = build_tree(source, titles, knowledge)
    identity = hashlib.sha256(canonical(dict(source=source, knowledge=knowledge,
        prefix_sha256=hashlib.sha256(prefix).hexdigest(), nodes=nodes))).hexdigest()[:24]
    marker = f"PCNAV4 {identity}\n".encode("ascii")
    directory_start = len(prefix) + len(marker)
    cursor = directory_start + len(nodes) * DIRECTORY_WIDTH
    directory, blocks = [], []
    for node in nodes:
        records = [encoded((c["label"], c["kind"], c["target"])) for c in node["children"]]
        count, width = len(records), max(map(len, records))
        require(0 < width <= 512 and count * width <= 131072, "Navigation block exceeds runtime limits")
        block = b"".join(record.ljust(width, b" ") for record in records)
        crc = checksum(block)
        metadata_crc = checksum(struct.pack(">IIII", cursor, count, width, crc))
        record = encoded((cursor, count, width, crc, metadata_crc))
        require(len(record) <= DIRECTORY_WIDTH, "Directory record exceeds 96 bytes")
        directory.append(record.ljust(DIRECTORY_WIDTH, b" "))
        node.update(start=cursor, count=count, width=width, checksum=crc, metadata_checksum=metadata_crc)
        blocks.append(block)
        cursor += len(block)
    require(cursor <= 0xffffffff, "Navigation positions exceed uint32_t")
    result = prefix + marker + b"".join(directory) + b"".join(blocks)
    by_native_index = [r["id"] for r in knowledge]
    coverage = {}
    for mode in SEARCH_MODES:
        refs = [(f"legacy:{c['target']}" if c["kind"] == 1 else f"native:{by_native_index[c['target']]}")
                for n in nodes if n["mode"] == mode for c in n["children"] if c["kind"] != 0]
        coverage[mode] = dict(links=len(refs), unique=len(set(refs)), unique_refs=sorted(set(refs)))
        expected = {ref for ref in source["articles"] if ref.startswith("native:") or
                    ((int(ref.split(":")[1]) < 146) == (mode == "Theory"))}
        require(set(refs) == expected, f"Missing article references in {mode}")
    mode_stats = [dict(links=coverage[mode]["links"],
                       legacy_unique=sum(ref.startswith("legacy:") for ref in coverage[mode]["unique_refs"]),
                       native_unique=sum(ref.startswith("native:") for ref in coverage[mode]["unique_refs"]),
                       unique=coverage[mode]["unique"]) for mode in SEARCH_MODES]
    alphabet_root = next(n["id"] for n in nodes if n["mode"] == "Alphabet")
    manifest = dict(version=4, identity=identity, offset=len(prefix), total_bytes=len(result),
        node_count=len(nodes), roots=[0, 1, 2, alphabet_root], search_modes=list(SEARCH_MODES),
        alphabet_letters=[c["label"] for c in nodes[alphabet_root]["children"]],
        alphabet_quantities=len(source["quantities"]), directory_start=directory_start, directory_width=96,
        root_topiclabels=list(ROOT_LABELS), modes=coverage, mode_stats=mode_stats, legacy_modes=old_coverage,
        unique_refs=sorted(source["articles"]), unique=546, prefix_sha256=hashlib.sha256(prefix).hexdigest(),
        catalog_identity=catalog["identity"], native_ids=by_native_index,
        legacy_bodies_sha256=[hashlib.sha256(b.encode("ascii")).hexdigest() for b in bodies], nodes=nodes)
    declarations = ("// Generated by tools/generate_navigation.py; do not edit.\n"
        f"inline constexpr uint32_t navigationOffset={len(prefix)};\n"
        f"inline constexpr const char* navigationMarker={json.dumps(marker.decode('ascii'))};\n"
        f"inline constexpr Table navigationDirectory{{0,{len(nodes)},{directory_start},96}};\n"
        f"inline constexpr uint32_t navigationAlphabetRoot={alphabet_root};\n")
    write_changed(package, result)
    write_changed(header, declarations.encode("ascii"))
    write_changed(build_dir / "navigation-manifest.json",
                  (json.dumps(manifest, indent=2, ensure_ascii=True) + "\n").encode("ascii"))
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=SOURCE)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build")
    parser.add_argument("--header", type=Path, default=ROOT / "src/navigation_index.inc")
    args = parser.parse_args()
    try:
        manifest = generate(args.source, args.build_dir, args.header)
    except (ValueError, KeyError, TypeError, OSError) as error:
        parser.exit(1, f"Navigation generation failed: {error}\n")
    counts = "; ".join(f"{mode}: {value['unique']} unique / {value['links']} links"
                       for mode, value in manifest["modes"].items())
    print(f"PASS NAVIGATION: {manifest['node_count']} nodes; {counts}; {manifest['total_bytes']} bytes")


if __name__ == "__main__":
    main()
