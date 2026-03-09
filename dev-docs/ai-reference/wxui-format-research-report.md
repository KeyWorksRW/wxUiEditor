# wxUiEditor .wxui File Format — Comprehensive Research Report

> **Generated from:** Full analysis of all XML definition files in `src/xml/`, existing AI docs in `docs/ai-context/wxui-format/`, generated C++ files in `src/wxui/`, and the project's own `wxUiEditor.wxui` file.

---

## 1. XML Structure

### Root Element
```xml
<?xml version="1.0"?>
<wxUiEditorData data_version="20">
  <node class="Project" ...>
    <!-- All content here -->
  </node>
</wxUiEditorData>
```

- **`data_version`**: Integer format version. Current values: 19–20. Used for backward compatibility and auto-migration.
- **No XML namespaces** or external DTD references in `.wxui` files.
- The DTD (`gen.dtd`) exists only for the internal XML definition files in `src/xml/`.

### Node Element
Every UI component is a `<node>` element:
```xml
<node class="ClassName" property1="value1" property2="value2">
  <event name="wxEVT_TYPE">HandlerName</event>
  <node class="ChildClass" ... />
</node>
```

- **`class`** attribute: Required. Identifies the widget/component type.
- **Properties**: XML attributes on the `<node>` element.
- **Events**: Child `<event>` elements (not attributes).
- **Children**: Nested `<node>` elements.

### Hierarchy
```
Project
├── Images
│   └── embedded_image (multiple)
├── Data
│   ├── data_folder
│   ├── data_string
│   └── data_xml
├── folder / sub_folder (organizational)
│   └── Forms...
├── wxFrame / wxDialog / PanelForm / ... (forms)
│   ├── wxBoxSizer / wxGridSizer / ... (sizers)
│   │   ├── wxButton / wxTextCtrl / ... (widgets)
│   │   └── spacer
│   ├── wxMenuBar
│   │   └── wxMenu
│   │       ├── wxMenuItem
│   │       ├── submenu
│   │       └── separator
│   ├── wxToolBar
│   │   ├── tool
│   │   ├── tool_dropdown
│   │   └── toolSeparator
│   └── wxStatusBar
└── ToolBar / MenuBar / PopupMenu (standalone forms)
```

### Property Storage Rule (CRITICAL)
**Only non-default values are stored.** If a property equals its default, it must NOT appear in the `.wxui` file. This keeps files compact and version-control-friendly.

---

## 2. Property Format

### Property Type Catalog

| Type | Format | Example |
|------|--------|---------|
| `string` | Plain text | `var_name="m_button"` |
| `string_escapes` | Text with `\n`, `\t`, `&amp;` support | `label="Line1\nLine2"` |
| `string_edit` | Multi-line text (newlines as `@@`) | `help="Status bar text"` |
| `string_edit_escapes` | Multi-line + escapes | `label="Hello\nWorld"` |
| `string_edit_single` | Single-line editable | `html_url="http://..."` |
| `string_code_single` | Single-line code | `cpp_conditional="..."` |
| `code_edit` | Multi-line code block | `source_preamble="..."` |
| `bool` | `0` or `1` | `disabled="1"` |
| `int` | Signed integer | `sashpos="200"` |
| `uint` | Unsigned integer | `border_size="5"` |
| `float` | Floating point | `sashgravity="0.5"` |
| `option` | Single value from list | `orientation="wxVERTICAL"` |
| `bitlist` | Pipe-separated flags | `style="wxTE_MULTILINE\|wxTE_READONLY"` |
| `id` | Widget identifier | `id="wxID_OK"` or `id="myID=100"` |
| `image` | Bitmap reference | (see §10 below) |
| `wxSize` | Width,Height | `size="400,300"` or `size="-1,-1"` |
| `wxPoint` | X,Y | `pos="10,20"` |
| `wxColour` | Color specification | `colour="#FF0000"` or `colour="255,0,0"` |
| `wxFont` | Font specification | (complex; set via editor) |
| `path` | Directory path | `art_directory="../art"` |
| `file` | File path | `base_file="my_dialog_base"` |
| `stringlist` | Newline-separated strings | `contents="Item1\nItem2\nItem3"` |
| `stringlist_semi` | Semicolon-separated strings | `col_label_values="Col1;Col2"` |
| `stringlist_escapes` | String list with escapes | `additional_inheritance="MyClass"` |
| `include_files` | Header file list | `local_hdr_includes="myheader.h"` |
| `checklist_item` | Items with check state | (wxCheckListBox contents) |
| `statbar_fields` | Status bar field defs | (wxStatusBar fields) |
| `html_edit` | HTML content | `html_content="<p>Hello</p>"` |
| `animation` | Animation file ref | (wxAnimationCtrl) |
| `custom_mockup` | Custom control display | (gen_CustomControl) |

### Lambda Event Handlers
In `.wxui` files, lambda handlers use `@@` as line separators:
```xml
wxEVT_UPDATE_UI="[](wxUpdateUIEvent&amp; event)@@{@@    event.Enable(false);@@}"
```

### Default Values in XML Definitions
In definition files, default values are specified as text content:
```xml
<property name="border_size" type="uint">5</property>
<!-- Options have default after all option elements -->
<property name="orientation" type="option">
    <option name="wxVERTICAL"/>
    <option name="wxHORIZONTAL"/> wxVERTICAL </property>
```

