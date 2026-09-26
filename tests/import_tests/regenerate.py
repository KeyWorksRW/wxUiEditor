#!/usr/bin/env python3
"""
regenerate.py

Regenerate the import goldens under tests/import_tests/.

Every importer is deliberately lossy, so this script does NOT compare against the
original project file -- it re-serializes each sample and overwrites the golden
.wxui file beside it. A changed golden therefore means importer output changed;
review the git diff this script prints at the end before committing.

Usage:
    python tests/import_tests/regenerate.py [debug|release] [--verify]

    debug|release  Which build to run (default: release).
    --verify       After regenerating, re-import every sample with --verify_import
                   to prove the goldens are byte-stable (the idempotency check).

The sample file in each importer subdirectory is the source of truth; the matching
.wxui is derived output and should only ever be written by this script.
"""

import subprocess
import sys
from pathlib import Path
from typing import List

TESTS_DIR = Path(__file__).resolve().parent
REPO_ROOT = TESTS_DIR.parent.parent

# One sample per importer, plus the Windows Resource files that predate this script.
IMPORTER_DIRS = ("winres", "wxcrafter", "formbuilder", "wxsmith", "wxglade", "dialogblocks")
SAMPLE_EXTENSIONS = ("rc", "wxcp", "fbp", "wxs", "wxg", "pjd")


def find_samples() -> List[Path]:
    """Every importer sample, ordered by importer directory then extension."""
    samples: List[Path] = []
    for directory in IMPORTER_DIRS:
        for extension in SAMPLE_EXTENSIONS:
            samples.extend(sorted((TESTS_DIR / directory).glob(f"*.{extension}")))
    return samples


def locate_editor(build_type: str) -> Path:
    """Absolute path to the wxUiEditor executable for the requested configuration."""
    suffix = ".exe" if sys.platform == "win32" else ""
    name = f"wxUiEditorD{suffix}" if build_type == "debug" else f"wxUiEditor{suffix}"
    return REPO_ROOT / "bin" / build_type.capitalize() / name


def run_editor(editor: Path, switch: str, sample: Path) -> bool:
    """Run the editor with a single import switch; returns True when it exits 0."""
    result = subprocess.run([str(editor), switch, str(sample)], cwd=str(REPO_ROOT), check=False)
    return result.returncode == 0


def main() -> int:
    arguments = [argument.lower() for argument in sys.argv[1:]]
    build_type = "debug" if "debug" in arguments else "release"
    verify = "--verify" in arguments

    editor = locate_editor(build_type)
    if not editor.is_file():
        print(f"ERROR: {editor} not found.")
        print(f"Build the {build_type.capitalize()} configuration first.")
        return 1

    print(f"Using {editor}\n")

    samples = find_samples()
    regenerated = 0
    failed = 0
    for sample in samples:
        relative = sample.relative_to(TESTS_DIR).as_posix()
        if run_editor(editor, "--save_import", sample):
            print(f"  regenerated  {relative}.wxui")
            regenerated += 1
        else:
            print(f"  FAILED       {relative}")
            failed += 1

    print(f"\nRegenerated {regenerated} golden(s), {failed} failure(s).")
    if failed:
        return 1

    if verify:
        mismatched = 0
        for sample in samples:
            relative = sample.relative_to(TESTS_DIR).as_posix()
            if not run_editor(editor, "--verify_import", sample):
                print(f"  NOT IDEMPOTENT  {relative}")
                mismatched += 1
        if mismatched:
            print(f"\n{mismatched} golden(s) could not be reproduced.")
            return 1
        print(f"\nVerified all {regenerated} golden(s) are reproducible.")

    # Show what actually changed -- this is the diff summary to review in the PR.
    if (REPO_ROOT / ".git").is_dir():
        print("\n-------------- golden diffs --------------")
        subprocess.run(
            ["git", "-C", str(REPO_ROOT), "--no-pager", "diff", "--stat", "--", "tests/import_tests"],
            check=False,
        )

    return 0


if __name__ == "__main__":
    sys.exit(main())
