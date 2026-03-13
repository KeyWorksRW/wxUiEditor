# gen_enums.h — Generator Enumerations Reference

## Overview

`src/gen_enums.h` is the **central registry** for every property, widget type, and code
generator in wxUiEditor. It defines four core enumerations and their bidirectional lookup
maps, forming the bridge between the XML definition files that describe wxWidgets controls
and the C++ code that creates, edits, and generates code for those controls.

Every property shown in the Property Panel, every widget available in the Add menu, and every
code generator that emits C++, Python, Ruby, Perl, Fortran, Go, Julia, LuaJIT, or Rust code
is identified by an enum value defined in this file.

## Relationship to XML Definition Files

The XML files in `src/xml/` declaratively describe every wxWidgets control, form, sizer, and
shared property interface that wxUiEditor supports. These XML files are **not** read at
runtime from disk — they are parsed, compressed, and embedded as base64 data in
`wxue_data.cpp` during the build process. The DTD schema (`src/xml/gen.dtd`) defines the
XML structure.

### XML Files

| File | Purpose |
|------|---------|
| `gen.dtd` | DTD schema — defines `<gen>`, `<property>`, `<event>`, `<inherits>`, `<option>` elements |
| `widgets.xml` | Widget definitions (wxButton, wxCheckBox, wxTextCtrl, etc.) |
| `forms.xml` | Top-level form definitions (wxFrame, wxDialog, wxWizard, DocViewApp, etc.) |
| `sizers.xml` | Sizer definitions (wxBoxSizer, wxFlexGridSizer, wxGridBagSizer, etc.) |
| `interfaces.xml` | Shared interface definitions inherited by multiple generators |

### How XML Maps to Enums

Each `<gen>` element in XML corresponds to a `GenName` enum value, and each `<property>`
element corresponds to a `PropName` enum value with a `PropType` classification:

```xml
<!-- From widgets.xml -->
<gen class="wxButton" image="wxButton" type="widget">
    <inherits class="wxWindow"/>
    <inherits class="Window Events"/>
    <inherits class="sizer_child"/>
    <property name="label" type="string">Button</property>
    <property name="bitmap" type="image"/>
    <property name="style" type="bitlist">
        <option name="wxBU_LEFT"/>
        <option name="wxBU_TOP"/>
    </property>
</gen>
```

This maps to:
- **GenName:** `gen_wxButton` — identifies which code generator handles this widget
- **GenType:** `type_widget` — classifies the node type in the tree
- **PropName:** `prop_label`, `prop_bitmap`, `prop_style` — identifies each property
- **PropType:** `type_string`, `type_image`, `type_bitlist` — tells the Property Panel how
  to display and edit each property

The `<inherits>` element pulls shared properties from interface definitions in
`interfaces.xml`, such as `wxWindow` (common window properties like size, position, colors)
and `Window Events` (common event handlers like mouse, keyboard, focus events).

## Enum Sections

### PropType — Property Display Types

```cpp
enum PropType : std::uint8_t { ... };  // ~37 values
```

**Purpose:** Determines how the Property Panel renders and edits a property value. Each XML
`<property type="...">` attribute maps to one of these values.

**Categories:**

| Type | Property Panel Behavior |
|------|------------------------|
| `type_string` | Single-line text editor |
| `type_string_edit` | Multi-line text editor |
| `type_string_escapes` | Text editor with escape sequence support (`\n`, `\t`, etc.) |
| `type_string_edit_escapes` | Multi-line editor with escape sequences |
| `type_string_edit_single` | Multi-line editor that generates a single C++ string |
| `type_string_code_single` | Code editor (syntax highlighted, single output string) |
| `type_bool` | Checkbox (true/false) |
| `type_int`, `type_uint`, `type_float` | Numeric input fields |
| `type_option` | Dropdown with enumerated choices |
| `type_bitlist` | Checklist of combinable flag values (joined with `\|`) |
| `type_bitmap`, `type_image` | Image picker with bundle support |
| `type_wxColour` | Color picker |
| `type_wxFont` | Font picker |
| `type_wxPoint`, `type_wxSize` | Coordinate/dimension editors |
| `type_id` | wxWidgets ID picker (wxID_ANY, wxID_OK, custom, etc.) |
| `type_stringlist` | Newline-separated list of strings |
| `type_stringlist_semi` | Semicolon-separated list of strings |
| `type_code_edit` | Code editor with syntax highlighting |
| `type_html_edit` | HTML content editor |
| `type_animation` | Animation file picker |
| `type_file`, `type_path` | File/directory path browser |

**Lookup map:** `umap_PropTypes` maps XML type strings (e.g., `"string"`) to enum values.

---

### PropName — Property Identifiers

```cpp
enum PropName : std::uint16_t { ... };  // ~530 values
```

