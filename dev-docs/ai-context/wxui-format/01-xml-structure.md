# XML Structure and Schema

This document describes the XML structure and schema of `.wxui` files.

## Root Element

Every `.wxui` file begins with an XML declaration and a root `<wxUiEditorData>` element:

```xml
<?xml version="1.0"?>
<wxUiEditorData data_version="20">
  <!-- Project and UI content -->
</wxUiEditorData>
```

### Required Attributes

- **data_version**: Integer version number of the file format (current: 20)
  - Used for backward compatibility when opening older projects
  - wxUiEditor automatically migrates older versions on load
  - File will be updated to current version on next save only if migration was required

### Schema Notes

- No XML namespace declarations are used
- No DTD reference in the root element (DTD exists separately for definition files)
- All elements use lowercase names
- Attributes use snake_case naming convention

## Project Settings

The first child node must be a `<node>` with `class="Project"`:

```xml
<wxUiEditorData data_version="20">
  <node
    class="Project"
    art_directory="../art"
    code_preference="C++"
    generate_languages="C++"
    internationalize="0"
    optional_comments="1"
    cpp_line_length="110"
    generate_cmake="1"
    cmake_file="wxui_code.cmake">
    <!-- Child nodes (forms, images, etc.) -->
  </node>
</wxUiEditorData>
```

### Project Node Properties

**CRITICAL for AI tools:** `.wxui` files only store properties that differ from their default values. Do NOT include properties with default values when generating `.wxui` files - this keeps files compact and is considered a bug if done incorrectly.

The Project node supports properties organized by category:

#### Language-Independent Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `code_preference` | option | `C++` | Preferred language (C++, Perl, Python, Ruby, XRC) |
| `generate_languages` | bitlist | `C++ | Python | Ruby` | Languages to generate (pipe-separated). For new projects, should only contain the `code_preference` language. |
| `art_directory` | path | `./` | Directory containing image resources |
| `dialog_units` | bool | `1` | Use dialog units for dimensions by default |
| `internationalize` | bool | `0` | Wrap strings in _() macro for i18n |
| `help_provider` | option | `none` | Help provider class (none, wxSimpleHelpProvider, wxHelpControllerHelpProvider) |
| `id_prefixes` | stringlist_semi | (empty) | Optional prefixes for custom IDs (semicolon-separated) |
| `id_suffixes` | stringlist_semi | (empty) | Optional suffixes for custom IDs (semicolon-separated) |
| `optional_comments` | bool | `0` | Add explanatory comments to generated code |

#### C++ Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `wxWidgets_version` | option | `3.2.0` | Minimum wxWidgets version (3.2.0, 3.3.0) |
| `minimum_compiler` | option | `11` | Minimum C++ standard (11, 17) |
| `base_directory` | path | (empty) | Generated base class output directory |
| `derived_directory` | path | (empty) | Generated derived class output directory |
| `source_ext` | option | `.cpp` | C++ source file extension (.cpp, .cc, .cxx) |
| `header_ext` | option | `.h` | C++ header file extension (.h, .hh, .hpp, .hxx) |
| `cpp_line_length` | uint | `110` | Target line length for C++ code |
| `trailing_return_type` | bool | `0` | Generate functions with trailing return type |
| `local_pch_file` | file | (empty) | Precompiled header file to include |
| `src_preamble` | code_edit | (empty) | Code at top of source files after PCH |
| `project_src_includes` | include_files | (empty) | Local headers to include in all source files |
| `name_space` | string | (empty) | Namespace to enclose class declarations |
| `generate_cmake` | bool | `0` | Auto-generate .cmake file when generating C++ |
| `cmake_file` | file | `wxui_code.cmake` | Filename of cmake file to create |
| `cmake_varname` | string | `wxue_generated_code` | Variable name in .cmake file |

#### Perl Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `wxPerl_version` | option | `3.005` | Minimum wxPerl version |
| `perl_output_folder` | path | (empty) | Output folder for all Perl code |
| `perl_line_length` | uint | `80` | Target line length for Perl code |
| `perl_project_preamble` | code_edit | (empty) | Code at top of every Perl file after comment block |

#### Python Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `wxPython_version` | option | `4.2.0` | Minimum wxPython version |
| `python_output_folder` | path | (empty) | Output folder for all Python code |
| `python_line_length` | uint | `90` | Target line length for Python code |
| `python_project_preamble` | code_edit | (empty) | Code at top of every Python file after comment block |

#### Ruby Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `wxRuby_version` | option | `1.6.1` | Minimum wxRuby version (1.5.0, 1.5.5, 1.6.1) |
| `ruby_output_folder` | path | (empty) | Output folder for all Ruby code |
| `ruby_line_length` | uint | `80` | Target line length for Ruby code |
| `disable_rubo_cop` | bool | `0` | Disable all RuboCop warnings in generated code |
| `ruby_project_preamble` | code_edit | (empty) | Code at top of every Ruby file after comment block |