---

## 3. Widget Classes

### Complete Widget Catalog

**Buttons** (`buttons.xml`):
| Class | Type | Default var_name |
|-------|------|-----------------|
| `wxButton` | widget | `m_button` |
| `CloseButton` | widget | `m_button` |
| `wxCommandLinkButton` | widget | `m_cmdBtn` |
| `wxCheckBox` | widget | `m_checkBox` |
| `Check3State` | widget | `m_checkBox3` |
| `wxRadioButton` | widget | `m_radioBtn` |
| `wxSpinCtrl` | widget | `m_spinCtrl` |
| `wxSpinCtrlDouble` | widget | `m_spinCtrlDouble` |
| `wxSpinButton` | widget | `m_spinBtn` |
| `wxToggleButton` | widget | `m_toggleBtn` |
| `wxBitmapToggleButton` | widget | `m_bmpToggleBtn` |
| `wxContextHelpButton` | widget | `m_contextHelp` |

**Text Controls** (`text_ctrls.xml`):
| Class | Type | Default var_name |
|-------|------|-----------------|
| `wxStaticText` | widget | `m_staticText` |
| `wxTextCtrl` | widget | `m_textCtrl` |
| `wxRichTextCtrl` | widget | `m_richText` |
| `wxStyledTextCtrl` | widget | `m_scintilla` |

**Choice/List Controls** (`boxes.xml`):
| Class | Type | Default var_name |
|-------|------|-----------------|
| `wxComboBox` | widget | `m_comboBox` |
| `wxBitmapComboBox` | widget | `m_bmpComboBox` |
| `wxChoice` | widget | `m_choice` |
| `wxListBox` | widget | `m_listBox` |
| `wxSimpleHtmlListBox` | widget | `m_simpleHtmlListBox` |
| `wxCheckListBox` | widget | `m_checkListBox` |
| `wxRadioBox` | widget | `m_radioBox` |
| `wxStaticBox` | staticbox | `m_staticBox` |

**General Widgets** (`widgets.xml`):
| Class | Type | Default var_name |
|-------|------|-----------------|
| `wxActivityIndicator` | widget | `m_activityIndicator` |
| `wxAnimationCtrl` | widget | `m_animCtrl` |
| `wxBannerWindow` | widget | `m_banner` |
| `gen_CustomControl` | widget | (user-defined) |
| `wxRearrangeCtrl` | widget | `m_rearrangeCtrl` |
| `wxStaticBitmap` | widget | `m_bitmap` |
| `wxStaticLine` | widget | `m_staticLine` |
| `wxSlider` | widget | `m_slider` |
| `wxGauge` | widget | `m_gauge` |
| `wxScrollBar` | widget | `m_scrollBar` |
| `wxHyperlinkCtrl` | widget | `m_hyperlink` |
| `wxSearchCtrl` | widget | `m_searchCtrl` |
| `wxHtmlWindow` | widget | `m_htmlWin` |
| `wxWebView` | widget | `m_webview` |
| `wxCalendarCtrl` | widget | `m_calendar` |
| `wxGenericDirCtrl` | widget | `m_genericDirCtrl` |
| `wxFileCtrl` | widget | `m_fileCtrl` |

**Pickers** (`pickers.xml`):
| Class | Type | Default var_name |
|-------|------|-----------------|
| `wxFilePickerCtrl` | widget | `m_filePicker` |
| `wxDirPickerCtrl` | widget | `m_dirPicker` |
| `wxFontPickerCtrl` | widget | `m_fontPicker` |
| `wxColourPickerCtrl` | widget | `m_colourPicker` |
| `wxDatePickerCtrl` | widget | `m_datePicker` |
| `wxTimePickerCtrl` | widget | `m_timePicker` |

**Data Controls** (`data_ctrls.xml`):
| Class | Type | Default var_name |
|-------|------|-----------------|
| `wxDataViewCtrl` | dataviewctrl | `m_dataViewCtrl` |
| `wxDataViewTreeCtrl` | dataviewtreectrl | `m_dataViewTreeCtrl` |
| `wxDataViewListCtrl` | dataviewlistctrl | `m_dataViewListCtrl` |
| `dataViewColumn` | dataviewcolumn | `m_dataViewColumn` |
| `dataViewListColumn` | dataviewlistcolumn | `m_dataViewListColumn` |
| `wxGrid` | widget | `m_grid` |
| `wxPropertyGrid` | propgrid | `m_propertyGrid` |
| `wxPropertyGridManager` | propgridman | `m_propertyGridManager` |
| `propGridPage` | propgridpage | `m_propertyGridPage` |
| `propGridItem` | propgriditem | `m_propertyGridItem` |
| `propGridCategory` | propgrid_category | `m_propertyGridItem` |
| `wxTreeCtrl` | widget | `m_treeCtrl` |
| `wxTreeListCtrl` | treelistctrl | `m_treeListCtrl` |
| `TreeListCtrlColumn` | treelistctrlcolumn | (no var_name) |
| `wxListView` | widget | `m_listview` |
| `wxEditableListBox` | widget | `m_edit_listbox` |

