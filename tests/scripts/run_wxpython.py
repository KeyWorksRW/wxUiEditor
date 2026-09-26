#!/usr/bin/env python3
"""
run_wxpython.py

Tier-2 validation of the wxPython code wxUiEditor generates.

`python -m py_compile` (Tier 1, what codegen_test/run_test.cmd runs) only proves the
generated modules parse. This script proves they run: it imports every generated
module and constructs every top-level window class it defines, inside a real
wx.App. Initializing wxWidgets needs a display connection, so on Linux this has to
run under `xvfb-run -a`.

Usage:
    python tests/scripts/run_wxpython.py <generated_dir>

    <generated_dir>  Directory holding the generated .py modules. For the
                     codegen_test project that is codegen_test/python.

A directory that holds no .py modules is reported as a failure: passing an empty
directory would otherwise "validate" nothing and still exit 0.

Exit codes:
    0  every module imported and every top-level window was constructed
    1  usage error, wxPython missing, or at least one import/construction failure
"""

import argparse
import importlib
import inspect
import os
import sys
import traceback
from pathlib import Path
from types import ModuleType
from typing import Iterator


def parse_arguments(arguments: list[str]) -> argparse.Namespace:
    """The generated directory is the only argument."""
    parser = argparse.ArgumentParser(
        description="Construct every top-level window in the generated wxPython modules.")
    parser.add_argument(
        "generated_dir", type=Path, help="directory holding the generated .py modules")
    return parser.parse_args(arguments)


def import_wx() -> ModuleType | None:
    """Return the wxPython module, or print the install command and return None."""
    try:
        import wx  # pylint: disable=import-outside-toplevel
    except ImportError as error:
        print(f"FAIL: wxPython is not importable ({error})")
        print("      install it with: python -m pip install --only-binary=:all: wxPython")
        return None
    return wx


def module_files(generated_dir: Path) -> list[Path]:
    """Every generated module, in a stable order."""
    return sorted(path for path in generated_dir.glob("*.py") if path.stem != "__init__")


def window_classes(wx: ModuleType, module: ModuleType) -> Iterator[tuple[str, type]]:
    """The top-level window classes defined in module (not the ones it imports)."""
    for name, member in inspect.getmembers(module, inspect.isclass):
        # getmembers() also returns imported classes, wx.Window itself among them.
        # Only a class whose __module__ is this module was generated here.
        if member.__module__ != module.__name__:
            continue
        if issubclass(member, wx.Window):
            yield name, member


def report(what: str, phase: str, details: str) -> None:
    """Print a failure with its traceback indented underneath."""
    print(f"  FAIL    {what} ({phase})")
    for line in details.rstrip().splitlines():
        print(f"            {line}")


def validate_module(wx: ModuleType, module_path: Path, parent) -> int:
    """Import one module and construct every window it defines; returns the failure count."""
    name = module_path.stem

    try:
        module = importlib.import_module(name)
    except Exception:
        report(name, "import", traceback.format_exc())
        return 1

    classes = list(window_classes(wx, module))
    if not classes:
        print(f"  SKIP    {name} (defines no top-level window)")
        return 0

    failures = 0
    for class_name, window_class in classes:
        try:
            window = window_class(parent)
        except Exception:
            report(f"{name}.{class_name}", "construct", traceback.format_exc())
            failures += 1
            continue
        print(f"  OK      {name}.{class_name}")
        window.Destroy()

    return failures


def main(arguments: list[str]) -> int:
    options = parse_arguments(arguments)

    generated_dir = options.generated_dir.resolve()
    if not generated_dir.is_dir():
        print(f"FAIL: generated directory not found: {generated_dir}")
        return 1

    module_paths = module_files(generated_dir)
    if not module_paths:
        print(f"FAIL: no generated .py modules in {generated_dir}")
        return 1

    wx = import_wx()
    if wx is None:
        return 1

    # Generated code loads resources with paths relative to the module directory
    # (e.g. wx.Bitmap("../art/english.xpm")), so imports and construction both have
    # to run from there.
    sys.path.insert(0, str(generated_dir))
    os.chdir(generated_dir)

    print(f"Validating {len(module_paths)} module(s) in {generated_dir}")

    # wx.App() makes the display connection every window below needs. The parent frame
    # is never shown -- it exists only because the generated dialogs, wizard, property
    # sheet and popup window all require a parent.
    app = wx.App(False)
    parent = wx.Frame(None)

    failures = 0
    for module_path in module_paths:
        failures += validate_module(wx, module_path, parent)

    parent.Destroy()
    # The App has to outlive every window it created.
    del app

    if failures:
        print(f"\nFAILED: {failures} module/class failure(s).")
        return 1

    print(f"\nPASSED: {len(module_paths)} module(s) constructed.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
