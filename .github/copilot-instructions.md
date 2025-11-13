# AI Coding Guidelines for wxUiEditor
C++ project generating C++, Perl, Python, and Ruby code for wxWidgets UI applications.

## Project-Specific Guidelines

### Node Classes
- `Node` class: `nodes/node.h` | Pass `Node*` to functions (not `NodeSharedPtr`)
- `NodeProperty` class: `nodes/node_prop.h`
- **Prefer `as_view()` over `as_string()`** for efficiency in both `Node` and `NodeProperty`
- Use `as_string()` only when Node/NodeProperty is unavailable

### Generator Classes
All generators in `src/generate/` inherit from `src/generate/base_generator.h`

### Protected Code Sections
**Never edit between `// Do not edit any code above` and `// End of generated code` markers** - auto-generated sections will be overwritten. Add modifications after `// End of generated code`.

### String Conversions
- **Use `wxString::ToStdString()`** for `wxString` → `std::string`/`std::string_view`
- Note that wxString::ToStdString() returns const std::string&
- Use `wxString::utf8_string()` only for specific UTF-8 encoding needs
- Files using `std::format` must `#include <format>`
- Avoid `tt_string` and `tt_string_view` in new code

### Array Conversions
Convert C-style `char*` arrays to `std::array` using `std::to_array`

### Debug Assertions
Use `ASSERT`, `ASSERT_MSG`, `FAIL_MSG` from `assertion_dlg.h` (preferred over raw `assert()` or `throw`)

## Agent Instructions

### Code Review - Line Number Accuracy
When reviewing/analyzing code or referencing specific lines:
1. Use `grep_search` tool with file path to find exact line numbers
2. Report line numbers exactly as they appear in search results
3. For multiple instances, include surrounding context for clarity
4. Never estimate - always search to verify line numbers

### Build Verification - Critical Process
When running builds, you MUST verify actual success/failure by examining the command output:
1. **ALWAYS** use `run_in_terminal` tool for builds instead of relying on task completion messages
2. **ALWAYS** check the actual terminal output for failure indicators
3. Check for **FAILURE** indicators in output:
   - `error:`, `error C[0-9]`, `undefined reference`, `unresolved external symbol`
   - `FAILED:`, `ninja: build stopped:`, `cannot find`, `fatal error`
   - `Command exited with code 1` or other non-zero exit codes
   - `compilation terminated` or similar build termination messages
4. Check for **SUCCESS** indicators:
   - `ninja: no work to do.` (already built)
   - Final linking message like `Linking CXX executable` followed by no errors
   - No error indicators present in output
5. **CRITICAL**: If you see "The task succeeded with no problems" but need to verify actual build status, use `run_in_terminal` to execute the build command directly
6. **NEVER** assume success based on task completion alone - always examine actual build output
7. If errors found: analyze error messages, identify root cause, fix issues, then rebuild to verify

### PowerShell Environment Commands
When working in PowerShell environment (Windows):
1. **File Output**: Use PowerShell cmdlets instead of Unix commands:
   - Use `Select-Object -Last 100` instead of `tail -100`
   - Use `Select-Object -First 50` instead of `head -50`
   - Use `Get-Content file.txt | Select-Object -Last 20` for file tail operations
   - Use `Get-Content file.txt -TotalCount 20` for file head operations
2. **Exit Code Checking**: Use `$LASTEXITCODE` to check previous command exit status
3. **Directory Navigation**: Use `cd` or `Set-Location`, both work in PowerShell
4. **File Operations**: Prefer PowerShell cmdlets:
   - `Get-ChildItem` instead of `ls` or `dir`
   - `Copy-Item` instead of `cp`
   - `Remove-Item` instead of `rm`
5. **Text Processing**:
   - Use `Select-String` instead of `grep`
   - Use `Measure-Object` instead of `wc`
   - Use `Sort-Object` instead of `sort`

# Language-Specific Coding Standards

## C++ Standards

### Naming Conventions
- Variables: `snake_case` | Classes/Methods/Functions: `PascalCase` | Constants: `UPPER_SNAKE_CASE`
- Enum types: `PascalCase` (singular: `Color` not `Colors`) | Enum values: `snake_case`

### Code Style
- Indentation: 4 spaces | Line length: 100 chars max | Blank lines separate logical sections

### Function Declarations
- Use trailing return type syntax: `auto FunctionName() -> ReturnType`

### Enums
- Use `enum class` with explicit underlying type: `enum class Color : std::uint8_t { red, green, blue };`

### Conditionals
- Always use braces (even for single-line statements in `if`, `while`, `for`)
- No `else` after exit statements (`return`, `throw`, `break`)

### Modern C++ Features
- Prefer `auto` for variable declarations when type is obvious
- Use `constexpr` for compile-time constants and functions
- Use range-based `for` loops over traditional loops when iterating containers
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) instead of raw pointers
- Use structured bindings for clarity: `auto [x, y] = get_point();`
- Prefer C++20 ranges library algorithms over manual loops when applicable

### Library Priority (in order)
1. **C++ Standard Library (`std::`)** – Always check first
2. **wxWidgets library** – When standard library insufficient
3. **`ttwx::` namespace** (`src/ttwx/ttwx.h`) – Project utilities
4. **Frozen containers** (`frozen/include/frozen`) – Immutable collections

### Immutable Containers
- Use frozen types: `frozen::set`, `frozen::unordered_set`, `frozen::map`, `frozen::unordered_map`
- Include appropriate frozen header | Prefer `constexpr`/`constinit` for compile-time init

### Reducing Function Complexity
- Create helper functions as **private class methods** (not anonymous namespace)
- Make helpers `static` if no instance member access needed
- Share state via **private member variables** when needed across helpers

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
- Variables/Functions: `snake_case` | Classes: `PascalCase` | `Wx::` methods: `PascalCase` | Constants: `UPPER_SNAKE_CASE`
- Begin wxWidgets functions with `Wx:` | Use leading `_` for unused parameters (e.g., `_event`)

### Code Style
- Indentation: 2 spaces | Line length: 80 chars max | Blank lines separate logical sections

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