**Containers** (`containers.xml`):
| Class | Type | Default var_name |
|-------|------|-----------------|
| `wxPanel` | container | `panel` |
| `wxSplitterWindow` | splitter | `m_splitter` |
| `wxScrolledCanvas` | widget | `m_scrolledCanvas` |
| `wxScrolledWindow` | widget | `m_scrolledWindow` |
| `wxCollapsiblePane` | widget | `m_collapsiblePane` |
| `wxChoicebook` | choicebook | `m_choicebook` |
| `wxListbook` | listbook | `m_listbook` |
| `wxAuiNotebook` | notebook | `m_auiNotebook` |
| `wxNotebook` | notebook | `m_notebook` |
| `wxToolbook` | notebook | `m_toolbook` |
| `wxTreebook` | notebook | `m_treebook` |
| `wxSimplebook` | simplebook | `m_simplebook` |
| `BookPage` | bookpage | `page` |
| `PageCtrl` | page | (label only) |

**Bars, Menus, and Ribbons** (`bars.xml`):
| Class | Type | Default var_name |
|-------|------|-----------------|
| `ToolBar` | toolbar_form | (standalone form) |
| `wxToolBar` | toolbar | `m_toolbar` |
| `tool` | tool | (no default var) |
| `tool_dropdown` | tool_dropdown | (no default var) |
| `toolSeparator` | tool_separator | (no default var) |
| `toolStretchable` | tool_separator | (no default var) |
| `AuiToolBar` | aui_toolbar_form | (standalone form) |
| `wxAuiToolBar` | aui_toolbar | `m_auiToolbar` |
| `auitool` | aui_tool | (no default var) |
| `auitool_label` | aui_tool | (no default var) |
| `auitool_spacer` | aui_tool | (no default var) |
| `auitool_stretchable` | aui_tool | (no default var) |
| `wxStatusBar` | statusbar | `m_statusBar` |
| `wxInfoBar` | widget | `m_infoBar` |
| `MenuBar` | menubar_form | (standalone form) |
| `PopupMenu` | popup_menu | (standalone form) |
| `wxContextMenuEvent` | ctx_menu | (handler_name) |
| `wxMenuBar` | menubar | `m_menubar` |
| `wxMenu` | menu | `m_menu` |
| `submenu` | submenu | `submenu` |
| `wxMenuItem` | menuitem | `menu_item` |
| `separator` | menuitem | `separator` |
| `MdiFrameMenuBar` | mdi_menubar | `m_default_menubar` |
| `MdiDocMenuBar` | doc_menubar | `m_doc_menubar` |
| `wxRibbonBar` | ribbonbar | `m_rbnBar` |
| `wxRibbonPage` | ribbonpage | `rbnPage` |
| `wxRibbonPanel` | ribbonpanel | `rbnPanel` |
| `wxRibbonButtonBar` | ribbonbuttonbar | `rbnBtnBar` |
| `ribbonButton` | ribbonbutton | (label only) |
| `wxRibbonToolBar` | ribbontoolbar | `rbnToolBar` |
| `ribbonTool` | ribbontool | (id only) |
| `wxRibbonGallery` | ribbongallery | `rbnGallery` |
| `ribbonGalleryItem` | ribbongalleryitem | (bitmap only) |
| `ribbonSeparator` | ribbontool | (no properties) |

**MDI / Doc-View** (`mdi.xml`):
| Class | Type |
|-------|------|
| `DocViewApp` | DocViewApp |
| `DocumentImage` | wx_document |
| `DocumentRichTextCtrl` | wx_document |
| `DocumentSplitterWindow` | wx_document |
| `DocumentStyledTextCtrl` | wx_document |
| `DocumentTextCtrl` | wx_document |
| `ViewImage` | wx_view |
| `ViewRichTextCtrl` | wx_view |
| `ViewSplitterWindow` | wx_view |
| `ViewStyledTextCtrl` | wx_view |
| `ViewTextCtrl` | wx_view |

**Project-Level** (`project.xml`):
| Class | Type |
|-------|------|
| `Project` | project |
| `folder` | folder |
| `sub_folder` | sub_folder |
| `Images` | images |
| `embedded_image` | embedded_image |
| `wxTimer` | widget |
| `Data` | data |
| `data_folder` | data_folder |
| `data_string` | data_string |
| `data_xml` | data_xml |

---

## 4. Sizer Classes

### Sizer Catalog (`sizers.xml`)

| Class | Type | Key Properties |
|-------|------|---------------|
| `wxBoxSizer` | sizer | `orientation` (wxVERTICAL/wxHORIZONTAL) |
| `VerticalBoxSizer` | sizer | Pre-configured vertical wxBoxSizer |
| `wxStaticBoxSizer` | sizer | `orientation`, `label` |
| `StaticCheckboxBoxSizer` | sizer | Checkbox in sizer header |
| `StaticRadioBtnBoxSizer` | sizer | Radio button in sizer header |
| `wxWrapSizer` | sizer | `orientation`, `flags` (wxWRAPSIZER_DEFAULT_FLAGS, wxEXTEND_LAST_ON_EACH_LINE, wxREMOVE_LEADING_SPACES) |
| `wxGridSizer` | sizer | `rows`, `cols`, `vgap`, `hgap` |
| `wxFlexGridSizer` | sizer | inherits flexgridsizerbase: `rows`, `cols`, `vgap`, `hgap`, `growablecols`, `growablerows`, `flexible_direction`, `non_flexible_grow_mode` |
| `wxGridBagSizer` | sizer | inherits flexgridsizerbase + GridBag positioning |
| `TextSizer` | sizer | `text` for sizing reference text |
| `spacer` | spacer | `width`, `height` |
| `wxStdDialogButtonSizer` | sizer | Standard buttons: OK, Yes, Save, Apply, No, Cancel, Close, Help, ContextHelp (all bool); `default_button` option |

