# Advanced Features

Advanced .wxui capabilities for complex applications.

## Form Inheritance

.wxui supports deriving forms from other forms for code reuse:

```xml
<!-- Base form: StandardDialog.wxui -->
<object class="wxDialog" name="StandardDialogBase">
  <title>Standard Dialog</title>
  <size>400,300</size>
  <!-- Common UI elements -->
</object>

<!-- Derived form: SpecialDialog.wxui -->
<object class="Project">
  <inherited_class>StandardDialogBase</inherited_class>
  <inherited_file>standard_dialog_base.h</inherited_file>
  <object class="wxDialog" name="SpecialDialogBase">
    <!-- Extends StandardDialogBase -->
  </object>
</object>
```

## Images and Bitmaps

### Embedded Images
```xml
<object class="wxBitmapButton" name="m_logo_btn">
  <bitmap>logo.png</bitmap>  <!-- Relative to .wxui file -->
</object>
```

### Image Lists
Define reusable image collections:
```xml
<object class="Data" name="ImageList">
  <data_list>icon1.png;icon2.png;icon3.png</data_list>
</object>

<object class="wxListCtrl" name="m_list">
  <image_list>ImageList</image_list>
</object>
```

### SVG Support
```xml
<object class="wxBitmapButton">
  <bitmap>scalable_icon.svg</bitmap>
  <svg_size>32,32</svg_size>  <!-- Rasterize to specific size -->
</object>
```

## Menu Bars and Menus

```xml
<object class="wxFrame" name="MainFrameBase">
  <object class="wxMenuBar" name="m_menubar">

    <!-- File Menu -->
    <object class="wxMenu">
      <label>&amp;File</label>

      <object class="wxMenuItem" name="m_new">
        <label>&amp;New\tCtrl+N</label>
        <bitmap>new.png</bitmap>
        <event name="wxEVT_MENU">OnNew</event>
      </object>

      <object class="wxMenuItem" name="m_open">
        <label>&amp;Open...\tCtrl+O</label>
        <event name="wxEVT_MENU">OnOpen</event>
      </object>

      <object class="separator"/>

      <object class="wxMenuItem" name="m_exit">
        <id>wxID_EXIT</id>
        <label>E&amp;xit\tAlt+F4</label>
        <event name="wxEVT_MENU">OnExit</event>
      </object>
    </object>

    <!-- Edit Menu -->
    <object class="wxMenu">
      <label>&amp;Edit</label>

      <object class="wxMenuItem" name="m_undo">
        <id>wxID_UNDO</id>
        <label>&amp;Undo\tCtrl+Z</label>
        <event name="wxEVT_MENU">OnUndo</event>
      </object>

      <!-- Checkable menu item -->
      <object class="wxMenuItem" name="m_word_wrap">
        <label>&amp;Word Wrap</label>
        <checkable>1</checkable>
        <checked>1</checked>
        <event name="wxEVT_MENU">OnWordWrap</event>
      </object>

      <!-- Submenu -->
      <object class="submenu">
        <label>Recent Files</label>
        <object class="wxMenuItem" name="m_recent1">
          <label>file1.txt</label>
        </object>
      </object>
    </object>
  </object>
</object>
```

**Key Features:**
- `&` before letter creates keyboard mnemonic (Alt+F for File)
- `\t` separates label from accelerator (Ctrl+N)
- Standard IDs (wxID_EXIT, wxID_UNDO) provide platform-native icons
- Checkable items toggle state
- Submenus nest wxMenu objects
- Separators divide menu sections

## Toolbars

```xml
<object class="wxFrame">
  <object class="wxToolBar" name="m_toolbar">
    <bitmapsize>24,24</bitmapsize>
    <margins>2,2</margins>
    <separation>5</separation>
    <style>wxTB_FLAT|wxTB_HORIZONTAL|wxTB_TEXT</style>

    <object class="tool" name="m_tool_new">
      <label>New</label>
      <bitmap>new24.png</bitmap>
      <tooltip>Create new file</tooltip>
      <event name="wxEVT_TOOL">OnToolNew</event>
    </object>

    <object class="tool" name="m_tool_open">
      <label>Open</label>
      <bitmap>open24.png</bitmap>
      <event name="wxEVT_TOOL">OnToolOpen</event>
    </object>

    <object class="separator"/>

    <!-- Toggle tool -->
    <object class="tool" name="m_tool_bold">
      <label>Bold</label>
      <bitmap>bold24.png</bitmap>
      <kind>wxITEM_CHECK</kind>
      <event name="wxEVT_TOOL">OnToolBold</event>
    </object>

    <!-- Tool with dropdown menu -->
    <object class="tool" name="m_tool_font">
      <label>Font</label>
      <bitmap>font24.png</bitmap>
      <kind>wxITEM_DROPDOWN</kind>
      <event name="wxEVT_TOOL">OnToolFont</event>
      <event name="wxEVT_TOOL_DROPDOWN">OnToolFontDropdown</event>
    </object>
  </object>
</object>
```