#### XRC Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `xrc_directory` | path | (empty) | Directory to generate XRC files in |
| `xrc_add_var_comments` | bool | `1` | Add var_comments to XRC file |
| `xrc_no_whitespace` | bool | `0` | No leading whitespace (smaller but less readable) |
| `xrc_indent_attributes` | bool | `1` | Indent attributes (ignored if xrc_no_whitespace is 1) |
| `xrc_indent_with_spaces` | bool | `0` | Indent with 2 spaces instead of single tab |
| `combine_all_forms` | bool | `1` | Create single XRC file containing all forms |
| `combined_xrc_file` | file | (empty) | Filename for combined XRC output |
| `xrc_art_directory` | path | `./` | Directory containing images for XRC |

**Version-specific properties:**
- Properties added in later versions will be documented here as they are introduced

**Remember:** Only include properties in generated `.wxui` files if their values differ from the defaults shown above.

## Widget Hierarchy

Widgets are represented as nested `<node>` elements within the Project node:

```xml
<node class="Project">
  <node class="wxFrame" class_name="MainFrame">
    <node class="wxBoxSizer" orientation="wxVERTICAL">
      <node class="wxButton" label="OK" var_name="m_okButton" />
      <node class="wxButton" label="Cancel" var_name="m_cancelButton" />
    </node>
  </node>
</node>
```

### Hierarchy Rules

1. **Project is root**: All forms and resources are children of the Project node
2. **Forms contain layouts**: Frames/Dialogs contain sizers or direct child widgets
3. **Sizers contain widgets**: Sizers organize child widgets
4. **Nesting depth unlimited**: Complex UIs can have deep hierarchies
5. **Order matters**: Nodes appear in the order they're added to parent containers

### Valid Parent-Child Relationships

| Parent Class | Valid Children |
|--------------|----------------|
| `Project` | `wxFrame`, `wxDialog`, `wxPanel`, `folder`, `Images`, `Data` |
| `folder` | Forms (wxFrame, wxDialog, etc.), `sub_folder` |
| `wxFrame`, `wxDialog` | Sizers, individual widgets |
| `wxPanel` | Sizers, individual widgets |
| Sizers | Widgets, nested sizers, spacers |
| `Images` | `embedded_image` |
| `Data` | `data_folder`, `data_string`, `data_xml` |

## Node Structure

Each `<node>` element represents a UI component:

```xml
<node
  class="widget_class_name"
  property1="value1"
  property2="value2">
  <!-- Optional child nodes -->
</node>
```

### Required Attributes

- **class**: Identifies the widget/component type
  - Must match a class defined in wxUiEditor's XML definitions
  - Examples: `wxButton`, `wxTextCtrl`, `wxBoxSizer`, `wxFrame`

### Common Properties (as attributes)

Properties appear as XML attributes on the `<node>` element:

```xml
<node
  class="wxButton"
  var_name="m_button"
  label="Click Me"
  id="wxID_OK"
  style="wxBU_EXACTFIT"
  tooltip="Click this button"
  disabled="0"
  hidden="0" />
```

### Property Storage Optimization

**Critical**: Only properties that differ from their default values are written to the file.

**Example**: If a wxButton has default size (-1,-1), the `size` attribute won't appear in the XML:

```xml
<!-- Default size (-1,-1) - no size attribute needed -->
<node class="wxButton" var_name="m_button" label="OK" />

<!-- Custom size - size attribute required -->
<node class="wxButton" var_name="m_button" label="OK" size="100,30" />
```

This keeps `.wxui` files compact and version-control-friendly.

## Property Naming Conventions

Properties use snake_case and match the names shown in wxUiEditor's Property panel:

| Property Name | Purpose |
|---------------|---------|
| `var_name` | C++ member variable name |
| `class_name` | Generated class name (for forms) |
| `label` | Display text on control |
| `tooltip` | Tooltip text |
| `size` | Width,height dimensions |
| `pos` | X,Y position |
| `minimum_size` | Minimum width,height |
| `maximum_size` | Maximum width,height |
| `foreground_colour` | Text color |
| `background_colour` | Background color |
| `font` | Font specification |
| `style` | Window style flags |
| `window_extra_style` | Extended window styles |
| `borders` | Sizer border flags |
| `border_size` | Sizer border pixels |
| `alignment` | Sizer alignment flags |
| `proportion` | Sizer stretch proportion |

### Internal Property Naming

**For developers**: Internally, wxUiEditor prefixes property names with `prop_`:
- XML/UI: `label`
- Code enum: `prop_label`
- Access: `node->as_view(prop_label)`

