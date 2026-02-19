# .wxui File Format — AI Agent Reference

> **Purpose:** Condensed reference enabling AI agents to convert wxWidgets C++ code (from training data) into valid `.wxui` XML files. Leverages existing wxWidgets knowledge — focuses on the XML mapping, not on teaching wxWidgets itself.

## Core Concept

A `.wxui` file is a declarative XML representation of a wxWidgets UI. Every wxWidgets constructor call, sizer addition, event binding, and property setter maps to XML attributes on `<node>` elements. If you know how to write wxWidgets C++ code, you can generate `.wxui` XML by reversing the constructor/setup pattern into attributes.

---

## 1. XML Structure

```xml
<?xml version="1.0"?>
<wxUiEditorData data_version="20">
  <node class="Project" generate_languages="C++">
    <!-- Optional embedded images -->
    <node class="Images" base_file="ui_images">
      <node class="embedded_image" bitmap="SVG;icon.svg;[24,24]" />
    </node>
    <!-- Forms (direct children or inside folders) -->
    <node class="folder" label="Dialogs">
      <node class="wxDialog" class_name="MyDialogBase" ...>
        <!-- UI tree here -->
      </node>
    </node>
    <node class="wxFrame" class_name="MainFrameBase" ...>
      <!-- UI tree here -->
    </node>
  </node>
</wxUiEditorData>
```

### Rules
- Every UI element is a `<node class="ClassName">` — properties are **XML attributes**, not child elements
- Events are **attributes**: `wxEVT_BUTTON="OnClick"` (NOT child `<event>` elements)
- **Only non-default values are stored** — omit any property that equals its default
- Self-closing tags for leaf nodes: `<node class="wxButton" label="OK" />`
- Nesting represents containment: sizer children are nested `<node>` elements

### Data Version
Use `data_version="20"`. To query the installed version: `wxUiEditor.exe --data-version`

---

## 2. Property Encoding

| C++ Concept | XML Attribute Format | Example |
|-------------|---------------------|---------|
| Style flags | Pipe-separated | `style="wxDEFAULT_DIALOG_STYLE\|wxRESIZE_BORDER"` |
| Size | `width,height` | `size="800,600"` or `size="-1,-1"` |
| Position | `x,y` | `pos="10,20"` |
| Boolean | `0` or `1` | `disabled="1"` |
| Color | Hex or RGB | `foreground_colour="#FF0000"` or `colour="255,0,0"` |
| Bitmap (SVG) | `SVG;file;[w,h]` | `bitmap="SVG;save.svg;[16,16]"` |
| Bitmap (PNG) | `Embed;file` | `bitmap="Embed;icon.png"` |
| Bitmap (Art) | `Art;artID\|clientID` | `bitmap="Art;wxART_FILE_OPEN\|wxART_MENU"` |
| String list | Items separated by `\n` | `contents="Item1\nItem2\nItem3"` |
| Multi-line code | Lines separated by `@@` | `source_preamble="#include \"foo.h\"@@#include \"bar.h\""` |
| Font | Symbolic | `font="extra large,,semi-bold"` |
| FlexGrid growable | `col:proportion` | `growablecols="0:1,1:2"` |
| Widget ID | Standard or custom | `id="wxID_OK"` or `id="myID=100"` |

### XML Entity Escaping
Use standard XML entities in attribute values: `&amp;` `&lt;` `&gt;` `&quot;`

---

## 3. Form Types → wxWidgets Base Classes

| .wxui `class` | wxWidgets class | Typical Use |
|---------------|----------------|-------------|
| `wxFrame` | wxFrame | Main application window |
| `wxDialog` | wxDialog | Modal/modeless dialogs |
| `PanelForm` | wxPanel | Reusable panel components |
| `wxWizard` | wxWizard | Multi-step wizard |
| `wxPropertySheetDialog` | wxPropertySheetDialog | Tab-based settings |
| `wxPopupWindow` | wxPopupWindow | Popup overlay |
| `wxPopupTransientWindow` | wxPopupTransientWindow | Auto-dismiss popup |
| `ToolBar` | wxToolBar | Standalone toolbar (form-level) |
| `MenuBar` | wxMenuBar | Standalone menubar (form-level) |
| `PopupMenu` | wxMenu | Context/popup menu |

### Form Attributes

