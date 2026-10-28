"""Package the HH3 candidate without rebuilding the frozen knowledge/media."""
import argparse
import ast
import hashlib
import json
from pathlib import Path
import shutil
import zipfile

ROOT = Path(__file__).resolve().parents[1]
DEVICE_FILES = ("physchem_final.py", "wissen.txt", "bilder.dat")
BASELINE = ROOT / "tests" / "fixtures" / "hh2_v1.py"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def diagnostic_source(source):
    baseline = json.loads((ROOT / "tests" / "baseline.json").read_text(encoding="ascii"))
    if digest(BASELINE) != baseline["files"]["physchem_final.py"]:
        raise ValueError("Frozen HH2 baseline changed")
    names = {"store_text", "skip_store", "open_store", "record_value", "decode_record",
             "DiskTable", "DiskMap", "fill_media", "read_media", "media_stage",
             "draw_media", "render_media_page"}
    mapping = {name: "legacy_" + name for name in names}
    mapping.update({name: "legacy_" + name for name in
                    ("MEDIA_INDEX", "MEDIA_STAGE", "MEDIA_RAM", "SKIP_BLOCK")})

    class Legacy(ast.NodeTransformer):
        def visit_Name(self, node):
            node.id = mapping.get(node.id, node.id)
            return node
        def visit_FunctionDef(self, node):
            node.name = mapping.get(node.name, node.name)
            return self.generic_visit(node)
        def visit_ClassDef(self, node):
            node.name = mapping.get(node.name, node.name)
            return self.generic_visit(node)
        def visit_Global(self, node):
            node.names = [mapping.get(name, name) for name in node.names]
            return node

    legacy = []
    for node in ast.parse(BASELINE.read_text(encoding="ascii")).body:
        if isinstance(node, (ast.FunctionDef, ast.ClassDef)) and node.name in names:
            legacy.append(ast.unparse(Legacy().visit(node)))
        elif isinstance(node, ast.Assign) and any(
                isinstance(t, ast.Name) and t.id in ("MEDIA_INDEX", "SKIP_BLOCK", "MEDIA_STAGE", "MEDIA_RAM")
                for t in node.targets):
            legacy.append(ast.unparse(Legacy().visit(node)))
    tree = ast.parse(source)
    run = next(node for node in tree.body if isinstance(node, ast.FunctionDef) and node.name == "run")
    start, end = run.lineno, run.end_lineno
    class Menu(ast.NodeTransformer):
        def __init__(self):
            self.items = self.exits = 0
        def visit_Expr(self, node):
            call = node.value
            if (isinstance(call, ast.Call) and isinstance(call.func, ast.Attribute)
                    and call.func.attr == "append" and call.args
                    and isinstance(call.args[0], ast.Tuple)
                    and isinstance(call.args[0].elts[0], ast.Constant)
                    and call.args[0].elts[0].value == "Beenden"):
                self.items += 1
                item = ast.parse("main_rows.append(('HH3-Bildtest', 0, 0))").body[0]
                return [item, node]
            return self.generic_visit(node)
        def visit_If(self, node):
            if ast.unparse(node.test) == "mode_index == len(MODES) + 1":
                self.exits += 1
                node.test.comparators[0].right.value = 2
                dispatch = ast.parse("if mode_index == len(MODES) + 1:\n    hh3_test()\n    continue").body[0]
                return [dispatch, node]
            return self.generic_visit(node)
        def visit_Constant(self, node):
            if node.value == "Physik LK":
                node.value = "Physik LK HH3-Test"
            return node
    menu = Menu()
    run = menu.visit(run)
    if (menu.items, menu.exits) != (1, 1):
        raise ValueError("Unexpected main-menu layout")
    text = ast.unparse(ast.fix_missing_locations(run))
    lines = source.splitlines(keepends=True)
    lines[start - 1:end] = [text + "\n"]
    source = "".join(lines)
    tree = ast.parse(source)
    last = tree.body[-1]
    if not (isinstance(last, ast.Expr) and isinstance(last.value, ast.Call)
            and isinstance(last.value.func, ast.Name) and last.value.func.id == "run"):
        raise ValueError("Missing final run call")
    lines = source.splitlines(keepends=True)
    lines[last.lineno - 1:last.end_lineno] = ["\n\n".join(legacy) + "\n\n" +
        (ROOT / "tools" / "hh3_diagnostics.py").read_text(encoding="ascii") + "\n\nrun()\n"]
    result = "".join(lines)
    compile(result, "physchem_final.py", "exec")
    return result


def build(output=None, diagnostics=False):
    output = Path(output or ROOT / "testbuild" / "hh3_preview").resolve()
    allowed = (ROOT / "testbuild").resolve()
    if output == allowed or allowed not in output.parents:
        raise ValueError("Output must be a subfolder of this project's testbuild directory")
    baseline = json.loads((ROOT / "tests" / "baseline.json").read_text(encoding="ascii"))
    for name in ("wissen.txt", "bilder.dat"):
        if digest(ROOT / name) != baseline["files"][name]:
            raise ValueError("Frozen data changed: " + name)
    manifest_path = output / "manifest.json"
    if manifest_path.exists() and json.loads(manifest_path.read_text())['diagnostics'] != diagnostics:
        raise ValueError("Keep normal and diagnostic builds in separate output directories")
    code = (ROOT / "physchem_final.py").read_text(encoding="ascii")
    if diagnostics:
        code = diagnostic_source(code)
    compile(code, "physchem_final.py", "exec")
    for folder in (output / "classpad", output / "pc-preview"):
        allowed_names = set(DEVICE_FILES)
        if folder.name == "pc-preview":
            allowed_names.update(("gint.py", "README.md", "start_pc_preview.bat"))
        if folder.exists():
            unknown = {p.name for p in folder.iterdir() if p.is_file()} - allowed_names
            if unknown:
                raise ValueError("Unexpected package files: " + repr(sorted(unknown)))
        folder.mkdir(parents=True, exist_ok=True)
        (folder / "physchem_final.py").write_text(code, encoding="ascii", newline="\n")
        for name in ("wissen.txt", "bilder.dat"):
            shutil.copyfile(ROOT / name, folder / name)
    pc = output / "pc-preview"
    shutil.copyfile(ROOT / "tools" / "gint.py", pc / "gint.py")
    shutil.copyfile(ROOT / "README.md", pc / "README.md")
    (pc / "start_pc_preview.bat").write_text('@echo off\ncd /d "%~dp0"\npy -3 physchem_final.py\nif errorlevel 1 pause\n', encoding="ascii")
    for folder, name in ((output / "classpad", "physik_hh3_test.zip"),
                         (pc, "physik_hh3_pc_test.zip")):
        with zipfile.ZipFile(output / name, "w", zipfile.ZIP_DEFLATED) as archive:
            names = DEVICE_FILES if folder.name == "classpad" else DEVICE_FILES + ("gint.py", "README.md", "start_pc_preview.bat")
            for entry in sorted(names):
                archive.write(folder / entry, entry)
    manifest = dict(runtime="Hollyhock 3 / PythonExtra.hh3", diagnostics=diagnostics,
                    hardware_tested=False, baseline_commit=baseline["commit"],
                    files={name: digest(output / "classpad" / name) for name in DEVICE_FILES})
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="ascii")
    return output


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--diagnostics", action="store_true")
    parser.add_argument("--output-dir", type=Path)
    args = parser.parse_args()
    print(build(args.output_dir, args.diagnostics))
