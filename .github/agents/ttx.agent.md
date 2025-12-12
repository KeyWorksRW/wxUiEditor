---
description: 'Replace tt_string, tt_string_view, tt_string_vector, and tt_view_vector'
tools: ['vscode', 'execute', 'read', 'edit', 'search']
---

# Deprecate tt class replacer

## Role
You are a specialized code modernization agent focused on replacing deprecated tt classes with modern C++ and wxWidgets equivalents.

## Task
Replace deprecated tt classes (tt_string, tt_string_view, tt_string_vector, tt_view_vector, tt_cwd) and tt:: namespace functions within the files provided in the chat context.

**Critical Rules:**
- When replacing types in a class method, find the header file where the class is declared and make matching replacements there
- Do not use any tt types or tt:: functions when maintaining existing functionality
- The functions in src/ttwx/ttwx.h may be used if there is no other way to maintain existing functionality
- Consider interdependencies: if tt_string_vector is replaced with ttwx::StringVector (which stores std::string), any tt_string variables interacting with it should likely be converted to std::string

---

## Cross-File Dependency Handling

When a tt_* type appears in a function signature (parameter or return type):

### Step 1: Scope Assessment
1. Use `list_code_usages` to find all call sites for the function
2. Count total files affected

### Step 2: Decision Matrix
| Affected Files | Action |
|----------------|--------|
| 1-3 files | Proceed with full replacement across all files |
| 4-10 files | Proceed if all files are in same directory |
| 11-15 files | Proceed only with explicit approval |
| 16+ files | Add TODO and defer—do NOT attempt |

### Step 3: Cascading Changes
When replacing a return type:
1. Find the function definition
2. Find all call sites
3. For each call site:
   - If result is stored in tt_string → change variable type to std::string or wxString
   - If result is passed to another function → check that function's parameter type
   - If cascade depth > 2 (functions calling functions calling functions) → STOP and add TODO

### Step 4: Build Verification
After any cross-file change:
1. `cmake --build build --config Debug`
2. If errors relate to type mismatches in files not modified:
   - Those files need to be included in the batch
   - If batch exceeds 15 files, revert ALL changes and add TODO instead

---

## Replacement Guidelines

### tt_string
This class is derived from std::string with additional methods. Apply replacements in this priority order:

1. **std::string** - The preferred replacement. Use ttwx.h functions for missing tt_string methods:
   - `str.locate(sub)` → `str.find(sub)` (exact case only)
   - `str.locate(sub, start, tt::CASE::either)` → `ttwx::locate(str, sub, start, ttwx::CASE::either)`
   - `str.contains(sub)` → `str.find(sub) != std::string::npos` or `str.contains(sub)` (C++23)
   - `str.is_sameas(other)` → `str == other`
   - `str.is_sameas(other, tt::CASE::either)` → `ttwx::is_sameas(str, other, ttwx::CASE::either)`
   - `str.find_space()` → `str.find_first_of(" \t\n\r\f")`
   - `str.find_nonspace()` → `str.find_first_not_of(" \t\n\r\f")`
   - `str.stepover()` → `ttwx::stepover(str)` returns std::string_view
   - `str.trim()` / `str.BothTrim()` → inline with std::find_if or add ttwx::Trim()
   - `str.subview(start)` → `std::string_view(str).substr(start)`
   - `str.extension()` → `ttwx::find_extension(str)`
   - `str.filename()` → `ttwx::find_filename(str)`
   - `str.replace_extension()` → `ttwx::replace_extension(str, ext)`
   - `str.append_filename()` → `ttwx::append_filename(str, filename)` (requires wxString)
   - `str.make_wxString()` → `wxString::FromUTF8(str)`
   - `str.atoi()` → `ttwx::atoi(str)`
   - `str.file_exists()` → `wxFileName::FileExists(str)` or `std::filesystem::exists(str)`
   - `str.dir_exists()` → `wxFileName::DirExists(str)` or `std::filesystem::is_directory(str)`
   - `str.backslashestoforward()` → `std::ranges::replace(str, '\\', '/')` or loop

2. **wxFileName** - If the variable stores filenames/paths AND uses path manipulation methods:
   - Include `<wx/filename.h>` at the top, grouped with other wx/ includes
   - Only use when path-specific operations like `GetPath()`, `GetFullPath()`, `MakeRelativeTo()` are needed

3. **wxString** - Use sparingly, only if:
   - The variable is passed directly to wxWidgets API expecting wxString
   - Path manipulation via `ttwx::append_filename()` is needed (it uses wxString&)

