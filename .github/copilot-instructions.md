<!--
This file is optimized for Claude Sonnet 4.5 and equivalent high-capability models (e.g., GPT-4o, Gemini 1.5 Pro).
Structure prioritizes efficient parsing with hierarchical organization and clear semantic markers.
-->

# AI Coding Guidelines
C++ project generating C++, Perl, Python, and Ruby code for wxWidgets UI applications.

## Project Context
- **Language:** C++23 with wxWidgets GUI framework
- **Architecture:** Visual UI designer/code generator for wxWidgets applications
- **Platform:** Cross-platform via wxWidgets abstractions (wxDir, wxFileName, etc.)
- **Build System:** CMake + Ninja

## Agent Command Interpretation
When the user types "run" (or similar: "go", "execute", "start"):
- **Always interpret as:** Execute the current agent's instructions on attached files
- **Never interpret as:** Run/launch the application this workspace builds
- To build/run the application, user will explicitly say "build the project" or "run the application"

When the user types "fix" and has selected a comment line:
- **The selected text will contain the comment** — use this to identify what to fix
- **If line starts with `// CR:`** → Read the entire comment block, understand the code review issue, and fix the problem on the following line(s)
- **If line starts with `// TODO`** → Read the entire comment block and implement what the TODO describes
- After fixing, remove the `// CR:` or `// TODO` comment block
- **Fix ONLY the single comment at the cursor position** — do not scan for or fix other CR/TODO comments in the file

## File Operations

### Creating New Files
**When creating a new file, ALWAYS open it in VSCode immediately after creation:**
- After using `create_file` tool, the file path should be provided to the user
- The user should be able to view the newly created file without manually searching for it
- This applies to all file types: source code, configuration files, documentation, etc.

## Critical Project Rules

### 🚫 Protected Code Sections
**NEVER edit between `// Do not edit any code above` and `// End of generated code` markers**
- Auto-generated sections will be overwritten
- Add modifications AFTER the `// End of generated code` marker only

### 🔴 Line Endings (ABSOLUTE)
**ALWAYS use LF (`\n`) line endings, NEVER CRLF (`\r\n`)**
- All files in this project use Unix-style line endings (LF only)
- This applies even when running on Windows
- When creating or editing files, ensure line endings remain LF
- Do not convert existing LF line endings to CRLF

### 🚫 Legacy Code Restrictions (ABSOLUTE)
**NEVER use `src/tt/` types in new code:** `tt_string`, `tt_string_view`, `tt_cwd`, `tt_view_vector`

**Use instead (choose based on usage pattern):**

| Legacy Type | Replacement Type |
|-------------|------------------|
| `tt_string` | `wxue::string` (extends `std::string` with utility methods) |
| `tt_string_view` | `wxue::string_view` (extends `std::string_view` with utility methods) |
| `tt_cwd` | `wxue::cwd` (current working directory with optional restore) |
| `tt_view_vector` | `wxue::ViewVector` (vector of string views) |
| `tt_string_vector` | `wxue::StringVector` (vector of strings) |
| `tt::` namespace functions | `wxue::` namespace equivalents |

**When to use wxue:: vs standard types:**

| Usage Pattern | Preferred Type |
|---------------|----------------|
| Needs utility methods (locate, contains, trim, etc.) | `wxue::string` or `wxue::string_view` |
| File path manipulation (extension, filename, etc.) | `wxue::string` |
| Uses `<<` operator chaining for string building | `wxString` |
| Passed to wxWidgets APIs | `wxString` |
| Uses wx features (Printf, MakeLower, Format, etc.) | `wxString` |
| Pure internal processing, no special methods needed | `std::string` |
| String views without utility methods | `std::string_view` |

**Refactoring existing tt_* code:**
- Refactoring `tt_*` types to `wxue::` equivalents is encouraged
- The `wxue::` classes are drop-in replacements with identical method signatures
- Header: `#include "wxue_namespace/wxue_string.h"` (includes both string and string_view)
- Both classes require `wxUSE_UNICODE_UTF8` and `wxUSE_UTF8_LOCALE_ONLY` to be enabled

**Rationale:** The `wxue::` namespace provides modern replacements that extend standard library types rather than creating custom string classes. wxString's `<<` operator supports multiple types with unlimited chaining, making code more concise than std::string's `+=` operator.

### ⚡ Performance-Critical Paths
**Directories:** `src/nodes/`, `src/generate/` (executed frequently during code generation)

**Optimization priorities:**
1. In-place operations: `erase()`, `append()` > `substr()`, string concatenation
2. View parameters: `std::string_view` > `std::string`
3. Cache frequently-accessed values: `get_CodePreference()` results
4. Node access: `as_view()` > `as_string()` (always prefer views)

### 🏗️ Core Architecture

**Node system:**
- `Node` class: `nodes/node.h` → Pass as `Node*` (not `NodeSharedPtr`)
- `NodeProperty` class: `nodes/node_prop.h`

