# DEPRECATED

With the change to using CMake’s FetchContent instead of maintaining our own copy of the wxWidgets source code, this
sub-directory can no longer be built.

A different method of testing is in progress -- see the tests/ directory.

## Automated validation (CI)

The wxPython output of this project is checked on Linux by `.github/workflows/unit_tests.yml`:

1. `wxUiEditor --gen_python codegen_test.wxui` regenerates `python/`.
2. `tests/scripts/run_wxpython.py python/` imports every generated module and constructs every top-level window it
   defines, under `xvfb-run`. This is Tier 2 -- `py_compile` alone would only prove the modules parse.
3. Python and wxPython are both pinned, wxPython is installed from a binary wheel only, and the block is
   `continue-on-error`: a missing wheel must never red-line the pipeline.

Locally: generate the code, then run `python tests/scripts/run_wxpython.py codegen_test/python` (needs a display and
wxPython).
