---
description: 'Implement FFI code generation for kwxFFI languages (Fortran, Go, Julia, LuaJIT, Perl, Rust)'
agents: ['code-sweep', 'research']
tools: [vscode/askQuestions, agent, web, keyworks.key/key_open, keyworks.key/key_term, keyworks.key/key_git, keyworks.key/key_memory, keyworks.key/key_symbols, keyworks.key/key_file_info, keyworks.key/key_bash, keyworks.key/key_read_file, keyworks.key/key_guide, keyworks.key/key_build, keyworks.key/key_grep, keyworks.key/key_rename_symbol, keyworks.key/key_bookmark, keyworks.key/key_edit_file, keyworks.key/key_create_file, keyworks.key/key_create_directory]
---

# FFI Code Generator Agent

Implement kwxFFI code generation for 6 languages: Fortran, Go, Julia, LuaJIT, Perl, Rust.

## Critical Context

Read `.github/copilot-instructions.md` at session start — it contains project-wide coding standards. Read `.private/generator_plan.md` for the Phase 6 tier breakdown.

## Architecture Overview

```
Generator Flow:
  Per-language writer (gen_fortran.cpp etc.)
    → BaseCodeGenerator::GenConstruction() walks node tree
      → Widget generator (gen_frame.cpp etc.) calls ConstructionCode(Code& code)
        → Code class uses traits + strategies for language-specific syntax
```

**Three layers handle language differences:**
1. **LanguageTraits** — Static data (literals, operators, prefixes). Queried via `code.get_traits()`.
2. **LanguageStrategy** — Polymorphic behavior (construction, events, imports). Accessed via `code.get_strategy()` or `m_strategy` in BaseCodeGenerator.
3. **Code class** — Fluent string builder with `is_cpp()`, `is_python()`, `is_ruby()`, `is_ffi()` checks.

### Strategy Hierarchy
```
LanguageStrategy (abstract)
├── CppStrategy
└── CWrapperStrategy (abstract)
    ├── WxBindingStrategy (abstract)
    │   ├── PythonStrategy
    │   └── RubyStrategy
    └── FFIStrategy (abstract)          ← All 6 FFI languages
        ├── FortranStrategy
        ├── GoStrategy
        ├── JuliaStrategy
        ├── LuaJITStrategy
        ├── PerlStrategy
        └── RustStrategy
```

### The Key Simplification

All 6 FFI languages share **ONE naming convention** — identical constant names, identical function names (wx_button_create, wx_frame_create, etc.). The only per-language differences are **pure syntax**: variable declarations, imports, callbacks, string handling. This means:

- **One `is_ffi()` block** per shared generator file covers all 6 languages
- **FFIStrategy::MapClassName()** converts `wxFooBar` → `wx_foo_bar` for all 6
- Per-language strategies only override syntax-specific methods

### 🔴 LanguageTraits Is the Single Source of Truth

**`language_traits.h` defines ALL static language characteristics.** Never hardcode values that traits already provide — always read from traits at runtime.

**Key traits available** (see `LanguageTraits` struct in `language_traits.h`):

| Trait | Purpose | Example Values |
|-------|---------|----------------|
| `indent_size` | Spaces per indent level | 2 (Ruby), 3 (Fortran), 4 (most) |
| `true_literal` / `false_literal` | Boolean literals | `"true"`, `"True"`, `"1"`, `".TRUE."` |
| `null_literal` | Null/nil/none | `"nullptr"`, `"None"`, `"nil"`, `"C_NULL_PTR"` |
| `empty_string` | Empty string literal | `"wxEmptyString"`, `"\"\""` |
| `self_reference` | Self/this reference | `"this"`, `"self"`, `"$self"` |
| `member_operator` | Member access | `"->"`, `"."`, `"%"` |
| `scope_operator` | Scope resolution | `"::"`, `"."` |
| `line_comment` | Comment prefix | `"// "`, `"# "`, `"! "`, `"-- "` |
| `stmt_end` | Statement terminator | `";"`, `""` |
| `wx_prefix` | wxWidgets prefix | `"wx"`, `"wx."`, `"Wx::"`, `"wx_"` |
| `block_begin` / `block_end` | Block delimiters | `"{"` / `"}"`, `""` / `"end"` |
| `conditional_begin` / `conditional_end` | If-statement syntax | `"if ("` / `")"`, `"if "` / `" then"` |