**Purpose:** Uniquely identifies every property that any node can have. This is the most
heavily-used enum in the codebase — generators, the Property Panel, importers, and the
clipboard system all reference properties by their `PropName` value.

All values use the `prop_` prefix. The enum is organized roughly alphabetically within
functional groups.

**Major Property Categories:**

| Category | Examples | Approx. Count |
|----------|----------|---------------|
| **UI Layout** | `prop_pos`, `prop_size`, `prop_min_size`, `prop_proportion`, `prop_border`, `prop_align` | 20+ |
| **Appearance** | `prop_background_colour`, `prop_foreground_colour`, `prop_font`, `prop_label`, `prop_tooltip` | 25+ |
| **Window Styles** | `prop_style`, `prop_window_style`, `prop_window_extra_style`, `prop_hidden`, `prop_disabled` | 15+ |
| **Bitmaps** | `prop_bitmap`, `prop_disabled_bmp`, `prop_pressed_bmp`, `prop_focus_bmp`, `prop_current` | 8+ |
| **Grid/List** | `prop_cols`, `prop_rows`, `prop_colspan`, `prop_rowspan`, `prop_column_labels`, `prop_column_sizes` | 35+ |
| **Code Generation** | `prop_class_name`, `prop_var_name`, `prop_base_file`, `prop_construction`, `prop_class_access` | 40+ |
| **Validation** | `prop_validator_type`, `prop_validator_style`, `prop_validator_data_type` | 5+ |
| **AUI Docking** | `prop_aui_layer`, `prop_aui_name`, `prop_aui_position`, `prop_dock`, `prop_floatable` | 15+ |
| **STC (Styled Text)** | `prop_stc_lexer`, `prop_stc_wrap_mode`, `prop_stc_indentation_size`, `prop_fold_flags` | 15+ |
| **Templates** | `prop_template_description`, `prop_template_directory`, `prop_template_extension` | 7 |
| **Language Settings** | Per-language output configuration (see below) | ~150 |

**Language-Specific Properties:**

Each supported language has a set of properties controlling its code output. These follow the
pattern `prop_<lang>_<setting>`:

```
prop_cpp_line_length       prop_python_line_length     prop_ruby_line_length
prop_cpp_conditional       prop_python_output_folder   prop_ruby_output_folder
prop_base_file             prop_python_base_file       prop_ruby_base_file
                           prop_python_inherit_name    prop_ruby_inherit_name
```

The six kwxFFI languages (Fortran, Go, Julia, LuaJIT, Perl, Rust) each have similar sets:
`prop_fortran_*`, `prop_go_*`, `prop_julia_*`, `prop_lua_*`, `prop_perl_*`, `prop_rust_*`.

**Special sentinel values:**
- `prop_name_array_size` — Always the last real value; used to size static arrays
- `prop_unknown` — Alias for `prop_name_array_size`; returned when a property name is not found
- `prop_processed` — Special marker used by importers

**Lookup functions and maps:**

```cpp
// Forward lookup: enum → string name
extern const std::map<PropName, std::string_view> map_PropNames;

// Reverse lookup: string name → enum
extern std::map<std::string_view, PropName, std::less<>> rmap_PropNames;

// Help text for each property
extern const std::map<PropName, std::string_view> map_PropHelp;

// Convenience functions
auto FindProp(std::string_view name) -> PropName;             // returns prop_unknown if not found
auto GetPropStringName(PropName prop) -> std::optional<wxue::string_view>;  // returns {} if not found
```

**Usage in generators:**

```cpp
// Access property values on a node
if (node->HasValue(prop_label)) {
    auto label = node->as_string(prop_label);      // std::string
    auto view  = node->as_view(prop_label);         // std::string_view (preferred)
    auto wx    = node->as_wxString(prop_label);     // wxString
    auto flag  = node->as_bool(prop_markup);        // bool
}

// In Code builder
if (code.HasValue(prop_bitmap)) {
    code.QuotedString(prop_label);
}
```

---

### GenType — Node Classification Types

```cpp
enum GenType : std::uint8_t { ... };  // ~75 values
```

**Purpose:** Classifies what kind of entity a node represents in the node tree. Used by the
Navigation Panel for display, by generators to determine code structure, and by drag-and-drop
logic to validate parent-child relationships.

**Type Categories:**

