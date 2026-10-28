"""Run self-contained HH3 tests and keep the report with the test package."""
import io
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]


def main():
    report = io.StringIO()
    suite = unittest.defaultTestLoader.discover(str(ROOT / "tests"))
    result = unittest.TextTestRunner(stream=report, verbosity=2).run(suite)
    text = report.getvalue()
    print(text)
    output = ROOT / "testbuild/hh3_preview/software-tests.txt"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(text, encoding="utf-8")
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())
