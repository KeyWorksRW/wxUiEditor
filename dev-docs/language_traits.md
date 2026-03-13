# language_traits.h — Language Traits & Strategy Infrastructure

## Overview

`src/generate/writers/language_traits.h` defines the two-tier system that enables wxUiEditor
to generate correct code for 9+ programming languages from a single set of shared generator
files. Rather than scattering `if (is_cpp()) ... else if (is_python()) ...` checks throughout
every generator, the system separates language differences into:

1. **LanguageTraits** (data-driven) — Static characteristics like boolean literals, comment
   syntax, operator tokens, and indentation. Queried via simple struct field access.
2. **LanguageStrategy** (polymorphic) — Behavioral differences like construction patterns,
   event binding, import systems, and class name mapping that require conditional logic.

Together they replace direct language identity checks with composable, extensible lookups.
Adding a new language requires only a new traits table and strategy class — no changes to
the hundreds of existing generator files.

## LanguageTraits Struct

### Purpose

`LanguageTraits` is a plain struct describing everything about a language that can be
expressed as declarative data. Every code generator accesses traits through the `Code` class
(`code.get_traits()`) or via `GetLanguageTraits(language)`.

### Complete Trait Reference

#### Literal Values

| Trait | Type | Purpose | C++ | Python | Ruby | Fortran | Go | Julia | LuaJIT | Perl | Rust |
|-------|------|---------|-----|--------|------|---------|----|-------|--------|------|------|
| `true_literal` | `string_view` | Boolean true | `"true"` | `"True"` | `"true"` | `".TRUE."` | `"true"` | `"true"` | `"true"` | `"1"` | `"true"` |
| `false_literal` | `string_view` | Boolean false | `"false"` | `"False"` | `"false"` | `".FALSE."` | `"false"` | `"false"` | `"false"` | `"0"` | `"false"` |
| `null_literal` | `string_view` | Null/nil/none | `"nullptr"` | `"None"` | `"nil"` | `"C_NULL_PTR"` | `"nil"` | `"nothing"` | `"nil"` | `"undef"` | `"std::ptr::null_mut()"` |
| `empty_string` | `string_view` | Empty string literal | `"wxEmptyString"` | `"\"\""` | `"\"\""` | `"\"\""` | `"\"\""` | `"\"\""` | `"\"\""` | `"\"\""` | `"\"\""` |

#### Object & Member Access

| Trait | Type | Purpose | C++ | Python | Ruby | Fortran | Go | Julia | LuaJIT | Perl | Rust |
|-------|------|---------|-----|--------|------|---------|----|-------|--------|------|------|
| `self_reference` | `string_view` | This/self keyword | `"this"` | `"self"` | `"self"` | (implicit) | (receiver) | `"self"` | `"self"` | `"$self"` | `"self"` |
| `member_operator` | `string_view` | Member access operator | `"->"` | `"."` | `"."` | `"%"` | `"."` | `"."` | `":"` | `"->"` | `"."` |
| `scope_operator` | `string_view` | Scope resolution | `"::"` | `"."` | `"::"` | `"%"` | `"."` | `"."` | `"."` | `"::"` | `"::"` |

#### Variable Declaration

| Trait | Type | Purpose | C++ | Python | Ruby | Fortran | LuaJIT | Perl | Rust |
|-------|------|---------|-----|--------|------|---------|----|------|------|
| `local_var_keyword` | `string_view` | Local variable keyword | `"auto* "` | `""` | `""` | `"type(c_ptr) :: "` | `"local "` | `"my $"` | `"let "` |

#### Comments & Formatting

| Trait | Type | Purpose | C++ | Python | Ruby | Fortran | Go | Julia | LuaJIT | Perl | Rust |
|-------|------|---------|-----|--------|------|---------|----|-------|--------|------|------|
| `line_comment` | `string_view` | Comment prefix (with trailing space) | `"// "` | `"# "` | `"# "` | `"! "` | `"// "` | `"# "` | `"-- "` | `"# "` | `"// "` |
| `indent_size` | `int` | Spaces per indent level | `4` | `4` | `2` | `3` | `4` | `4` | `4` | `4` | `4` |
| `line_offset` | `size_t` | Indent multiplier for line break calculation | `1` | `1` | `1` | `1` | `1` | `1` | `1` | `1` | `1` |
| `stmt_end` | `string_view` | Statement terminator | `";"` | `""` | `""` | `""` | `""` | `""` | `""` | `";"` | `";"` |

