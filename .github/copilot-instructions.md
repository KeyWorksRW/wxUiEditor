<!--
This file is optimized for Claude Sonnet 4.5 and equivalent high-capability models (e.g., GPT-4o, Gemini 1.5 Pro).
Structure prioritizes efficient parsing with hierarchical organization and clear semantic markers.
-->

# AI Coding Guidelines for wxUiEditor
C++ project generating C++, Perl, Python, and Ruby code for wxWidgets UI applications.

## Critical Project Rules

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

**Debugging:**
- Use `ASSERT`, `ASSERT_MSG`, `FAIL_MSG` from `assertion_dlg.h`
- Never use raw `assert()` or `throw`

**UI patterns:**
- Busy cursor: `wxBeginBusyCursor()` / `wxEndBusyCursor()` > `wxBusyCursor` (cross-platform reliability)

## Shell Environment

### Configuration Priority
1. **Check `.vscode/shell.md` first** - if exists, follow all instructions within
2. **Default**: Use cross-platform commands (git, cmake, ninja) when shell.md absent

### Build Verification Protocol (CRITICAL)

**Build command:** `cmake --build build --config Debug` (from project root only)

**Verification workflow:**
1. Use `run_in_terminal` for builds (ignore task completion status)
2. Parse terminal output for actual result
3. **Failure patterns:** `error:`, `error C[0-9]`, `undefined reference`, `unresolved external symbol`, `FAILED:`, `ninja: build stopped:`, `cannot find`, `fatal error`, non-zero exit, `compilation terminated`
4. **Success patterns:** `ninja: no work to do.` or final linking with no errors
5. On failure: analyze → fix → rebuild → verify

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

## C++ Standards

### Naming Conventions
- Variable Standards

## C++ (Primary Language)

### Naming & Style
```
Variables:          snake_case
Classes/Methods:    PascalCase
Constants:          UPPER_SNAKE_CASE
Enum types:         PascalCase (singular: Color not Colors)
Enum values:        snake_case

Indentation:        4 spaces
Line length:        100 chars max
Function syntax:    auto FunctionName() -> ReturnType
```

### Type System
```cpp
// Enums - always enum class with explicit type
enum class Color : std::uint8_t { red, green, blue };

// Conditionals - always braced, no else after exits
if (condition) {
    return early;
}
process_next();  // No else needed

// Modern features - prefer when applicable
auto value = compute();                    // When type obvious
constexpr int MAX = 100;                   // Compile-time constants
for (const auto& item : container) {}      // Range-based loops
auto [x, y] = get_point();                 // Structured bindings
std::unique_ptr<T> ptr;                    // Smart pointers
```

### Library Precedence (strict order)
1. **C++ Standard Library** (`std::`) - always check first
2. **wxWidgets** - when standard library insufficient (`wxString`, `wxFileName`, `wxDir`)
3. **`ttwx::` namespace** - project utilities (`src/ttwx/ttwx.h`)
4. **Frozen containers** - immutable collections (`frozen/include/frozen`)
5. **❌ NEVER `src/tt/` types** - see Legacy Code Restrictions

### Frozen Containers (Immutable)
```cpp
#inclI Context Documentation (Header Files)

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

###Generated Languages (Perl, Python, Ruby)

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
   Pre-Submission Checklist

### C++ Code
- [ ] Standard Library checked before external libraries
- [ ] Frozen headers included for immutable containers
- [ ] Enum classes have explicit underlying types
- [ ] Bool-returning functions marked `[[nodiscard]]`
- [ ] All conditionals braced (even single statements)
- [ ] Protected code sections preserved ("Do not edit" markers)
- [ ] Node/NodeProperty use `as_view()` methods
- [ ] Generators derive from `src/generate/base_generator.h`

### Multi-Language Generation
- [ ] Correct indentation: C++/Perl/Python (4 spaces), Ruby (2 spaces)
- [ ] Line length compliance: Ruby (80), Python (90), C++/Perl (100)
- [ ] wxWidgets prefix: Perl/Ruby (`Wx:`), Python (`wx.Codegen Refactoring Protocol

**Applies to:** Any changes in `src/generate/` or `src/nodes/`

**Workflow:**
1. Make requested changes
2. Build: `cmake --build build --config Debug`
3. Verify: `cd ..\wxUiEditor_tests\; ../../wxUiEditor/build/bin/Debug/wxUiEditor.exe --verify_cpp wxUiTesting.wxui; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }`
4. On failure (exit code ≠ 0):
   - Read diff: `c:\rwCode\wxUiEditor_tests\quick\quick.log`
   - Analyze changes in generated code
   - Fix refactoring to preserve behavior
   - Rebuild and re-verify
5. **Success criteria:** Build clean + verification exit code 0

**Standalone verification command:**