**Generator system:**
- All generators inherit from `src/generate/base_generator.h`

**String conversions:**
- `wxString::ToStdString()` → `std::string`/`std::string_view` (returns `const std::string&`)
- `wxString::utf8_string()` → Use only for explicit UTF-8 encoding (e.g., filenames)
- `std::format` → Requires `#include <format>`
- `std::format` with `wxue::string`/`wxue::string_view` → Cast to base type: `static_cast<std::string>(str)` or `static_cast<std::string_view>(view)`

**String building preference:**
- `wxString` with `<<` operator → Preferred for multi-part string building (cleaner than `+=`)
- `std::format` → Preferred for fixed templates with placeholders

**Array conversions:**
- `std::to_array` for C-style `char*` arrays → `std::array`

**wxWidgets Headers:**
- Add `#include` when introducing wxWidgets classes
- Exception: `wxString` (precompiled header)
- Placement: Append to existing `wx/` include list (auto-sorted on save), or after top comments with blank lines

**Debugging:**
- Use `ASSERT`, `ASSERT_MSG`, `FAIL_MSG` from `assertion_dlg.h`
- Never use raw `assert()` or `throw`

## Build Environment

### Terminal Commands (CRITICAL)
**Always check terminal context before running commands.**

Look for the terminal type in the context:
- `Terminal: pwsh` → PowerShell 7.x syntax (modern features, `&&`, `||`, etc.)
- `Terminal: powershell` → Windows PowerShell 5.x syntax (traditional, `;` for chaining)
- `Terminal: bash` → bash syntax (`cat`, `$?`, `|`, etc.)
- `Terminal: cmd` → cmd syntax (`type`, `%ERRORLEVEL%`, etc.)

**Never assume bash by default.** Match syntax to the active terminal type.

### Build Commands
Use these methods for building (in priority order):
1. **Try task first:** `run_task` with "build debug" - fastest when available
2. **Fallback to cmake:** `cmake --build build --config Debug` - universal, works on all systems

**Pattern:** Try the task; if you get "Task not found", use cmake instead. Both methods work correctly.

**Note:** `cmake --build` respects whatever build system is configured (Ninja, Make, MSBuild, etc.).

### Build Verification Protocol (CRITICAL)

**Verification workflow:**
1. Build using `run_task` or `run_in_terminal`
2. **Check exit code** - Must be 0 for success
3. **Look for error indicators** in output:
   - `error:` or `error C` (compiler errors)
   - `undefined reference`/`unresolved external symbol` (linker errors)
   - `FAILED:`, `ninja: build stopped:` (build system errors)
   - `cannot find`/`No such file`, `fatal error` (missing dependencies)
   - `compilation terminated` (fatal compiler errors)
4. **Success patterns:** `ninja: no work to do.` or final linking with no errors
5. **If errors found:**
   - Analyze and fix root cause in source code
   - Re-run build to verify the fix

# Language-Specific Coding Standards

## C++ (Primary Language)

### Naming Conventions
- Variables: `snake_case` (minimum 3 characters, except loop counters `i`, `j`, `k`)
- Classes/Methods/Functions: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Enum types: `PascalCase` (singular, e.g., `Color` not `Colors`)
- Enum values: `snake_case`

### Code Style
- Indentation: 4 spaces
- Line length: 100 characters maximum
- Use blank lines to separate logical code sections

### Function Declarations and Definitions
- Always use trailing return type syntax for both declarations and definitions: `auto FunctionName() -> ReturnType`
- Functions returning `bool` must have `[[nodiscard]]` attribute

### Enum Guidelines
- Use `enum class` for type safety (never traditional enums)
- Specify underlying type using `std::` types (e.g., `std::uint8_t`)
- Example: `enum class Color : std::uint8_t { red, green, blue };`

### Conditionals
- Always use braces for conditional statements, even single-line statements
- Always use braces for statements that are part of `while` and `for` statements, even single-line statements
- Do not use `else` after a conditional that exits (return, throw, break)

### Modern C++ Features
- Prefer `auto` for variable declarations when type is obvious, but use lint-friendly variants:
  - `auto*` for pointer assignments (e.g., `auto* node = GetNode();`)
  - `const auto` for const values (e.g., `const auto count = GetCount();`)
  - `const auto&` for const references (e.g., `const auto& name = GetName();`)
  - `auto&` for non-const references
- Use `constexpr` for compile-time constants and functions
- Use range-based `for` loops over traditional loops when iterating containers
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) instead of raw pointers
- Use structured bindings for clarity: `auto [x, y] = get_point();`
- Prefer C++23 ranges library algorithms over manual loops when applicable
- **Prefer prefix increment/decrement (`++i`, `--i`) over postfix (`i++`, `i--`)** – more efficient for iterators and user-defined types, better practice even for built-in types
- **Discard return values using `std::ignore = `** not `(void)` – modern C++17+ idiom for intentionally discarding function return values

