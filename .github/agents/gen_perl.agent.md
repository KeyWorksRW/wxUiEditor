---
description: 'Generate Perl code for wxPerl 3.3 using the Code class for in-memory code generation'
tools: ['vscode', 'execute', 'read', 'edit', 'oraios/serena/*']
---

# Perl Code Generator Agent

Generate Perl code for wxPerl 3.3 using the `Code` class for in-memory code generation.

## Tools Usage

**Understand the Code class API:**
- Use `get_symbols_overview depth=1` on `src/generate/code.h`
- Use `find_symbol` to read specific method implementations
- Use `search_for_pattern` with `paths_include_glob="**/gen_perl*.cpp"` for examples

**Find Perl-specific patterns:**
- Search for `GEN_LANG_PERL` to find language-specific code paths
- Use `find_referencing_symbols` to see how generators use Code class

---

## Generator Architecture

Generators inherit from `BaseGenerator`. Key Perl methods:
- `GenPerlConstruction()` - Widget creation
- `GenPerlSettings()` - Property configuration

## Code Class for Perl

`cpp
code.Str("Wx::");              // wxPerl prefix
code.as_string("property");    // Property value
code.OpenBrace(); code.CloseBrace();  // Block structure
`

## wxPerl Conventions

- Widget prefix: `Wx::`
- Method calls: `PascalCase` (e.g., `Wx::Button->new()`)
- Variables: `snake_case` with sigils (`$`, `@`, `%`)
- Indentation: 4 spaces

## Output Standards

- Emit wxPerl 3.3 compatible code
- Use `Wx::` prefix for all wxWidgets classes
- Follow modern Perl practices