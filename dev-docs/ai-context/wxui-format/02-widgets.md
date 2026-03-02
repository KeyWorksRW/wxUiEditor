# Widget Catalog and Properties

This document provides a comprehensive catalog of widgets available in wxUiEditor with their properties and usage.

## Widget Categories

Widgets are organized into categories:
- **Forms**: Top-level windows (wxFrame, wxDialog, wxPanel)
- **Buttons**: Button controls (wxButton, wxCheckBox, wxRadioButton)
- **Text Controls**: Text input and display (wxTextCtrl, wxStaticText, wxRichTextCtrl)
- **Choice Controls**: Selection widgets (wxChoice, wxComboBox, wxListBox)
- **Data Controls**: Complex data display (wxListCtrl, wxTreeCtrl, wxGrid, wxDataViewCtrl)
- **Containers**: Layout containers (wxPanel, wxScrolledWindow, wxSplitterWindow)
- **Pickers**: File, directory, color, font pickers
- **Static Widgets**: Labels, bitmaps, lines
- **Gauges and Indicators**: Progress indicators, activity indicators
- **Custom**: User-defined custom controls

## Common Widget Properties

All widgets inherit these base properties from wxWindow:

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `var_name` | string | varies | C++ member variable name (e.g., `m_button`) |
| `id` | id | `wxID_ANY` | Widget identifier for events |
| `size` | wxSize | `-1,-1` | Width and height (auto-size if -1) |
| `pos` | wxPoint | `-1,-1` | X and Y position (auto-position if -1) |
| `minimum_size` | wxSize | `-1,-1` | Minimum width and height constraints |
| `maximum_size` | wxSize | `-1,-1` | Maximum width and height constraints |
| `tooltip` | string | (empty) | Tooltip text shown on hover |
| `disabled` | bool | `0` | Initial disabled state |
| `hidden` | bool | `0` | Initial visibility (0=visible, 1=hidden) |
| `foreground_colour` | wxColour | (default) | Text/foreground color |
| `background_colour` | wxColour | (default) | Background color |
| `font` | wxFont | (default) | Font specification |
| `window_style` | bitlist | (varies) | Window style flags |
| `window_extra_style` | bitlist | (empty) | Extended style flags |
| `window_name` | string | (empty) | Window name for resource lookups |
| `variant` | option | `normal` | Size variant (normal/small/mini/large) |
| `context_help` | string | (empty) | Context-sensitive help text |
| `platforms` | bitlist | `Windows\|Unix\|Mac` | Target platforms |
| `disable_language` | bitlist | (empty) | Languages to exclude from generation |

### Window Style Flags (window_style)

Common window style flags:

```
wxBORDER_DEFAULT    - Platform default border
wxBORDER_SIMPLE     - Thin border
wxBORDER_SUNKEN     - Sunken border
wxBORDER_RAISED     - Raised border
wxBORDER_STATIC     - Static control border (Windows)
wxBORDER_THEME      - Native themed border
wxBORDER_NONE       - No border
wxTAB_TRAVERSAL     - Tab key navigation
wxWANTS_CHARS       - Receive all char/key events
wxVSCROLL           - Vertical scrollbar
wxHSCROLL           - Horizontal scrollbar
wxALWAYS_SHOW_SB    - Always show scrollbars
wxCLIP_CHILDREN     - Clip children for flicker reduction
wxFULL_REPAINT_ON_RESIZE - Full repaint on resize
```

## Button Widgets

### wxButton

Standard push button.

