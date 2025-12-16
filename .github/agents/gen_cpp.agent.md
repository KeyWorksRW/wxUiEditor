---
description: 'Generate C++11 code for wxWidgets 3.2 using the Code class for in-memory code generation'
tools: ['vscode', 'execute', 'read', 'edit', 'oraios/serena/*']
---

# C++ Code Generator Agent

Generate C++11 code for wxWidgets 3.2 using the `Code` class for in-memory code generation.

## Tools Usage

**Understand the Code class API:**
- Use `get_symbols_overview depth=1` on `src/generate/code.h` first
- Use `find_symbol` with `include_body=True` to read specific method implementations
- Use `find_referencing_symbols` to see usage patterns in existing generators

**Find examples:**
- Use `search_for_pattern` to find generators handling similar widgets
- Use `get_symbols_overview` on generator files to understand structure

---

## Generator Architecture

All generators inherit from `BaseGenerator` (`src/generate/base_generator.h`).

**Key methods to implement:**
- `GenCppConstruction()` - Widget creation code
- `GenCppSettings()` - Property configuration
- `GenCppAfterChildren()` - Post-child-creation code (optional)

## Code Class Core Methods

### String Building
`cpp
code.Str("literal");           // Append literal
code.as_string("property");    // Property value as quoted string
code += variable;              // Append variable
code.Add("key") = "value";     // Property assignment pattern
`

### Structure
`cpp
code.OpenBrace();    code.CloseBrace();   // { }
code.Eol();                               // End of line
code.Indent();       code.Unindent();     // Indentation control
code.FormFunction("SetFont(").EndFunction();  // Function call
`

### Node Access
`cpp
node->as_string(prop_name);    // Get property as string
node->as_int(prop_name);       // Get property as int
node->hasValue(prop_name);     // Check if property set
`

## Output Standards

- Emit **C++11 code** compatible with wxWidgets 3.2
- Follow project naming: `PascalCase` for methods, `snake_case` for variables
- Use `auto*` for pointer assignments
- Include proper indentation via Code class methods