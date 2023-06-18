# Developer notes

If you are planning on contributing code, the following sections contain information about the code that might not be immediately obvious. Reading these sections may make the code easier to understand, as well as ensuring that PR's are written in a way that matches the rest of the code base. In addition, some of the directories also have README files that provide explanations specific to the files in that directory. See [generate](../src/generate/README.md), [nodes](../src/nodes/README.md), [winres](../src/winres/README.md) and [xml](../src/xml/README.md).

Note that the code requires a C++20 compliant compiler -- which means you should be using C++20 coding conventions. That includes using `std::string_view` (or `ttlib::sview`) for strings when practical. See the **Strings** section below for information about working with `wxString`.

## Debug builds

When you create a debug build, there will be an additional **Internal** menu to the right of the **Help** menu that will give you access to additional functionality for easier debugging.

## Strings

Internally strings are normally placed into `tt_string`, `tt_stringview` and `tt_wxString` classes. These classes inherit from `std::string`, `std::string_view` and `wxString` respectively, and provide additional functionality common across all three of these classes. When you need to convert a tt_string or tt_stringview to a wxString to pass to wxWidgets, use the method `make_wxString()`. If you need to pass a wxString to tt_string, use `utf8_string()`. These two methods ensure that UTF8/16 conversion is correctly handled on Windows. It also ensure a seamless transition between wxWidgets 3.2 and 3.3 where the underlying wxString is changed from UTF16 to UTF8.


## size_t and int_t

These two types are used to ensure optimal bit-width for the current platform (currently 32-bit or 64-bit). Note that `int_t` is not a standard type, but declared in `pch.h` as `typedef ptrdiff_t int_t;` to provide more readable code than using `ptrdiff_t`.

### Debugging macros

The `ASSERT`, `ASSERT_MSG` and `FAIL_MSG` macros are the preferred macros for debug checks. On Windows, they provide the option to break into a debugger if running a Debug build.

The `MSG_...` macros allow for display information in the custom logging window. The custom logging window has filters so that you can limit which messages are displayed. Unlike the `wxLog...` macros, none of these messages will ever be displayed to the user -- they are for your debugging use only.

## clang-format

All PRs get run through a github action that runs clang-format. This will not change your code, but will report a failure if clang-format would have changed your code formatting. To ensure a successful PR submission, run your code through clang-format before committing it.

If you are adding a comment to a function or variable in a header file, please wrap the comment to a maximum of 93 characters -- that makes the comment more likely to display correctly when displayed in a Intellisense popup.