### Library Priority (in order)
1. **C++ Standard Library (`std::`)** – Always check here first
2. **wxWidgets library** – Use when:
   - Standard library doesn't provide functionality
   - Building strings with `<<` operator (cleaner than std::string `+=`)
   - Using wx-specific features (Printf, MakeLower, Format, etc.)
3. **`wxue::` namespace** (from `src/wxue_namespace/`) – Project-specific string utilities
4. **Frozen containers** – Immutable collections (`frozen/include/frozen`)
5. **❌ NEVER `src/tt/` types** – See Legacy Code Restrictions
6. **❌ NEVER `ttwx::` namespace** – Obsolete, use `wxue::` equivalents

### String Type Selection Guide

| Use Case | Recommended Type |
|----------|------------------|
| Utility methods needed (locate, contains, trim, file paths) | `wxue::string` |
| Parsing with in-place view modification | `wxue::string_view` |
| Building strings with `<<` operator chaining | `wxString` |
| Passed directly to wxWidgets APIs | `wxString` |
| wx-specific features (Printf, Format, MakeLower) | `wxString` |
| Pure internal processing, no special methods | `std::string` |
| Lightweight views, no utility methods needed | `std::string_view` |

**Notes:**
- `wxue::string` extends `std::string` – all standard algorithms work
- `wxue::string_view` extends `std::string_view` – efficient parsing with `moveto_*()` methods
- Both provide `wx()` method for efficient `wxString` interop (requires `wxUSE_UNICODE_UTF8`)
- Header: `#include "wxue_namespace/wxue_string.h"` (includes both classes)

### Creating Helper Functions
- **Within a class and needing access to class members:** Create **private class methods**
  - If state information needs to be shared between two or more helper methods, add the state as a **private class member variable**
  - This approach keeps related functionality encapsulated within the class and makes the interface clearer
- **No access to class members needed:** Place the function in an **anonymous namespace**
  - This provides internal linkage and avoids polluting the global namespace
  - **Do NOT use `static` keyword** - anonymous namespace already provides internal linkage
  - Preferred over `static` functions at file scope in C++

### Frozen Containers (Immutable)
```cpp
#include <frozen/map.h>
#include <frozen/set.h>
#include <frozen/unordered_map.h>

// Compile-time immutable map
constexpr auto color_map = frozen::make_map<std::string_view, int>({
    {"red", 0xFF0000},
    {"green", 0x00FF00}
});
```

## Perl Coding Standards

### Naming Conventions
- Variables: `snake_case` | Packages: `PascalCase` | Wx:: methods: `PascalCase` | Constants: `UPPER_SNAKE_CASE`
- Begin wxWidgets functions with `Wx:`

### Code Style
- Indentation: 4 spaces | Line length: 100 chars max | Blank lines separate logical sections

## Python Coding Standards

### Naming Conventions
- Variables/Functions: `snake_case` | Packages: `PascalCase` | `wx.` methods: `PascalCase` | Constants: `UPPER_SNAKE_CASE`
- Begin wxWidgets functions with `wx.`

### Code Style
- Indentation: 4 spaces | Line length: 90 chars max | Blank lines separate logical sections

## Ruby Coding Standards

### Naming Conventions
- Variables: `snake_case` | Packages: `PascalCase` | Wx:: methods: `PascalCase` | Constants: `UPPER_SNAKE_CASE`
- Begin wxWidgets functions with `Wx:`

### Code Style
- Indentation: 2 spaces | Line length: 80 chars max | Blank lines separate logical sections

## Generated Languages (Perl, Python, Ruby)

### Quick Reference Matrix

| Aspect | Perl | Python | Ruby |
|--------|------|--------|------|
| Variables/Functions | `snake_case` | `snake_case` | `snake_case` |
| Classes/Packages | `PascalCase` | `PascalCase` | `PascalCase` |
| Constants | `UPPER_SNAKE_CASE` | `UPPER_SNAKE_CASE` | `UPPER_SNAKE_CASE` |
| wxWidgets prefix | `Wx:` | `wx.` | `Wx:` |
| Indentation | 4 spaces | 4 spaces | 2 spaces |
| Line length | 100 chars | 90 chars | 80 chars |
| Special | — | — | `_event` for unused params |

## Critical Reminders for All Code

### When Working on C++ Code
- ✅ Always check if Standard Library provides needed functionality before using external libraries
- ✅ Validate that enum classes use explicit underlying types
- ✅ Ensure all bool/int-returning functions have `[[nodiscard]]` attribute
- ✅ Verify conditionals use braces even for single statements
- ✅ **Respect protected code sections** – Do not edit between "Do not edit" and "End of generated code" markers
- ✅ Node/NodeProperty use `as_view()` methods
- ✅ Generators derive from `src/generate/base_generator.h`
