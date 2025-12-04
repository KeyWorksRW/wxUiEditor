<!--
This file is optimized for Claude Sonnet 4.5 and equivalent high-capability models (e.g., GPT-4o, Gemini 1.5 Pro).
Structure prioritizes efficient parsing with hierarchical organization and clear semantic markers.
-->

# AI Coding Guidelines for wxUiEditor
C++ project generating C++, Perl, Python, and Ruby code for wxWidgets UI applications.

## Project Context
- **Language:** C++20 with wxWidgets GUI framework
- **Architecture:** Visual UI designer/code generator for wxWidgets applications
- **Platform:** Cross-platform via wxWidgets abstractions (wxDir, wxFileName, etc.)
- **Build System:** CMake + Ninja

## Critical Project Rules

### 🚫 Protected Code Sections
**NEVER edit between `// Do not edit any code above` and `// End of generated code` markers**
- Auto-generated sections will be overwritten
- Add modifications AFTER the `// End of generated code` marker only

### 🚫 Legacy Code Restrictions (ABSOLUTE)
**NEVER use `src/tt/` types in new code:** `tt_string`, `tt_string_view`, `tt_cwd`, `tt_view_vector`

**Use instead:**
- Standard Library: `std::string`, `std::string_view`, `std::filesystem::path`
- Project types: `ttwx::ViewVector` (not `tt_view_vector`)
- Exception: Don't refactor existing usage unless explicitly requested

**Note for modernization agent:** When modernizing code, avoid using deprecated `src/tt/` types in any transformations

### ⚡ Performance-Critical Paths
**Directories:** `src/nodes/`, `src/generate/` (executed frequently during code generation)

**Optimization priorities:**
1. In-place operations: `erase()`, `append()` > `substr()`, string concatenation
2. View parameters: `std::string_view` > `std::string`
3. Cache frequently-accessed values: `get_CodePreference()` results
4. Node access: `as_view()` > `as_string()` (always prefer views)

**Complexity refactoring:** When reducing function complexity in `src/nodes/` or `src/generate/`, avoid extracting single-use helpers with complexity <20 (higher threshold due to performance impact)

### 🏗️ Core Architecture

**Node system:**
- `Node` class: `nodes/node.h` → Pass as `Node*` (not `NodeSharedPtr`)
- `NodeProperty` class: `nodes/node_prop.h`
- Access pattern: `node.as_view()` for performance, `as_string()` only when Node unavailable

**Generator system:**
- All generators inherit from `src/generate/base_generator.h`

**String conversions:**
- `wxString::ToStdString()` → `std::string`/`std::string_view` (returns `const std::string&`)
- `wxString::utf8_string()` → Use only for explicit UTF-8 encoding
- `std::format` → Requires `#include <format>`

**Array conversions:**
- `std::to_array` for C-style `char*` arrays → `std::array`

**wxWidgets Headers:**
- Add `#include` when introducing wxWidgets classes
- Exception: `wxString` (precompiled header)
- Placement: Append to existing `wx/` include list (auto-sorted on save), or after top comments with blank lines

**Debugging:**
- Use `ASSERT`, `ASSERT_MSG`, `FAIL_MSG` from `assertion_dlg.h`
- Never use raw `assert()` or `throw`

**UI patterns:**
- Busy cursor: `wxBeginBusyCursor()` / `wxEndBusyCursor()` > `wxBusyCursor` (cross-platform reliability)

## Shell Environment

### Configuration Priority
1. **Check `.private/shell.md` first** - if exists, follow all instructions within
2. **Default**: Use cross-platform commands (git, cmake, ninja) when shell.md absent

### Build Commands
Use these commands for building the project:
- **Debug build:** `cmake --build build --config Debug`
- **Release build:** `cmake --build build --config Release`
- **Via ninja directly:** `ninja -C build -f build-Debug.ninja` (if cmake is not available)

### Build Verification Protocol (CRITICAL)

**Verification workflow:**
1. Use `run_in_terminal` for builds (ignore task completion status)
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

### Local Environment Extensions
When working in a local VS Code environment, additional shell configuration may be available in `.private/shell.md` - check and follow those instructions if present for platform-specific command syntax and custom agent workflows.

### PowerShell Command Reference (Windows)

| Operation | PowerShell Syntax |
|-----------|-------------------|
| Tail file | `Get-Content file.txt \| Select-Object -Last 20` |
| Head file | `Get-Content file.txt -TotalCount 20` |
| Exit code | `$LASTEXITCODE` |
| Navigate | `cd` or `Set-Location` |
| List | `Get-ChildItem` |
| Copy | `Copy-Item` |
| Remove | `Remove-Item` |
| Search | `Select-String` |
| Count | `Measure-Object` |
| Sort | `Sort-Object` |