```xml
<node class="wxButton" var_name="m_button" label="Click Me" id="wxID_OK">
  <event name="wxEVT_BUTTON">OnButtonClick</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `label` | string | `MyButton` | Button text (leave blank for stock IDs) |
| `default` | bool | `0` | Make this the default button |
| `markup` | bool | `0` | Enable markup for fonts/colors in label |
| `auth_needed` | bool | `0` | Show authentication shield (Windows Vista+) |
| `bitmap` | image | (empty) | Bitmap in default state |
| `disabled_bmp` | image | (empty) | Bitmap when disabled |
| `pressed_bmp` | image | (empty) | Bitmap when pressed |
| `focus_bmp` | image | (empty) | Bitmap when focused |
| `current` | image | (empty) | Bitmap on mouse hover |
| `position` | option | (empty) | Bitmap position (wxLEFT/wxRIGHT/wxTOP/wxBOTTOM) |
| `margins` | wxSize | (default) | Margins between bitmap and text |

**Style Flags:**
```
wxBU_LEFT          - Left-justify label
wxBU_RIGHT         - Right-justify label
wxBU_TOP           - Align label to top
wxBU_BOTTOM        - Align label to bottom
wxBU_EXACTFIT      - Size to exact content (no padding)
wxBU_NOTEXT        - Show bitmap only, no text label
wxBORDER_NONE      - Borderless button
```

**Stock IDs** (automatic labels):
```
wxID_OK, wxID_CANCEL, wxID_YES, wxID_NO, wxID_APPLY, wxID_CLOSE,
wxID_HELP, wxID_SAVE, wxID_OPEN, wxID_DELETE, wxID_PRINT, etc.
```

### wxCheckBox

Checkbox with label.

```xml
<node class="wxCheckBox" var_name="m_checkbox" label="Enable feature" checked="1">
  <event name="wxEVT_CHECKBOX">OnCheckboxToggle</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `label` | string | `My Checkbox` | Checkbox label text |
| `checked` | bool | `0` | Initial checked state |

**Style Flags:**
```
wxALIGN_RIGHT - Align checkbox to right of text (instead of left)
```

### Check3State

Three-state checkbox (checked/unchecked/undetermined).

```xml
<node class="Check3State" var_name="m_check3" label="Option" initial_state="wxCHK_CHECKED" />
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `label` | string | `My Checkbox` | Checkbox label text |
| `initial_state` | option | `wxCHK_UNCHECKED` | Initial state |

**initial_state Options:**
```
wxCHK_UNCHECKED     - Unchecked
wxCHK_CHECKED       - Checked
wxCHK_UNDETERMINED  - Undetermined (third state)
```

**Style Flags:**
```
wxALIGN_RIGHT                 - Align checkbox to right of text
wxCHK_ALLOW_3RD_STATE_FOR_USER - User can set third state by clicking
```

### wxRadioButton

Radio button for mutually exclusive options.

```xml
<node class="wxRadioButton" var_name="m_radio1" label="Option 1" style="wxRB_GROUP" checked="1" />
<node class="wxRadioButton" var_name="m_radio2" label="Option 2" />
<node class="wxRadioButton" var_name="m_radio3" label="Option 3" />
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `label` | string | `My RadioBtn` | Radio button label text |
| `checked` | bool | `0` | Initial checked state |

**Style Flags:**
```
wxRB_GROUP     - Start a new group of radio buttons
wxRB_SINGLE    - Radio button not part of any group
wxALIGN_RIGHT  - Align button to right of text
```

**Usage Notes:**
- First radio button in a group must have `wxRB_GROUP` style
- Only one radio button per group can be checked
- Clicking one radio button unchecks others in the same group

## Text Widgets

### wxStaticText

Static text label (non-editable).

```xml
<node class="wxStaticText" var_name="m_label" label="Enter your name:" />
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `label` | string | `MyLabel` | Text to display |
| `markup` | bool | `0` | Enable simple markup for formatting |
| `wrap` | int | `-1` | Wrap width in pixels (-1 = no wrapping) |

**Style Flags:**
```
wxALIGN_LEFT              - Left-align text (default)
wxALIGN_RIGHT             - Right-align text
wxALIGN_CENTER_HORIZONTAL - Center text horizontally
wxST_NO_AUTORESIZE        - Don't auto-resize on SetLabel()
wxST_ELLIPSIZE_START      - Ellipsis at start if too long
wxST_ELLIPSIZE_MIDDLE     - Ellipsis in middle if too long
wxST_ELLIPSIZE_END        - Ellipsis at end if too long
```

**Markup Example:**
```xml
<node class="wxStaticText" label="This is &lt;b&gt;bold&lt;/b&gt; and this is &lt;span fgcolor='red'&gt;red&lt;/span&gt;" markup="1" />
```

### wxTextCtrl

Single or multi-line text input control.

```xml
<node class="wxTextCtrl" var_name="m_textCtrl" value="Initial text" hint="Enter text here">
  <event name="wxEVT_TEXT">OnTextChanged</event>
  <event name="wxEVT_TEXT_ENTER">OnEnterPressed</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `value` | string | (empty) | Initial text content |
