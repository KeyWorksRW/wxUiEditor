---
description: 'Fix clang-tidy warnings. Use "learn warning: [pattern]" to add new warnings, or "fix" to apply fixes from the Warning List.'
tools: ['vscode', 'execute', 'read', 'edit', 'oraios/serena/*']
---

# Lint Fixer Agent

Fix ONLY clang-tidy warnings listed in the Warning List. Build to verify after fixes.

## Rules
- Do NOT edit between "Do not edit" and "End of generated code" markers
- Use `nullptr` not `NULL`
- Reference files as plain text: `src/file.cpp:123` (no markdown links)

## Tools Usage

**For understanding code structure:**
- Use `get_symbols_overview` to get an overview of a file's symbols
- Use `find_symbol` with `include_body=True` to examine specific functions/methods

**For making edits:**
- Use `replace_symbol_body` to replace entire function/method implementations
- Use `insert_before_symbol` or `insert_after_symbol` for adding new code
- For signature changes affecting headers, use `find_symbol` to locate both .cpp and .h declarations

**For cross-file changes:**
- Use `find_referencing_symbols` to find all callers when changing function signatures
- When renaming parameters, update both header and implementation files

---

## Warning List

### 1. Auto Pointer Declarations
**Message:** `'auto ...' can be declared as 'auto *...'`

**Fix:** Use `auto*` when deducing pointer types.

```cpp
// Before
auto widget = GetWidget();

// After
auto* widget = GetWidget();
```

---

### 2. Named Function Parameters
**Message:** `All parameters should be named in a function`

**Fix:** Add names to unnamed parameters. For unused params, use `/* name unused */`.

```cpp
// Before
tt_string GetHelpURL(Node*)

// After
tt_string GetHelpURL(Node* /* node unused */)
```

---

### 3. Statement Should Be Inside Braces
**Message:** `Statement should be inside braces`

**Fix:** Add braces to single-statement `if`, `else`, `for`, `while`.

```cpp
// Before
if (condition)
    DoSomething();

// After
if (condition)
{
    DoSomething();
}
```

---

### 4. Auto Reference Should Be Const
**Message:** `'auto &...' can be declared as 'const auto &...'`

**Fix:** Add `const` to unmodified references.

```cpp
// Before
auto& style = node->as_string(prop_style);

// After
const auto& style = node->as_string(prop_style);
```

---

### 5. Member Variable Initialization
**Message:** `'...' should be initialized in a member initializer of the constructor`

**Fix:** Move initialization to the initializer list.

```cpp
// Before
MyClass::MyClass(Widget* w) : Base()
{
    m_widget = w;
}

// After
MyClass::MyClass(Widget* w)
    : Base(), m_widget(w)
{
}
```

---

### 6. C-Style Cast to Downcast
**Message:** `Do not use C-style cast to downcast from a base to a derived class`

**Fix:** Use `wxStaticCast()` or `wxDynamicCast()` for wxWidgets types.

```cpp
// Before
parent_sizer->Add((wxWindow*) created_object, flags);

// After
parent_sizer->Add(wxStaticCast(created_object, wxWindow), flags);
```

---

### 7. Else After Return
**Message:** `Do not use 'else' after 'return'`

**Fix:** Remove `else` after `return`, `continue`, `break`, or `throw`.

```cpp
// Before
if (found)
    return result;
else
    return nullptr;

// After
if (found)
    return result;
return nullptr;
```

---

### 8. Variable Name Too Short
**Message:** `Variable name '...' is too short, expected at least 3 characters`

**Fix:** Rename parameters and local variables with names < 3 characters. Exception: loop counters in `for` initializers (`for (int i = 0; ...)` is acceptable).

Common renames: `p` → `ptr`, `c` → `chr`, `n` → `count`, `id` → `obj_id`

**Critical:** When renaming function parameters in `.cpp` files, use `find_symbol` to locate and update the corresponding `.h` header declaration.

```cpp
// myfile.h - Before
void Process(Widget* w, int x);

// myfile.cpp - Before
void Process(Widget* w, int x) {
    char c = GetChar();
}

// After (both files)
void Process(Widget* widget, int value);
void Process(Widget* widget, int value) {
    char chr = GetChar();
}
```

---

### 9. Different Parameter Names
**Message:** `Function '...' has a definition with different parameter names`

**Fix:** Update .h parameter names to match .cpp definition. Use `find_symbol` to locate both files.

```cpp
// Header (.h) - Before
auto CreateNode(pugi::xml_node& node, Node* parent) -> NodeSharedPtr;

// Implementation (.cpp)
NodeSharedPtr CreateNode(pugi::xml_node& xml_obj, Node* parent) { ... }

// Header (.h) - After
auto CreateNode(pugi::xml_node& xml_obj, Node* parent) -> NodeSharedPtr;
```

---

### 10. Unused Parameter
**Message:** `Parameter '...' is unused`

**Fix:** Add TODO comment above function and `/* unused */` after parameter.

```cpp
// Before
auto ProcessFile(const Entry& entry, const wxString& path) -> void

// After
// TODO: unused function parameter: path
auto ProcessFile(const Entry& entry, const wxString& path /* unused */) -> void
```

---

### 11. Duplicate Include
**Message:** `Duplicate include`

**Fix:** Remove duplicate `#include` directives.

```cpp
// Before
#include <iostream>
#include <iostream>

// After
#include <iostream>
```

---

### 12. Loop Variable Copy
**Message:** `Loop variable is copied but only used as const reference`

**Fix:** Change `auto` to `const auto&` in range-based for loops.

```cpp
// Before
for (auto item : container)

// After
for (const auto& item : container)
```

---

### 13. Replace Loop with std::ranges
**Message:** `Replace loop by 'std::ranges::any_of()'`

**Fix:** Replace early-return loops with `std::ranges::any_of()`.

```cpp
// Before
for (auto iter : prefixes)
{
    if (line.starts_with(iter))
        return true;
}
return false;

// After
return std::ranges::any_of(prefixes,
    [&line](const auto& prefix) { return line.starts_with(prefix); });
```

---

### 14. std::endl
**Message:** `Do not use 'std::endl' with streams`

**Fix:** Replace `std::endl` with `'\n'`.

```cpp
// Before
std::cout << message << std::endl;

// After
std::cout << message << '\n';
```

---

<!-- ADD MORE WARNINGS HERE -->

---

## Modes

### Fix Mode (Default)
**Usage:** "fix" or "run"

Apply fixes from Warning List to attached files.

### Learn Mode
**Usage:** "learn warning: [pattern]"

1. Use `get_errors` to find matching warnings in attached file
2. Extract real before/after example
3. Add numbered entry to Warning List
4. Report what was added

---

## Workflow

1. Use `get_symbols_overview` to understand the file structure
2. Get warnings with `get_errors`
3. For each warning matching the Warning List:
   - Use `find_symbol` with `include_body=True` to get the function/method
   - Apply the fix using `replace_symbol_body` or line-based edits
   - For parameter changes, also update the header file
4. Build: `cmake --build build --config Debug`
5. If build fails, fix and rebuild
6. Report summary

## Report Format

```
## Lint Fix Summary

**File:** [filename]

**Fixes Applied:** [N total]
- Warning N (name): [count]

**Skipped:** [N] - [types not in list]

**Build:** ✅ Success / ❌ Fixed [issue]
```