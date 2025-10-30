# Guidance for AI Coding in wxUiEditor
This repository contains C++, Perl, Python, and Ruby code using wxWidgets for GUI functionality. Follow these standards and priorities for all code contributions.

## Project Overview
This is a C++ project used to generate C++, Perl, Python, and Ruby code for creating wxWidgets UI. The project itself makes extensive use of the wxWidgets library.

## Project-Specific Guidelines

### Node Class and NodeProperty
- `Node` class is declared in `nodes/node.h`
- Prefer passing `Node*` to functions rather than `NodeSharedPtr`
- `NodeProperty` class (used in all Nodes) is declared in `nodes/node_prop.h`
- **Always prefer `NodeProperty::as_view()` over `NodeProperty::as_string()`** - as_view() is more efficient
- **Always prefer `Node::as_view()` when Node or NodeSharedPtr is available** - as_view() is more efficient
- When neither Node nor NodeProperty is available, use as_string() as fallback

### Generator Classes
- All generator classes in `src/generate/` derive from the base interface in `src/generate/base_generator.h`
- When creating new generators, inherit from BaseGenerator and implement required virtual methods

### Protected Code Sections
- **Do not edit any code between `// Do not edit any code above` and `// End of generated code` comment blocks**
- These sections are automatically generated and will be overwritten
- Your modifications added after the `// End of generated code` comment block will be preserved

### String Conversions
- **Always use `wxString::ToStdString()`** for converting `wxString` to `std::string` or `std::string_view`
- Only use `wxString::utf8_string()` in rare cases where you specifically need UTF-8 encoded behavior that `ToStdString()` doesn't provide
- When passing a wxString to a function expecting std::string_view, use `ToStdString()`
- **If `std::format` is used, the file must `#include <format>` near the top**
- Do not use `tt_string` for any new code
- Do not use `tt_string_view` for any new code

### Array conversions

Convert C style char* arrays to std::array using std::to_array.

### Debug Checks
- Use `ASSERT`, `ASSERT_MSG`, and `FAIL_MSG` macros from `assertion_dlg.h` for debug checks
- These are preferred over raw assert() or throw statements

## Copilot Agent Build Instructions

**CRITICAL**: When running a build task, you MUST verify the actual build result correctly:

1. **Clear the terminal BEFORE starting the build** to prevent parsing errors from previous build output
2. **Always use `get_terminal_output` or `get_task_output` AFTER the build completes** to retrieve the full terminal output
3. **Search the output for these FAILURE indicators**:
   - Lines containing "error:" or "error C" (compiler errors)
   - Lines containing "undefined reference" or "unresolved external symbol" (linker errors)
   - Lines containing "FAILED:" (build system failures)
   - Exit code != 0 (check the exit code explicitly)
   - Lines containing "cannot find" or "No such file"
4. **Search the output for these SUCCESS indicators**:
   - "Build finished" or "Build succeeded"
   - Exit code = 0
   - No error indicators from the list above
5. **If ANY errors are found**, the build FAILED - report this to the user with specific error details
6. **Do NOT report success based on**:
   - The Problems panel alone
   - Absence of exceptions
   - Task completion without checking output
7. **After identifying errors**, analyze each error message and fix the root cause in the source code
8. **Re-run the build** after making fixes to verify they resolved the issues

# Language-Specific Coding Standards

## C++ Standards

### Naming Conventions
- Variables: `snake_case`
- Classes/Methods/Functions: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Enum types: `PascalCase` (singular, e.g., `Color` not `Colors`)
- Enum values: `snake_case`

### Code Style
- Indentation: 4 spaces
- Line length: 100 characters maximum
- Use blank lines to separate logical code sections

### Function Declarations
- Always use trailing return type syntax: `auto FunctionName() -> ReturnType`
- Functions returning `bool` or `int` must have `[[nodiscard]]` attribute

### Enum Guidelines
- Use `enum class` for type safety (never traditional enums)
- Specify underlying type using `std::` types (e.g., `std::uint8_t`)
- Example: `enum class Color : std::uint8_t { red, green, blue };`

### Conditionals
- Always use braces for conditional statements, even single-line statements
- Always use braces for statements that are part of `while` and `for` statements, even single-line statements
- Do not use `else` after a conditional that exits (return, throw, break)

### Modern C++ Features
- Prefer `auto` for variable declarations when type is obvious
- Use `constexpr` for compile-time constants and functions
- Use range-based `for` loops over traditional loops when iterating containers
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) instead of raw pointers
- Use structured bindings for clarity: `auto [x, y] = get_point();`
- Prefer C++20 ranges library algorithms over manual loops when applicable

### Library Priority (in order)
1. **C++ Standard Library (`std::`)** – Always check here first
2. **wxWidgets library** – Use when standard library doesn't provide functionality
3. **`ttwx::` namespace** (from `src/ttwx/ttwx.h`) – Project-specific utilities
4. **Frozen containers** (from `frozen/include/frozen`) – For immutable collections

### Immutable Containers
- Use frozen library types for immutable versions:
  - `frozen::set` (immutable `std::set`)
  - `frozen::unordered_set` (immutable `std::unordered_set`)
  - `frozen::map` (immutable `std::map`)
  - `frozen::unordered_map` (immutable `std::unordered_map`)
- Always include the appropriate frozen header file
- Prefer `constexpr` or `constinit` for frozen containers for compile-time initialization

## Perl Coding Standards

### Naming Conventions
- Variables: `snake_case`
- Packages: `PascalCase`
- Wx:: methods: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Begin wxWidgets functions with `Wx:`

### Code Style
- Indentation: 4 spaces
- Line length: 100 characters maximum
- Use blank lines to separate logical code sections

## Python Coding Standards

### Naming Conventions
- Variables/Functions: `snake_case`
- Packages: `PascalCase`
- `wx.` methods: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Begin wxWidgets functions with `wx.`

### Code Style
- Indentation: 4 spaces
- Line length: 90 characters maximum
- Use blank lines to separate logical code sections

## Ruby Coding Standards

### Naming Conventions
- Variables/Functions: `snake_case`
- Classes: `PascalCase`
- `Wx::` methods: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Begin wxWidgets functions with `Wx:`
- Use a leading `_` for unused parameters (e.g., `_event`)

### Code Style
- Indentation: 2 spaces
- Line length: 80 characters maximum
- Use blank lines to separate logical code sections

## Critical Reminders for All Code

### When Working on C++ Code
- ✅ Always check if Standard Library provides needed functionality before using external libraries
- ✅ Include appropriate frozen headers when using immutable containers
- ✅ Validate that enum classes use explicit underlying types
- ✅ Ensure all bool/int-returning functions have `[[nodiscard]]` attribute
- ✅ Verify conditionals use braces even for single statements
- ✅ **Respect protected code sections** – Do not edit between "Do not edit" and "End of generated code" markers
- ✅ For Node and NodeProperty classes, prefer `as_view()` methods
- ✅ All generator classes must derive from `src/generate/base_generator.h`

### When Working With Multiple Programming Languages
- **Indentation varies by language**: C++/Perl/Python use 4 spaces, Ruby uses 2 spaces
- **Line length limits vary**: Ruby (80), Python (90), C++/Perl (100)
- **wxWidgets method naming**: All use `PascalCase`
- **Function prefixes for wxWidgets**:
  - Perl and Ruby: Begin with `Wx:` (e.g., `Wx::CreateButton`)
  - Python: Begin with `wx.` (e.g., `wx.CreateButton`)
