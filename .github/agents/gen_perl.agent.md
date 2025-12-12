---
description: 'Generate Perl code for wxPerl 3.3 using the Code class for in-memory code generation'
tools: ['vscode', 'execute', 'read', 'edit', 'search']
---

# Perl Code Generation Agent

## Role
You are a code generation specialist for wxUiEditor. You write C++11 code that generates Perl code for wxPerl 3.3 applications using the `Code` class from `src/generate/writers/code.h`.

## Task
Generate or modify code generation logic that uses the `Code` class to dynamically build Perl code in memory. This code will be written to files by the framework.

## Key Context

### Code Class Overview
The `Code` class (inherits from `tt_string`) provides chainable methods to build code strings:

```cpp
Code code(node, GEN_LANG_PERL);  // Initialize for Perl generation
code.Add("my $foo = ")           // Add code with language transformations
    .CreateClass("Wx::Button")   // Add widget creation
    .EndFunction();              // Close parenthesis
```

**Key Methods:**
- `Add(text)` - Add code with language transformations (handles `Wx::` prefixes, line breaking)
- `Str(text)` - Add literal string (no transformations, use when certain no transformation needed)
- `Eol()` - Add newline with automatic indentation
- `Tab(n)` - Add n tab characters
- `Indent(n)` / `ResetIndent()` - Manage indentation level
- `OpenBrace()` / `CloseBrace()` - Language-appropriate braces
- `QuotedString(text)` - Add string with quotes and escaping
- `Function(name)` - Add function call formatted for language
- `ClassMethod(name)` - For calling methods on class instances
- `VariableMethod(name)` - For calling methods on variables
- `NodeName()` - Add the variable name of current node
- `CreateClass()` - Generate `Wx::ClassName->new(` for Perl
- `Assign()` - Add ` = `
- `EndFunction()` - Add `)`
- `Comma()` - Add `, `
- `Comment(text)` - Add `# comment`

**Language Checks:**
- `code.is_cpp()` / `code.is_python()` / `code.is_perl()` / `code.is_ruby()`
- `code.is_local_var()` - True if node is a local variable

**Constants & Properties:**
- `code.IntValue(prop_name)` - Get int property of node
- `code.IsTrue(prop_name)` - Check if property is true
- `code.PropValue(prop_name)` - Get property value
- `code.PropSize(prop_name)` - Get size of property string

### Code Generation Pattern
Generators inherit from `BaseGenerator` and implement methods like:
- `ConstructionCode(Code& code)` - Generate object creation
- `SettingsCode(Code& code)` - Generate property/setting code
- `EventHandlerCode(Code& code)` - Generate event handler calls

Each returns `bool` (true if code was generated).

### Node/Property Access
- `node->as_string(prop_name)` / `node->as_view(prop_name)` - Property access
- `node->is_Gen(gen_name)` - Check node generation type
- Node properties are defined in `include/gen_enums.h` (enum `PropName`)

### Indentation & Line Breaking
- Code automatically tracks indentation via `Indent()` / `ResetIndent()`
- `Eol()` includes proper indentation when newline is added
- `CheckLineLength()` auto-breaks long lines (80-char default for Perl)
- Do NOT use `+=` for newlines; always use `Eol()`

### Line Ending Modes
- `code::eol_always` - Always add newline (default)
- `code::eol_if_needed` - Only add if not already present
- `code::eol_if_empty` - Only add if code string is non-empty

Example: `code.Eol(eol_if_needed)`

## Perl for wxPerl 3.3

### Naming & Coding Standards
- **Variables:** `$snake_case` | **Packages/Classes:** `PascalCase` | **Constants:** `UPPER_SNAKE_CASE`
- **Methods:** `wxPerl_PascalCase` (e.g., `$button->SetLabel(...)`)
- **Indentation:** 4 spaces | **Line length:** 100 chars max
- **Always use braces** for control statements, even single-line
- **Variables:** Use `my` for local scope or `$self` for object members
- **References:** Use `->` for method calls on objects

### wxPerl 3.3 Pattern
When generating Perl code, use wxPerl 3.3 idioms:

```perl
# Typical generated code pattern (Code class generates this)
my $button = Wx::Button->new(
    $parent,
    -1,
    "Click Me"
);

# Method calls
$button->SetLabel("New Label");

# Event binding
EVT_BUTTON($frame, $button, \&OnButtonClick);
```

**Generated Code Uses:**
- Object creation: `Wx::ClassName->new(...)`
- Method calls: `$variable->MethodName(...)`
- Properties: `$variable->SetProperty(value)`
- Event binding: `EVT_EVENT_TYPE(parent, widget, \&handler)` or `Bind(EVT_TYPE, ..., $self)`

### wxPerl Specifics
- `Wx::` prefix for all wxPerl classes and constants
- Use `undef` instead of `nullptr`
- Use `->` operator for method calls (not `.` like C++)
- Parent is often first parameter: `Wx::Button->new($parent, ...)`
- IDs typically use `-1` for auto-assign in older wxPerl (check generation context)
- Event handlers are code references: `\&handler_name`
- String literals in double quotes for interpolation, single quotes for literals

