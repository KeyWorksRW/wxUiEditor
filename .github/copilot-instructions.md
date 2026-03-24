
## Project Context
- **Language:** C++23 with wxWidgets GUI framework
- **Architecture:** Visual UI designer/code generator for wxWidgets applications
- **Platform:** Cross-platform via wxWidgets abstractions (wxDir, wxFileName, etc.)

## Critical Project Rules

### 🚫 Protected Code Sections
**NEVER edit between `// Do not edit any code above` and `// End of generated code` markers**
- Auto-generated sections will be overwritten
- Add modifications AFTER the `// End of generated code` marker only
- If you don't follow this rule, then your changes will be lost on the next code generation run.

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

### Code Generation Verification (CRITICAL)

**wxUiEditor command-line tools for verifying generated code matches committed files.**

**⚠️ IMPORTANT: verify_* commands do NOT write files to disk.** They generate code in memory and compare against existing files. The log file shows what WOULD change if files were written:
- `-` lines = content currently in the file (would be removed)
- `+` lines = content the generator produces (would be added)

**You cannot use `git diff` to verify results** since no files are modified. Analyze the log file directly.

**Syntax:** All options use double-dash (`--`). No short options are defined.

```powershell
# Basic verification (from workspace root)
./bin/Debug/wxUiEditor.exe --verify_cpp <project_file.wxui>

# Verify specific form only
./bin/Debug/wxUiEditor.exe --verify_cpp <project_file.wxui> --form <ClassName>

# Example for this project
./bin/Debug/wxUiEditor.exe --verify_cpp ./src/wxui/wxUiEditor.wxui
```

**Available verification commands:**
- `--verify_cpp` - Verify C++ code generation
- `--verify_perl` - Verify Perl code generation
- `--verify_python` - Verify Python code generation
- `--verify_ruby` - Verify Ruby code generation
- `--verify_all` - Verify all language code generation

**Exit codes:**
- `0` = Success (no differences)
- `1` = Differences detected (check log file)
- `2` = File not found
- `3` = Invalid (e.g., form name not found)

**Log file location:** Same directory as project file, with `.log` extension.
Example: `./src/wxui/wxUiEditor.wxui` → `./src/wxui/wxUiEditor.log`

**⚠️ Common mistakes:**
- ❌ `-v_cpp` - Short options not supported
- ❌ `--verify_cpp=file.wxui` - Project file is positional, not an option value
- ✅ `--verify_cpp file.wxui` - Correct format

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

## Critical Reminders for All Code

### When Working on C++ Code
- ✅ Always check if Standard Library provides needed functionality before using external libraries
- ✅ Validate that enum classes use explicit underlying types
- ✅ Ensure all bool/int-returning functions have `[[nodiscard]]` attribute
- ✅ Verify conditionals use braces even for single statements
- ✅ **Respect protected code sections** – Do not edit between "Do not edit" and "End of generated code" markers
- ✅ Node/NodeProperty use `as_view()` methods
- ✅ Generators derive from `src/generate/base_generator.h`