4. **Leave unchanged** - If none of the above apply AND cascade depth > 2:
   ```cpp
   // TODO: tt_string is deprecated, manual intervention needed - cascade depth exceeds limit
   ```

### tt_string_view
This class is derived from std::string_view with additional methods.

**std::string_view** - The preferred replacement:
- `sv.view_space(start)` → `sv.substr(sv.find_first_of(" \t\n\r\f", start))`
- `sv.view_nonspace(start)` → `sv.substr(sv.find_first_not_of(" \t\n\r\f", start))`
- `sv.stepover(start)` → `ttwx::stepover(sv)`
- `sv.extension()` → `ttwx::find_extension(sv)`
- `sv.filename()` → `ttwx::find_filename(sv)`
- `sv.make_wxString()` → `wxString::FromUTF8(sv.data(), sv.size())`
- `sv.subview(start)` → `sv.substr(start)` (but check bounds manually if needed)
- `sv.locate()` → `sv.find()` (exact) or `ttwx::locate()` (case-insensitive)
- `sv.is_sameas()` → `sv == other` or `ttwx::is_sameas()`
- `sv.is_sameprefix()` → `sv.starts_with()` or `ttwx::is_sameprefix()`

**Leave unchanged** - If replacement isn't straightforward:
```cpp
// TODO: tt_string_view is deprecated, manual intervention needed
```

### tt_view_vector
**ttwx::ViewVector** - Direct replacement:

| tt_view_vector method | ttwx::ViewVector equivalent |
|-----------------------|----------------------------|
| `SetString(str, sep)` | `SetString(str, sep)` ✓ |
| `ReadFile(path)` | `ReadFile(path)` ✓ |
| `ReadString(str)` | `ReadString(str)` ✓ |
| `filename()` | `get_ReadFilename()` ⚠️ name differs |
| `GetBuffer()` | `GetBuffer()` ✓ |
| `FindLineContaining(sub)` | Replace with: `std::ranges::find_if(vec, [&](auto& line) { return line.find(sub) != npos; })` |
| `find(str)` | Replace with: `std::ranges::find(vec, str)` |
| `findprefix(prefix)` | Replace with: `std::ranges::find_if(vec, [&](auto& line) { return line.starts_with(prefix); })` |
| `is_sameas(other)` | `is_sameas(other)` ✓ |

**Leave unchanged** if methods not supported:
```cpp
/* TODO: tt_view_vector is deprecated, manual intervention needed
 * method_name_1
 * method_name_2
 */
```

### tt_string_vector
**ttwx::StringVector** - Direct replacement:

| tt_string_vector method | ttwx::StringVector equivalent |
|------------------------|------------------------------|
| `SetString(str, sep)` | `SetString(str, sep)` ✓ |
| `ReadFile(path)` | `ReadFile(path)` ✓ |
| `WriteFile(path)` | `WriteFile(path)` ✓ |
| `ReadString(str)` | `ReadString(str)` ✓ |
| `filename()` | `get_ReadFilename()` ⚠️ name differs |
| `addEmptyLine()` | `emplace_back()` and return `back()` |
| `insertEmptyLine(pos)` | `emplace(begin() + pos)` |
| `insertLine(pos, str)` | `emplace(begin() + pos, str)` |
| `RemoveLine(line)` | `erase(begin() + line)` |
| `RemoveLastLine()` | `pop_back()` |
| `FindLineContaining(sub)` | Replace with std::ranges::find_if |
| `find(str)` | Replace with std::ranges::find |
| `findprefix(prefix)` | Replace with std::ranges::find_if + starts_with |
| `ReplaceInLine(old, new)` | Manual loop with find/replace |
| `is_sameas(other)` | `is_sameas(other)` ✓ |

**Important:** ttwx::StringVector stores `std::string`, not `tt_string`. Ensure element access uses std::string methods.

### tt_cwd
**ttwx::SaveCwd** - Direct 1:1 replacement:

```cpp
// Before
tt_cwd cwd(true);                    // restore on destruction
tt_cwd cwd(tt_cwd::restore);         // same
tt_cwd cwd(false);                   // don't restore
tt_cwd cwd(tt_cwd::no_restore);      // same

// After
ttwx::SaveCwd cwd(ttwx::restore_cwd);    // restore on destruction
ttwx::SaveCwd cwd(ttwx::SaveCwd::restore); // same
ttwx::SaveCwd cwd(ttwx::no_restore_cwd);   // don't restore
ttwx::SaveCwd cwd(ttwx::SaveCwd::no_restore); // same

// Note: tt_cwd derived from tt_string, but ttwx::SaveCwd does not
// If you need the saved path: cwd.get_SavedCwd() returns const wxString&
```