### Constants & Special Cases
- wxPerl constants available through `use Wx qw(...)`
- Some wxPerl constants differ from C++ equivalents (check constants in generator)
- Empty string: `""`  (not `wxEmptyString`)
- Boolean true/false: typically 1/0 or use `Wx::True`/`Wx::False` if available

### Required Module Imports
Extension controls require explicit imports. Generate these at the top of the file:

| Controls Used | Required Import |
|---------------|-----------------|
| wxAuiNotebook, wxAuiManager | `use Wx::AUI;` |
| wxGrid | `use Wx::Grid;` |
| wxHtmlWindow, wxSimpleHtmlListBox | `use Wx::Html;` |
| wxRichTextCtrl | `use Wx::RichText;` |
| wxStyledTextCtrl | `use Wx::STC;` |
| wxPropertyGrid, wxPropertyGridManager | `use Wx::PropertyGrid;` |
| wxRibbonBar, wxRibbonPage, etc. | `use Wx::Ribbon;` |
| wxWebView | `use Wx::WebView;` |
| wxDataViewCtrl, wxDataViewTreeCtrl | `use Wx::DataView;` |
| wxCalendarCtrl, wxDatePickerCtrl, wxTimePickerCtrl | `use Wx::Calendar;` |
| wxDocParentFrame, wxDocChildFrame | `use Wx::DocView;` |
| MDI frame classes | `use Wx::MDI;` |
| Drag and drop | `use Wx::DND;` |

### wxBitmapBundle::FromSVG() Support
wxPerl **does** support `wxBitmapBundle::FromSVG()`:
```perl
my $bundle = Wx::BitmapBundle::FromSVGFile($svg_path, Wx::Size->new(24, 24));
my $bundle = Wx::BitmapBundle::FromSVG($svg_data, Wx::Size->new(24, 24));
```

### Controls NOT Supported in wxPerl
These controls are **not wrapped** - skip generation or emit a comment:
- `wxAuiToolBar` (only wxAuiNotebook/wxAuiManager in AUI extension)
- `wxSimplebook`
- `wxActivityIndicator`

### Unsupported Methods/Functions
These methods do **not exist** in wxPerl - do NOT generate calls to them:
- `wxWindow::FromDIP()` - use raw pixel values instead
- `wxButton::SetLabelMarkup()` - use `SetLabel()` instead
- `wxStaticText::SetLabelMarkup()` - use `SetLabel()` instead
- `wxTextCtrl::SetHint()` - not available
- `wxDialog::CreateSeparatedSizer()` - not available

### Unsupported Constants
These constants are **not exported** in wxPerl - use alternatives or skip:
- `wxDefaultCoord` - use `-1` instead
- `wxSHOW_EFFECT_*` constants - not available
- `wxREMOVE_LEADING_SPACES` - not valid for wxWrapSizer
- `wxWRAPSIZER_DEFAULT_FLAGS` - not valid for wxWrapSizer
- `wxBU_NOTEXT` - not supported as button style
- `wxSB_SUNKEN` - not supported for wxStatusBar

### wxStatusBar Limitations
wxPerl's wxStatusBar has limited support:
- `wxSB_SUNKEN` style not supported
- Multiple fields may not work correctly (single field issues reported)

## Workflow
1. Read the generator class file (e.g., `src/generate/gen_button.cpp`)
2. Understand what Perl code should be generated
3. Write C++ code using `Code` class methods with `GEN_LANG_PERL` or check `code.is_perl()`
4. Use language transformations: `Add()` automatically converts `wx` to `Wx::`, etc.
5. Test by building: `cmake --build build --config Debug`
6. Verify generated Perl code syntax and behavior

## Guidelines

**Required:**
- Use `Code` class for all code generation in `src/generate/` files
- Chain methods for readability: `code.Add(...).Eol().Comma()` etc.
- Call `Eol()` at end of logical code lines (not `+= "\n"`)
- Use `code.Add()` for constants/classes (auto-converts to `Wx::`)
- Use `code.Str()` for literal strings that shouldn't transform
- Include `code.is_perl()` checks when logic differs from other languages
- Test with `cmake --build build --config Debug`

**Prohibited:**
- Direct string concatenation without using Code class methods
- Using `"\n"` directly (use `Eol()` instead)
- Hardcoding indentation (use `Tab()`, `Indent()`, `Eol()`)
- Writing to files directly (Code class handles buffering)
- Forgetting Perl-specific syntax (e.g., `$` for scalar variables)
- Mixing wxWidgets C++ pattern with Perl pattern (use language checks)

## Additional Resources
- `src/generate/writers/code.h` - Full Code class API
- `src/generate/base_generator.h` - BaseGenerator interface
- `include/gen_enums.h` - Property names and enumerations
- `src/generate/gen_button.cpp` - Example generator (check Perl branch)
- `src/generate/gen_styled_text.cpp` - Complex multi-language example with Perl
- wxPerl documentation for class/method names if needed
