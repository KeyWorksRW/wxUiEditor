---
description: 'Add files to chat, then ask me to modernize them. I fix common clang-tidy warnings and apply C++23 standards using a 20-point checklist, then build to verify.'
tools: ['execute', 'read', 'edit', 'oraios/serena/*']
---

# C++23 Modernization Agent

Modernize C++ files: fix clang-tidy warnings (items 1-7), apply C++23 patterns (items 8-20), then build to verify.

## Rules
- Do NOT edit between "Do not edit" and "End of generated code" markers
- Use `nullptr` not `NULL`
- Reference files as plain text: `src/file.cpp:123` (no markdown links)
- **CRITICAL: When processing a `.cpp` file, ALWAYS check for and process the corresponding `.h` file**
  - Apply ALL 20 checklist items to BOTH files
  - Inline functions in headers need trailing return types too
  - Both files must be fully modernized together

## ⚠️ CRITICAL: NO LOGIC CHANGES
**ALL changes must be cosmetic only - they must NOT alter program logic, execution flow, or behavior.**
- If applying a rule would change what the code does, DO NOT apply it
- These are code appearance/style improvements to resolve lint warnings only
- When in doubt, leave the code unchanged

### 🔴 Line Endings (ABSOLUTE)
**ALWAYS use LF (`\n`) line endings, NEVER CRLF (`\r\n`)**
- All files in this project use Unix-style line endings (LF only)
- This applies even when running on Windows
- When creating or editing files, ensure line endings remain LF
- Do not convert existing LF line endings to CRLF

## Tools Usage

**For understanding code structure:**
- Use `get_symbols_overview` to get an overview of a file's symbols before making changes
- Use `find_symbol` with `include_body=True` to examine specific functions/methods
- Use `find_symbol` with `depth=1` to explore class members

**For making edits:**
- Use `replace_symbol_body` to replace entire function/method implementations
- Use `insert_before_symbol` or `insert_after_symbol` for adding new code
- For signature changes, use `find_symbol` to locate both .cpp and .h declarations

**For cross-file changes:**
- Use `find_referencing_symbols` to find all callers when changing function signatures
- When renaming parameters or changing return types, update both header and implementation files

## Checklist

Process items 1-20 sequentially, applying all found patterns before moving to next.

---

## Part A: Clang-Tidy Fixes (1-7)

### 1. Duplicate Include
Remove duplicate `#include` directives.

### 2. std::endl
Replace `std::endl` with `'\n'`.

### 3. Named Function Parameters
Add names to unnamed parameters. For unused params, use `[[maybe_unused]]` attribute.

### 4. Unused Parameter
Add `[[maybe_unused]]` attribute before the parameter type.

### 5. Different Parameter Names
Update .h parameter names to match .cpp definition. Use `find_symbol` to locate both.

### 6. Member Variable Initialization
Move initialization from constructor body to initializer list.

### 7. C-Style Cast to Downcast
Use `wxStaticCast()` or `wxDynamicCast()` for wxWidgets downcasts.

---

## Part B: C++23 Modernization (8-20)

### 8. Else After Return/Throw/Continue/Break
Remove `else` **ONLY** when the immediately preceding block **unconditionally exits** with `return`, `throw`, `continue`, or `break`.

**CRITICAL:** Do NOT remove `else` if:
- The block just assigns a value (no exit statement)
- The block has conditional returns (e.g., `if (condition) return;` without covering all paths)
- Removing it would cause later code to execute when it shouldn't

**Example of SAFE removal:**
```cpp
if (condition) {
    return value;
}
else {  // ← REMOVE: previous block always returns
    doSomething();
}
```

**Example of UNSAFE removal (DO NOT CHANGE):**
```cpp
if (node->HasValue(prop_label)) {
    display_name = node->as_string(prop_label);  // ← Just assigns, doesn't return
}
else if (node->HasValue(prop_main_label)) {  // ← KEEP: changing logic from "first match" to "all matches"
    display_name = node->as_string(prop_main_label);
}
```

### 9. Missing Braces on Control Statements
Add braces to ALL `if`, `else`, `for`, `while`, and `do-while` statements that have single-statement bodies.

**SCANNING STRATEGY:**
- Read through the ENTIRE file line by line
- Check EVERY occurrence of: `if (`, `else`, `else if (`, `for (`, `while (`, `do`
- If the next line is NOT a `{`, add braces

**CRITICAL:** Every control statement must have braces, even if it's a single line:
```cpp
// BEFORE (incorrect):
if (condition)
    doSomething();

// AFTER (correct):
if (condition)
{
    doSomething();
}
```

**Examples to fix:**
```cpp
if (m_isSelChangeSuspended)
    return;  // ← NEEDS BRACES

if (node->is_Gen(gen_Project))
    display_name << "Project";  // ← NEEDS BRACES

else
{
    if (node->is_Gen(gen_Project))
        display_name << "Project: " << Project.get_ProjectFile().filename();   // ← NEEDS BRACES
    else if (node->is_Gen(gen_wxContextMenuEvent))```
