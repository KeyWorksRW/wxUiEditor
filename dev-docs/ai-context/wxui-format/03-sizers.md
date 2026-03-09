# Layout Sizers

Comprehensive reference for sizer types, properties, and layout behavior in .wxui format.

## Overview

Sizers are the foundation of wxWidgets layout management. Unlike fixed positioning, sizers automatically arrange child widgets and handle window resizing. Every widget/sizer in a sizer has **sizer child properties** (proportion, alignment, borders, flags) that control its layout behavior.

## Sizer Child Properties

**Every widget or sizer added to a sizer** has these layout properties:

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `proportion` | uint | 0 | How much the item stretches. 0 = fixed size, 1+ = proportional sharing of extra space |
| `alignment` | flags | none | wxALIGN_LEFT, wxALIGN_RIGHT, wxALIGN_TOP, wxALIGN_BOTTOM, wxALIGN_CENTER, wxALIGN_CENTER_HORIZONTAL, wxALIGN_CENTER_VERTICAL |
| `borders` | flags | wxALL | Which sides have borders: wxALL, wxLEFT, wxRIGHT, wxTOP, wxBOTTOM |
| `border_size` | uint | 5 | Border width in pixels (auto-scaled on high DPI if 5/10/15) |
| `scale_border_size` | bool | 1 | Whether to scale border on high DPI |
| `flags` | flags | none | wxEXPAND, wxSHAPED, wxFIXED_MINSIZE, wxRESERVE_SPACE_EVEN_IF_HIDDEN |

**Grid Bag Sizer Additional Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `row` | int | 0 | Grid row position (0-based) |
| `column` | int | 0 | Grid column position (0-based) |
| `rowspan` | int | 1 | Number of rows item spans |
| `colspan` | int | 1 | Number of columns item spans |

**Critical Flag Combinations:**
- `wxEXPAND` - Item fills available space in sizer's orientation
- `wxEXPAND|wxALL` - Expand with borders on all sides
- `wxALIGN_CENTER_VERTICAL|wxALL` - Center vertically with borders
- Combining alignment with wxEXPAND overrides expansion in that direction

## wxBoxSizer

Arranges children in a single row (horizontal) or column (vertical).

```xml
<object class="wxBoxSizer" name="main_sizer">
  <orient>wxVERTICAL</orient>

  <object class="sizeritem">
    <object class="wxStaticText">
      <label>Title</label>
    </object>
    <flag>wxALL</flag>
    <border>10</border>
  </object>

  <object class="sizeritem">
    <object class="wxTextCtrl" name="m_text"/>
    <flag>wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM</flag>
    <border>10</border>
    <option>1</option>  <!-- proportion = 1, will stretch -->
  </object>
</object>
```

| Property | Type | Default | Options |
|----------|------|---------|---------|
| `orientation` | option | wxHORIZONTAL | wxVERTICAL, wxHORIZONTAL |
| `hide_children` | bool | 0 | Hides all children without removing from layout |
| `minimum_size` | wxSize | | Minimum sizer size (width,height) |

**Usage Patterns:**
- Main application layout: wxVERTICAL sizer with proportion to allocate vertical space
- Horizontal button rows: wxHORIZONTAL with proportion=0 for buttons, spacer with proportion=1 for right-alignment
- Most common sizer type - nested wxBoxSizers handle complex layouts

**Code Mapping (C++):**
```cpp
// wxBoxSizer with wxVERTICAL orientation
auto* main_sizer = new wxBoxSizer(wxVERTICAL);

// Add child with proportion=0, wxALL borders, border_size=10
main_sizer->Add(static_text, 0, wxALL, 10);

// Add child with proportion=1 (stretches), specific borders
main_sizer->Add(m_text, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
```

## wxStaticBoxSizer

wxBoxSizer with a labeled border frame around children.

```xml
<object class="wxStaticBoxSizer">
  <orient>wxVERTICAL</orient>
  <label>Group Name</label>
  <object class="sizeritem">
    <object class="wxCheckBox">
      <label>Option 1</label>
    </object>
  </object>
</object>
```

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `label` | string | "label" | Text displayed on the box border |
| `orientation` | option | wxVERTICAL | wxVERTICAL, wxHORIZONTAL |
| `id` | id | wxID_ANY | Control ID for events |
| `disabled` | bool | 0 | Disable all children |
| `hidden` | bool | 0 | Hide box and children |
| `hide_children` | bool | 0 | Hide children only |

**Events:** wxEVT_UPDATE_UI

## wxGridSizer

Arranges children in a fixed grid with equal-sized cells.