| `hint` | string | (empty) | Hint shown when empty and unfocused |
| `focus` | bool | `0` | Set focus to this control on form creation |
| `prop_maxlength` | string | (empty) | Maximum text length (0 = no limit) |
| `prop_auto_complete` | stringlist | (empty) | Autocomplete suggestions (semicolon-separated) |
| `prop_spellcheck` | bitlist | (empty) | Spell checking options |

**Style Flags:**
```
wxTE_PROCESS_ENTER   - Generate wxEVT_TEXT_ENTER when Enter pressed
wxTE_PROCESS_TAB     - Receive TAB key events
wxTE_MULTILINE       - Multi-line text control
wxTE_PASSWORD        - Password entry (show asterisks)
wxTE_READONLY        - Read-only, not editable
wxTE_RICH            - Rich text control (Windows, >64KB text)
wxTE_RICH2           - Rich text 2.0/3.0 (Windows)
wxTE_AUTO_URL        - Highlight URLs
wxTE_NOHIDESEL       - Always show selection (Windows)
wxTE_NO_VSCROLL      - No vertical scrollbar (multiline)
wxTE_LEFT            - Left-align text (default)
wxTE_CENTER          - Center-align text
wxTE_RIGHT           - Right-align text
wxTE_DONTWRAP        - No wrapping, show horizontal scrollbar
wxTE_CHARWRAP        - Wrap at any character
wxTE_WORDWRAP        - Wrap at word boundaries
wxTE_BESTWRAP        - Smart wrapping (default)
```

**Spell Check Options:**
```
enabled  - Enable spell checking
grammar  - Enable grammar checking
```

### wxRichTextCtrl

Advanced rich text editor with formatting.

```xml
<node class="wxRichTextCtrl" var_name="m_richText" value="Formatted text" minimum_size="400,250" />
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `value` | string | (empty) | Initial text content |
| `hint` | string | (empty) | Hint shown when empty |
| `focus` | bool | `0` | Set focus on form creation |

**Style Flags:**
```
wxRE_CENTRE_CARET - Keep caret centered vertically
wxRE_READONLY     - Read-only mode
```

**Events:** Supports numerous rich text events including character input, delete, style changes, URL clicks, etc.

## Choice Widgets

### wxChoice

Dropdown choice list (view-only selection).

```xml
<node class="wxChoice" var_name="m_choice" contents="Option 1;Option 2;Option 3" selection_int="0">
  <event name="wxEVT_CHOICE">OnChoiceSelected</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `contents` | stringlist | (empty) | Semicolon-separated list of choices |
| `selection_string` | string | (empty) | Initially selected item by text |
| `selection_int` | int | `-1` | Initially selected item by index |
| `focus` | bool | `0` | Set focus on form creation |

**Style Flags:**
```
wxCB_SORT - Sort choices alphabetically
```

### wxComboBox

Combo box with dropdown and optional text entry.

```xml
<node class="wxComboBox" var_name="m_combo" value="Current" contents="Item 1;Item 2;Item 3">
  <event name="wxEVT_COMBOBOX">OnComboSelection</event>
  <event name="wxEVT_TEXT">OnComboTextChanged</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `value` | string | (empty) | Current text/selection |
| `contents` | stringlist | (empty) | Dropdown choices (semicolon-separated) |
| `selection_string` | string | (empty) | Initially selected item |
| `selection_int` | int | `-1` | Initially selected index |
| `hint` | string | (empty) | Hint when empty |
| `focus` | bool | `0` | Set focus on form creation |

**Style Flags:**
```
wxCB_SIMPLE     - Always show list
wxCB_DROPDOWN   - Dropdown list (default)
wxCB_READONLY   - Cannot enter text, only select from list
wxCB_SORT       - Sort items alphabetically
wxTE_PROCESS_ENTER - Generate event on Enter key
```

### wxListBox

List box with single or multiple selection.

```xml
<node class="wxListBox" var_name="m_listBox" contents="Item 1;Item 2;Item 3" selection_int="0">
  <event name="wxEVT_LISTBOX">OnListBoxSelect</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `contents` | stringlist | (empty) | List items (semicolon-separated) |