### Sizer Items
- `sizeritem`: XML import wrapper for child items
- `gbsizeritem`: GridBagSizer child (row, column, rowspan, colspan)
- `splitteritem`: SplitterWindow child

### All Sizers Inherit
- `sizer_dimension`: `minimum_size` property
- `sizer_child`: alignment, borders, flags, proportion (see §9)

---

## 5. Form Classes

### Form Catalog (`forms.xml`)

| Class | Type | Base Widget |
|-------|------|-------------|
| `wxFrame` | frame_form | wxFrame |
| `wxDocParentFrame` | frame_form | wxDocParentFrame |
| `wxDocChildFrame` | frame_form | wxDocChildFrame |
| `wxDocMDIParentFrame` | frame_form | wxDocMDIParentFrame |
| `wxDocMDIChildFrame` | frame_form | wxDocMDIChildFrame |
| `wxAuiMDIParentFrame` | frame_form | wxAuiMDIParentFrame |
| `wxAuiMDIChildFrame` | frame_form | wxAuiMDIChildFrame |
| `wxDialog` | form | wxDialog |
| `PanelForm` | panel_form | wxPanel (standalone) |
| `wxPropertySheetDialog` | propsheetform | wxPropertySheetDialog |
| `wxWizard` | wizard | wxWizard |
| `wxWizardPageSimple` | wizardpagesimple | wxWizardPageSimple |
| `wxPopupWindow` | form | wxPopupWindow |
| `wxPopupTransientWindow` | form | wxPopupTransientWindow |
| `RibbonBar` | ribbonbar_form | wxRibbonBar (standalone) |
| `ToolBar` | toolbar_form | wxToolBar (standalone) |
| `AuiToolBar` | aui_toolbar_form | wxAuiToolBar (standalone) |
| `MenuBar` | menubar_form | wxMenuBar (standalone) |
| `PopupMenu` | popup_menu | wxMenu (standalone) |

### Common Form Properties
All forms typically inherit these interface classes:
- **`Language Settings`**: Per-form language file settings (base_file, perl_file, python_file, ruby_file, etc.)
- **`XRC Settings`**: xrc_file
- **`C++ Settings`**: base_file, source_preamble, generate_ids, class_decoration, name_space
- **`C++ Header Settings`**: header_preamble, local_hdr_includes, inserted_hdr_code
- **`C++ Derived Class Settings`**: use_derived_class, derived_class_name, derived_file
- **`wxTopLevelWindow`** (events): wxEVT_ACTIVATE, wxEVT_CLOSE_WINDOW, wxEVT_ICONIZE, wxEVT_MAXIMIZE, wxEVT_MOVE, wxEVT_SHOW, wxEVT_IDLE
- **`Window Events`**: Focus, General, Keyboard, Mouse events
- **`wxWindow`**: Common widget properties (id, size, pos, styles, colours, etc.)

### Form-Specific Properties

**wxFrame / wxDialog**:
```xml
<node class="wxFrame"
  class_name="MainFrame"
  title="My Application"
  style="wxDEFAULT_FRAME_STYLE"
  center="wxBOTH"
  icon="SVG;app_icon.svg;[16,16]"
  size="800,600">
```

**wxDialog** adds:
- Event: `wxEVT_INIT_DIALOG`
- Style options: `wxWANTS_CHARS`, `wxFULL_REPAINT_ON_RESIZE`
- Inherited: `Dialog Window Settings` (variant, pos, size, colours, font, subclass)

**PanelForm** (standalone panel):
- `mockup_size`: Display size in the editor
- Full Window Settings category
- Common use: reusable panel components

**wxPropertySheetDialog**:
- `prop_book_type`: NOTEBOOK, CHOICEBOOK, LISTBOOK, TOOLBOOK, TREEBOOK
- `prop_buttons`: wxOK, wxCANCEL, wxYES, wxNO, wxAPPLY, wxCLOSE, wxHELP
- Standard button click/update events

**wxWizard**:
- `border`: Border around page area
- `bitmap`: Wizard page bitmap
- `bmp_placement`, `bmp_min_width`, `bmp_background_colour`

---

## 6. Event Handling

### Event Format in `.wxui` Files
```xml
<node class="wxButton" var_name="m_btn">
  <event name="wxEVT_BUTTON">OnButtonClick</event>
</node>
```

### Lambda Handlers
```xml
<event name="wxEVT_UPDATE_UI">[](wxUpdateUIEvent&amp; event)@@{@@    event.Enable(false);@@}</event>
```
- `@@` = line break in lambda body
- XML entities required: `&amp;`, `&lt;`, `&gt;`

### Event Categories (from `Window Events` interface)

**Focus Events**: `wxEVT_KILL_FOCUS`, `wxEVT_SET_FOCUS`