#### Construction & Prefix

| Trait | Type | Purpose | C++ | Python | Ruby |
|-------|------|---------|-----|--------|------|
| `construction_suffix` | `string_view` | Appended after class name in object creation | `""` | `""` | `".new"` |
| `wx_prefix` | `string_view` | wxWidgets identifier prefix | `"wx"` | `"wx."` | `"Wx::"` |

All six FFI languages use `"wx_"` as their `wx_prefix`.

#### Logical Operators

| Trait | Type | Purpose | C++ | Python | Ruby | Fortran | LuaJIT |
|-------|------|---------|-----|--------|------|---------|----|
| `logical_and` | `string_view` | AND operator (with spaces) | `" && "` | `" and "` | `" && "` | `" .AND. "` | `" and "` |
| `logical_or` | `string_view` | OR operator (with spaces) | `" \|\| "` | `" or "` | `" \|\| "` | `" .OR. "` | `" or "` |

#### Block Structure

| Trait | Type | Purpose | C++ | Python | Ruby | Fortran | Go | LuaJIT | Perl | Rust |
|-------|------|---------|-----|--------|------|---------|----|--------|------|------|
| `block_begin` | `string_view` | Block open delimiter | `"{"` | `""` | `""` | `""` | `"{"` | `""` | `"{"` | `"{"` |
| `block_end` | `string_view` | Block close delimiter | `"}"` | `""` | `"end"` | `"end"` | `"}"` | `"end"` | `"}"` | `"}"` |

#### Conditional Syntax

| Trait | Type | Purpose | C++ | Python | Ruby | Fortran | Go | LuaJIT |
|-------|------|---------|-----|--------|------|---------|----|--------|
| `conditional_begin` | `string_view` | If-statement open | `"if ("` | `"if "` | `"if "` | `"if ("` | `"if "` | `"if "` |
| `conditional_end` | `string_view` | If-statement close | `")"` | `":"` | `""` | `") then"` | `""` | `" then"` |

### Classification Enums

#### Family

Reflects the calling convention to wxWidgets:

| Value | Languages | Description |
|-------|-----------|-------------|
| `native_cpp` | C++ | Direct wxWidgets C++ API calls |
| `wx_binding` | Python, Ruby | C wrapper with near-full parity |
| `wx_binding_legacy` | (reserved) | Legacy wxPerl (being replaced) |
| `ffi` | Fortran, Go, Julia, LuaJIT, Perl, Rust | kwxFFI C API with full C++ parity |

#### FeatureParity

Coverage relative to the C++ reference implementation:

| Value | Languages | Meaning |
|-------|-----------|---------|
| `full` | C++, all FFI languages | Everything supported |
| `near_full` | Python, Ruby | Wrapper gaps exist for some features |
| `declining` | (legacy wxPerl) | Being replaced |

#### ConstructionStyle

How widgets are instantiated:

| Value | Pattern | Languages |
|-------|---------|-----------|
| `cpp_new` | `new wxButton(parent, ...)` | C++ |
| `binding_call` | `wx.Button(parent, ...)` / `Wx::Button.new(parent, ...)` | Python, Ruby |
| `ffi_function` | `wx_button_create(parent, ...)` | All FFI languages |

### Feature Flags

| Flag | Type | Purpose |
|------|------|---------|
| `supports_markup` | `bool` | Language supports `SetLabelMarkup()` |
| `supports_lambda_events` | `bool` | Language supports inline event handlers (C++, Rust) |
| `needs_explicit_types` | `bool` | Variable declarations require type annotations (C++, Fortran, Rust, Go) |
| `has_header_files` | `bool` | Language generates separate header files (C++ only) |
| `uses_snake_case_methods` | `bool` | Method names converted to snake_case |
| `removes_empty_parens` | `bool` | Remove trailing `()` from method calls (Ruby) |
| `supports_namespaces` | `bool` | Language has namespace/module support |
| `supports_classes` | `bool` | Language has class/object support |

### Convenience Query Methods

```cpp
is_cpp_family()       // family == Family::native_cpp
is_ffi_family()       // family == Family::ffi
is_binding_family()   // family == wx_binding || wx_binding_legacy
has_full_parity()     // feature_parity == FeatureParity::full
```

## LanguageStrategy — Behavioral Polymorphism

### Purpose

`LanguageStrategy` handles differences that cannot be reduced to simple data lookups —
construction patterns, event binding syntax, import systems, platform conditionals, and
class name transformations.