| `selection_string` | string | (empty) | Initially selected item |
| `selection_int` | int | `-1` | Initially selected index |
| `focus` | bool | `0` | Set focus on form creation |

**Type Options:**
```
wxLB_SINGLE   - Single selection (default)
wxLB_MULTIPLE - Multiple selection (toggle with click)
wxLB_EXTENDED - Extended selection (Shift/Ctrl + click/arrow keys)
```

**Style Flags:**
```
wxLB_HSCROLL    - Horizontal scrollbar if needed (Windows)
wxLB_ALWAYS_SB  - Always show vertical scrollbar
wxLB_NEEDED_SB  - Show vertical scrollbar only when needed
wxLB_NO_SB      - No vertical scrollbar (Windows)
wxLB_SORT       - Sort items alphabetically
```

## Picker Widgets

### wxFilePickerCtrl

File selection with browse button.

```xml
<node class="wxFilePickerCtrl" var_name="m_filePicker" message="Select a file" wildcard="Text files (*.txt)|*.txt">
  <event name="wxEVT_FILEPICKER_CHANGED">OnFileChanged</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `initial_path` | path | (empty) | Initial file path |
| `message` | string | (empty) | Dialog title message |
| `wildcard` | string | (empty) | File filter (e.g., "*.txt\|*.doc") |
| `focus` | bool | `0` | Set focus on form creation |

**Style Flags:**
```
wxFLP_USE_TEXTCTRL        - Show text control for path
wxFLP_OPEN                - Open file dialog
wxFLP_SAVE                - Save file dialog
wxFLP_OVERWRITE_PROMPT    - Confirm before overwriting (with wxFLP_SAVE)
wxFLP_FILE_MUST_EXIST     - Selected file must exist (with wxFLP_OPEN)
wxFLP_CHANGE_DIR          - Change working directory on selection
wxFLP_SMALL               - Small button instead of "Browse"
```

### wxDirPickerCtrl

Directory selection with browse button.

```xml
<node class="wxDirPickerCtrl" var_name="m_dirPicker" message="Select a directory">
  <event name="wxEVT_DIRPICKER_CHANGED">OnDirChanged</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `initial_path` | path | (empty) | Initial directory path |
| `message` | string | (empty) | Dialog title message |
| `focus` | bool | `0` | Set focus on form creation |

**Style Flags:**
```
wxDIRP_DEFAULT_STYLE  - Default (wxDIRP_DIR_MUST_EXIST | wxDIRP_USE_TEXTCTRL)
wxDIRP_USE_TEXTCTRL   - Show text control for path
wxDIRP_DIR_MUST_EXIST - Selected directory must exist
wxDIRP_CHANGE_DIR     - Change working directory on selection
wxDIRP_SMALL          - Small button instead of "Browse"
```

### wxColourPickerCtrl

Color selection with color button.

```xml
<node class="wxColourPickerCtrl" var_name="m_colourPicker" colour="#FF0000">
  <event name="wxEVT_COLOURPICKER_CHANGED">OnColourChanged</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `colour` | wxColour | (default) | Initial color |
| `focus` | bool | `0` | Set focus on form creation |

**Style Flags:**
```
wxCLRP_USE_TEXTCTRL       - Show text control for color value
wxCLRP_SHOW_LABEL         - Show color name/value as label
wxCLRP_DEFAULT_STYLE      - Default style
```

### wxFontPickerCtrl

Font selection with font button.

```xml
<node class="wxFontPickerCtrl" var_name="m_fontPicker" min_point_size="8" max_point_size="72">
  <event name="wxEVT_FONTPICKER_CHANGED">OnFontChanged</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `initial_font` | wxFont | (default) | Initial font |
| `min_point_size` | uint | `0` | Minimum font size in points |
| `max_point_size` | uint | `100` | Maximum font size in points |
| `focus` | bool | `0` | Set focus on form creation |