**General Events**: `wxEVT_CONTEXT_MENU`, `wxEVT_ERASE_BACKGROUND`, `wxEVT_HELP`, `wxEVT_PAINT`, `wxEVT_SIZE`, `wxEVT_UPDATE_UI`

**Keyboard Events**: `wxEVT_CHAR`, `wxEVT_CHAR_HOOK`, `wxEVT_KEY_DOWN`, `wxEVT_KEY_UP`

**Mouse Events**: `wxEVT_ENTER_WINDOW`, `wxEVT_LEAVE_WINDOW`, `wxEVT_LEFT_DCLICK`, `wxEVT_LEFT_DOWN`, `wxEVT_LEFT_UP`, `wxEVT_MIDDLE_DCLICK`, `wxEVT_MIDDLE_DOWN`, `wxEVT_MIDDLE_UP`, `wxEVT_RIGHT_DCLICK`, `wxEVT_RIGHT_DOWN`, `wxEVT_RIGHT_UP`, `wxEVT_AUX1_DCLICK`, `wxEVT_AUX1_DOWN`, `wxEVT_AUX1_UP`, `wxEVT_AUX2_DCLICK`, `wxEVT_AUX2_DOWN`, `wxEVT_AUX2_UP`, `wxEVT_MOTION`, `wxEVT_MOUSEWHEEL`

**Top-Level Window Events** (from `wxTopLevelWindow`): `wxEVT_ACTIVATE`, `wxEVT_CLOSE_WINDOW`, `wxEVT_ICONIZE`, `wxEVT_MAXIMIZE`, `wxEVT_MOVE`, `wxEVT_MOVING`, `wxEVT_MOVE_START`, `wxEVT_MOVE_END`, `wxEVT_SHOW`, `wxEVT_IDLE`

### Common Widget Events

| Widget | Events |
|--------|--------|
| wxButton | `wxEVT_BUTTON` |
| wxCheckBox | `wxEVT_CHECKBOX` |
| wxRadioButton | `wxEVT_RADIOBUTTON` |
| wxTextCtrl | `wxEVT_TEXT`, `wxEVT_TEXT_ENTER`, `wxEVT_TEXT_URL`, `wxEVT_TEXT_MAXLEN` |
| wxComboBox | `wxEVT_COMBOBOX`, `wxEVT_TEXT`, `wxEVT_TEXT_ENTER`, `wxEVT_COMBOBOX_DROPDOWN`, `wxEVT_COMBOBOX_CLOSEUP` |
| wxChoice | `wxEVT_CHOICE` |
| wxListBox | `wxEVT_LISTBOX`, `wxEVT_LISTBOX_DCLICK` |
| wxSlider | `wxEVT_SLIDER` |
| wxSpinCtrl | `wxEVT_SPINCTRL`, `wxEVT_TEXT`, `wxEVT_TEXT_ENTER` |
| wxMenuItem | `wxEVT_MENU`, `wxEVT_UPDATE_UI` |
| tool | `wxEVT_TOOL`, `wxEVT_TOOL_DROPDOWN`, `wxEVT_TOOL_RCLICKED`, `wxEVT_UPDATE_UI` |
| wxNotebook | `wxEVT_NOTEBOOK_PAGE_CHANGED`, `wxEVT_NOTEBOOK_PAGE_CHANGING` |
| wxSplitterWindow | `wxEVT_SPLITTER_SASH_POS_CHANGED`, `wxEVT_SPLITTER_SASH_POS_CHANGING`, `wxEVT_SPLITTER_DCLICK`, `wxEVT_SPLITTER_UNSPLIT` |
| wxStyledTextCtrl | Extensive: `wxEVT_STC_CHANGE`, `wxEVT_STC_CHARADDED`, `wxEVT_STC_MODIFIED`, `wxEVT_STC_MARGINCLICK`, etc. |
| wxGrid | `wxEVT_GRID_CELL_LEFT_CLICK`, `wxEVT_GRID_CELL_CHANGED`, `wxEVT_GRID_SELECT_CELL`, etc. |
| wxTreeCtrl | `wxEVT_TREE_SEL_CHANGED`, `wxEVT_TREE_ITEM_ACTIVATED`, `wxEVT_TREE_ITEM_EXPANDING`, etc. |
| wxDataViewCtrl | `wxEVT_DATAVIEW_SELECTION_CHANGED`, `wxEVT_DATAVIEW_ITEM_ACTIVATED`, etc. |

---

## 7. Style Flags

### Frame Styles
```
wxDEFAULT_FRAME_STYLE (= wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCAPTION|wxCLOSE_BOX|wxCLIP_CHILDREN)
wxCAPTION, wxCLOSE_BOX, wxMAXIMIZE_BOX, wxMINIMIZE_BOX, wxRESIZE_BORDER
wxSYSTEM_MENU, wxSTAY_ON_TOP, wxFRAME_TOOL_WINDOW, wxFRAME_NO_TASKBAR
wxFRAME_FLOAT_ON_PARENT, wxFRAME_SHAPED, wxICONIZE, wxMAXIMIZE
```

### Dialog Styles
```
wxDEFAULT_DIALOG_STYLE (= wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX)
+ wxRESIZE_BORDER, wxSTAY_ON_TOP, wxDIALOG_NO_PARENT
wxWANTS_CHARS, wxFULL_REPAINT_ON_RESIZE
```

