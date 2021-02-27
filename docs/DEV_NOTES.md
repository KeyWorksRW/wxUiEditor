# Developer notes

The following sections contain information about the code that might not be immediately obvious. Reading these sections may make the code easier to understand, as well as ensuring that PR's are written in a way that matches the rest of the code base.

Note that the code requires a C++17 compliant compiler -- which means you should be using C++17 coding conventions. That includes using `std::string_view` (or `ttlib::cview`) for strings when practical. See the **Strings** section below for information about working with `wxString`.

## Debug builds

When you create a debug build, there will be an additional **Debug** menu to the right of the **Help** menu that will give you access to additional functionality for easier debugging. In addition most items in the Navigation pane will have one or more additional context menu commands available under Debug builds.

## ART files

Original art images are in the `art_src/` directory. All art is automatically converted into XPM or PNG header files as part of the build process. The converted art is _not_ part of the repository, so to change artwork, only the original needs to be changed.

## Strings

The only time **wxString** should be used in **wxUiEditor** is when a string will be used for UI or for filenames. The **ttLib** class **ttString** derives from **wxString** and is often used in the code for working with filenames instead of passing the string to a **wxFileName**.

Both `ttString` and `ttlib::cstr` have a `wx_str()` method that will perform UTF8<->UTF16 conversion when compiled for Windows, and no conversion when compiled for other platforms. `ttlib::cstr` is essentially std::string with additional methods including `wx_str()`.

In most cases, **ttlib::cview** is used instead of **std::string_view**. **ttlib::cview** inherits from **std::string_view** but points to a zero-terminated string.

## size_t and int_t

These two types are used to ensure optimal bit-width for the current platform (currently 32-bit or 64-bit). Note that `int_t` is not a standard type, but declared in `pch.h` as `typedef ptrdiff_t int_t;` to provide more readable code than using `ptrdiff_t`.

### Localization

Strings can potentially be localized whenever you use the `_tt()`, `_ttv()`, `_ttc()` or `_ttwx()` macros. These macros use an id for the string stored in strings.h, with the various language versions of the strings in `strings.cpp`. The function `_ttSetCurLanguage()` is used to change the language at runtime.

### Debugging macros

The `ASSERT...` and `FAIL...` macros are the preferred macros for debug checks. They work on all platforms, but the message box is slightly different on Windows and non-Windows platforms. On Windows, the ASSERT dialog will contain an Abort button allowing you to immediately terminate the program even if you aren't running it under a debugger (this is not available with the wxASSERT macros).

The `THROW()` macro will generate an assertion message in debug builds. The same message will be available in the `catch` statement in release builds so that you can optionally display it to the user.

The `MSG_...` macros allow for display information in the custom logging window. The custom logging window has filters so that you can limit which messages are displayed. Unlike the `wxLog...` macros, none of these messages will ever be displayed to the user -- they are for your debugging use only.

If you are fairly certain a condition will be fixed before shipping a release version, then use the ASSERT/FAIL macros. If it's possible that the condition could occur in a release build, and you have a catch setup to handle it, use THROW (in debug builds, this will assert first).

### MessageBox

The `appMsgBox` is the preferred method for displaying warning messages to the user since it can use localized strings that don't require any special tools to create (see the **Localization** section above). Since the msg parameter is `ttlib::cstr` the message string will be converted to UTF16 when compiled for Windows before being displayed. It will be displayed normally (no conversion needed) on non-Windows platforms.

## Testing

The `testing/` directory contains the source code for building **wxUiTesting.exe**. This app can be used to verify that generated code compiles, that controls and sizers are working as expected, and it can be referred to in an Issue to display a problem and/or verify a fix.

## clang-format

All code should be run through clang-format to ensure consistency.

If you are adding a comment to a function or variable in a header file, please wrap the comment to a maximum of 93 characters -- that makes the comment more likely to display correctly when displayed in a Intellisense popup.