| Category | Types | Purpose |
|----------|-------|---------|
| **Forms** | `type_form`, `type_frame_form`, `type_panel_form`, `type_toolbar_form`, `type_menubar_form`, `type_ribbonbar_form`, `type_wizard`, `type_propsheetform` | Top-level form types that generate classes |
| **Sizers** | `type_sizer`, `type_sizeritem`, `type_gbsizer`, `type_gbsizeritem`, `type_splitter`, `type_splitteritem`, `type_staticbox` | Layout management |
| **Widgets** | `type_widget`, `type_container`, `type_panel` | Standard controls and containers |
| **Books** | `type_bookpage`, `type_choicebook`, `type_listbook`, `type_notebook`, `type_simplebook`, `type_auinotebook`, `type_page` | Notebook/book controls and their pages |
| **Menus** | `type_menu`, `type_menubar`, `type_menuitem`, `type_submenu`, `type_popup_menu`, `type_ctx_menu` | Menu system |
| **Toolbars** | `type_toolbar`, `type_tool`, `type_tool_dropdown`, `type_tool_separator`, `type_aui_toolbar`, `type_aui_tool` | Toolbar controls |
| **Ribbon** | `type_ribbonbar`, `type_ribbonpage`, `type_ribbonpanel`, `type_ribbonbuttonbar`, `type_ribbonbutton`, `type_ribbongallery`, `type_ribbongalleryitem`, `type_ribbontool`, `type_ribbontoolbar` | Ribbon UI |
| **Property Grid** | `type_propgrid`, `type_propgridman`, `type_propgridpage`, `type_propgriditem`, `type_propgrid_category` | PropertyGrid controls |
| **Data** | `type_data_folder`, `type_data_list`, `type_data_string`, `type_embed_image`, `type_images` | Embedded data and images |
| **Tree/List** | `type_dataviewctrl`, `type_dataviewcolumn`, `type_dataviewlistctrl`, `type_dataviewlistcolumn`, `type_dataviewtreectrl`, `type_treelistctrl`, `type_treelistctrlcolumn` | Tree/list controls |
| **Organization** | `type_folder`, `type_sub_folder`, `type_project`, `type_interface` | Non-visual organization |
| **DocView** | `type_DocViewApp`, `type_wx_document`, `type_wx_view`, `type_mdi_menubar`, `type_doc_menubar` | Document/View architecture |
| **Other** | `type_timer`, `type_statusbar`, `type_oldbookpage` | Miscellaneous |

**Special sentinel:**
- `gen_type_array_size` — Always the last value; used to size arrays
- `gen_type_unknown` — Alias for `gen_type_array_size`

**Lookup map:** `map_GenTypes` maps `GenType` → `std::string_view`.

---

### GenName — Code Generator Identifiers

```cpp
enum GenName : std::uint8_t { ... };  // ~150+ values
```

**Purpose:** Identifies which C++ generator class handles code generation for a given node.
Each value corresponds to a generator class (typically in `src/generate/`) that knows how to
emit construction code, settings, and event bindings for that widget type.

The enum includes both **top-level generators** (actual widgets) and **interface generators**
(shared property sets pulled in via `<inherits>`).

**Sections (in enum order):**

#### 1. Interface Generators (categories)

These don't generate widgets — they define shared property and event sets:

| Generator | Purpose |
|-----------|---------|
| `gen_Boolean_Validator` through `gen_Text_Validator` | 9 validator type interfaces |
| `gen_Bitmaps`, `gen_Command_Bitmaps` | Shared bitmap properties |
| `gen_Code_Generation` | Code generation settings |
| `gen_Window_Events` | Common window events (from `interfaces.xml`) |
| `gen_XrcSettings` | XRC export settings |
| `gen_wxWindow`, `gen_wxTopLevelWindow` | Base window property sets |
| `gen_sizer_child`, `gen_sizeritem_settings` | Sizer item properties |
| `gen_flexgridsizerbase` | FlexGridSizer shared properties |

#### 2. Language Categories

Identify which code languages are available for generation:

| Generator | Language |
|-----------|----------|
| `gen_Code` | C++ |
| `gen_wxPython` | Python (wxPython) |
| `gen_wxRuby` | Ruby (wxRuby3) |
| `gen_XRC` | XRC (XML resource format) |
| `gen_kwxPerl` | Perl (kwxFFI) |
| `gen_kwxFortran` | Fortran (kwxFFI) |
| `gen_kwxGo` | Go (kwxFFI) |
| `gen_kwxJulia` | Julia (kwxFFI) |
| `gen_kwxLua` | LuaJIT (kwxFFI) |
| `gen_kwxRust` | Rust (kwxFFI) |

#### 3. Language Settings

Per-language configuration sections shown in the Property Panel:

`gen_LanguageSettings` (umbrella), `gen_CPlusSettings`, `gen_PythonSettings`,
`gen_RubySettings`, `gen_PerlSettings`, `gen_FortranSettings`, `gen_GoSettings`,
`gen_JuliaSettings`, `gen_LuaSettings`, `gen_RustSettings`

#### 4. Special-Purpose Generators

| Generator | Purpose |
|-----------|---------|
| `gen_Data`, `gen_data_string`, `gen_data_xml` | Embedded data generation |
| `gen_Images`, `gen_embedded_image` | Embedded image handling |
| `gen_folder`, `gen_sub_folder`, `gen_data_folder` | Navigation Panel organization |
| `gen_Project` | Root project node |

