# wxui Project File Format

wxUiEditor project files use the `.wxui` extension and are XML-based documents that store your complete UI design. Understanding the file format can help with version control, troubleshooting, and advanced editing scenarios.

## File Structure

A `.wxui` file consists of a hierarchical tree of `<node>` elements, where each node represents a component in your UI:

```xml
<?xml version="1.0"?>
<wxUiEditorData data_version="19">
  <node class="Project" art_directory="../art" ...>
    <node class="wxFrame" class_name="MainFrame" ...>
      <node class="wxBoxSizer" orientation="wxVERTICAL">
        <node class="wxButton" label="Click Me" var_name="m_button" />
      </node>
    </node>
  </node>
</wxUiEditorData>
```

### Root Element

The root `<wxUiEditorData>` element includes a `data_version` attribute that tracks the file format version. wxUiEditor uses this to maintain backward compatibility when opening projects created with older versions.

### Node Elements

Each `<node>` element represents a UI component and has:

- **class attribute**: Identifies the component type (wxButton, wxDialog, wxBoxSizer, etc.)
- **Property attributes**: Component-specific settings (label, size, position, style, etc.)
- **Child nodes**: Nested components (a sizer contains buttons, a frame contains sizers, etc.)

## Property Storage

wxUiEditor uses an **optimized property storage** approach:

- **Only non-default values are saved**: If a property hasn't changed from its default value, it won't appear in the `.wxui` file
- **Automatic defaults on load**: When opening a file, any missing property automatically uses the default value defined in wxUiEditor's control definitions
- **Smaller file sizes**: This optimization keeps project files compact and version-control-friendly

### Example

If you create a wxButton without changing its size, the `.wxui` file won't include `size="-1,-1"` because that's the default. But if you set a custom size of `100,30`, the file will include `size="100,30"`.

## Property Names

Property names in `.wxui` files match what you see in the Property panel. For example:
- `label` - The text displayed on a button or control
- `var_name` - The C++ variable name for the control
- `bitmap` - Image file path for buttons or static bitmaps
- `style` - Window style flags

**Note for developers**: Internally, wxUiEditor prefixes property names with `prop_` when accessing them in code. For instance, the `hint` property is accessed via `node->as_view(prop_hint)`.

## Platform-Specific Controls

The `platforms` property lets you specify which operating systems should create a particular control:

```xml
<node class="wxButton" platforms="Windows|Mac" ... />
```

Valid platform values:
- `Windows` - Control appears on Windows
- `Unix` - Control appears on Linux/Unix
- `Mac` - Control appears on macOS

This is useful when you need platform-specific UI variations (like different button layouts for native feel on each OS).

## Language-Specific Code Generation

You can disable code generation for specific controls in certain languages using the `disable_language` property:

```xml
<node class="CustomControl" disable_language="wxPython|wxRuby" ... />
```

This is helpful when a control or feature isn't available in all target languages.

## Common Node Classes

Here are some frequently used node classes you'll see in `.wxui` files:

### Forms
- `wxFrame` - Top-level application window
- `wxDialog` - Modal or modeless dialog box
- `wxPanel` - Container panel
- `wxWizard` - Multi-page wizard dialog

### Layout
- `wxBoxSizer` - Horizontal or vertical layout
- `wxGridSizer` - Fixed grid layout
- `wxFlexGridSizer` - Flexible grid layout
- `wxGridBagSizer` - Grid with row/column spanning

### Controls
- `wxButton`, `wxCheckBox`, `wxRadioButton` - Standard buttons
- `wxTextCtrl`, `wxComboBox`, `wxChoice` - Input controls
- `wxListBox`, `wxListCtrl`, `wxTreeCtrl` - List and tree views
- `wxStaticText`, `wxStaticBitmap` - Static display elements

### Project Management
- `Project` - Root project settings
- `folder`, `sub_folder` - Organize forms into folders
- `Images` - Embedded image resources
- `Data` - Embedded data files

## Property Types

Properties in `.wxui` files use different data types:

- **string**: Text values (labels, hints, tooltips)
- **bool**: Boolean flags (`0` or `1`)
- **int/uint**: Integer values (border sizes, proportions)
- **option**: Enumerated choice (orientation, alignment)
- **bitlist**: Combination of flags (window styles, border flags)
- **wxSize**: Width and height (e.g., `100,50`)
- **wxPoint**: X and Y coordinates (e.g., `10,20`)
- **wxColour**: RGB color values
- **wxFont**: Font specifications
- **image**: Image file paths or embedded references
- **file/path**: File system paths

## Version Control Best Practices

Because `.wxui` files are XML text:

1. **Use line-oriented diffs**: Git and other VCS tools can show meaningful changes
2. **Avoid unnecessary reformatting**: wxUiEditor preserves XML structure to minimize diff noise
3. **Review property changes**: Pay attention to changed default values across versions
4. **Commit frequently**: Small, focused commits make it easier to track UI evolution

## Editing `.wxui` Files Directly

While wxUiEditor is designed for visual editing, you can manually edit `.wxui` files in a text editor:

- **Backup first**: Always keep a backup before manual editing
- **Validate XML**: Ensure the file is well-formed XML
- **Match property names exactly**: Property names are case-sensitive
- **Preserve node hierarchy**: Parent-child relationships must be valid
- **Test in wxUiEditor**: Open the file after editing to verify it loads correctly

**Warning**: Manual edits may be overwritten when wxUiEditor saves the file, especially if you edit properties that the visual editor also manages.

## Data Version Evolution

The `data_version` attribute tracks wxUiEditor's file format version. When opening older projects:

- wxUiEditor automatically migrates data to the current format
- Properties may be renamed or restructured (transparent to users)
- Deprecated controls may be converted to modern equivalents
- The file will be updated to the current version on next save

Always keep backups before opening very old `.wxui` files in newer versions.

## Related Documentation

- **[Images](images.md)** - How images are embedded and referenced
- **[XRC Export](XRC.md)** - Exporting to wxWidgets XRC format
- **Import Guides** - Converting from other UI designers to `.wxui` format