# Language-Specific Coding Standards

## C++ (Primary Language)

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

### Function Declarations and Definitions
- Always use trailing return type syntax for both declarations and definitions: `auto FunctionName() -> ReturnType`
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
- **Prefer prefix increment/decrement (`++i`, `--i`) over postfix (`i++`, `i--`)** – more efficient for iterators and user-defined types, better practice even for built-in types

### Library Priority (in order)
1. **C++ Standard Library (`std::`)** – Always check here first
2. **wxWidgets library** – Use when standard library doesn't provide functionality
3. **`ttwx::` namespace** (from `src/ttwx/ttwx.h`) – Project-specific utilities
4. **Frozen containers** – Immutable collections (`frozen/include/frozen`)
5. **❌ NEVER `src/tt/` types** – See Legacy Code Restrictions

### Reducing Function Complexity
- When asked to reduce the complexity of a class method, create helper functions as **private class methods** rather than using an anonymous namespace
- If the new private class methods can be made `static`, then do so
- If state information needs to be shared between two or more helper methods, add the state as a **private class member variable**
- This approach keeps related functionality encapsulated within the class and makes the interface clearer

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

## AI Context Documentation (Header Files)

**When to add:** Documenting header files or adding architectural context

**Process:**
1. Analyze: Design patterns, class hierarchy, key methods, system integration
2. Add comment block after license header
3. Update `docs/contributors/architecture.md`

**AI Context format:**
```cpp
// AI Context: [One-sentence "This file implements..." summary]
// [3-8 lines covering: patterns, class roles, method flow, constraints, memory management]
```

**Include:** Architecture, patterns, responsibilities, constraints, behavior
**Exclude:** Implementation details, API docs, history, TODOs

**Example:**
```cpp
// AI Context: This file implements a command pattern-based undo/redo system for wxUiEditor.
// UndoAction is an abstract base class requiring derived classes to implement Change()
// (apply/redo), Revert() (undo), and GetMemorySize(). GroupUndoActions allows multiple actions to
// be treated as a single undoable operation. UndoStack manages two vectors (undo/redo stacks) where
// Push() executes Change() and adds to undo stack, Undo() calls Revert() and moves to redo stack,
// and Redo() calls Change() and moves back to undo stack. The stack can be locked to execute
// actions without affecting undo/redo history. Actions may optionally store Node pointers and
// control UI selection events.
```

**Contributor docs format** (`docs/contributors/architecture.md`):
```markdown
## [Descriptive Title]

**File:** `src/path/to/file.h`

[Convert AI Context to paragraphs. Focus on architectural understanding.]

---
```

Reference: `docs/contributors/ai-context-template.md`
- Documentation lives with code and is version controlled
- Single source of truth automatically extracted to docs

See `docs/contributors/ai-context-template.md` for complete guidelines and examples.

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

## Refactoring Code Generation Functions

When asked to "Refactor codegen function [name]" or when refactoring any code in `src/generate/` or `src/nodes/`:

1. **Make the requested changes** to the function/class
2. **Build the project**: `cmake --build build --config Debug`
3. **Verify code generation unchanged**:
   ```powershell
   cd ..\ wxUiEditor_tests\; ../../wxUiEditor/build/bin/Debug/wxUiEditor.exe --verify_cpp wxUiTesting.wxui; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
   ```
4. On failure (exit code ≠ 0):
   - Read diff: `c:\rwCode\wxUiEditor_tests\quick\quick.log`
   - Analyze changes in generated code
   - Fix refactoring to preserve behavior
   - Rebuild and re-verify
5. **Success criteria:** Build clean + verification exit code 0

## Critical Reminders for All Code

### When Working on C++ Code
- ✅ Always check if Standard Library provides needed functionality before using external libraries
- ✅ Validate that enum classes use explicit underlying types
- ✅ Ensure all bool/int-returning functions have `[[nodiscard]]` attribute
- ✅ Verify conditionals use braces even for single statements
- ✅ **Respect protected code sections** – Do not edit between "Do not edit" and "End of generated code" markers
- ✅ Node/NodeProperty use `as_view()` methods
- ✅ Generators derive from `src/generate/base_generator.h`

## Pre-Submission Checklist

### C++ Code
- [ ] Frozen headers included for immutable containers (if applicable)
- [ ] Protected code sections preserved ("Do not edit" markers)

### Multi-Language Generation
- [ ] Correct indentation: C++/Perl/Python (4 spaces), Ruby (2 spaces)
- [ ] Line length compliance: Ruby (80), Python (90), C++/Perl (100)
- [ ] wxWidgets prefix: Perl/Ruby (`Wx:`), Python (`wx.`)
