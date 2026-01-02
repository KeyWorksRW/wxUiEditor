---
description: 'Generate Ruby code for wxRuby 1.6.1 using the Code class for in-memory code generation'
tools: ['execute', 'read', 'edit', 'oraios/serena/*']
---

# Ruby Code Generator Agent

Generate Ruby code for wxRuby 1.6.1 using the `Code` class for in-memory code generation.

### 🔴 Line Endings (ABSOLUTE)
**ALWAYS use LF (`\n`) line endings, NEVER CRLF (`\r\n`)**
- All files in this project use Unix-style line endings (LF only)
- This applies even when running on Windows
- When creating or editing files, ensure line endings remain LF
- Do not convert existing LF line endings to CRLF

## Tools Usage

**Understand the Code class API:**
- Use `get_symbols_overview depth=1` on `src/generate/code.h`
- Use `find_symbol` to read specific method implementations
- Use `search_for_pattern` with `paths_include_glob="**/gen_ruby*.cpp"` for examples

**Find Ruby-specific patterns:**
- Search for `GEN_LANG_RUBY` to find language-specific code paths
- Use `find_referencing_symbols` to see how generators use Code class

---

## Generator Architecture

Generators inherit from `BaseGenerator`. Key Ruby methods:
- `GenRubyConstruction()` - Widget creation
- `GenRubySettings()` - Property configuration

## Code Class for Ruby

`cpp
code.Str("Wx::");              // wxRuby prefix
code.as_string("property");    // Property value
code.Str("end");               // Block termination
`

## wxRuby Conventions

- Widget prefix: `Wx::`
- Method calls: `PascalCase` for classes (e.g., `Wx::Button.new`)
- Methods/variables: `snake_case`
- Unused event params: `_event`
- Indentation: 2 spaces
- Block termination: `end`

## Output Standards

- Emit wxRuby 1.6.1 compatible code
- Use `Wx::` prefix for all wxWidgets classes
- Follow Ruby style conventions (2-space indent)
- Use `_event` for unused event parameters
