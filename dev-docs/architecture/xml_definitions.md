# XML Definition Files

wxUiEditor's control catalog is defined through a collection of XML files located in `src/xml/`. These files describe how wxWidgets controls, forms, and properties are structured, and serve as the blueprint for both the UI editor and code generation.

## Overview

The XML definition system consists of:
- **gen.dtd** - Document Type Definition (DTD) specifying the XML schema
- **interfaces.xml** - Reusable property collections and interface definitions
- **Individual control files** - Specific definitions for controls, forms, sizers, etc.

## File List

| File | Purpose |
|------|---------|
| `gen.dtd` | XML schema definition for all generator files |
| `interfaces.xml` | Common property interfaces (Window Events, Bitmaps, etc.) |
| `project.xml` | Project and folder node definitions |
| `forms.xml` | Frame, dialog, and wizard definitions |
| `buttons.xml` | Button controls (wxButton, wxCheckBox, wxRadioButton, etc.) |
| `containers.xml` | Container controls (wxPanel, wxScrolledWindow, etc.) |
| `sizers.xml` | Layout sizers (wxBoxSizer, wxGridSizer, etc.) |
| `text_ctrls.xml` | Text input controls (wxTextCtrl, wxRichTextCtrl, etc.) |
| `data_ctrls.xml` | Data display controls (wxListCtrl, wxTreeCtrl, wxGrid, etc.) |
| `pickers.xml` | Picker controls (wxColourPickerCtrl, wxFilePickerCtrl, etc.) |
| `widgets.xml` | Miscellaneous widgets (wxGauge, wxSlider, etc.) |
| `bars.xml` | Menu, toolbar, and status bar definitions |
| `mdi.xml` | MDI (Multiple Document Interface) frame definitions |
| `tips.xml` | Tooltip and help control definitions |

## XML File Structure

Each XML file follows this structure:

```xml
<?xml version="1.0"?>
<!DOCTYPE GeneratorDefinitions SYSTEM "gen.dtd">
<GeneratorDefinitions>
    <gen class="wxButton" image="wxButton" type="widget">
        <!-- Property definitions -->
        <!-- Event definitions -->
        <!-- Inherited interfaces -->
    </gen>
</GeneratorDefinitions>
```

## The `<gen>` Element

Each `<gen>` element defines either a wxWidgets control class or an interface:

### Attributes

- **class**: The name of the wxWidgets class or interface
- **type**: The component type (`widget`, `form`, `sizer`, `interface`, `project`, etc.)
- **image**: Icon identifier for the toolbox (references embedded images)
- **flags**: Optional behavior flags

### Example

```xml
<gen class="wxCheckBox" image="wxCheckBox" type="widget">
    <property name="var_name" type="string">m_checkBox</property>
    <property name="label" type="string_escapes">My Checkbox</property>
    <property name="checked" type="bool">0</property>
    <event name="wxEVT_CHECKBOX" class="wxCommandEvent"/>
</gen>
```

## Property Definitions

Properties define the configurable attributes of a control:

```xml
<property name="label" type="string_escapes">Default Label</property>
```

### Property Attributes

- **name**: Property identifier (must match enum in `gen_enums.cpp`)
- **type**: Data type (see Property Types below)
- **help**: Tooltip description shown in the Property panel

The text content of the `<property>` element is the default value.

### Property Types