### Window Styles (universal)
```
wxBORDER_DEFAULT, wxBORDER_SIMPLE, wxBORDER_SUNKEN, wxBORDER_RAISED
wxBORDER_STATIC, wxBORDER_THEME, wxBORDER_NONE
wxTRANSPARENT_WINDOW, wxTAB_TRAVERSAL, wxWANTS_CHARS
wxVSCROLL, wxHSCROLL, wxALWAYS_SHOW_SB, wxCLIP_CHILDREN
wxFULL_REPAINT_ON_RESIZE
```

### Extra Window Styles
```
wxWS_EX_VALIDATE_RECURSIVELY, wxWS_EX_BLOCK_EVENTS
wxWS_EX_TRANSIENT, wxWS_EX_PROCESS_IDLE, wxWS_EX_PROCESS_UI_UPDATES
```

### Sizer Alignment Flags
```
wxALIGN_LEFT, wxALIGN_TOP, wxALIGN_RIGHT, wxALIGN_BOTTOM
wxALIGN_CENTER, wxALIGN_CENTER_HORIZONTAL, wxALIGN_CENTER_VERTICAL
```

### Sizer Item Flags
```
wxEXPAND, wxSHAPED, wxFIXED_MINSIZE, wxRESERVE_SPACE_EVEN_IF_HIDDEN
```

### Border Flags
```
wxALL, wxLEFT, wxRIGHT, wxTOP, wxBOTTOM
```

---

## 8. Common Properties (from `wxWindow` interface)

These properties are available on virtually all widgets:

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `id` | id | `wxID_ANY` | Widget identifier |
| `variant` | option | `normal` | Size variant: normal, small, mini, large |
| `pos` | wxPoint | (empty) | Position (-1,-1 = auto) |
| `size` | wxSize | (empty) | Size (-1,-1 = auto) |
| `minimum_size` | wxSize | (empty) | Minimum size override |
| `maximum_size` | wxSize | (empty) | Maximum size |
| `window_style` | bitlist | (empty) | Window style flags |
| `window_extra_style` | bitlist | (empty) | Extended style flags |
| `tooltip` | string_edit_escapes | (empty) | Tooltip text |
| `disabled` | bool | `0` | Initial disabled state |
| `hidden` | bool | `0` | Initially hidden |
| `foreground_colour` | wxColour | (empty) | Foreground color |
| `background_colour` | wxColour | (empty) | Background color |
| `font` | wxFont | (empty) | Font override |
| `context_help` | string_escapes | (empty) | Context help text |
| `subclass` | string | (empty) | Custom subclass name |
| `subclass_params` | string_escapes | (empty) | Additional constructor params |
| `subclass_header` | file | (empty) | Header for subclass |
| `window_name` | string_escapes | (empty) | Named window identifier |
| `platforms` | bitlist | `Windows\|Unix\|Mac` | Target platforms |
| `disable_language` | bitlist | (empty) | Languages to skip (C++, wxPerl, wxPython, wxRuby) |

---

## 9. Sizer Item Properties (from `sizer_child` interface)

Every widget that can be a sizer child inherits these properties:

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `alignment` | bitlist | (empty) | Alignment flags (wxALIGN_*) |
| `borders` | bitlist | `wxALL` | Border sides |
| `border_size` | uint | `5` | Border size in pixels (DPI-scaled) |
| `scale_border_size` | bool | `1` | Scale border on high DPI |
| `flags` | bitlist | (empty) | Stretch flags (wxEXPAND, etc.) |
| `proportion` | uint | `0` | Stretch proportion (0 = no stretch) |
| `row` | int | `0` | Grid bag row |
| `column` | int | `0` | Grid bag column |
| `rowspan` | int | `1` | Grid bag row span |
| `colspan` | int | `1` | Grid bag column span |

### Property Overrides
When inheriting `sizer_child` or `wxWindow`, definitions can override defaults:
```xml
<inherits class="wxWindow">
    <property name="proportion" type="uint">1</property>
    <property name="flags" type="bitlist">wxEXPAND</property>
</inherits>
```
This means some widgets (like `wxWebView`) have different sizer defaults than the base.

### Property Hiding
Definitions can hide inherited properties:
```xml
<inherits class="wxWindow">
    <hide name="variant"/>
    <hide name="foreground_colour"/>
</inherits>
```

---

## 10. Image/Bitmap Properties

### Bitmap Format Strings
Images are stored as formatted strings with type prefixes:

| Format | Syntax | Example |
|--------|--------|---------|
| SVG | `SVG;filename.svg;[width,height]` | `bitmap="SVG;save.svg;[16,16]"` |
| Embedded PNG | `Embed;filename.png` | `bitmap="Embed;icon.png"` |
| Art Provider | `Art;artID\|clientID` | `bitmap="Art;wxART_FILE_OPEN\|wxART_MENU"` |

### Bitmap Properties (from `Bitmaps` interface)

| Property | Type | Description |
|----------|------|-------------|
| `bitmap` | image | Default state bitmap |
| `disabled_bmp` | image | Disabled state |
| `pressed_bmp` | image | Pressed/pushed state |
| `focus_bmp` | image | Keyboard focus state |
| `current` | image | Mouse hover state |
| `position` | option | Bitmap position: wxLEFT, wxRIGHT, wxTOP, wxBOTTOM |
| `margins` | wxSize | Margins between bitmap and text (MSW only) |

