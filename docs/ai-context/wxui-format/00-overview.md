# .wxui File Format Overview

This document provides a high-level overview of the `.wxui` file format used by wxUiEditor.

## Purpose

The `.wxui` format is an XML-based project file that defines wxWidgets user interface layouts. wxUiEditor reads these files and generates code in multiple languages (C++, Python, Ruby, Perl).

## Key Characteristics

### 1. XML-Based Structure
- Human-readable and editable text format
- Version control friendly (meaningful diffs)
- Hierarchical node structure matches UI component tree
- No namespaces or external DTD references

### 2. Multi-Language Code Generation
From a single `.wxui` file, wxUiEditor can generate:
- **C++**: Base and derived classes with header/source files
- **Python**: wxPython classes
- **Ruby**: wxRuby classes
- **Perl**: wxPerl modules
- **XRC**: wxWidgets XML Resource files

### 3. Optimized Property Storage
- Only non-default property values are stored
- Reduces file size and version control noise
- Missing properties automatically use defaults from wxUiEditor's definitions
- Changes are minimal and focused

### 4. Base/Derived Class Pattern
wxUiEditor generates a **base class** (regenerated) and optionally a **derived class** (user-edited):
- **Base class**: Auto-generated, contains UI construction and protected members
- **Derived class**: User-created, implements event handlers and custom logic
- Protected code markers prevent accidental base class edits

## Basic Structure

A `.wxui` file contains:

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

## File Extension

All wxUiEditor project files use the `.wxui` extension.

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

## Typical Workflow

1. **Create/Open Project**: Use wxUiEditor to create a new `.wxui` file or open an existing one
2. **Design UI Visually**: Add widgets, configure properties, design layout
3. **Bind Events**: Specify event handlers for user interactions
4. **Generate Code**: wxUiEditor creates base classes in target language(s)
5. **Implement Logic**: Write derived classes implementing event handlers
6. **Iterate**: Modify UI in wxUiEditor, regenerate base classes (derived classes unchanged)

## When to Edit `.wxui` Files Manually

**Recommended scenarios:**
- Batch renaming (search/replace with care)
- Fixing merge conflicts from version control
- Debugging generation issues
- Scripted project modifications

**Not recommended:**
- Adding new widgets (use wxUiEditor's visual editor)
- Complex layout changes (easier visually)
- Learning the format (start with simple visual editing)

**Warning**: Manual edits may be overwritten when wxUiEditor saves. Always backup before manual editing.

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

## Quick Start Example

Here's a minimal "Hello World" dialog:

```xml
<?xml version="1.0"?>
<wxUiEditorData data_version="19">
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

## Related Documentation

- **For Contributors**: See [../../contributors/xml_definitions.md](../../contributors/xml_definitions.md) for details on wxUiEditor's internal XML definition system
- **For Users**: See [../../users/wxui_file_format.md](../../users/wxui_file_format.md) for user-friendly format overview
