# .wxui File Format Overview

This document provides a high-level overview of the `.wxui` file format used by wxUiEditor.

<!-- REVIEW:DONE @2026-02-07 -->
## Purpose

The `.wxui` format is an XML-based project file that defines wxWidgets user interface layouts. wxUiEditor reads these files and generates code in multiple languages (C++, Python, Ruby, Perl).

<!-- REVIEW:DONE @2026-02-07 -->
## Key Characteristics

### 1. XML-Based Structure
- Human-readable and editable text format
- Version control friendly (meaningful diffs)
- Hierarchical node structure matches UI component tree
- No namespaces or external DTD references

### 2. Multi-Language Code Generation
From a single `.wxui` file, wxUiEditor can generate:
- **C++**: Base and derived classes with header/source files
- **Fortran**: kwxFortran classes
- **GO**: kwxGO classes
- **Julia**: kwxJulia classes
- **luaJIT**: kwxLuaJIT classes
- **Perl**: kwxPerl modules
- **Python**: wxPython classes
- **Ruby**: wxRuby classes
- **Rust**: kwxRust modules
- **XRC**: wxWidgets XML Resource files

Each project has a `code_preference` property that specifies the primary language for code generation. When creating a new project, only the preferred language should be enabled in the `generate_languages` property. Additional languages can be enabled later if needed for multi-language generation.

### 3. Optimized Property Storage
- Only non-default property values are stored
- Reduces file size and version control noise
- Missing properties automatically use defaults from wxUiEditor's definitions
- Changes are minimal and focused

### 4. C++ Base/Derived Class Pattern

wxUiEditor generates a **base class** (auto-regenerated) and optionally a **derived class** (user-edited):
- **Base class**: Auto-generated, contains UI construction code
  - Contains protected/private member variables for widgets
  - Protected virtual event handler methods
  - Protected code markers prevent accidental edits
  - Regenerated whenever .wxui file changes
- **Derived class**: Optional (enabled by default), user-created for custom logic
  - Inherits from base class
  - Implements event handler overrides
  - Never overwritten by wxUiEditor
- **Non-derived class mode**: When derived class is disabled
  - Base class members may be changed to private instead of protected
  - User can either edit the base class directly (after "End of generated code" marker), or create a separate source file including the generated base class header file.

<!-- REVIEW:DONE @2026-02-07 -->
## Basic Structure

A `.wxui` file contains contents similar to the following:

```xml
<?xml version="1.0"?>
<wxUiEditorData data_version="19">
  <node class="Project" ...>
    <!-- Project settings -->

    <node class="Images">
      <!-- Embedded images -->
    </node>

    <node class="wxFrame" class_name="MainFrame" ...>
      <!-- Form definition with widgets and sizers -->
    </node>

    <node class="wxDialog" class_name="MyDialog" ...>
      <!-- Another form -->
    </node>
  </node>
</wxUiEditorData>
```

### Core Components

1. **Project Metadata**
   - Target languages and code preferences
   - Image and data directories
   - CMake integration settings
   - Code generation options

2. **Widget Definitions**
   - Buttons, text controls, lists, trees, etc.
   - Properties (labels, sizes, colors, etc.)
   - Variable names for code generation

3. **Layout Information**
   - Sizers for automatic layout
   - Size and position constraints
   - Alignment and spacing rules

4. **Event Handler Bindings**
   - Map wxWidgets events to handler functions
   - Handler names for virtual function generation

5. **Resource Management**
   - Embedded images
   - Embedded data files
   - External resource references

<!-- REVIEW:DONE @2026-02-07 -->
## File Extension

All wxUiEditor project files use the `.wxui` extension.

<!-- REVIEW:DONE @2026-02-07 -->
## Design Philosophy

### Declarative UI Definition
The `.wxui` file describes **what** the UI should look like, not **how** to construct it. wxUiEditor handles the code generation details.

### Single Source of Truth
One `.wxui` file serves as the authoritative UI definition for all target languages. This ensures consistency across language implementations.

### Minimize User Editing
While `.wxui` files are text-based and can be manually edited, wxUiEditor is designed for **visual editing**. The format prioritizes:
- Readability for debugging
- Version control clarity
- Robustness against manual errors

### Forward and Backward Compatibility
The `data_version` attribute enables:
- Opening projects from older wxUiEditor versions
- Automatic migration to current format
- Future format evolution without breaking existing projects

**Determining the correct `data_version`:**
When creating a new `.wxui` project file, AI tools should:

1. Check if wxUiEditor is available (in system PATH or via Windows file association)
2. If available, execute: `wxUiEditor --data-version`
   - If the command returns an integer > 15, use that value
   - If the command fails or returns invalid data, fall back to step 3
3. Default to `data_version="20"` (compatible with wxUiEditor 1.2.1+)

Example (PowerShell):
```powershell
$dataVersion = 20  # default fallback
try {
    $result = & wxUiEditor.exe --data-version 2>$null
    if ($result -match '^\d+$' -and [int]$result -gt 15) {
        $dataVersion = [int]$result
    }
} catch {
    # wxUiEditor not found or command failed, use default
}
```

**Note:** The `--data-version` command-line option is available in wxUiEditor 1.3.0 and later. Earlier versions will not recognize this command. On Windows, the installer registers wxUiEditor with a `.wxui` file association, making it accessible even if not in the system PATH.

<!-- REVIEW:DONE @2026-02-07 -->
## Typical User Workflow

1. **Create/Open Project**: Use wxUiEditor to create a new `.wxui` file or open an existing one
2. **Design UI Visually**: Add widgets, configure properties, design layout
3. **Bind Events**: Specify event handlers for user interactions
4. **Generate Code**: wxUiEditor creates base classes in target language(s)
5. **Implement Logic**: Add custom code (C++ uses derived classes; other languages add code after the generated section)
6. **Iterate**: Modify UI in wxUiEditor, regenerate base classes (custom code unchanged)

<!-- REVIEW:DONE @2026-02-07 -->
## When to Edit `.wxui` Files Manually

**Recommended scenarios:**
- Batch renaming (search/replace with care)
- Fixing merge conflicts from version control
- Scripted project modifications
- Adding new widgets (use wxUiEditor's visual editor)
- Complex layout changes (easier visually)

**Warning**: Manual edits *will* be overwritten if wxUiEditor is running with the same project file loaded and then saves. Always backup before manual editing.

<!-- REVIEW:DONE @2026-02-07 -->
## AI-Assisted Development

This documentation enables AI tools to:
- Generate valid `.wxui` files from natural language descriptions
- Modify existing `.wxui` files programmatically
- Validate `.wxui` file structure and properties
- Suggest UI improvements based on wxWidgets best practices
- Translate UI descriptions between different formats

AI tools should:
1. Follow the XML schema strictly (see [01-xml-structure.md](01-xml-structure.md))
2. Use only valid widget classes and properties (see [02-widgets.md](02-widgets.md), [03-sizers.md](03-sizers.md))
3. Respect property data types and value constraints
4. Generate complete, well-formed XML
5. Include only non-default property values
6. Use appropriate variable naming conventions

<!-- REVIEW:DONE @2026-02-07 -->
## Quick Start Example

Here's a minimal "Hello World" dialog:

```xml
<?xml version="1.0"?>
<wxUiEditorData data_version="20">
  <node class="Project" code_preference="C++">
    <node class="wxDialog" class_name="HelloDialog" title="Hello">
      <node class="wxBoxSizer" orientation="wxVERTICAL">
        <node class="wxStaticText" label="Hello, World!" />
      </node>
    </node>
  </node>
</wxUiEditorData>
```

This generates a dialog with a single text label, demonstrating the core structure of `.wxui` files.

<!-- REVIEW:DONE @2026-02-07 -->
## Further Reading

Read the documentation in order for comprehensive understanding:

- **[01-xml-structure.md](01-xml-structure.md)** - XML schema, node structure, property format
- **[02-widgets.md](02-widgets.md)** - Complete widget catalog with properties
- **[03-sizers.md](03-sizers.md)** - Layout management and sizer types
- **[04-events.md](04-events.md)** - Event binding reference
- **[05-code-generation.md](05-code-generation.md)** - How .wxui maps to generated code
- **[06-examples.md](06-examples.md)** - Complete working examples
- **[07-advanced.md](07-advanced.md)** - Advanced features (inheritance, custom widgets, etc.)
- **[08-troubleshooting.md](08-troubleshooting.md)** - Common issues and solutions

<!-- REVIEW:DONE @2026-02-07 -->
## Reference Implementation

For a comprehensive example of `.wxui` format and generated code across multiple languages, see the **wxUiEditor** repository:

**Repository:** https://github.com/KeyWorksRW/wxUiEditor

This repository contains:
- **src/wxui/wxUiEditor.wxui**: An extensive project demonstrating nearly every form type and control
- **Generated code examples** in:
  - `cpp/` - C++ implementation

This is particularly useful for AI tools to:
- Understand how complex `.wxui` projects are structured
- Learn patterns for specific widgets and layout combinations
- Validate generated code against working reference implementations
