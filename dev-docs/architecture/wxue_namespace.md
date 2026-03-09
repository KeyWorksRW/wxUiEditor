# wxue Namespace

The `wxue::` namespace contains project-specific utilities for wxUiEditor, located in `src/wxue_namespace/`. These classes are modern replacements for the legacy `tt_*` types (from `src/tt/`) and `ttwx::` namespace functions (from `src/ttwx/`).

## Build Requirements

All wxue:: headers require these wxWidgets configuration macros:
- `wxUSE_UNICODE_UTF8` — Enables efficient `wxString` interop via the `wx()` method
- `wxUSE_UTF8_LOCALE_ONLY` — Required for consistent UTF-8 handling

## Contents

| File | Purpose |
|------|---------|
| `wxue.h` | Core utilities: enums, character classification, path helpers, `SaveCwd` class |
| `wxue_string.h` | `wxue::string` and `wxue::string_view` classes (combined header) |
| `wxue_view_vector.h` | `wxue::ViewVector` — vector of `std::string_view` entries |
| `wxue_string_vector.h` | `wxue::StringVector` — vector of `wxString` entries |

### Implementation Files

- `wxue.cpp` — Namespace function implementations
- `wxue_string.cpp` — `wxue::string` method implementations
- `wxue_string_view.cpp` — `wxue::string_view` method implementations
- `wxue_view_vector.cpp` — `ViewVector` method implementations
- `wxue_string_vector.cpp` — `StringVector` method implementations

## Legacy Type Mapping

| Legacy Type | Replacement |
|-------------|-------------|
| `tt_string` | `wxue::string` |
| `tt_string_view` | `wxue::string_view` |
| `tt_cwd` | `wxue::SaveCwd` |
| `tt_view_vector` | `wxue::ViewVector` |
| `tt_string_vector` | `wxue::StringVector` |
| `ttwx::SaveCwd` | `wxue::SaveCwd` |
| `tt::` functions | `wxue::` equivalents |

## Classes

### `wxue::string`

Extends `std::string` with utility methods for string manipulation and file path operations.

**Key Features:**
- **String manipulation:** `locate()`, `contains()`, `trim()`, `find_space()`, `find_nonspace()`, `stepover()`
- **Case-insensitive comparisons:** `is_sameas()`, `is_sameprefix()`
- **File path operations:** `extension()`, `filename()`, `replace_extension()`, `make_relative()`
- **Mutation methods:** `erase_from()`, `Replace()`, `MakeLower()`, `MakeUpper()`
- **File system operations:** `file_exists()`, `dir_exists()`, `last_write_time()`
- **wxString interop:** `wx()` method returns efficient `wxString` reference

### `wxue::string_view`

Extends `std::string_view` with utility methods for parsing and substring operations.

**Key Features:**
- All read-only methods from `wxue::string` (locate, contains, trim, etc.)
- **Parsing methods:** `moveto_*()` methods that modify the view in-place
- **Note:** `string_view` methods are delegated from `string` to avoid code duplication

### `wxue::SaveCwd`

RAII class that saves the current working directory on construction and optionally restores it on destruction.

**Usage:**
```cpp
{
    wxue::SaveCwd save_cwd;  // Saves current directory
    wxSetWorkingDirectory("/some/other/path");
    // ... do work in other directory ...
}  // Automatically restores original directory
```

### `wxue::ViewVector`

Vector of `std::string_view` entries derived from `std::vector<std::string_view>`.

**Key Features:**
- Parse strings into views based on separators
- Read line-oriented files into memory-efficient views
- Standard vector iteration and algorithms work

**When to use:** Prefer over `StringVector` when you don't need to modify entries (more memory efficient).

### `wxue::StringVector`

Vector of `wxString` entries derived from `std::vector<wxString>`.

**Key Features:**
- Parse strings into separate string copies
- Read line-oriented files into modifiable strings
- Standard vector iteration and algorithms work

**When to use:** Use when you need to add, remove, or modify entries.

## Namespace Functions

### String Utilities

| Function | Purpose |
|----------|---------|
| `locate(str, sub)` | Find substring (case-sensitive by default) |
| `is_sameas(a, b)` | Compare strings (case options) |
| `is_sameprefix(str, prefix)` | Check if string starts with prefix (case options) |
| `find_space(str)` | Find first whitespace character |
| `find_nonspace(str)` | Find first non-whitespace character |
| `stepover(str)` | Skip leading whitespace |
| `atoi(str)` | Convert string to integer |
| `trim(str)` | Trim whitespace (left, right, or both) |

### Character Classification

Safe wrappers that cast to `unsigned char` before calling standard functions:
- `is_alnum()`, `is_alpha()`, `is_digit()`, `is_punct()`, `is_whitespace()`

### File Path Helpers

| Function | Purpose |
|----------|---------|
| `find_extension(path)` | Find position of file extension |
| `find_filename(path)` | Find position of filename in path |
| `append_filename(path, name)` | Append filename to path with proper separator |

## Enums

Defined in `wxue.h` for use with string methods:

- **`TRIM`** — `left`, `right`, `both` (whitespace trimming direction)
- **`CASE`** — `exact`, `either` (case sensitivity for comparisons)
- **`REPLACE`** — `first`, `all` (replacement scope)

## Rationale for Custom String Classes

### Why This Approach Is Valid

1. **The "standard types only" approach was tried and failed.** Migrating to `std::string` + `wxString` + `ttwx::` utilities created a fragmented, arguably worse codebase than the original custom type (see [Design Decisions](design_decisions.md)).

2. **The custom types extend standard types.** This means:
   - All standard algorithms work
   - Implicit conversion to `std::string_view` works
   - Full interop with the standard library
   - They're *extensions*, not foreign types

3. **Precedent in real projects:**
   - **Qt** uses `QString` pervasively
   - **LLVM** has `StringRef` used throughout
   - **Chromium** has `base::StringPiece`
   - **Abseil** provides `absl::string_view`

### Capability Comparison

| Need | `std::string` | `wxString` | `wxue::string` |
|------|---------------|------------|----------------|
| `std::string_view` interop | ✓ | ✗ | ✓ |
| wxWidgets API integration | ✗ (needs conversion) | ✓ | ✓ (via `wx()`) |
| Utility methods (file paths, code gen) | ✗ | ✗ | ✓ |
| Standard algorithm compatibility | ✓ | Partial | ✓ |

### Recommendation

Use `wxue::string` as the default for new code where string manipulation is involved (code generation, file handling, etc.). For truly local, short-lived strings that never interact with wxWidgets or need utilities, plain `std::string` is fine. See [Design Decisions](design_decisions.md) for the full rationale.