```xml
<object class="wxGridSizer">
  <rows>2</rows>
  <cols>3</cols>
  <vgap>5</vgap>
  <hgap>5</hgap>
  <!-- 6 children will fill 2 rows x 3 columns -->
</object>
```

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `rows` | uint | 0 | Number of rows (0 = auto-calculate) |
| `cols` | uint | 2 | Number of columns (0 if rows>0 for auto-calc) |
| `vgap` | uint | 0 | Vertical gap between cells (pixels) |
| `hgap` | uint | 0 | Horizontal gap between cells (pixels) |

**Behavior:**
- All cells same size (largest child determines cell size)
- If `rows=0`, calculates rows = ceil(children / cols)
- If `rows>0` and `cols=0`, calculates cols = ceil(children / rows)
- Children fill left-to-right, top-to-bottom

## wxFlexGridSizer

Grid sizer where row heights and column widths adjust to content.

```xml
<object class="wxFlexGridSizer">
  <rows>0</rows>
  <cols>2</cols>
  <vgap>5</vgap>
  <hgap>5</hgap>
  <growablecols>1</growablecols>  <!-- column 1 grows -->
  <growablerows>0,2</growablerows>  <!-- rows 0 and 2 grow -->
  <flexible_direction>wxBOTH</flexible_direction>
</object>
```

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `rows` | uint | 0 | Number of rows (0 = auto-calculate) |
| `cols` | uint | 2 | Number of columns |
| `vgap` | uint | 0 | Vertical gap between cells |
| `hgap` | uint | 0 | Horizontal gap between cells |
| `growablecols` | string | | Comma-separated column indices that grow (e.g., "0,2") |
| `growablerows` | string | | Comma-separated row indices that grow |
| `flexible_direction` | option | wxBOTH | wxVERTICAL, wxHORIZONTAL, wxBOTH |
| `non_flexible_grow_mode` | option | wxFLEX_GROWMODE_SPECIFIED | wxFLEX_GROWMODE_NONE, wxFLEX_GROWMODE_SPECIFIED, wxFLEX_GROWMODE_ALL |

**Growable Behavior:**
- Only specified rows/columns grow when window resizes
- `flexible_direction` controls which direction can be flexible
- Common for form layouts: labels in column 0 (fixed), inputs in column 1 (growable)

## wxGridBagSizer

Most flexible grid - items can span multiple rows/columns and be positioned precisely.

```xml
<object class="wxGridBagSizer">
  <vgap>5</vgap>
  <hgap>5</hgap>
  <growablecols>1</growablecols>

  <object class="gbsizeritem">
    <object class="wxStaticText">
      <label>Name:</label>
    </object>
    <cellpos>0,0</cellpos>  <!-- row 0, col 0 -->
    <flag>wxALIGN_CENTER_VERTICAL|wxALL</flag>
  </object>

  <object class="gbsizeritem">
    <object class="wxTextCtrl"/>
    <cellpos>0,1</cellpos>  <!-- row 0, col 1 -->
    <cellspan>1,2</cellspan>  <!-- spans 1 row, 2 columns -->
    <flag>wxEXPAND|wxALL</flag>
  </object>
</object>
```

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `vgap` | uint | 0 | Vertical gap between cells |
| `hgap` | uint | 0 | Horizontal gap between cells |
| `growablecols` | string | | Comma-separated growable column indices |
| `growablerows` | string | | Comma-separated growable row indices |
| `flexible_direction` | option | wxBOTH | wxVERTICAL, wxHORIZONTAL, wxBOTH |
| `empty_cell_size` | wxSize | | Size for cells without items |

**Children use `<gbsizeritem>` not `<sizeritem>`:**
- `cellpos` (row,column) - Grid position
- `cellspan` (rowspan,colspan) - How many cells item spans

## wxWrapSizer

Like wxBoxSizer but wraps items to next row/column when space runs out.

```xml
<object class="wxWrapSizer">
  <orient>wxHORIZONTAL</orient>
  <flag>wxEXTEND_LAST_ON_EACH_LINE|wxREMOVE_LEADING_SPACES</flag>
</object>
```

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `orientation` | option | wxHORIZONTAL | Primary direction before wrapping |
| `wrap_flags` | flags | wxEXTEND_LAST_ON_EACH_LINE\|wxREMOVE_LEADING_SPACES | Wrapping behavior |

**Flags:**
- `wxEXTEND_LAST_ON_EACH_LINE` - Last item on line uses remaining space
- `wxREMOVE_LEADING_SPACES` - Remove spacers from line start

## Special Sizer Types

### StaticCheckboxBoxSizer
wxStaticBoxSizer with a checkbox in the label that can enable/disable children.

| Property | Type | Default |
|----------|------|---------|
| `checkbox_var_name` | string | m_checkBox |
| `checked` | bool | 1 |
| `style` | flags | wxALIGN_RIGHT (optional) |

**Events:** wxEVT_CHECKBOX, wxEVT_UPDATE_UI