This is transparent to AI tools generating `.wxui` files - always use the unprefixed names.

## Variable Naming

The `var_name` property specifies the member variable name for widgets. Since C++ is the most common generated language, variable names follow C++ conventions and are automatically converted for other languages during code generation.

```xml
<node class="wxButton" var_name="m_okButton" label="OK" />
```

### Naming Conventions

1. **Prefix with m_**: Member variables conventionally start with `m_` for C++ class members (protected/private)
   - The `m_` prefix is omitted for local variables
   - When generating code for other languages, wxUiEditor automatically converts the name (e.g., `m_okButton` becomes `ok_button` in Python/Ruby)
2. **Use camelCase**: After the prefix, use camelCase (e.g., `m_myButton`)
3. **Descriptive names**: Choose names that describe the widget's purpose
4. **Unique within class**: Each var_name must be unique within the form

### Generated Code

The `var_name` becomes a protected member in the generated base class:

**XML:**
```xml
<node class="wxDialog" class_name="MyDialog">
  <node class="wxButton" var_name="m_okButton" label="OK" />
</node>
```

**Generated C++ (my_dialog_base.h):**
```cpp
class MyDialogBase : public wxDialog
{
protected:
    wxButton* m_okButton;

public:
    MyDialogBase(wxWindow* parent, ...);
};
```

### Special Cases

- **Forms**: Use `class_name` instead of `var_name` for the class name
- **Sizers**: Must have `var_name` for code generation (e.g., `sizer`), otherwise no variable will be created for the sizer
- **Spacers**: Spacers don't have `var_name` (they're not accessible in code)

## Event Handler Binding

Event handlers are defined as child `<event>` elements:

```xml
<node class="wxButton" var_name="m_button" label="Click Me">
  <event name="wxEVT_BUTTON">OnButtonClick</event>
</node>
```

### Event Element Format

```xml
<event name="event_type">handler_function_name</event>
```

- **name attribute**: wxWidgets event type (wxEVT_BUTTON, wxEVT_TEXT, etc.)
- **Element text**: Name of the handler function (without parentheses or parameters)

### Multiple Events

A widget can have multiple event handlers:

```xml
<node class="wxTextCtrl" var_name="m_textCtrl">
  <event name="wxEVT_TEXT">OnTextChanged</event>
  <event name="wxEVT_TEXT_ENTER">OnEnterPressed</event>
  <event name="wxEVT_SET_FOCUS">OnGainFocus</event>
</node>
```

See [04-events.md](04-events.md) for complete event reference.

## Data Types

Properties use specific data type formats:

### String Types
```xml
label="Simple text"
tooltip="Text with\nescapes"
```

### Boolean Types
```xml
disabled="0"   <!-- false -->
hidden="1"     <!-- true -->
```

### Integer Types
```xml
border_size="5"
proportion="1"
```

### Size/Point Types
```xml
size="100,50"              <!-- width, height -->
pos="10,20"                <!-- x, y -->
minimum_size="200,100"
```

### Color Types
```xml
foreground_colour="#FF0000"           <!-- Hex RGB -->
background_colour="255,128,64"        <!-- Decimal RGB -->
```

### Option Types
```xml
orientation="wxVERTICAL"
code_preference="C++"
```

### Bitlist Types (flags)
```xml
style="wxBORDER_SIMPLE|wxTAB_TRAVERSAL"
borders="wxALL"
alignment="wxALIGN_CENTER|wxALIGN_CENTER_VERTICAL"
```

Multiple flags are combined with the pipe character `|`.

## Complete Minimal Example

Here's a minimal valid `.wxui` file:

```xml
<?xml version="1.0"?>
<wxUiEditorData data_version="19">
  <node class="Project" code_preference="C++">
    <node class="wxDialog" class_name="MyDialog" title="Hello">
      <node class="wxBoxSizer" orientation="wxVERTICAL">
        <node class="wxStaticText" label="Hello World!" />
        <node class="wxButton" var_name="m_button" label="OK" id="wxID_OK">
          <event name="wxEVT_BUTTON">OnOK</event>
        </node>
      </node>
    </node>
  </node>
</wxUiEditorData>
```

This minimal example demonstrates:
- Root structure with data version
- Project node with language preference
- Dialog form with class name and title
- Vertical box sizer for layout
- Static text label
- Button with variable name, label, stock ID, and event handler

## Next Steps

- [02-widgets.md](02-widgets.md) - Complete widget catalog with all properties
- [03-sizers.md](03-sizers.md) - Layout management and sizer types
- [04-events.md](04-events.md) - Event types and handler signatures
- [05-code-generation.md](05-code-generation.md) - How XML maps to generated code
