# wxUiEditor Code Architecture

This document provides architectural overviews of key wxUiEditor systems, automatically extracted from AI Context comments in header files.

## Undo/Redo System

**File:** `src/undo_stack.h`

wxUiEditor implements a command pattern-based undo/redo system. The `UndoAction` abstract base class requires derived classes to implement three core methods:
- `Change()` - Applies or re-applies the action (used when pushing to undo stack and during redo)
- `Revert()` - Reverses the action (used during undo)
- `GetMemorySize()` - Returns the memory footprint of the action

The `GroupUndoActions` class allows multiple actions to be treated as a single undoable operation with a unified undo string.

The `UndoStack` manages two vectors (undo and redo stacks):
- `Push()` executes `Change()` and adds the action to the undo stack
- `Undo()` calls `Revert()` on the last action and moves it to the redo stack
- `Redo()` calls `Change()` on the last redone action and moves it back to the undo stack

The stack can be locked to execute actions without affecting undo/redo history, useful for programmatic changes that shouldn't be undoable. Actions may optionally store Node pointers and control UI selection events to maintain proper editor state during undo/redo operations.

---

## XML Generator Definition Schema

**File:** `src/xml/gen.dtd`

wxUiEditor uses a custom XML schema to define how wxWidgets controls, forms, and properties are structured. The DTD file (`gen.dtd`) specifies the grammar for all XML definition files in the `src/xml/` directory (such as `buttons.xml`, `forms.xml`, `sizers.xml`, `interfaces.xml`, etc.).

Each `<gen>` element defines either a wxWidgets control class or an interface (collection of reusable properties). The structure supports:

- **Properties**: Define control attributes with strongly-typed values (string, bool, int, image, wxColour, wxFont, etc.) and optional default values
- **Inheritance**: The `<inherits>` element pulls common properties from interface classes defined in `interfaces.xml`, avoiding XML duplication across similar controls
- **Options**: Enumerated choices for option-type properties (like alignment values or style flags)
- **Events**: Declaration of wxWidgets event types (wxEVT_BUTTON, wxEVT_PAINT, etc.) that controls can handle
- **Categories**: Organizational grouping for properties and events in the UI

These XML files are not compiled directly into wxUiEditor. Instead, the application parses them, strips comments and whitespace, compresses the data using zlib, and embeds it as a base64-encoded unsigned char array in `wxue_data.cpp`. This allows wxUiEditor to define its control catalog in human-readable XML while minimizing the binary size.

---

## wxui Project File Format

**Context:** XML files in `src/xml/` directory

wxUiEditor project files (`.wxui` extension) are XML documents that store complete UI designs. These files follow a structure defined by the XML schema files but use a simplified format optimized for storage.

Key characteristics of `.wxui` files:

- **Hierarchical node structure**: Each control, form, or container is represented as a `<node>` element with a `class` attribute identifying its type (wxButton, wxFrame, wxBoxSizer, etc.)
- **Property storage optimization**: Only properties that differ from their default values are written to the file. When loading, missing properties automatically use defaults defined in the XML schema
- **Property naming**: Property names in `.wxui` files match those in the XML definitions, but wxUiEditor internally prefixes them with `prop_` (e.g., the `hint` property is accessed via `node->as_view(prop_hint)` in code)
- **Hidden properties**: The XML schema can mark properties as hidden via `<hide>` nodes, preventing them from appearing in the Property panel while still being valid in the file format
- **Interface inheritance**: Controls automatically inherit properties from interface classes via `<inherits>` relationships, reducing redundancy in both the XML definitions and generated `.wxui` files
- **Data version tracking**: The root element includes a `data_version` attribute to handle format evolution and backward compatibility

Property names defined in the XML files must exactly match the enumeration values in `gen_enums.cpp` to maintain the mapping between the file format and wxUiEditor's internal representation.

---

*This document is automatically generated from AI Context comments in header files. To add a new section, ensure your header file has a properly formatted AI Context comment block.*