### StaticRadioBtnBoxSizer
wxStaticBoxSizer with a radio button in the label.

| Property | Type | Default |
|----------|------|---------|
| `radiobtn_var_name` | string | m_radioBtn |
| `checked` | bool | 1 |

**Events:** wxEVT_RADIOBUTTON, wxEVT_UPDATE_UI

### wxStdDialogButtonSizer
Standard dialog buttons (OK, Cancel, Help, etc.) with platform-native ordering.

```xml
<object class="wxStdDialogButtonSizer">
  <object class="button">
    <flag>wxOK</flag>
  </object>
  <object class="button">
    <flag>wxCANCEL</flag>
  </object>
</object>
```

| Property | Type | Description |
|----------|------|-------------|
| `OK`, `Yes`, `Save` | bool | Affirmative button (mutually exclusive) |
| `Cancel`, `Close` | bool | Negative button (mutually exclusive) |
| `Apply`, `No`, `Help`, `ContextHelp` | bool | Additional buttons |
| `default_button` | option | Which button is default (OK, Yes, Save, No, Cancel, Close, none) |
| `static_line` | bool | Add separator line (platform-dependent) |

**Events:** OKButtonClicked, YesButtonClicked, SaveButtonClicked, ApplyButtonClicked, NoButtonClicked, CancelButtonClicked, CloseButtonClicked, HelpButtonClicked, ContextHelpButtonClicked (all wxEVT_BUTTON), plus wxEVT_UPDATE_UI variants

## Spacers

Fixed or stretchy empty space in sizers.

```xml
<object class="spacer">
  <size>10,5</size>  <!-- width=10, height=5 -->
  <flag>wxEXPAND</flag>
</object>

<!-- Stretchy spacer (pushes items apart) -->
<object class="spacer">
  <option>1</option>  <!-- proportion=1 -->
</object>
```

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `width` | uint | 0 | Spacer width (ignored if proportion>0 or vertical sizer) |
| `height` | uint | 0 | Spacer height (ignored if proportion>0 or horizontal sizer) |
| `proportion` | uint | 0 | If >0, stretches proportionally |
| `add_default_border` | bool | 0 | Adds 5px (10px on Windows high DPI) |

**Common Patterns:**
- Fixed spacer: width/height set, proportion=0
- Stretchy spacer: proportion=1, used to push buttons right or align items
- DPI-aware gap: add_default_border=1

## Code to XML Conversion Guide

**Recognizing Sizers in Source Code:**

```cpp
// wxBoxSizer
auto* sizer = new wxBoxSizer(wxVERTICAL);
// → <object class="wxBoxSizer"><orient>wxVERTICAL</orient>

// wxGridSizer
auto* grid = new wxGridSizer(2, 3, 5, 5);  // rows, cols, vgap, hgap
// → <object class="wxGridSizer"><rows>2</rows><cols>3</cols><vgap>5</vgap><hgap>5</hgap>

// wxFlexGridSizer
auto* flex = new wxFlexGridSizer(2, 5, 5);  // cols, vgap, hgap
flex->AddGrowableCol(1);
flex->AddGrowableRow(0);
// → <object class="wxFlexGridSizer"><cols>2</cols><vgap>5</vgap><hgap>5</hgap>
//    <growablecols>1</growablecols><growablerows>0</growablerows>

// Adding children
sizer->Add(widget, proportion, flags, border);
// → <object class="sizeritem">
//     <object class="WidgetType"/>
//     <option>proportion</option>
//     <flag>flags</flag>
//     <border>border</border>
//   </object>

// Common flag patterns
wxEXPAND|wxALL → <flag>wxEXPAND|wxALL</flag>
wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT → <flag>wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT</flag>
```

**Nested Sizers Pattern:**
```cpp
auto* main_sizer = new wxBoxSizer(wxVERTICAL);
auto* button_sizer = new wxBoxSizer(wxHORIZONTAL);
button_sizer->Add(ok_btn, 0, wxALL, 5);
button_sizer->Add(cancel_btn, 0, wxALL, 5);
main_sizer->Add(button_sizer, 0, wxALIGN_RIGHT|wxALL, 10);
```
```xml
<object class="wxBoxSizer" name="main_sizer">
  <orient>wxVERTICAL</orient>
  <object class="sizeritem">
    <object class="wxBoxSizer" name="button_sizer">
      <orient>wxHORIZONTAL</orient>
      <object class="sizeritem">
        <object class="wxButton" name="ok_btn"/>
        <flag>wxALL</flag>
        <border>5</border>
      </object>
      <object class="sizeritem">
        <object class="wxButton" name="cancel_btn"/>
        <flag>wxALL</flag>
        <border>5</border>
      </object>
    </object>
    <flag>wxALIGN_RIGHT|wxALL</flag>
    <border>10</border>
  </object>
</object>
```