#### 5. DocView Generators

For Document/View architecture support:

`gen_wxMdiWindow`, `gen_DocViewApp`, `gen_MdiFrameMenuBar`, `gen_MdiDocMenuBar`,
`gen_DocumentTextCtrl`, `gen_ViewTextCtrl`, etc.

#### 6. Widget Generators (bulk of the enum)

All standard wxWidgets control generators, from `gen_wxActivityIndicator` through
`gen_wxWrapSizer`. These are the generators bound to the **Add menu** — every widget type
a user can add to a form has a corresponding generator here.

Notable groupings include:
- **Buttons:** `gen_wxButton`, `gen_wxToggleButton`, `gen_wxBitmapToggleButton`, `gen_wxCommandLinkButton`
- **Text:** `gen_wxTextCtrl`, `gen_wxStyledTextCtrl`, `gen_wxRichTextCtrl`, `gen_wxSearchCtrl`
- **Lists:** `gen_wxListBox`, `gen_wxCheckListBox`, `gen_wxChoice`, `gen_wxComboBox`, `gen_wxBitmapComboBox`
- **Pickers:** `gen_wxColourPickerCtrl`, `gen_wxFontPickerCtrl`, `gen_wxFilePickerCtrl`, `gen_wxDirPickerCtrl`, `gen_wxDatePickerCtrl`, `gen_wxTimePickerCtrl`
- **Containers:** `gen_wxPanel`, `gen_wxScrolledWindow`, `gen_wxCollapsiblePane`, `gen_wxSplitterWindow`
- **Sizers:** `gen_wxBoxSizer`, `gen_VerticalBoxSizer`, `gen_wxFlexGridSizer`, `gen_wxGridSizer`, `gen_wxGridBagSizer`, `gen_wxWrapSizer`, `gen_wxStaticBoxSizer`
- **Books:** `gen_wxNotebook`, `gen_wxChoicebook`, `gen_wxListbook`, `gen_wxToolbook`, `gen_wxTreebook`, `gen_wxSimplebook`, `gen_wxAuiNotebook`
- **Data Views:** `gen_wxDataViewCtrl`, `gen_wxDataViewListCtrl`, `gen_wxDataViewTreeCtrl`, `gen_wxGrid`
- **Ribbon:** `gen_wxRibbonBar`, `gen_wxRibbonPage`, `gen_wxRibbonPanel`, `gen_wxRibbonButtonBar`, `gen_wxRibbonGallery`, `gen_wxRibbonToolBar`

**Special sentinel values:**
- `gen_name_array_size` — Last real value; used to size arrays
- `gen_unknown` — Alias for `gen_name_array_size`
- `gen_svg_embedded_image` — Not an actual generator; used by NavigationPanel for display

**Lookup maps:**

```cpp
// Forward: enum → string name
extern const std::map<GenName, std::string_view> map_GenNames;

// Reverse: string name → enum
extern std::map<std::string_view, GenName, std::less<>> rmap_GenNames;
```

---

## Global Maps Outside the Namespace

These maps are declared at file scope (outside `GenEnum` namespace):

```cpp
// Maps property names to their wxWidgets C++ macro equivalents
extern const std::map<GenEnum::PropName, std::string_view> map_PropMacros;

// Reverse: macro name → property
extern std::map<std::string_view, GenEnum::PropName, std::less<>> map_MacroProps;

// Reverse: generator name string → GenName enum
extern std::map<std::string_view, GenEnum::GenName, std::less<>> rmap_GenNames;
```

`map_PropMacros` / `map_MacroProps` support converting between property names and their C++
macro equivalents (e.g., style flag names used in wxWidgets headers).

## GenLang — Language Identifier Flags

While not defined in `gen_enums.h` (it lives in `src/pch.h`), `GenLang` is closely related
and frequently used alongside these enums:

```cpp
enum GenLang : std::uint16_t
{
    GEN_LANG_NONE      = 0,
    GEN_LANG_CPLUSPLUS = 1,
    GEN_LANG_PYTHON    = 1 << 2,
    GEN_LANG_RUBY      = 1 << 3,
    GEN_LANG_FORTRAN   = 1 << 4,
    GEN_LANG_GO        = 1 << 5,
    GEN_LANG_JULIA     = 1 << 6,
    GEN_LANG_LUAJIT    = 1 << 7,
    GEN_LANG_PERL      = 1 << 8,
    GEN_LANG_RUST      = 1 << 9,
    GEN_LANG_XRC       = 1 << 10,
    GEN_LANG_XML       = 1 << 11,
};
```

Values are bit flags, allowing a bitmask to represent multiple languages simultaneously
(e.g., for specifying which languages a form should generate code for).
