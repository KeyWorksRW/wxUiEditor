# Cloning

If you are cloning, rather than forking, then ideally you want to clone the repository with the following command:

```
    git clone --recurse-submodules https://github.com/KeyWorksRW/wxUiEditor.git
```

If the above was not used, then the following two commands need to be run after cloning:

```
	git submodule init
	git submodule update
```

To get submodule updates, run:

```
	git submodule update --remote
```

There are two submodules: `ttLib` and `wxSnapshot`.

# Developer notes

If you are planning on contributing code, the following sections contain information about the code that might not be immediately obvious. Reading these sections may make the code easier to understand, as well as ensuring that PR's are written in a way that matches the rest of the code base.

Note that the code requires a C++17 compliant compiler -- which means you should be using C++17 coding conventions. That includes using `std::string_view` (or `ttlib::cview`) for strings when practical. See the **Strings** section below for information about working with `wxString`.

## Debug builds

When you create a debug build, there will be an additional **Internal** menu to the right of the **Help** menu that will give you access to additional functionality for easier debugging.

## Strings

The only time **wxString** should be used in **wxUiEditor** is when a string will be used for UI or for filenames. The **ttLib** class **ttString** derives from **wxString** and is often used in the code for working with filenames instead of passing the string to a **wxFileName**.

Both `ttString` and `ttlib::cstr` have a `wx_str()` method that will perform UTF8<->UTF16 conversion when compiled for Windows, and no conversion when compiled for other platforms. `ttlib::cstr` is essentially std::string with additional methods including `wx_str()`.

In most cases, **ttlib::cview** is used instead of **std::string_view**. **ttlib::cview** inherits from **std::string_view** but points to a zero-terminated string.

## size_t and int_t

These two types are used to ensure optimal bit-width for the current platform (currently 32-bit or 64-bit). Note that `int_t` is not a standard type, but declared in `pch.h` as `typedef ptrdiff_t int_t;` to provide more readable code than using `ptrdiff_t`.

### Debugging macros

The `ASSERT...` and `FAIL...` macros are the preferred macros for debug checks. On Windows, they provide the option to break into a debugger if running a Debug build.

The `MSG_...` macros allow for display information in the custom logging window. The custom logging window has filters so that you can limit which messages are displayed. Unlike the `wxLog...` macros, none of these messages will ever be displayed to the user -- they are for your debugging use only.

## clang-format

All PRs get run through a github action that runs clang-format. This will not change your code, but will report a failure if clang-format would have changed your code formatting. To ensure a successful PR submission, run your code through clang-format before committing it.

If you are adding a comment to a function or variable in a header file, please wrap the comment to a maximum of 93 characters -- that makes the comment more likely to display correctly when displayed in a Intellisense popup.