### Strategy Hierarchy

```
LanguageStrategy (abstract)
│   Holds: const LanguageTraits& m_traits
│
├── CppStrategy
│   Direct C++ wxWidgets — unique construction, #if/#endif platform conditionals
│
└── CWrapperStrategy (abstract)
    All non-C++ languages go through C wrappers
    │
    ├── WxBindingStrategy (abstract)
    │   CPython/Ruby C extensions, near-full parity
    │   │
    │   ├── PythonStrategy
    │   │   wx.Button(parent, ...), if wx.Platform == "msw":
    │   │
    │   └── RubyStrategy
    │       Wx::Button.new(parent, ...), if Wx::PLATFORM == 'WXMSW'
    │
    └── FFIStrategy (abstract)
        kwxFFI C API, full C++ parity, identical naming for all 6 languages
        │
        ├── FortranStrategy     — use module, type :: name, .TRUE./.FALSE.
        ├── GoStrategy          — import "pkg", var x T, { }
        ├── JuliaStrategy       — using Module, dynamic typing
        ├── LuaJITStrategy      — require("mod"), local x, then/end
        ├── PerlStrategy        — use Module, my $x, 1/0 for bools
        └── RustStrategy        — use module, let x: T, lambda events
```

### Abstract Interface

```cpp
class LanguageStrategy {
    // Widget construction (new/create/init patterns)
    virtual void EmitConstruction(Code& code, Node* node) = 0;

    // Event binding (Bind/connect/signal)
    virtual void EmitEventBinding(Code& code, Node* node, NodeEvent* event) = 0;

    // Import/use/module statements
    virtual void EmitImport(Code& code, std::string_view module) = 0;

    // Variable declarations with language syntax
    virtual void EmitVarDecl(Code& code, std::string_view type, std::string_view name) = 0;

    // Feature/property support queries
    virtual auto IsFeatureSupported(Node* node, GenEnum::PropName prop) -> bool = 0;

    // Platform-conditional code (#if, if wx.Platform, etc.)
    virtual void EmitPlatformBegin(Code& code, std::string_view platforms) = 0;
    virtual void EmitPlatformEnd(WriteCode* writer) = 0;

    // Class name transformation (wxButton → wx.Button, Wx::Button, wx_button)
    virtual auto MapClassName(std::string_view wx_class_name) -> std::string = 0;

    // Embedded image data emission
    virtual void EmitImageData(Code& code, const EmbeddedImage* embed, WriteCode* writer) = 0;
};
```

### Key Strategy Behaviors

#### MapClassName — Class Name Transformation

Each language family transforms wxWidgets class names differently:

| Input | C++ | Python | Ruby | FFI (all 6) |
|-------|-----|--------|------|-------------|
| `"wxButton"` | `"wxButton"` | `"wx.Button"` | `"Wx::Button"` | `"wx_button"` |
| `"wxCalendarCtrl"` | `"wxCalendarCtrl"` | `"wx.adv.CalendarCtrl"` | `"Wx::CalendarCtrl"` | `"wx_calendar_ctrl"` |

The FFI conversion uses a PascalCase-to-snake_case algorithm:
```
wxFooBar → wx_ + foo_bar → wx_foo_bar
```

#### EmitPlatformBegin — Platform Conditionals

Each language family emits platform checks differently:

```cpp
// C++:       #if defined(__WINDOWS__)
// Python:    if wx.Platform == "msw":
// Ruby:      if Wx::PLATFORM == 'WXMSW'
// FFI:       if (wx_get_platform() == WX_PLATFORM_WINDOWS)  (with language-specific syntax)
```

## Factory Functions

```cpp
// Get traits struct for a language (nullptr for XRC/XML)
auto GetLanguageTraits(GenLang language) -> const LanguageTraits*;

// Create strategy instance for a language (nullptr for XRC/XML)
auto CreateLanguageStrategy(GenLang language) -> std::unique_ptr<LanguageStrategy>;
```

## Usage Patterns in Code Generators

### Pattern 1: Boolean Literals via Traits

The `Code` class wraps trait access in fluent methods:

```cpp
// In Code class:
auto Code::True() -> Code& {
    *this << m_traits->true_literal;   // "true", "True", "1", ".TRUE."
    return *this;
}

// In a generator:
code.Eol().Str(font_var).VariableMethod("SetUnderlined(").True().EndFunction();
// C++:     font.SetUnderlined(true);
// Python:  font.SetUnderlined(True)
// Perl:    $font->SetUnderlined(1);
// Fortran: call font%SetUnderlined(.TRUE.)
```