```xml
<node class="wxDialog"
  class_name="MyDialogBase"       <!-- Generated class name (PascalCase + "Base") -->
  base_file="mydialog_base"       <!-- Output filename without extension -->
  derived_class_name="MyDialog"   <!-- User's derived class (PascalCase, no "Base") -->
  title="My Dialog"               <!-- Window title -->
  style="wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER"
  size="600,400"                  <!-- Initial size (omit for auto-size) -->
  minimum_size="400,300"          <!-- Minimum size -->
  center="wxBOTH"                 <!-- Center on screen: no, wxHORIZONTAL, wxVERTICAL, wxBOTH -->
  icon="SVG;app.svg;[16,16]"     <!-- Title bar icon (frames) -->
  persist="1"                     <!-- Remember position/size across sessions -->
  use_derived_class="0"           <!-- Set to 0 to skip derived class generation -->
  private_members="1"             <!-- Make members private instead of protected -->
  pure_virtual_functions="1"      <!-- Event handlers = 0 (pure virtual) -->
  wxEVT_INIT_DIALOG="OnInit"     <!-- Form-level event handler -->
  wxEVT_CLOSE_WINDOW="OnClose">
```

**Default styles:**
- wxFrame: `wxDEFAULT_FRAME_STYLE` (omit — it's the default)
- wxDialog: `wxDEFAULT_DIALOG_STYLE` (omit — it's the default)

### C++ Code Generation Settings (form-level attributes)
- `generate_ids="0"` — Disable auto-generated ID enums
- `initial_enum_string="START_IDS"` — First enum value
- `local_src_includes="header1.h;header2.h"` — Extra source includes
- `local_hdr_includes="header.h"` — Extra header includes
- `source_preamble="..."` — Code at top of .cpp (use `@@` for newlines)
- `header_preamble="..."` — Code after includes in .h
- `inserted_hdr_code="..."` — Code in public: section of .h
- `name_space="MyApp"` — Wrap class in namespace

---

## 4. Containment Rules

### What goes where

```
Project
├── Images → embedded_image
├── Data → data_string, data_xml, data_folder
├── folder / sub_folder → forms
├── wxFrame
│   ├── ONE sizer (required)
│   ├── wxMenuBar (optional, sibling of sizer)
│   ├── wxToolBar (optional, child of sizer or frame)
│   └── wxStatusBar (optional)
├── wxDialog
│   └── ONE sizer (required)
├── PanelForm
│   └── ONE sizer (required)
└── wxWizard
    └── wxWizardPageSimple (multiple)
        └── ONE sizer each
```

### Sizer children
- Widgets (wxButton, wxTextCtrl, wxStaticText, etc.)
- Nested sizers (wxBoxSizer, wxFlexGridSizer, etc.)
- `spacer` nodes
- Container widgets (wxSplitterWindow, wxNotebook, etc.)

### Special containers
- **wxSplitterWindow**: Exactly **2** children (panels or widgets)
- **wxNotebook/wxChoicebook/wxListbook/wxTreebook**: Children are `BookPage` nodes
- **BookPage**: Contains exactly ONE sizer
- **wxToolBar**: Children are `tool`, `toolSeparator`, `toolStretchable`
- **wxMenuBar**: Children are `wxMenu`
- **wxMenu**: Children are `wxMenuItem`, `separator`, `submenu`

---

## 5. Sizer Types and Properties

### Sizer catalog

| .wxui `class` | wxWidgets class | Key attributes |
|---------------|----------------|----------------|
| `wxBoxSizer` | wxBoxSizer | `orientation` (wxVERTICAL/wxHORIZONTAL) |
| `VerticalBoxSizer` | wxBoxSizer | Pre-configured vertical (same as wxBoxSizer + wxVERTICAL) |
| `wxStaticBoxSizer` | wxStaticBoxSizer | `orientation`, `label` |
| `StaticCheckboxBoxSizer` | wxStaticBoxSizer | Has checkbox in header |
| `StaticRadioBtnBoxSizer` | wxStaticBoxSizer | Has radio button in header; `radiobtn_var_name`, `checked` |
| `wxWrapSizer` | wxWrapSizer | `orientation` |
| `wxGridSizer` | wxGridSizer | `rows`, `cols`, `vgap`, `hgap` |
| `wxFlexGridSizer` | wxFlexGridSizer | `rows`, `cols`, `vgap`, `hgap`, `growablecols`, `growablerows`, `flexible_direction`, `non_flexible_grow_mode` |
| `wxGridBagSizer` | wxGridBagSizer | Same as FlexGrid + per-child `row`, `column`, `rowspan`, `colspan` |
| `TextSizer` | wxBoxSizer | `text` — sizes based on reference text width |
| `wxStdDialogButtonSizer` | wxStdDialogButtonSizer | Standard OK/Cancel/etc. buttons |
| `spacer` | Spacer item | `width`, `height`, `proportion` |

### Sizer item attributes (on every widget/sizer child)

| Attribute | Default | Maps to |
|-----------|---------|---------|
| `proportion` | `0` | `wxSizerFlags(N)` |
| `flags` | (empty) | `.Expand()`, `.Shaped()`, etc. |
| `borders` | `wxALL` | `.Border(sides)` |
| `border_size` | `5` | `.Border(sides, N)` |
| `alignment` | (empty) | `.Align(flags)` / `.Center()` |

**Important:** `borders=""` (empty string) means NO border at all. The default `wxALL` with `border_size="5"` produces `.Border(wxALL)`.

### wxStdDialogButtonSizer

```xml
<node class="wxStdDialogButtonSizer" flags="wxEXPAND"
      OK="1" Cancel="1"              <!-- Which buttons to show (bool) -->
      OKButtonClicked="OnOK" />      <!-- Event handler for OK button -->
```

Available buttons: `OK`, `Yes`, `Save`, `Apply`, `No`, `Cancel`, `Close`, `Help`, `ContextHelp`
Event attributes: `OKButtonClicked`, `YesButtonClicked`, `SaveButtonClicked`, `ApplyButtonClicked`, `NoButtonClicked`, `CancelButtonClicked`, `CloseButtonClicked`, `HelpButtonClicked`
Option: `default_button` — which button gets default focus (wxOK, wxYES, wxSAVE, wxCLOSE, wxNO)

---

## 6. Widget Quick Reference

### Common widgets

| .wxui `class` | Default `var_name` | Key attributes |
|---------------|-------------------|----------------|
| `wxButton` | `m_button` | `label`, `id`, `bitmap` |
| `wxStaticText` | `m_staticText` | `label`, `wrap` (px width) |
| `wxTextCtrl` | `m_textCtrl` | `style` (wxTE_MULTILINE, wxTE_READONLY, etc.), `value` |
| `wxCheckBox` | `m_checkBox` | `label`, `checked` |
| `wxRadioButton` | `m_radioBtn` | `label`, `checked` |
| `wxComboBox` | `m_comboBox` | `contents`, `selection`, `style` |
| `wxChoice` | `m_choice` | `contents`, `selection` |
| `wxListBox` | `m_listBox` | `contents`, `style` |
| `wxSpinCtrl` | `m_spinCtrl` | `min`, `max`, `initial` |
| `wxSlider` | `m_slider` | `minValue`, `maxValue`, `value`, `style` |
| `wxGauge` | `m_gauge` | `range`, `value`, `style` |
| `wxStaticLine` | `m_staticLine` | `style` (wxLI_HORIZONTAL/wxLI_VERTICAL) |
| `wxStaticBitmap` | `m_bitmap` | `bitmap` |
| `wxHyperlinkCtrl` | `m_hyperlink` | `label`, `url` |
| `wxCalendarCtrl` | `m_calendar` | `style` |
| `wxSearchCtrl` | `m_searchCtrl` | `style` |
| `wxStyledTextCtrl` | `m_scintilla` | `lexer`, `read_only`, `use_tabs`, `wrap_mode`, many more |

### Picker widgets

| .wxui `class` | Default `var_name` | Key attributes |
|---------------|-------------------|----------------|
| `wxFilePickerCtrl` | `m_filePicker` | `message`, `wildcard`, `style` |
| `wxDirPickerCtrl` | `m_dirPicker` | `message`, `style` |
| `wxColourPickerCtrl` | `m_colourPicker` | `colour`, `style` |
| `wxFontPickerCtrl` | `m_fontPicker` | `style` |
| `wxDatePickerCtrl` | `m_datePicker` | `style` |
| `wxTimePickerCtrl` | `m_timePicker` | `style` |

### Container widgets

| .wxui `class` | Default `var_name` | Notes |
|---------------|-------------------|-------|
| `wxPanel` | `panel` | Generic container with one child sizer |
| `wxSplitterWindow` | `m_splitter` | Exactly 2 children; `min_pane_size`, `sashpos`, `splitmode` |
| `wxNotebook` | `m_notebook` | Children are `BookPage` nodes |
| `wxChoicebook` | `m_choicebook` | Same pattern as notebook |
| `wxListbook` | `m_listbook` | Same pattern as notebook |
| `wxTreebook` | `m_treebook` | Same pattern as notebook |
| `wxAuiNotebook` | `m_auiNotebook` | Same pattern as notebook |
| `wxSimplebook` | `m_simplebook` | Same pattern, no visible tabs |
| `wxScrolledWindow` | `m_scrolledWindow` | Scrollable container |
| `wxCollapsiblePane` | `m_collapsiblePane` | Expandable/collapsible section |

### BookPage pattern
```xml
<node class="wxNotebook" flags="wxEXPAND">
  <node class="BookPage" label="Tab 1" var_name="m_page1"
        background_colour="wxSYS_COLOUR_BTNFACE" window_style="wxTAB_TRAVERSAL">
    <node class="wxBoxSizer" orientation="wxVERTICAL">
      <!-- Page contents -->
    </node>
  </node>
  <node class="BookPage" label="Tab 2" var_name="m_page2"
        background_colour="wxSYS_COLOUR_BTNFACE" window_style="wxTAB_TRAVERSAL">
    <node class="wxBoxSizer" orientation="wxVERTICAL">
      <!-- Page contents -->
    </node>
  </node>
</node>
```

### Data controls

| .wxui `class` | Default `var_name` | Notes |
|---------------|-------------------|-------|
| `wxGrid` | `m_grid` | Configurable rows/cols/labels |
| `wxTreeCtrl` | `m_treeCtrl` | `style` flags |
| `wxDataViewCtrl` | `m_dataViewCtrl` | Children are `dataViewColumn` |
| `wxDataViewListCtrl` | `m_dataViewListCtrl` | Children are `dataViewListColumn` |
| `wxDataViewTreeCtrl` | `m_dataViewTreeCtrl` | Built-in tree column |
| `wxTreeListCtrl` | `m_treeListCtrl` | Children are `TreeListCtrlColumn` |
| `wxListView` | `m_listview` | List control view |
| `wxPropertyGrid` | `m_propertyGrid` | Children are `propGridItem`/`propGridCategory` |
| `wxPropertyGridManager` | `m_propertyGridManager` | Children are `propGridPage` |

### Bars, menus, ribbons

```xml
<!-- Toolbar (child of frame or sizer) -->
<node class="wxToolBar" style="wxTB_FLAT|wxTB_HORIZONTAL" var_name="m_toolbar"
      borders="" flags="wxEXPAND">
  <node class="tool" bitmap="SVG;save.svg;[24,24]" id="wxID_SAVE"
        label="Save" tooltip="Save" wxEVT_TOOL="OnSave" />
  <node class="tool" bitmap="SVG;cut.svg;[24,24]" id="wxID_CUT"
        kind="wxITEM_CHECK" label="Toggle" />
  <node class="toolSeparator" />
  <node class="tool" bitmap="SVG;cpp.svg;[16,16]" id="ID_CPP"
        kind="wxITEM_RADIO" label="C++" wxEVT_TOOL="OnCPlus" />
</node>

<!-- MenuBar (child of wxFrame, sibling of the top-level sizer) -->
<node class="wxMenuBar">
  <node class="wxMenu" label="&amp;File" var_name="m_menuFile">
    <node class="wxMenuItem" label="&amp;Open..." shortcut="Ctrl+O"
          id="wxID_OPEN" bitmap="Art;wxART_FILE_OPEN|wxART_MENU"
          help="Open a file" wxEVT_MENU="OnOpen" />
    <node class="separator" />
    <node class="submenu" label="&amp;Recent">
      <!-- wxMenuItem children -->
    </node>
    <node class="wxMenuItem" label="E&amp;xit" id="wxID_EXIT" shortcut="Alt+F4" />
  </node>
  <node class="wxMenu" label="&amp;Help">
    <node class="wxMenuItem" label="&amp;About" id="wxID_ABOUT" />
  </node>
</node>

<!-- StatusBar (child of wxFrame) -->
<node class="wxStatusBar" var_name="m_statusBar" fields="2" />
```

### Tool attributes
- `bitmap` — Tool icon (required)
- `id` — Tool identifier
- `label` — Text label
- `tooltip` — Hover text
- `help` — Status bar help text
- `kind` — `wxITEM_NORMAL` (default), `wxITEM_CHECK`, `wxITEM_RADIO`
- Event: `wxEVT_TOOL`, `wxEVT_UPDATE_UI`

### MenuItem attributes
- `label` — Menu text (use `&amp;` for accelerator key)
- `shortcut` — Keyboard shortcut (e.g., `Ctrl+S`, `Alt+F4`)
- `id` — Standard ID or custom
- `kind` — `wxITEM_NORMAL` (default), `wxITEM_CHECK`, `wxITEM_RADIO`
- `bitmap` — Menu icon
- `help` — Status bar text
- `checked` — Initial check state for CHECK/RADIO items
- Events: `wxEVT_MENU`, `wxEVT_UPDATE_UI`

---

## 7. Common Widget Attributes

These attributes are available on virtually all widgets:

| Attribute | Type | Default | Notes |
|-----------|------|---------|-------|
| `var_name` | string | (per widget) | Member variable name, `m_` prefix convention |
| `class_access` | string | `protected:` | `"protected:"`, `"public:"`, or `"none"` (local only) |
| `id` | id | `wxID_ANY` | `wxID_OK`, `wxID_CANCEL`, `wxID_ANY`, or custom |
| `size` | wxSize | (auto) | Explicit size `"w,h"`, -1 = auto |
| `minimum_size` | wxSize | (none) | `"w,h"` |
| `maximum_size` | wxSize | (none) | `"w,h"` |
| `pos` | wxPoint | (auto) | Usually omitted (sizer-managed) |
| `disabled` | bool | `0` | `"1"` to start disabled |
| `hidden` | bool | `0` | `"1"` to start hidden |
| `focus` | bool | `0` | `"1"` to receive initial focus |
| `tooltip` | string | (empty) | Tooltip text |
| `foreground_colour` | wxColour | (none) | `"#RRGGBB"` or `"R,G,B"` |
| `background_colour` | wxColour | (none) | `"#RRGGBB"` or system color name |
| `font` | wxFont | (none) | Symbolic: `"extra large,,semi-bold"` |
| `window_style` | bitlist | (varies) | wxWidgets window style flags |
| `window_extra_style` | bitlist | (none) | Extra window styles |
| `subclass` | string | (none) | Custom class name replacing the widget |
| `subclass_header` | file | (none) | Header file for custom class |
| `platforms` | bitlist | `Windows\|Unix\|Mac` | Target platforms (omit only non-defaults) |

---

## 8. Event Handling

### Format
Events are attributes on the `<node>` element:
```xml
<node class="wxButton" label="OK" wxEVT_BUTTON="OnOK" />
```

### Named handlers (most common)
```xml
wxEVT_BUTTON="OnClick"        <!-- Generates virtual void OnClick(wxCommandEvent&) -->
wxEVT_MENU="OnFileOpen"       <!-- Menu command handler -->
wxEVT_CLOSE_WINDOW="OnClose"  <!-- Window close handler -->
wxEVT_INIT_DIALOG="OnInit"    <!-- Dialog initialization -->
```

### Lambda handlers (inline code)
```xml
wxEVT_MENU="[](wxCommandEvent&amp;)@@{@@Project.DoSomething();@@}"
```
- `@@` separates lines in the lambda body
- Must XML-escape `&` as `&amp;`

### Language-specific handler override
```xml
wxEVT_UPDATE_UI="[](wxUpdateUIEvent&amp; event)@@{@@event.Enable(false);@@}[python:OnUpdateUI]"
```
Use `[python:HandlerName]` suffix to specify a different handler for Python generation.

### Common events by widget type

| Widget | Events |
|--------|--------|
| wxButton | `wxEVT_BUTTON` |
| wxCheckBox | `wxEVT_CHECKBOX` |
| wxRadioButton | `wxEVT_RADIOBUTTON` |
| wxTextCtrl | `wxEVT_TEXT`, `wxEVT_TEXT_ENTER` |
| wxComboBox | `wxEVT_COMBOBOX`, `wxEVT_TEXT` |
| wxChoice | `wxEVT_CHOICE` |
| wxListBox | `wxEVT_LISTBOX`, `wxEVT_LISTBOX_DCLICK` |
| wxSlider | `wxEVT_SLIDER` |
| wxSpinCtrl | `wxEVT_SPINCTRL` |
| wxMenuItem | `wxEVT_MENU`, `wxEVT_UPDATE_UI` |
| tool | `wxEVT_TOOL`, `wxEVT_UPDATE_UI` |
| wxNotebook | `wxEVT_NOTEBOOK_PAGE_CHANGED`, `wxEVT_NOTEBOOK_PAGE_CHANGING` |
| wxSplitterWindow | `wxEVT_SPLITTER_SASH_POS_CHANGED`, `wxEVT_SPLITTER_DCLICK` |
| wxStyledTextCtrl | `wxEVT_STC_CHANGE`, `wxEVT_STC_CHARADDED`, `wxEVT_STC_MODIFIED` |
| wxTreeCtrl | `wxEVT_TREE_SEL_CHANGED`, `wxEVT_TREE_ITEM_ACTIVATED` |
| wxGrid | `wxEVT_GRID_CELL_LEFT_CLICK`, `wxEVT_GRID_CELL_CHANGED` |
| wxHyperlinkCtrl | `wxEVT_HYPERLINK` |
| Form (wxFrame) | `wxEVT_CLOSE_WINDOW`, `wxEVT_SIZE`, `wxEVT_IDLE` |
| Form (wxDialog) | `wxEVT_INIT_DIALOG`, `wxEVT_CLOSE_WINDOW` |

### Window events (available on all widgets)
Focus: `wxEVT_SET_FOCUS`, `wxEVT_KILL_FOCUS`
Mouse: `wxEVT_LEFT_DOWN`, `wxEVT_LEFT_UP`, `wxEVT_LEFT_DCLICK`, `wxEVT_RIGHT_DOWN`, `wxEVT_MOTION`, `wxEVT_MOUSEWHEEL`, `wxEVT_ENTER_WINDOW`, `wxEVT_LEAVE_WINDOW`
Keyboard: `wxEVT_KEY_DOWN`, `wxEVT_KEY_UP`, `wxEVT_CHAR`, `wxEVT_CHAR_HOOK`
General: `wxEVT_SIZE`, `wxEVT_PAINT`, `wxEVT_CONTEXT_MENU`, `wxEVT_UPDATE_UI`, `wxEVT_HELP`

---

## 9. Validators

Connect controls to member variables for automatic data transfer:

```xml
<node class="wxTextCtrl"
  validator_variable="m_value"        <!-- Member variable name -->
  validator_data_type="wxString"      <!-- Variable type -->
  get_function="GetValue"             <!-- Accessor function (optional) -->
  validator_style="wxFILTER_NONE" />  <!-- Filter: wxFILTER_NONE, wxFILTER_ASCII,
                                           wxFILTER_ALPHA, wxFILTER_ALPHANUMERIC,
                                           wxFILTER_DIGITS, wxFILTER_NUMERIC,
                                           wxFILTER_INCLUDE_LIST, wxFILTER_EXCLUDE_LIST -->
```

### Numeric validators (wxSpinCtrl, wxSlider)
```xml
validator_variable="m_count" validator_data_type="int"
minValue="0" maxValue="100"
```

---

## 10. Custom Controls

```xml
<node class="gen_CustomControl"
  class_name="MyWidget"                 <!-- Class to instantiate -->
  construction="new MyWidget(#parent, #id, #pos, #size)"  <!-- Constructor template -->
  parameters="wxWindow*, wxWindowID, const wxPoint&, const wxSize&"
  header="mywidget.h" />               <!-- Header file -->
```

Or use `subclass` on existing widgets:
```xml
<node class="wxPanel" subclass="MyPanel" subclass_header="../panels/mypanel.h"
      window_style="0" />
```

---

## 11. Project Settings

```xml
<node class="Project"
  code_preference="C++"                 <!-- Primary language -->
  generate_languages="C++"              <!-- Languages to generate (bitlist) -->
  art_directory="../art"                <!-- Image search directory -->
  internationalize="1"                  <!-- Wrap strings in _() -->
  optional_comments="1"                 <!-- Add explanatory comments -->
  generate_cmake="1"                    <!-- Auto-generate .cmake file -->
  cpp_line_length="110"                 <!-- Max line length for generated code -->
  wxWidgets_version="3.2.0"            <!-- Target wx version (3.2.0 or 3.3.0) -->
  local_pch_file="pch.h"              <!-- Precompiled header -->
  name_space="MyApp">                  <!-- Namespace for all classes -->
```

---

## 12. Complete Examples

### Minimal Dialog with OK/Cancel

C++ equivalent:
```cpp
class SettingsBase : public wxDialog {
    wxTextCtrl* m_textName;
    wxCheckBox* m_checkEnabled;
    void OnInit(wxInitDialogEvent&);
};
// Create():
//   auto* sizer = new wxBoxSizer(wxVERTICAL);
//   auto* grid = new wxFlexGridSizer(2, 0, 0);
//   grid->AddGrowableCol(1);
//   grid->Add(new wxStaticText(this, wxID_ANY, "Name:"), ...);
//   m_textName = new wxTextCtrl(this, wxID_ANY);
//   grid->Add(m_textName, wxSizerFlags().Expand().Border(wxALL));
//   m_checkEnabled = new wxCheckBox(this, wxID_ANY, "Enabled");
//   sizer->Add(grid, wxSizerFlags().Expand().Border(wxALL));
//   sizer->Add(m_checkEnabled, wxSizerFlags().Border(wxALL));
//   sizer->Add(CreateSeparatedSizer(CreateStdDialogButtonSizer(wxOK|wxCANCEL)), ...);
```

.wxui equivalent:
```xml
<node class="wxDialog"
  class_name="SettingsBase"
  base_file="settings_base"
  derived_class_name="Settings"
  title="Settings"
  wxEVT_INIT_DIALOG="OnInit">
  <node class="wxBoxSizer" orientation="wxVERTICAL">
    <node class="wxFlexGridSizer" cols="2" growablecols="1" flags="wxEXPAND">
      <node class="wxStaticText" label="Name:" />
      <node class="wxTextCtrl" var_name="m_textName" flags="wxEXPAND" />
    </node>
    <node class="wxCheckBox" label="Enabled" var_name="m_checkEnabled" />
    <node class="wxStdDialogButtonSizer" flags="wxEXPAND" />
  </node>
</node>
```

### Frame with MenuBar and Toolbar

```xml
<node class="wxFrame"
  class_name="MainFrameBase"
  base_file="mainframe_base"
  derived_class_name="MainFrame"
  title="My Application"
  size="1000,700"
  minimum_size="800,600"
  icon="SVG;app.svg;[16,16]"
  wxEVT_CLOSE_WINDOW="OnClose">

  <!-- Main content sizer -->
  <node class="wxBoxSizer" orientation="wxVERTICAL">
    <node class="wxToolBar" style="wxTB_FLAT|wxTB_HORIZONTAL"
          borders="" flags="wxEXPAND">
      <node class="tool" bitmap="SVG;new.svg;[24,24]" id="wxID_NEW"
            label="New" tooltip="Create new file" wxEVT_TOOL="OnNew" />
      <node class="tool" bitmap="SVG;open.svg;[24,24]" id="wxID_OPEN"
            label="Open" tooltip="Open file" wxEVT_TOOL="OnOpen" />
      <node class="tool" bitmap="SVG;save.svg;[24,24]" id="wxID_SAVE"
            label="Save" tooltip="Save file" />
      <node class="toolSeparator" />
    </node>

    <!-- Main content area -->
    <node class="wxSplitterWindow" min_pane_size="200"
          style="wxSP_LIVE_UPDATE" flags="wxEXPAND" proportion="1">
      <node class="wxPanel" var_name="m_leftPanel">
        <node class="wxBoxSizer" orientation="wxVERTICAL">
          <node class="wxTreeCtrl" var_name="m_tree"
                style="wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT"
                flags="wxEXPAND" proportion="1" />
        </node>
      </node>
      <node class="wxPanel" var_name="m_rightPanel">
        <node class="wxBoxSizer" orientation="wxVERTICAL">
          <node class="wxTextCtrl" var_name="m_editor"
                style="wxTE_MULTILINE" flags="wxEXPAND" proportion="1" />
        </node>
      </node>
    </node>
  </node>

  <!-- MenuBar (sibling of the sizer, child of the frame) -->
  <node class="wxMenuBar">
    <node class="wxMenu" label="&amp;File">
      <node class="wxMenuItem" label="&amp;New" id="wxID_NEW"
            shortcut="Ctrl+N" wxEVT_MENU="OnNew" />
      <node class="wxMenuItem" label="&amp;Open..." id="wxID_OPEN"
            shortcut="Ctrl+O" wxEVT_MENU="OnOpen" />
      <node class="wxMenuItem" label="&amp;Save" id="wxID_SAVE"
            shortcut="Ctrl+S" />
      <node class="separator" />
      <node class="wxMenuItem" label="E&amp;xit" id="wxID_EXIT" />
    </node>
    <node class="wxMenu" label="&amp;Help">
      <node class="wxMenuItem" label="&amp;About" id="wxID_ABOUT"
            wxEVT_MENU="OnAbout" />
    </node>
  </node>

  <node class="wxStatusBar" />
</node>
```

### Reusable Panel with Toolbar

```xml
<node class="PanelForm"
  class_name="EditorPanelBase"
  base_file="editor_panel_base"
  derived_class_name="EditorPanel">
  <node class="wxBoxSizer" orientation="wxVERTICAL">
    <node class="wxToolBar" style="wxTB_HORIZONTAL|wxTB_TEXT"
          borders="" flags="wxEXPAND">
      <node class="tool" bitmap="Art;wxART_GO_BACK|wxART_TOOLBAR"
            id="wxID_BACKWARD" label="Back" wxEVT_TOOL="OnBack"
            wxEVT_UPDATE_UI="OnUpdateBack" />
      <node class="toolSeparator" />
      <node class="tool" bitmap="SVG;cpp.svg;[16,16]" id="ID_CPP"
            kind="wxITEM_RADIO" label="C++" wxEVT_TOOL="OnCPlus" />
      <node class="tool" bitmap="SVG;python.svg;[16,16]" id="ID_PYTHON"
            kind="wxITEM_RADIO" label="Python" wxEVT_TOOL="OnPython" />
    </node>
    <node class="wxStyledTextCtrl" read_only="1" use_tabs="0"
          wrap_mode="word" flags="wxEXPAND" proportion="1" />
  </node>
</node>
```

### Dialog with Notebook

```xml
<node class="wxDialog"
  class_name="PreferencesBase"
  base_file="preferences_base"
  derived_class_name="Preferences"
  title="Preferences"
  style="wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER"
  persist="1"
  wxEVT_INIT_DIALOG="OnInit">
  <node class="wxBoxSizer" orientation="wxVERTICAL" flags="wxEXPAND">
    <node class="wxNotebook" flags="wxEXPAND" proportion="1">
      <node class="BookPage" label="General" background_colour="wxSYS_COLOUR_BTNFACE"
            window_style="wxTAB_TRAVERSAL">
        <node class="wxBoxSizer" orientation="wxVERTICAL">
          <node class="wxCheckBox" label="Show tooltips" var_name="m_checkTooltips" />
          <node class="wxCheckBox" label="Auto-save" var_name="m_checkAutoSave" />
        </node>
      </node>
      <node class="BookPage" label="Editor" background_colour="wxSYS_COLOUR_BTNFACE"
            window_style="wxTAB_TRAVERSAL">
        <node class="wxBoxSizer" orientation="wxVERTICAL">
          <node class="wxFlexGridSizer" cols="2" growablecols="1" flags="wxEXPAND">
            <node class="wxStaticText" label="Font size:" />
            <node class="wxSpinCtrl" var_name="m_spinFontSize"
                  min="8" max="72" initial="12" />
            <node class="wxStaticText" label="Tab width:" />
            <node class="wxSpinCtrl" var_name="m_spinTabWidth"
                  min="1" max="8" initial="4" />
          </node>
        </node>
      </node>
    </node>
    <node class="wxStdDialogButtonSizer" flags="wxEXPAND" OKButtonClicked="OnOK" />
  </node>
</node>
```

---

## 13. Conversion Checklist

When converting wxWidgets C++ code to .wxui:

1. **Identify the form type** — `wxFrame`, `wxDialog`, `PanelForm`, etc.
2. **Set form attributes** — `class_name`, `base_file`, `title`, `style`, `size`
3. **Map the sizer hierarchy** — Each `new wxBoxSizer(wxVERTICAL)` → `<node class="wxBoxSizer" orientation="wxVERTICAL">`
4. **Map widget constructors** → `<node class="wxWidgetClass">` with properties as attributes
5. **Map `sizer->Add()` calls** — Extract `proportion`, `flags`, `border` from wxSizerFlags
6. **Map `Bind()` calls** — `wxEVT_*="HandlerName"` on the widget node
7. **Map menu/toolbar** — Nested `wxMenuBar` → `wxMenu` → `wxMenuItem` / `wxToolBar` → `tool`
8. **Omit defaults** — Don't include `borders="wxALL"` + `border_size="5"` (they're defaults)
9. **Use standard IDs** — `wxID_OK`, `wxID_CANCEL`, `wxID_SAVE`, etc. where applicable
10. **Set `var_name`** only when the C++ code stores it as a member variable

### Common wxSizerFlags → XML mapping

| C++ Code | XML Attributes |
|----------|---------------|
| `wxSizerFlags().Expand().Border(wxALL)` | `flags="wxEXPAND"` (borders default to wxALL) |
| `wxSizerFlags(1).Expand().Border(wxALL)` | `flags="wxEXPAND" proportion="1"` |
| `wxSizerFlags().Border(wxLEFT\|wxRIGHT, 10)` | `borders="wxLEFT\|wxRIGHT" border_size="10"` |
| `wxSizerFlags().Expand()` (no border) | `flags="wxEXPAND" borders=""` |
| `wxSizerFlags().Center()` | `alignment="wxALIGN_CENTER"` |
| `wxSizerFlags(1).Expand().Border(wxALL, 0)` | `flags="wxEXPAND" proportion="1" borders=""` |
| `AddStretchSpacer(1)` | `<node class="spacer" proportion="1" />` |
| `AddSpacer(10)` | `<node class="spacer" height="10" width="10" />` |

---

## 14. Tips

- **wxStdDialogButtonSizer default buttons**: By default, OK and Cancel are shown. Specify `OK="1" Cancel="1"` only if you also need other buttons.
- **MenuBar placement**: In wxFrame, the wxMenuBar node is a child of the frame node, **sibling** of the main sizer — not inside the sizer.
- **StatusBar**: Also a direct child of wxFrame, not inside a sizer.
- **ToolBar**: Can be inside the main sizer (with `borders="" flags="wxEXPAND"`) or direct child of frame.
- **Empty `borders=""`**: Explicitly means no border. Different from omitting the attribute (which defaults to `wxALL`).
- **`class_access="none"`**: The widget becomes a local variable, not a class member. Use for wxStaticText labels and other widgets you don't need to reference later.
- **System colors**: Use `wxSYS_COLOUR_BTNFACE`, `wxSYS_COLOUR_WINDOW`, etc. for system-appropriate colors.
- **Persist**: Setting `persist="1"` on dialogs saves/restores their position and size between sessions.
