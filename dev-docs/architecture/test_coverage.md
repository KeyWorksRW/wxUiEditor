# Test Coverage

While wxUiEditor itself makes extensive use of it's own code generation, it is of course limited to C++ code. There are three additional ways for testing code generation in all the computer languages that wxUiEditor can generate code for.

## codegen_test

This directory is the primary location of code testing. It is designed to generate code in all supported languages and then verify that the code can compile (C++) or that it passes syntax checking (Perl, Python, Ruby).

Currently, the C++ code is part of the github action for daily builds. For now, the other languages are only tested locally (e.g., run_test.cmd). Eventually, syntax checking for python and ruby will be added to the daily build tests.

A significant difference between this and wxUiTesting/sdi is that there is no `images` form. The assumption is that most people won't use this, so it is important to verify that images get handled correctly with the actual forms.

Note that none of the generated code is tracked. This is _not_ a way to check differences in how code is generated (see wxUiTesting section below for code generate tracking).

## wxUiTesting (https://github.com/KeyWorksRW/wxUiTesting)

This repository is designed to be run from all generated code languages. This allows you to visually check that the UI looks correct. It also contains local scripts for checking syntax (e.g., run_test.cmd). There are currently three sub-sections:

- SDI (primary and most extensive test code)
- MDI (for testing various main window types)
- Quick (for quick testing of some change to code generation)

Note that the Quick directory only tracks enough code to actually launch the applications. That means that if you add event handling, be aware that the code only exists on your local machine.