### Pattern 2: Indentation from Traits

```cpp
// In GenerateClass() — set indentation from traits
m_source->SetTabToSpaces(GetLanguageTraits(m_language)->indent_size);
// Ruby: 2 spaces, Fortran: 3 spaces, most others: 4 spaces
```

### Pattern 3: Comment Prefix

```cpp
const auto& traits = code.get_traits();
code.Str(traits.line_comment).Str("Auto-generated code");
// C++:     // Auto-generated code
// Python:  # Auto-generated code
// Fortran: ! Auto-generated code
// LuaJIT:  -- Auto-generated code
```

### Pattern 4: Statement Terminator

```cpp
code.Str("widget_create(parent)").Str(traits.stmt_end);
// C++:    widget_create(parent);
// Perl:   widget_create(parent);
// Python: widget_create(parent)
// Ruby:   widget_create(parent)
```

### Pattern 5: Class Name Mapping via Strategy

```cpp
auto mapped = code.get_strategy()->MapClassName("wxButton");
code.Str(mapped).Str("_create(");
// C++:    wxButton(
// Python: wx.Button(
// Ruby:   Wx::Button.new(
// FFI:    wx_button_create(
```

### Pattern 6: Code Class Initialization

The `Code` class caches traits at construction time for fast access throughout code
generation:

```cpp
void Code::Init(Node* node, GenLang language) {
    m_traits = GetLanguageTraits(language);

    if (m_traits) {
        m_indent_size = m_traits->indent_size;
        m_language_wxPrefix = m_traits->wx_prefix;
    }

    // Line break calculation uses traits
    const size_t line_offset = m_traits ? m_traits->line_offset : 1;
    m_break_length -= (static_cast<size_t>(m_indent_size) * line_offset);
}
```

### Pattern 7: Family-Based Branching

When the same logic applies to all languages in a family, use family checks instead of
individual language checks:

```cpp
if (traits.is_cpp_family()) {
    // C++ only: header files, preprocessor directives
}
else if (traits.is_binding_family()) {
    // Python + Ruby: wrapper-specific adjustments
}
else if (traits.is_ffi_family()) {
    // All 6 FFI languages: identical naming, identical constants
}
```

### Pattern 8: FFI Languages Share One Code Path

Because all six FFI languages use identical function/constant naming, most generators need
only a single `is_ffi()` branch:

```cpp
if (code.is_ffi()) {
    // This one block handles Fortran, Go, Julia, LuaJIT, Perl, and Rust
    auto ffi_class = code.get_strategy()->MapClassName("wxButton");
    code.Str(ffi_class + "_create(");
    // Per-language syntax differences are handled by traits automatically
}
```

## Anti-Patterns

### ❌ Hardcoded Literals

```cpp
// WRONG — use traits
code.Str("true");              // Should be: code.True()
code.Str(";");                 // Should be: code.Str(traits.stmt_end)
m_source->SetTabToSpaces(4);   // Should be: traits.indent_size
code.Str("// ");               // Should be: traits.line_comment
```

### ❌ Exhaustive Language Checks

```cpp
// WRONG — use family-based branching
if (code.is_fortran()) { ... }
else if (code.is_go()) { ... }
else if (code.is_julia()) { ... }
else if (code.is_luajit()) { ... }
else if (code.is_perl()) { ... }
else if (code.is_rust()) { ... }

// RIGHT — all FFI languages share one path
if (code.is_ffi()) { ... }
```

## Design Rationale

The traits + strategy system was designed to scale code generation from the original 4
languages (C++, Python, Ruby, XRC) to 10+ languages without combinatorial explosion.

**Key insight:** The six kwxFFI languages (Fortran, Go, Julia, LuaJIT, Perl, Rust) all call
the same C API with identical function and constant names. Their differences are purely
syntactic — how you declare a variable, write a comment, or structure a block. Traits
capture these syntactic differences as data, while the shared `FFIStrategy` base class
handles the common behavioral patterns.

**Adding a new FFI language requires:**
1. A new traits table in `language_traits.cpp` (fill in the struct fields)
2. A new strategy class inheriting from `FFIStrategy` (override only syntax-specific methods)
3. Register both in the factory functions (`GetLanguageTraits`, `CreateLanguageStrategy`)
4. No changes to any shared generator files — they already handle `is_ffi()` generically