| Type | Description | Example Values |
|------|-------------|----------------|
| `string` | Plain text string | `"My Label"` |
| `string_escapes` | String with escape sequences | `"Line 1\nLine 2"` |
| `string_edit` | Multi-line editable string | Code snippets |
| `bool` | Boolean value | `0`, `1` |
| `int` | Signed integer | `-10`, `0`, `100` |
| `uint` | Unsigned integer | `0`, `5`, `1000` |
| `float` | Floating-point number | `1.5`, `3.14` |
| `id` | wxWidgets identifier | `wxID_ANY`, `ID_CUSTOM` |
| `option` | Single choice from list | See Options below |
| `bitlist` | Multiple flags combined | See Bitlists below |
| `image` | Image file reference | `"SVG;icon.svg;[24,24]"` |
| `bitmap` | Bitmap image | `"Embed;image.png"` |
| `animation` | Animation file | `"animation.gif"` |
| `file` | File path | `"myfile.cpp"` |
| `path` | Directory path | `"./src"` |
| `wxSize` | Width and height | `"100,50"` |
| `wxPoint` | X and Y coordinates | `"10,20"` |
| `wxColour` | RGB color | `"#FF0000"` |
| `wxFont` | Font specification | Font descriptor |
| `stringlist` | List of strings | Semicolon-separated |
| `uintpairlist` | List of uint pairs | `"0:1,1:2"` |

## Option Properties

Option-type properties define a single choice from a list:

```xml
<property name="orientation" type="option">
    <option name="wxVERTICAL" help="Align items vertically"/>
    <option name="wxHORIZONTAL" help="Align items horizontally"/>
    wxVERTICAL
</property>
```

The text content after the `<option>` elements is the default selection.

## Bitlist Properties

Bitlist properties allow multiple flags to be combined:

```xml
<property name="style" type="bitlist">
    <option name="wxBORDER_SIMPLE" help="Thin border"/>
    <option name="wxBORDER_RAISED" help="Raised border"/>
    <option name="wxTAB_TRAVERSAL" help="Tab key navigation"/>
</property>
```

Users can select multiple options, which are combined with `|` in generated code.

## Event Definitions

Events declare the wxWidgets events a control can handle:

```xml
<event name="wxEVT_BUTTON" class="wxCommandEvent"
       help="Generated when the button is clicked"/>
```

### Event Attributes

- **name**: wxWidgets event macro name
- **class**: Event class type (wxCommandEvent, wxMouseEvent, etc.)
- **help**: Description of when the event fires

## Interface Inheritance

The `<inherits>` element pulls properties and events from interface definitions:

```xml
<gen class="wxButton" type="widget">
    <inherits class="wxWindow"/>
    <inherits class="Window Events"/>
    <inherits class="Bitmaps"/>
    <!-- Additional button-specific properties -->
</gen>
```

This avoids duplicating common properties across many controls.

### Common Interfaces (from interfaces.xml)

- **wxWindow**: Base window properties (id, size, position, colors, font, etc.)
- **Window Events**: Focus, mouse, keyboard, and paint events
- **wxTopLevelWindow**: Top-level window events (close, iconize, maximize, etc.)
- **Bitmaps**: Bitmap properties for buttons (bitmap, disabled_bmp, pressed_bmp, etc.)
- **sizer_child**: Sizer child properties (proportion, alignment, borders, etc.)
- **Text Validator**: Text validation properties for input controls
- **Boolean Validator**: Boolean validation for checkboxes
- **Language Settings**: C++, Python, Perl, Ruby, XRC generation settings

### Hiding Inherited Properties

You can hide specific inherited properties:

```xml
<inherits class="wxWindow">
    <hide name="platforms"/>
</inherits>
```

This includes the property in the internal model but prevents it from showing in the Property panel.

## Categories

Categories organize properties and events in the Property panel:

```xml
<gen class="Window Events" type="interface">
    <category name="Focus Events">
        <event name="wxEVT_KILL_FOCUS" class="wxFocusEvent"/>
        <event name="wxEVT_SET_FOCUS" class="wxFocusEvent"/>
    </category>
    <category name="Mouse Events">
        <event name="wxEVT_LEFT_DOWN" class="wxMouseEvent"/>
        <event name="wxEVT_LEFT_UP" class="wxMouseEvent"/>
    </category>
</gen>
```

## Property Name Mapping

**Critical**: Property names in XML must **exactly match** the enumeration values in `src/gen_enums.cpp`.

The naming convention:
- **XML property name**: `hint`, `label`, `var_name`
- **Code enumeration**: `prop_hint`, `prop_label`, `prop_var_name` (prefixed with `prop_`)
- **Access in code**: `node->as_view(prop_hint)`