### Embedded Images (Project Level)
```xml
<node class="Images">
  <node class="embedded_image" bitmap="SVG;toolbar/save.svg;[24,24]" />
  <node class="embedded_image" bitmap="Embed;icons/logo.png" />
</node>
```

The `art_directory` Project property specifies the base directory for image lookups.

---

## 11. Variable Naming

### Conventions
- **`var_name`**: On widgets — becomes a class member variable (C++) or local variable
  - Prefix: `m_` for member variables (auto-converted per language)
  - Style: camelCase after prefix (e.g., `m_okButton`, `m_textCtrl`)
  - Must be unique within the form
- **`class_name`**: On forms — becomes the generated class name
  - Style: PascalCase (e.g., `MainFrameBase`, `MyDialogBase`)
  - Suffix "Base" is conventional for generated base classes
- **`derived_class_name`**: On forms — the user's derived class name
  - Style: PascalCase without "Base" suffix (e.g., `MainFrame`, `MyDialog`)

### Default var_name Patterns
Each widget class has a default `var_name` that follows conventions:
- Buttons: `m_button`, `m_checkBox`, `m_radioBtn`, `m_toggleBtn`
- Text: `m_staticText`, `m_textCtrl`, `m_richText`, `m_scintilla`
- Lists: `m_listBox`, `m_comboBox`, `m_choice`
- Containers: `panel`, `m_splitter`, `m_notebook`
- Bars: `m_toolbar`, `m_menubar`, `m_statusBar`
- Pickers: `m_filePicker`, `m_colourPicker`, `m_datePicker`
- Sizers: (no default var_name for most sizers — only generated if needed)

### Access Control
The `class_access` property controls variable visibility:
- `"protected:"` — Default for most widgets; accessible in derived classes
- `"public:"` — Accessible from outside the class
- `"none"` — No member variable generated (local only)

---

## 12. Key Observations

### Architecture Patterns

1. **Inheritance System**: Widget definitions use `<inherits class="InterfaceName"/>` to compose properties and events from shared interface classes. A widget's complete property set = its own properties + all inherited interface properties.

2. **Interface Classes** (all defined in `interfaces.xml`):
   - `wxWindow` — Common widget properties (id, size, pos, style, etc.)
   - `sizer_child` — Sizer layout properties (alignment, borders, proportion)
   - `sizer_dimension` — Sizer minimum_size
   - `flexgridsizerbase` — Flex grid properties (growable rows/cols)
   - `Window Events` — Focus, keyboard, mouse, general event categories
   - `wxTopLevelWindow` — Top-level window events (activate, close, resize)
   - `Dialog Window Settings` — Dialog-specific settings
   - `Bitmaps` / `Command Bitmaps` — Multi-state bitmap properties
   - `Language Settings` — Empty interface; triggers per-form language settings in code
   - `C++ Settings`, `C++ Header Settings`, `C++ Derived Class Settings`
   - `wxPerl Settings`, `wxPython Settings`, `wxRuby Settings`, `XRC Settings`
   - `Folder * Overrides` — Directory/namespace overrides for folders
   - 8 Validator interfaces: `Text Validator`, `Boolean Validator`, `Integer Validator`, `String Validator`, `Choice Validator`, `List Validator`, `Colour Validator`, `DateTime Validator`, `CheckBoxState Validator`
   - `wxTreeCtrlBase` — Shared tree control events
   - `wxMdiWindow` — MDI window base properties

3. **Definition File Structure** (`src/xml/*.xml`):
   ```xml
   <GeneratorDefinitions>
     <gen class="ClassName" image="icon_name" type="widget_type">
       <inherits class="InterfaceName"/>
       <inherits class="AnotherInterface">
         <property name="prop" type="type">override_default</property>
         <hide name="hidden_prop"/>
       </inherits>
       <property name="prop_name" type="prop_type" help="...">default</property>
       <category name="Category Name">
         <property .../>
         <event .../>
       </category>
       <event name="wxEVT_TYPE" class="wxEventClass" help="..."/>
     </gen>
   </GeneratorDefinitions>
   ```

4. **Gen `type` Values** (determines parent-child validation and code generation behavior):
   - Form types: `form`, `frame_form`, `panel_form`, `wizard`, `wizardpagesimple`, `propsheetform`, `ribbonbar_form`, `toolbar_form`, `aui_toolbar_form`, `menubar_form`, `popup_menu`, `ctx_menu`, `DocViewApp`
   - Container types: `container`, `splitter`, `panel`, `staticbox`
   - Book types: `notebook`, `choicebook`, `listbook`, `simplebook`, `bookpage`, `page`
   - Widget types: `widget`, `dataviewctrl`, `dataviewtreectrl`, `dataviewlistctrl`, `dataviewcolumn`, `dataviewlistcolumn`, `propgrid`, `propgridman`, `propgridpage`, `propgriditem`, `propgrid_category`, `treelistctrl`, `treelistctrlcolumn`
   - Sizer types: `sizer`, `spacer`
   - Bar types: `toolbar`, `aui_toolbar`, `statusbar`, `menubar`, `menu`, `submenu`, `menuitem`, `mdi_menubar`, `doc_menubar`, `ribbonbar`, `ribbonpage`, `ribbonpanel`, `ribbonbuttonbar`, `ribbonbutton`, `ribbontoolbar`, `ribbontool`, `ribbongallery`, `ribbongalleryitem`
   - Tool types: `tool`, `tool_dropdown`, `tool_separator`, `aui_tool`
   - Data/Project types: `project`, `folder`, `sub_folder`, `images`, `embedded_image`, `data`, `data_folder`, `data_string`, `data_xml`
   - MDI/Doc types: `wx_document`, `wx_view`
   - Interface type: `interface` (never instantiated in .wxui files)