**Access patterns in generator code:**

```cpp
// In GenerateClass() — initialize WriteCode indentation from traits
m_source->SetTabToSpaces(GetLanguageTraits(m_language)->indent_size);

// In widget generators — query traits via Code object
const auto& traits = code.get_traits();
code.Str(traits.true_literal);     // Emits language-correct "true"
code.Str(traits.line_comment);     // Emits language-correct comment prefix
code.Str(traits.stmt_end);         // Emits ";" or "" as appropriate

// Via strategy (for behavioral differences)
auto mapped = code.get_strategy()->MapClassName("wxButton");  // "wx_button" for FFI
```

**❌ NEVER do this:**
```cpp
m_source->SetTabToSpaces(4);           // Hardcoded — use traits.indent_size
code.Str("true");                      // Hardcoded — use traits.true_literal
code.Str(";");                         // Hardcoded — use traits.stmt_end
```

## Key File Map

### Infrastructure (read, rarely edit)
| File | Purpose |
|------|---------|
| `src/generate/writers/language_traits.h` | LanguageTraits struct + LanguageStrategy base |
| `src/generate/writers/language_traits.cpp` | Traits tables + factory functions |
| `src/generate/writers/code.h` | Code class — fluent builder (50+ methods) |
| `src/generate/writers/code.cpp` | Code class implementation |
| `src/generate/writers/gen_base.h` | BaseCodeGenerator — abstract base for writers |
| `src/generate/writers/gen_construction.cpp` | GenConstruction() — walks nodes, calls generators |
| `src/generate/writers/strategy_ffi.h/.cpp` | FFIStrategy base — MapClassName, defaults |
| `src/generate/writers/strategy_cwrapper.h/.cpp` | CWrapperStrategy — shared non-C++ base |

### Per-Language Files (edit for GenerateClass)
| File | Language |
|------|----------|
| `src/generate/writers/gen_fortran.h/.cpp` | Fortran writer (stub) |
| `src/generate/writers/gen_go.h/.cpp` | Go writer (stub) |
| `src/generate/writers/gen_julia.h/.cpp` | Julia writer (stub) |
| `src/generate/writers/gen_luajit.h/.cpp` | LuaJIT writer (stub) |
| `src/generate/writers/gen_perl.h/.cpp` | Perl writer (stub) |
| `src/generate/writers/gen_rust.h/.cpp` | Rust writer (stub) |
| `src/generate/writers/strategy_fortran.h/.cpp` | Fortran syntax overrides |
| `src/generate/writers/strategy_go.h/.cpp` | Go syntax overrides |
| `src/generate/writers/strategy_julia.h/.cpp` | Julia syntax overrides |
| `src/generate/writers/strategy_luajit.h/.cpp` | LuaJIT syntax overrides |
| `src/generate/writers/strategy_perl.h/.cpp` | Perl syntax overrides |
| `src/generate/writers/strategy_rust.h/.cpp` | Rust syntax overrides |

### Shared Widget Generators (edit to add `is_ffi()` branches)
Located in `src/generate/`. Each has a `BaseGenerator` subclass with:
- `ConstructionCode(Code& code)` — widget creation
- `SettingsCode(Code& code)` — property configuration
- `AfterChildrenCode(Code& code)` — post-children code (layout, show, etc.)
- `HeaderCode(Code& code)` — C++ header declarations (skip for FFI)

### Reference Implementations (read for patterns)
| File | Why |
|------|-----|
| `src/generate/writers/gen_python.cpp` | Most complete writer — study GenerateClass() structure |
| `src/generate/writers/gen_ruby.cpp` | Second example — note snake_case differences |
| `src/generate/writers/strategy_python.cpp` | Binding strategy patterns |
| `src/generate/gen_frame_common.cpp` | Complex generator with all 3 language branches |

## FFI Naming Convention

All kwxFFI languages use these mappings:

| wxWidgets C++ | kwxFFI Equivalent |
|--------------|-------------------|
| `wxButton` | `wx_button` (type) |
| `new wxButton(parent, ...)` | `wx_button_create(parent, ...)` |
| `button->SetLabel("text")` | `wx_button_set_label(button, "text")` |
| `wxFooBar` | `wx_foo_bar` (PascalCase → snake_case with wx_ prefix) |
| `wxID_OK` | `WX_ID_OK` (constants: UPPER_SNAKE_CASE) |
| `wxDefaultPosition` | `WX_DEFAULT_POSITION` |
| `wxDefaultSize` | `WX_DEFAULT_SIZE` |

**Construction pattern:** `wx_<classname>_create(parent, id, label, pos, size, style)`

## Workflow: Adding FFI Support to a Generator

### Step 1: Read the existing generator
```
read_file gen_frame_common.cpp   # Understand C++/Python/Ruby patterns
```

### Step 2: Identify what's needed
- **ConstructionCode**: How is the widget created? Translate to `wx_<class>_create(...)`.
- **SettingsCode**: What properties are set after creation? Translate to `wx_<class>_set_<prop>(...)`.
- **AfterChildrenCode**: What happens after children? (Layout, Show, etc.)

### Step 3: Add `is_ffi()` branches
In the shared generator file, add an `else if (code.is_ffi())` block. Use traits/strategies where possible:

```cpp
// Pattern: When traits/Code class handle the differences
// (Many cases need NO is_ffi() branch — traits handle it automatically)

// Pattern: When FFI needs a distinct code path
else if (code.is_ffi())
{
    // FFIStrategy::MapClassName handles wxButton → wx_button
    auto ffi_class = code.get_strategy()->MapClassName("wxButton");
    code.Str(ffi_class + "_create(");
    // ... parameters ...
    code.EndFunction();
}
```

### Step 4: Implement GenerateClass() in per-language writers
Study `gen_python.cpp::GenerateClass()` for the structure. FFI writers need:
1. File header / comment block
2. Import/use/module statements
3. Class/module/type definition
4. Constructor — delegates to shared generators via `GenConstruction()`
5. Event handler stubs (Phase 6 Tier 4)
6. Close class/module

### Step 5: Build
Run `ninja -f build-Debug.ninja` using `key_build`.

## Per-Language Syntax Quick Reference
| Aspect | Fortran | Go | Julia | LuaJIT | Perl | Rust |
|--------|---------|-----|-------|--------|---------|------|
| Variable | `type(c_ptr) :: x` | `var x T` | `x` (dynamic) | `local x` | `my $x` | `let x: T` |
| Import | `use module` | `import "pkg"` | `using Module` | `require("mod")` | `use Module` | `use module` |
| Function def | `subroutine f()` | `func f()` | `function f()` | `function f()` | `sub f` | `fn f()` |
| Comment | `! ` | `// ` | `# ` | `-- ` | `# ` | `// ` |
| Statement end | (none) | (none) | (none) | (none) | `;` | `;` |
| Self reference | (implicit) | (receiver) | `self` | `self` | `$self` | `self` |
| Block begin | `then`/implicit | `{` | (indent) | `then`/`do` | `{` | `{` |
| Block end | `end` | `}` | `end` | `end` | `}` | `}` |

## Tier-by-Tier Approach
Work through Phase 6 tiers in order. For each tier:
1. **Read the relevant generator files** to understand existing C++/Python/Ruby code
2. **Add `is_ffi()` branches** to shared generators (or extend traits/strategies)
3. **Update per-language GenerateClass()** in writer stubs to emit the control
4. **Build** — must be clean (0 errors)
5. **Verify** — `--verify_cpp` must still pass (no C++/Python/Ruby regressions)
6. **Report** what was implemented and what you need the boss to visually verify

## Demo Reference files:
- Fortran: C:\rwCode\wxLanguages\kwxFortran\examples\demo.f90
- GO: C:\rwCode\wxLanguages\kwxGO\examples\demo\main.go
- Julia: C:\rwCode\wxLanguages\kwxJulia\examples\controls_demo.jl
- LuaJIT: C:\rwCode\wxLanguages\kwxLuaJit-dev\examples\demo.lua
