# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

## Project Overview

This is a multi-language project that includes C++, Perl, Python, and Ruby components. The project uses wxWidgets for GUI functionality across multiple languages.

## Project-Specific Guidelines

### Node Class and NodeProperty
- `Node` class is declared in `nodes/node.h`
- Prefer passing `Node*` to functions rather than `NodeSharedPtr`
- `NodeProperty` class (used in all Nodes) is declared in `nodes/node_prop`
- Prefer using `NodeProperty::as_view()` instead of `NodeProperty::as_string()`
- Prefer using `Node::as_view()` when Node or NodeSharedPtr is available

### Generator Classes
- All generator classes derive from the base interface in `generate/base_generator.h`

### File and Code Restrictions
- Do not edit any code between the comment block "// Do not edit any code above the "End of generated code" comment block.

### String Conversions
- **Always use `wxString::ToStdString()`** for converting `wxString` to `std::string` or `std::string_view`. This method returns a reference to a string object.
- Only use `wxString::utf8_string()` in rare cases where you specifically need UTF-8 encoded behavior that `ToStdString()` doesn't provide.
- When passing to a function expecting `std::string_view`, use `ToStdString()` as the view will bind to the returned string object.

### Code Formatting

After modifying any C++ code files, always run the "Format Document" command on each modified file to ensure consistent formatting. Use the VSCode command `editor.action.formatDocument` on files you've changed.

### Debug Checks
- The `ASSERT`, `ASSERT_MSG`, and `FAIL_MSG` macros are the preferred macros for debug checks declared in assertion_dlg.h

## Language-Specific Coding Standards

### C++ Standards

**Naming Conventions:**
- Variables: `snake_case`
- Classes: `PascalCase`
- Methods/Functions: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Enum types: `PascalCase` (singular form, e.g., `Color` not `Colors`)
- Enum values: `snake_case`

**Code Style:**
- Indentation: 4 spaces
- Line length: 100 characters maximum
- Use blank lines to separate logical code sections

**Function Declarations:**
- Always use trailing return type syntax
- Functions returning `bool` must have `[[nodiscard]]` attribute

**Enum Guidelines:**
- Use `enum class` for type safety (never traditional enums)
- Specify underlying type using `std::` types (e.g., `std::uint8_t`) to minimize size

**Conditionals:**
- Always use braces for conditional statements, even single-line statements
- Do not use `else` after a conditional that exits (return, throw, break)

**Modern C++ Features:**
- Prefer `auto` for variable declarations when type is obvious
- Use `constexpr` for compile-time constants and functions
- Use range-based for loops over traditional loops when iterating containers
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) instead of raw pointers
- Use structured bindings and concepts for clarity and safety
- Prefer C++20 ranges library algorithms over manual loops when applicable

**Library Priority (in order):**
1. C++ Standard Library (`std::`) - Always check here first
2. wxWidgets library - Use when standard library doesn't provide functionality
3. `ttwx::` namespace (from `utils/ttwx.h`) - Project-specific utilities
4. Frozen containers (from `./frozen/include/frozen`) - For immutable collections

**Immutable Containers:**
- Use frozen library types for immutable versions of: `std::set`, `std::unordered_set`, `std::map`, `std::unordered_map`
- Corresponding frozen types: `frozen::set`, `frozen::unordered_set`, `frozen::map`, `frozen::unordered_map`
- Always include the appropriate frozen header file
- Prefer `constexpr` or `constinit` for frozen containers

### Perl Standards

**Naming Conventions:**
- Variables: `snake_case`
- Packages: `PascalCase`
- Wx:: methods: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Begin wxWidgets functions with Wx:

**Code Style:**
- Indentation: 4 spaces
- Line length: 100 characters maximum
- Use blank lines to separate logical code sections

### Python Standards

**Naming Conventions:**
- Variables/Functions: `snake_case`
- Packages: `PascalCase`
- wx. methods: `PascalCase`
- Begin wxWidgets functions with wx.
- Constants: `UPPER_SNAKE_CASE`

**Code Style:**
- Indentation: 4 spaces
- Line length: 90 characters maximum
- Use blank lines to separate logical code sections

### Ruby Standards

**Naming Conventions:**
- Variables/Functions: `snake_case`
- Classes: `PascalCase`
- Wx:: methods: `PascalCase`
- Begin wxWidgets functions with Wx:
- Constants: `UPPER_SNAKE_CASE`
- Use a leading `_` for an unused parameter, e.g. `_event`.

**Code Style:**
- Indentation: 2 spaces
- Line length: 80 characters maximum
- Use blank lines to separate logical code sections

## Key Files and Directories

- `src/ttwx/ttwx.h` - Contains `ttwx::` namespace utilities
- `./frozen/include/frozen` - Frozen library for immutable containers

## Important Reminders

When working on C++ code:
- Always check if Standard Library provides needed functionality before using external libraries
- Include appropriate frozen headers when using immutable containers
- Validate that enum classes use explicit underlying types
- Ensure all bool-returning functions have `[[nodiscard]]` attribute
- Verify conditionals use braces even for single statements

When working across multiple languages:
- Note the different indentation standards (C++/Perl/Python: 4 spaces, Ruby: 2 spaces)
- Note the different line length limits (Ruby: 80, Python: 90, C++/Perl: 100)
- All languages use `PascalCase` for wxWidgets/Wx methods
- Perl and Ruby begin wxWidgets functions with Wx:
- Python beings wxWidgets functions with wx.