### Code Generation Mapping

From the generated C++ examples:
- **Header file** (`.h`): Class declaration with constructor, `Create()` method, protected member variables
- **Source file** (`.cpp`): `Create()` implementation with widget construction, sizer setup, event binding
- **Protected code markers**:
  ```cpp
  // ************* End of generated code ***********
  // DO NOT EDIT THIS COMMENT BLOCK!
  // Code below this comment block will be preserved
  // </auto-generated>
  ```
- **Sizer usage**: `wxSizerFlags(proportion).Expand().Border(wxALL)` pattern
- **Constructor pattern**: Both no-arg constructor and full-parameter `Create()` (two-phase creation)
- **Include guards**: `#pragma once`
- **Lint suppression**: `// clang-format off`, `// NOLINTBEGIN`, `// cppcheck-suppress-begin *`

### Existing AI Documentation Status

The `docs/ai-context/wxui-format/` directory contains a README and 9 numbered doc files (00 through 08). Of these:
- `00-overview.md` and `01-xml-structure.md`: Well-developed with REVIEW:DONE markers
- `02-widgets.md`: Partially populated with button and text widget details
- Files 03-08: Likely contain TODO placeholders (per the README's note about "placeholder content")

### Validator System

Validators connect form controls to member variables for data transfer:
- `validator_variable`: Member variable name
- `validator_data_type`: Variable type (wxString, int, double, wxColour, wxDateTime, etc.)
- `get_function` / `set_function`: Accessor functions
- `validator_style`: Filter flags for text validation (wxFILTER_NONE through wxNUM_VAL_NO_TRAILING_ZEROES)
- `minValue` / `maxValue`: Range for numeric validators
- `precision`: Decimal digits for float validators

### Special Widget: `gen_CustomControl`

Allows embedding user-defined controls:
```xml
<node class="gen_CustomControl"
  class_name="MyCustomWidget"
  construction="new MyCustomWidget(#parent, #id, #pos, #size)"
  parameters="wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size"
  prop_header="mycustomwidget.h"
  custom_mockup="..." />
```

### Platform Conditional Generation

Widgets can be conditionally generated per platform:
```xml
platforms="Windows|Unix"  <!-- Skip Mac -->
disable_language="wxPerl|wxRuby"  <!-- Skip Perl and Ruby generation -->
cpp_conditional="wxUSE_MEDIACTRL"  <!-- C++ #if guard -->
```

### wxStyledTextCtrl Complexity

The `wxStyledTextCtrl` widget has the most extensive property set of any widget:
- 100+ lexer language options
- Line numbering, folding, margin configuration
- 5 configurable margins (0-4) with type, width, mask, sensitivity
- Selection modes, tab/indent settings, wrapping modes
- 30+ events for editor interaction

### Data Version Evolution

The `data_version` attribute on the root element allows format evolution:
- Current stable range: 19–20
- Higher versions may add new properties or change defaults
- wxUiEditor auto-migrates older files on load
- Programmatic version detection: `wxUiEditor.exe --data-version`

---

## Appendix: XML Definition Files

| File | Size | Content |
|------|------|---------|
| `src/xml/interfaces.xml` | 58KB / 908 lines | All shared interfaces (wxWindow, sizer_child, validators, language settings, events) |
| `src/xml/data_ctrls.xml` | 57KB / 894 lines | Data controls (wxGrid, wxTreeCtrl, wxDataView*, wxPropertyGrid*, wxTreeList*) |
| `src/xml/forms.xml` | ~32KB / 1027 lines | All form types (frames, dialogs, panels, wizards) |
| `src/xml/bars.xml` | ~45KB / 813 lines | Toolbars, menus, ribbons, status bars |
| `src/xml/text_ctrls.xml` | ~30KB / 612 lines | Text controls including wxStyledTextCtrl |
| `src/xml/widgets.xml` | ~30KB / 597 lines | General widgets (sliders, gauges, calendar, HTML, WebView, etc.) |
| `src/xml/containers.xml` | ~30KB / 574 lines | Containers and book controls |
| `src/xml/buttons.xml` | ~15KB / 305 lines | All button variants |
| `src/xml/boxes.xml` | ~15KB / 296 lines | Choice/list box controls |
| `src/xml/sizers.xml` | ~15KB / 296 lines | All sizer types |
| `src/xml/pickers.xml` | ~12KB / 224 lines | File, dir, font, colour, date, time pickers |
| `src/xml/project.xml` | ~10KB / 200 lines | Project, folder, images, data, timer |
| `src/xml/mdi.xml` | ~18KB / 350 lines | MDI/Doc-View classes |
| `src/xml/tips.xml` | ~1KB | UI tips (HTML format, not related to .wxui format) |