**Toolbar Tool Types:**
- `wxITEM_NORMAL` - Standard clickable tool (default)
- `wxITEM_CHECK` - Toggle tool (stays pressed)
- `wxITEM_RADIO` - Radio tool (one in group pressed)
- `wxITEM_DROPDOWN` - Tool with dropdown menu

## Status Bars

```xml
<object class="wxFrame">
  <object class="wxStatusBar" name="m_statusbar">
    <fields>3</fields>
    <widths>-1,100,100</widths>  <!-- -1 = stretchable -->
    <styles>wxSB_NORMAL,wxSB_RAISED,wxSB_SUNKEN</styles>
  </object>
</object>
```

## Validators

Attach validators for automatic data transfer and validation:

```xml
<object class="wxTextCtrl" name="m_age">
  <validator_type>wxIntegerValidator</validator_type>
  <validator_variable>m_age_value</validator_variable>
  <validator_data_type>int</validator_data_type>
</object>

<object class="wxCheckBox" name="m_enabled">
  <validator_type>wxGenericValidator</validator_type>
  <validator_variable>m_enabled_state</validator_variable>
  <validator_data_type>bool</validator_data_type>
</object>
```

**Common Validators:**
- `wxIntegerValidator` - Integer validation with optional range
- `wxFloatingPointValidator` - Float/double validation
- `wxTextValidator` - Text validation (filters, patterns)
- `wxGenericValidator` - Generic data transfer

## Custom Widgets

```xml
<!-- Register custom widget class -->
<object class="CustomWidget" name="MyCustomControl">
  <class_name>MyCustomControl</class_name>
  <header_file>my_custom_control.h</header_file>
  <base_class>wxWindow</base_class>

  <!-- Custom properties -->
  <custom_property name="data_source" type="string">database.db</custom_property>
  <custom_property name="refresh_rate" type="int">1000</custom_property>
</object>
```

## Data Lists

Define reusable string lists:

```xml
<object class="Data" name="CountryList">
  <data_list>USA;Canada;Mexico;UK;France;Germany</data_list>
</object>

<object class="wxChoice" name="m_country">
  <data_list>CountryList</data_list>
</object>

<object class="wxComboBox" name="m_country2">
  <data_list>CountryList</data_list>
</object>
```

## Build Integration

### CMake Integration

```cmake
# Find wxUiEditor executable
find_program(WXUIEDITOR_EXECUTABLE
    NAMES wxUiEditor wxuieditor
    PATHS "C:/Program Files/wxUiEditor" "/usr/local/bin"
)

if(WXUIEDITOR_EXECUTABLE)
    # Custom target to regenerate code from .wxui files
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/generated/main_frame_base.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/generated/main_frame_base.h
        COMMAND ${WXUIEDITOR_EXECUTABLE}
                --generate ${CMAKE_CURRENT_SOURCE_DIR}/ui/main_frame.wxui
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ui/main_frame.wxui
        COMMENT "Generating UI code from main_frame.wxui"
    )

    add_custom_target(generate_ui ALL
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/generated/main_frame_base.cpp
    )
endif()

add_executable(MyApp
    src/main.cpp
    src/main_frame.cpp
    generated/main_frame_base.cpp
)

add_dependencies(MyApp generate_ui)
```

### Recommended Project Structure

```
MyProject/
├── ui/
│   ├── main_frame.wxui
│   ├── settings_dialog.wxui
│   └── about_dialog.wxui
├── generated/           # Auto-generated base classes
│   ├── main_frame_base.cpp
│   ├── main_frame_base.h
│   ├── settings_dialog_base.cpp
│   └── settings_dialog_base.h
├── src/                 # User implementation
│   ├── main.cpp
│   ├── main_frame.cpp
│   ├── main_frame.h
│   ├── settings_dialog.cpp
│   └── settings_dialog.h
├── resources/
│   └── images/
│       ├── icon.png
│       └── logo.svg
└── CMakeLists.txt
```

### Code Regeneration Workflow

1. Edit .wxui file in wxUiEditor
2. Save (triggers auto-generation of base classes)
3. Build system detects changed generated files
4. Recompiles affected translation units

**Manual regeneration:**
```bash
wxUiEditor --generate ui/main_frame.wxui
```

**Batch generation:**
```bash
for file in ui/*.wxui; do
    wxUiEditor --generate "$file"
done
```

## Multi-Language Projects

Generate code for multiple languages from single .wxui:

```xml
<object class="Project">
  <code_preference>C++</code_preference>
  <python_pref>python3</python_pref>
  <ruby_pref>wxRuby3</ruby_pref>

  <!-- Paths for each language -->
  <file_cpp>generated/dialog_base</file_cpp>
  <file_python>generated/dialog_base</file_python>
  <file_ruby>generated/dialog_base</file_ruby>
  <file_perl>generated/DialogBase</file_perl>
</object>
```

Generate all:
```bash
wxUiEditor --generate_all dialog.wxui
```
