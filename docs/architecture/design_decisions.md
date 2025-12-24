# Design Decisions

This document records significant architectural and design decisions made during wxUiEditor development, including the context and rationale behind each choice.

---

## Custom String Classes: `wxue::string` and `wxue::string_view`

**Date:** December 2025

### Context

wxUiEditor is a C++ application that generates code for multiple languages (C++, Python, Perl, Ruby) targeting wxWidgets. The codebase requires extensive string manipulation for:

- File path handling across platforms
- Code generation with mixed content (identifiers, literals, formatting)
- Parsing and processing XML project files
- Integration with wxWidgets APIs

### Problem

The project originally used custom string types (`tt_string`, `tt_string_view`) that derived from `std::string` and `std::string_view`. A migration effort was undertaken to replace these with standard types (`std::string`, `wxString`) to improve code readability and reusability.

However, this migration revealed significant issues:

1. **wxString limitations with `std::string_view`:**
   - Construction: `wxString(std::string_view)` exists (explicit)
   - But `operator<<` does not support `std::string_view`
   - No method returns `std::string_view` directly
   - `ToStdString()` returns `const std::string&`, not a view, nor can it be directly edited

2. **Private internals prevent extension:**
   - `wxString` stores its data in a private `m_impl` member
   - Derived classes cannot access the underlying container
   - Cannot add `std::string_view` support via inheritance

3. **Growing wrapper complexity:**
   - Replacing the custom type required adding free functions in a `ttwx::` namespace
   - These utilities grew to compensate for lost functionality
   - The result was a fragmented system: `std::string` + `wxString` + `ttwx::` functions
   - This was arguably *worse* for readability than the original custom type

### Decision

Create a new custom string class `wxue::string` (and `wxue::string_view`) that:

1. Derives from `std::string` (maintaining standard library compatibility)
2. Provides seamless wxString interoperation
3. Includes the utility methods needed for code generation
4. Uses a namespace that reflects the project identity

### Rationale for Namespace Choice

The namespace `wxue::` was chosen over the original `tt_` prefix for several reasons:

| Aspect | `tt_string` (old) | `wxue::string` (new) |
|--------|-------------------|----------------------|
| **Meaning** | Opaque (`tt` was an internal abbreviation) | Clear (`wxue` = **wx** **U**i **E**ditor) |
| **Convention** | C-style prefix | Modern C++ namespace |
| **Consistency** | Mixed (class used prefix, utilities used `tt::`) | Unified namespace |
| **Discoverability** | Requires tribal knowledge | Self-documenting |

### Alternatives Considered

1. **Use `std::string` everywhere with `wxString` only at API boundaries:**
   Rejected because the constant conversion overhead and boundary management added complexity without eliminating the need for utility functions.

2. **Keep `tt_string` as-is:**
   Rejected because the naming was opaque and inconsistent with the `tt::` namespace used for related utilities.

3. **Use `wxString` as the primary type:**
   Rejected due to poor `std::string_view` support and inability to extend via inheritance.

### Consequences

**Positive:**
- Single, consistent string type with clear ownership
- Full `std::string_view` support (construction, `operator<<`, direct access)
- Namespace clearly identifies project-specific code
- Migration path is straightforward (near 1:1 rename from `tt_string`)

**Negative:**
- Still a custom type (but this is unavoidable given requirements)
- Requires documentation for new contributors

---

*Additional design decisions will be documented here as they arise.*
