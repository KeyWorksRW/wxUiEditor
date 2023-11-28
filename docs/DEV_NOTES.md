# Developer notes

Contributions are welcome! If you are interested in contributing, please read the following sections.

Note that in addition to this file, some of the directories also have README files that provide explanations specific to the files in that directory. See [generate](../src/generate/README.md), [nodes](../src/nodes/README.md), [winres](../src/winres/README.md) and [xml](../src/xml/README.md).

The code requires a C++20 compliant compiler -- which means you should be using C++20 coding conventions. That includes using `std::string_view` (or `tt_stringview`) for strings when practical. See the **Strings** section below for information about working with `wxString`.

## wxWidgets libraries

The `wxWidgets/` directory contains a snapshot of wxWidgets 3.3.x. Note that the `setup.h` file used to build this is fairly restritctive, avoiding many sections of the library that aren't used by wxUiEditor. This particular build is not intended to be used for anything other than wxUiEditor.

## Debug builds

When you create a Debug build, there will be an additional `Testing` and `Internal` menu to the right of the **Help** menu that will give you access to additional functionality for easier debugging. You can also get these menus in a Release build if you set INTERNAL_BLD_TESTING to true in your CMake configuration.

## Strings

Internally strings are normally placed into `tt_string` or `tt_stringview` classes. These classes inherit from `std::string` and `std::string_view` respectively, and provide additional functionality common across both of these classes. Note that the wxWidgets library is built as UTF8 even on Windows (new option in wxWidgets 3.3).

### Debugging macros

The `ASSERT`, `ASSERT_MSG` and `FAIL_MSG` macros are the preferred macros for debug checks. On Windows, they provide the option to break into a debugger if running a Debug build.

The `MSG_...` macros allow for display information in the custom logging window. The custom logging window has filters so that you can limit which messages are displayed. Unlike the `wxLog...` macros, none of these messages will ever be displayed to the user -- they are for your debugging use only.

## clang-format

All PRs get run through a github action that runs clang-format. This will report a failure if clang-format would have changed your code formatting. To ensure a successful PR submission, run your code through clang-format before committing it.

If you are adding a comment to a function or variable in a header file, please wrap the comment to a maximum of 93 characters -- that makes the comment more likely to display correctly when displayed in a Intellisense popup.

## Comments: // and /* */

Prior to AI being available to generate code, comments in the code explained _why_ the code was written not _what_ the code did. The reasoning is that _what_ the code did should be obvious just by reading the code, but _why_ the code was written in a specific way might not be obvious.

With AI, prefixing code with a comment that says _what_ the code should do will help the AI generate more accurate code. In addition, a comment that explains _what_ the code is doing can help AI training making the code more likely to be generated for someone else. This project is Open Source, and uses an Apache License, so there are no restrictions on how a section of the code is used in other projects.

## Default variable names

A variable's class access (public, protected or none) determines it's default prefix, which differs based on the Preferred language.

Note: Ruby uses a leading `_` for an unused parameter, e.g. `_event`.

#### none

- C++: no prefix
- Python: no prefix
- Ruby: no prefix

#### protected

- C++: `m_`
- Python: self. if not a form
- Ruby: `@`

#### public

- C++: `m_`
- Python: self. if not a form
- Ruby: `@`

## Ruby Notes

The project file has a setting for disabling robocop warnings. In a Release build, this disables all warnings, however in a Debug build only specific warnings that we can't reasonably prevent are disabled.