**Style Flags:**
```
wxFNTP_USE_TEXTCTRL       - Show text control
wxFNTP_FONTDESC_AS_LABEL  - Show font description as label
wxFNTP_USEFONT_FOR_LABEL  - Draw label using selected font
```

## Container Widgets

### wxPanel

General-purpose container panel.

```xml
<node class="wxPanel" var_name="m_panel">
  <node class="wxBoxSizer" orientation="wxVERTICAL">
    <!-- Child widgets -->
  </node>
</node>
```

**Properties:**
- Inherits all wxWindow properties
- Default `window_style`: `wxTAB_TRAVERSAL`

### wxScrolledWindow

Scrollable window container.

```xml
<node class="wxScrolledWindow" var_name="m_scrollPanel" scroll_rate_x="5" scroll_rate_y="5">
  <!-- Child widgets -->
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `scroll_rate_x` | uint | `5` | Horizontal scroll increment |
| `scroll_rate_y` | uint | `5` | Vertical scroll increment |

**Style Flags:**
```
wxHSCROLL - Horizontal scrollbar
wxVSCROLL - Vertical scrollbar
```

### wxSplitterWindow

Window split into two resizable panes.

```xml
<node class="wxSplitterWindow" var_name="m_splitter" splitmode="wxSPLIT_VERTICAL" sashpos="200">
  <node class="wxPanel"><!-- Left/top pane --></node>
  <node class="wxPanel"><!-- Right/bottom pane --></node>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `splitmode` | option | `wxSPLIT_VERTICAL` | Split orientation |
| `sashpos` | int | `0` | Initial sash position |
| `sashsize` | int | `-1` | Sash size (-1 = platform default) |
| `sashgravity` | float | `0.0` | Resize gravity (0.0-1.0) |
| `min_pane_size` | uint | `150` | Minimum pane size |
| `persist_name` | string | (empty) | Name for saving sash position |

**Split Mode Options:**
```
wxSPLIT_VERTICAL   - Vertical split (left/right panes)
wxSPLIT_HORIZONTAL - Horizontal split (top/bottom panes)
```

**Style Flags:**
```
wxSP_3D              - 3D effect border and sash (default)
wxSP_3DSASH          - 3D effect sash only
wxSP_3DBORDER        - Synonym for wxSP_BORDER
wxSP_BORDER          - Standard border
wxSP_NOBORDER        - No border
wxSP_NOSASH          - Invisible sash (prevents resizing)
wxSP_THIN_SASH       - Thin sash
wxSP_NO_XP_THEME     - No Windows XP theming
wxSP_PERMIT_UNSPLIT  - Allow unsplitting
wxSP_LIVE_UPDATE     - Resize immediately (no XOR line)
```

## Custom Control

### gen_CustomControl

Integration point for custom wxWidgets controls.

```xml
<node class="gen_CustomControl"
      class_name="MyCustomControl"
      prop_header="#include \"my_custom.h\""
      parameters="(this, ${id}, ${pos}, ${size}, ${style})">
  <event name="wxEVT_CUSTOM_EVENT">OnCustomEvent</event>
</node>
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `class_name` | string | `CustomClass` | Name of the custom control class |
| `prop_header` | string | (empty) | Header file to include |
| `namespace` | string | (empty) | C++ namespace (C++ only) |
| `construction` | code | (empty) | Custom construction code |
| `parameters` | string | `(this)` | Constructor parameters |
| `settings_code` | code | (empty) | Additional setup code after construction |
| `custom_mockup` | custom_mockup | `wxBitmap;-1;-1` | Mockup panel representation |

**Parameter Macros:**
```
${id}                  - Replaced with id property
${pos}                 - Replaced with pos property
${size}                - Replaced with size property
${window_style}        - Replaced with window_style property
${window_extra_style}  - Replaced with window_extra_style property
${window_name}         - Replaced with window_name property
this                   - Parent window (self in Python)
```

## Next Steps

- [03-sizers.md](03-sizers.md) - Layout management with sizers
- [04-events.md](04-events.md) - Event types and handler signatures
- [05-code-generation.md](05-code-generation.md) - How widgets map to generated code
- [06-examples.md](06-examples.md) - Complete working examples
