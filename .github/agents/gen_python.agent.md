---
description: 'Generate Python code for wxPython 4.2 using the Code class for in-memory code generation'
tools: ['vscode', 'execute', 'read', 'edit', 'oraios/serena/*']
---

# Python Code Generator Agent

Generate Python code for wxPython 4.2 using the `Code` class for in-memory code generation.

## Tools Usage

**Understand the Code class API:**
- Use `get_symbols_overview depth=1` on `src/generate/code.h`
- Use `find_symbol` to read specific method implementations
- Use `search_for_pattern` with `paths_include_glob="**/gen_python*.cpp"` for examples

**Find Python-specific patterns:**
- Search for `GEN_LANG_PYTHON` to find language-specific code paths
- Use `find_referencing_symbols` to see how generators use Code class

---

## Generator Architecture

Generators inherit from `BaseGenerator`. Key Python methods:
- `GenPythonConstruction()` - Widget creation
- `GenPythonSettings()` - Property configuration

## Code Class for Python

`cpp
code.Str("wx.");               // wxPython prefix
code.as_string("property");    // Property value
code.Indent(); code.Unindent();  // Python indentation (critical!)
`

## wxPython Conventions

- Widget prefix: `wx.`
- Method calls: `PascalCase` (e.g., `wx.Button()`)
- Variables/functions: `snake_case`
- Indentation: 4 spaces (mandatory for Python syntax)

## Output Standards

- Emit wxPython 4.2 compatible code
- Use `wx.` prefix for all wxWidgets classes
- Correct indentation is critical for valid Python
- Follow PEP 8 style guidelines