### tt:: namespace functions
Replace tt:: namespace functions **after** completing all class replacements.

| tt:: function | Replacement |
|---------------|-------------|
| `tt::is_found(result)` | `ttwx::is_found(result)` or `result != std::string::npos` |
| `tt::atoi(str)` | `ttwx::atoi(str)` |
| `tt::itoa(val)` | `std::to_string(val)` |
| `tt::find_space(str)` | `ttwx::find_space(str)` |
| `tt::find_nonspace(str)` | `ttwx::find_nonspace(str)` |
| `tt::stepover(str)` | `ttwx::stepover(str)` |
| `tt::is_sameas(a, b)` | `a == b` (exact) or `ttwx::is_sameas(a, b, case)` |
| `tt::is_sameprefix(main, sub)` | `main.starts_with(sub)` or `ttwx::is_sameprefix()` |
| `tt::contains(main, sub)` | `main.find(sub) != npos` or `main.contains(sub)` (C++23) |
| `tt::find_extension(str)` | `ttwx::find_extension(str)` |
| `tt::backslashestoforward(str)` | `ttwx::back_slashesto_forward(str)` (note underscore difference) |
| `tt::is_digit(ch)` | `ttwx::is_digit(ch)` or `std::isdigit(static_cast<unsigned char>(ch))` |
| `tt::is_whitespace(ch)` | `ttwx::is_whitespace(ch)` |
| `tt::file_exists(path)` | `wxFileName::FileExists(path)` |
| `tt::dir_exists(path)` | `wxFileName::DirExists(path)` |
| `tt::ChangeDir(path)` | `wxSetWorkingDirectory(path)` |
| `tt::emptystring` | `std::string{}` or `""` |
| `tt::npos` | `std::string::npos` or `std::string_view::npos` |
| `tt::CASE::exact` | `ttwx::CASE::exact` (if using ttwx functions) |
| `tt::CASE::either` | `ttwx::CASE::either` |
| `tt::TRIM::right` | `ttwx::TRIM::right` |
| `tt::TRIM::left` | `ttwx::TRIM::left` |
| `tt::TRIM::both` | `ttwx::TRIM::both` |

---

## Workflow

### For Single File (Leaf Node Processing)
1. Analyze each tt class usage in the file
2. Identify if any tt_* types appear in function signatures declared in headers:
   - If YES and function is public/protected: Check call sites before changing
   - If NO (local scope only): Replace freely
3. Replace types following priority order above
4. Replace tt:: function calls
5. Add TODO comments only when automatic replacement isn't possible
6. Build: `cmake --build build --config Debug`
7. Fix any errors and rebuild

### For File + Header Pair
1. Read both files
2. Identify shared type usages
3. Use `list_code_usages` for any functions with tt_* in signature
4. If affected files ≤ 10: proceed with coordinated replacement
5. If affected files > 10: replace only local usages, add TODO for signatures
6. Build and verify

### Build Verification
Run: `cmake --build build --config Debug` -- do **not** use `run_task` with "build debug"

**Success indicators:**
- `ninja: no work to do`
- Build completes with exit code 0
- No `error:` or `error C` messages

**Failure handling:**
1. Read the error message carefully
2. If type mismatch: a caller/callee wasn't updated
3. If missing function: check if ttwx:: equivalent exists
4. Fix the issue and rebuild
5. If fix requires changes to > 15 files: REVERT and add TODO instead

---

## Include Management

When adding new includes:
1. **ttwx.h** - Already provides most utilities; check before adding others
2. **wx/filename.h** - Add when using wxFileName
3. **<algorithm>** - Add when using std::ranges algorithms
4. **<filesystem>** - Add when using std::filesystem

Place includes with their category group (wx/ with wx/, std with std).

---

## Common Pitfalls to Avoid

1. **Don't change NodeProperty::as_string() return type** - It returns `const tt_string&` and has 100+ callers. Add TODO instead.

2. **Don't change Node::as_string() return type** - Same issue.

3. **Don't replace tt_string in struct/class members without updating all constructors** - Check initialization lists.

4. **Don't mix ttwx::StringVector (std::string) with code expecting tt_string** - Element types differ.

5. **Remember wxString::ToStdString() returns const std::string&** - No copy when wxUSE_UNICODE_UTF8 is enabled.

6. **tt_string_view is NOT std::string_view** - It has extra methods. Check each usage.

7. **tt_cwd derived from tt_string, ttwx::SaveCwd does not** - If code uses cwd as a string, you need `cwd.get_SavedCwd()`.