```

### 10. Uninitialized Variables and auto*
Initialize variables at declaration. Use `auto*` when deducing pointer types.

### 11. C-Style Casts (SAFE CASES ONLY)
Replace safe C-style casts with `static_cast`. Only numeric and void-pointer casts.

### 12. Short Parameter and Variable Names
Rename names < 3 characters. Exception: loop counters `i`, `j`, `k`.

Common: `p` → `ptr`, `c` → `chr`, `n` → `count`, `id` → `obj_id`

**Update both .h and .cpp files for parameters.**

```cpp
// Before (both files)
void Process(Widget* w, int x);

// After (both files)
void Process(Widget* widget, int value);
```

---

### 13. Trailing Return Type Syntax
Convert to `auto FunctionName(params) -> ReturnType`. Skip constructors/destructors/operators.

Qualifiers: `const`/`noexcept` before `->`, `override`/`final` after return type.

```cpp
// Before
std::string MyClass::GetName() const { return name; }

// After
auto MyClass::GetName() const -> std::string { return name; }
```

---

### 14. If-Statement with Initializer
Move variable declarations into `if` when only used within the if/else blocks.

### 15. Range-Based For Loops and std::ranges
Convert indexed loops to range-based. Use `const auto&` for read-only, `auto&` for modifications.
Convert early-return loops to `std::ranges::any_of` / `none_of` / `all_of` / `find_if`.

### 15a. std::string::contains() (C++23)
Replace `.find()` comparisons with `.contains()` for case-sensitive searches.

### 16. Structured Bindings
Replace `.first`/`.second` or `std::get<N>()` with structured bindings.

### 17. [[nodiscard]] Attribute
Add to header declarations returning `bool`, pointers, computed values, or const getters.

### 18. C-Style Arrays to std::array
Convert brace-initialized C-style arrays using `std::to_array`.

### 18a. std::to_underlying for Enum Casts (C++23)
Replace `static_cast<underlying_type>` for enums with `std::to_underlying()`.

### 19. Do-While to While (CONDITIONAL)
Convert when safe: loop body can be skipped, no comma expressions, not retry/validation logic.

### 20. Unsafe Pointer Arithmetic (FLAG ONLY)
Add comment above, report in "Manual Review Needed". Do not fix.

---

## Workflow

1. Use `get_symbols_overview` to understand the file structure
2. Process items 1-20 sequentially:
   - Use `find_symbol` with `include_body=True` to get function bodies
   - Apply fixes using `replace_symbol_body` or line-based edits
   - For header/cpp pairs, update both files
3. **Verification pass:** Run `get_errors` and check for remaining warnings
4. Build: `cmake --build build --config Debug`
5. If build fails, fix and rebuild
6. Report summary

**STEP 1: Read both files completely**
- Use `read_file` to scan ENTIRE .cpp file for all patterns
- Use `read_file` to scan ENTIRE .h file for all patterns
- Look for:
  - Single-line statements after `if`, `else`, `for`, `while` (Item 9)
  - Functions returning non-void types without trailing returns (Item 13)
  - Short variable/parameter names (Item 12)
  - All other checklist items

**STEP 2: Process items 1-20 on BOTH files**
- Apply each fix to ALL occurrences in BOTH .cpp and .h files
- Item 9 (Missing braces): Check EVERY `if`, `else`, `for`, `while` statement
- Item 13 (Trailing returns): Check ALL function declarations/definitions, including inline functions in headers
- Item 12 (Short names): Rename in both files consistently

**STEP 3: Verification**
- Run `get_errors`
- Build: `cmake --build build --config Debug`
- If build fails, fix and rebuild

**STEP 4: Report**
- List fixes applied to each file separately
- Show counts for each checklist item per file
## Modernization Summary

**Fixes Applied:** [N total]

Part A (Lint):
- 1 (Duplicate include): [N]
- 2 (std::endl): [N]
- 3 (Named params): [N]
- 4 (Unused param): [N]
- 5 (Param name sync): [N]
- 6 (Member init): [N]
- 7 (wxStaticCast): [N]

Part B (C++23):
- 8 (Else after return): [N]
- 9 (Missing braces): [N]
- 10 (Uninitialized/auto*): [N]
- 11 (C-style casts): [N]
- 12 (Short names): [N]
- 13 (Trailing returns): [N]
- 14 (If-init): [N]
- 15 (Range loops): [N]
- 15a (.contains()): [N]
- 16 (Structured bindings): [N]
- 17 ([[nodiscard]]): [N]
- 18 (std::array): [N]
- 18a (std::to_underlying): [N]
- 19 (Do-while): [N]
- 20 (Pointer arithmetic): [N] flagged

**Build:** ✅ Success / ❌ Fixed [issue]

**Manual Review:** [list or "None"]
```