### Example Mapping

```xml
<!-- In buttons.xml -->
<property name="label" type="string_escapes">MyButton</property>
```

```cpp
// In gen_enums.cpp
enum GenEnum::PropName {
    prop_label,
    // ... other properties
};

// In code
auto label = node->as_view(prop_label);  // Access the label property
```

## How XML Files Are Built into wxUiEditor

The XML files are **not** compiled directly into wxUiEditor's binary. Instead:

1. **wxUiEditor parses the XML files** (using its own XML parser)
2. **Comments, line breaks, and unnecessary whitespace are removed**
3. **The condensed XML is compressed using zlib**
4. **The compressed data is base64-encoded**
5. **Encoded data is stored as an `unsigned char` array in `src/wxui/wxue_data.cpp`**

This approach allows human-readable, maintainable XML definitions while minimizing the final binary size.

### Regenerating wxue_data.cpp

When you modify XML files, you must regenerate `wxue_data.cpp`:

1. Open wxUiEditor
2. Open the wxUiEditor project file (`src/wxui/wxUiEditor.wxui`)
3. Select **Generate** from the menu
4. The XML files are parsed and `wxue_data.cpp` is automatically updated

**Important**: Always rebuild `wxue_data.cpp` after editing XML files, or your changes won't appear in wxUiEditor.

## Adding a New Control

To add a new wxWidgets control to wxUiEditor:

### 1. Choose or create an XML file

Add your control definition to the appropriate XML file (e.g., `buttons.xml` for button-like controls).

### 2. Define the `<gen>` element

```xml
<gen class="wxNewControl" image="my_icon" type="widget">
    <inherits class="wxWindow"/>
    <inherits class="Window Events"/>
    <property name="var_name" type="string">m_newCtrl</property>
    <property name="my_property" type="string">default value</property>
    <event name="wxEVT_NEW_CONTROL" class="wxCommandEvent"/>
</gen>
```

### 3. Add property enumerations to gen_enums.cpp

If you added new properties, add corresponding `prop_*` enum values in `src/gen_enums.cpp`.

### 4. Regenerate wxue_data.cpp

Follow the regeneration steps above.

### 5. Implement code generators

Create code generation classes in `src/generate/` to output C++/Python/Ruby code for your control.

## Best Practices

### Use Interfaces for Common Properties

Instead of repeating properties across many controls, define them once in `interfaces.xml` and use `<inherits>`.

### Provide Helpful Descriptions

The `help` attribute on properties and events appears as tooltips in the Property panel. Make them clear and concise.

### Set Sensible Defaults

The default value in a `<property>` element determines:
- Initial value when a control is added to a form
- What value is considered "unchanged" (and thus not saved to `.wxui` files)

### Group Related Options

Use categories to organize properties and events logically in the Property panel.

### Match wxWidgets Documentation

Property names and event names should match wxWidgets conventions for consistency.

### Test Your Changes

After modifying XML files:
1. Regenerate `wxue_data.cpp`
2. Rebuild wxUiEditor
3. Create a test form with your control
4. Generate code and verify output
5. Check that properties appear correctly in the Property panel

## Validation

wxUiEditor validates XML files against `gen.dtd` when parsing. Common errors:

- **Missing required attributes**: Every `<gen>` needs `class` and `type`
- **Invalid property types**: Use only types defined in the DTD
- **Malformed XML**: Unclosed tags, incorrect nesting, etc.
- **Property name mismatches**: Names must match `gen_enums.cpp`

## Related Files

- `src/nodes/node_init.cpp` - Initializes control properties from XML definitions
- `src/gen_enums.cpp` - Property name enumerations (must match XML names)
- `src/wxui/wxue_data.cpp` - Embedded compressed XML data (auto-generated)
- `src/import/` - Import code that parses XML definitions

## Further Reading

- [architecture.md](architecture.md) - Overview of wxUiEditor's internal architecture
- [wxui_file_format.md](../users/wxui_file_format.md) - `.wxui` project file format documentation
