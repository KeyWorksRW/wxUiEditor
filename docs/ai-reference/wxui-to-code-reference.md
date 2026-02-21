# .wxui XML → wxWidgets Code: Real-World Reference

Extracted from wxUiEditor's own `src/wxui/wxUiEditor.wxui` and its generated C++ output.

---

## 1. Project Root Structure

Every `.wxui` file starts with:

```xml
<?xml version="1.0"?>
<wxUiEditorData data_version="19">
  <node
    class="Project"
    art_directory="../art_src"
    generate_languages="C++"
    optional_comments="1"
    cpp_line_length="125"
    generate_cmake="1">

    <!-- Optional: Images node for embedded resources -->
    <node class="Images" add_externs="1" auto_add="1" base_file="ui_images">
      <node class="embedded_image" bitmap="SVG;wxUiEditor.svg;[64,64]" />
      <node class="embedded_image" bitmap="Embed;default.png" />
    </node>

    <!-- Optional: Data node for XML data files -->
    <node class="Data">
      <node class="data_xml" data_file="../xml/forms.xml" var_name="forms" />
    </node>

    <!-- Forms organized in folders -->
    <node class="folder" label="Main UI">
      <!-- wxFrame, wxDialog, PanelForm definitions here -->
    </node>
  </node>
</wxUiEditorData>
```

**Key rule**: Only properties differing from defaults are stored. Default values are omitted.

---

## 2. Form Examples from wxUiEditor.wxui

### 2a. wxFrame with Toolbar, Splitter, MenuBar (MainFrameBase)

```xml
<node
  class="wxFrame"
  class_name="MainFrameBase"
  base_file="mainframe_base"
  generate_ids="0"
  initial_enum_string="START_MAINFRAME_IDS"
  local_src_includes="mainframe.h;project_handler.h"
  derived_class_name="MainFrame"
  minimum_size="800,800"
  size="1000,1000"
  window_style="wxTAB_TRAVERSAL"
  wxEVT_CLOSE_WINDOW="OnClose">

  <!-- Top-level sizer -->
  <node class="wxBoxSizer" class_access="protected:" orientation="wxVERTICAL"
        var_name="m_mainframe_sizer">

    <!-- Toolbar -->
    <node class="wxToolBar" style="wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER"
          var_name="m_toolbar" borders="" flags="wxEXPAND">
      <node class="toolSeparator" />
      <node class="tool" bitmap="SVG;wxUiEditor.svg;[24,24]"
            id="id_DifferentProject" label="Different Project..."
            tooltip="Different Project... (Ctrl+D)" var_name="tool4" />
      <node class="tool" bitmap="SVG;save.svg;[24,24]"
            id="wxID_SAVE" label="Save" tooltip="Save current project" var_name="tool3" />
      <node class="tool" bitmap="SVG;generate.svg;[24,24]"
            id="id_GenerateCode" label="Generate..." tooltip="Generate code..."
            var_name="Generate" wxEVT_TOOL="OnGenerateCode" />
      <node class="toolSeparator" />
      <!-- Check-style tools -->
      <node class="tool" bitmap="SVG;alignleft.svg;[24,24]"
            id="id_AlignLeft" kind="wxITEM_CHECK" label=""
            tooltip="Align left" var_name="AlignLeft" />
    </node>

    <!-- Splitter Window -->
    <node class="wxSplitterWindow" min_pane_size="2" style="wxSP_LIVE_UPDATE"
          var_name="m_MainSplitter" borders="" flags="wxEXPAND" proportion="1">
      <!-- Left panel (with subclass) -->
      <node class="wxPanel" class_access="protected:" var_name="m_nav_panel"
            subclass="NavigationPanel" subclass_header="../panels/nav_panel.h"
            window_style="0" flags="wxEXPAND" />
      <!-- Right panel with child sizer -->
      <node class="wxPanel" class_access="protected:" var_name="m_panel_right"
            window_style="0" flags="wxEXPAND">
        <node class="wxBoxSizer" class_access="protected:" orientation="wxVERTICAL"
              var_name="m_right_panel_sizer" />
      </node>
    </node>
  </node>

  <!-- MenuBar (sibling of the sizer, child of the frame) -->
  <node class="wxMenuBar">
    <node class="wxMenu" label="&amp;File" var_name="m_menuFile">
      <node class="wxMenuItem" bitmap="SVG;wxUiEditor.svg;[16,16]"
            help="Open a project" id="id_DifferentProject"
            label="&amp;Different Project..." shortcut="Ctrl+D"
            var_name="menuItem3" wxEVT_MENU="OnDifferentProject" />
      <node class="submenu" class_access="protected:" label="Open &amp;Recent"
            var_name="m_submenu_recent" />
      <node class="wxMenuItem" bitmap="SVG;import.svg;[16,16]"
            label="&amp;Import Project..." var_name="menu_import"
            wxEVT_MENU="[](wxCommandEvent&amp;)@@{@@Project.NewProject(false);@@}" />
      <node class="separator" />
      <node class="wxMenuItem" id="wxID_SAVE" label="&amp;Save"
            shortcut="Ctrl+S" help="Save current project" />
    </node>
    <node class="wxMenu" label="&amp;Edit" var_name="m_menuEdit">
      <!-- Submenu for Move commands -->
      <node class="submenu" label="Move">
        <node class="wxMenuItem" id="id_MoveUp" label="Up" shortcut="Alt+Up"
              help="Moves selected item up"
              wxEVT_MENU="[](wxCommandEvent&amp;)@@{@@wxGetFrame().MoveNode(MoveDirection::Up);@@}" />
      </node>
    </node>
  </node>
</node>
```

**Generated C++ patterns** (mainframe_base.h):
```cpp
class MainFrameBase : public wxFrame
{
public:
    MainFrameBase() = default;
    MainFrameBase(wxWindow* parent, wxWindowID id = wxID_ANY, ...);
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY, ...);

protected:
    // Virtual event handlers -- override them in your derived class
    virtual void OnClose(wxCloseEvent& event) { event.Skip(); }
    virtual void OnDifferentProject(wxCommandEvent& event) { event.Skip(); }
    // ... more virtual handlers ...

    // Class member variables
    NavigationPanel* m_nav_panel;
    wxBoxSizer* m_mainframe_sizer;
    wxMenu* m_menuFile;
    wxMenuBar* m_menubar;
    wxSplitterWindow* m_MainSplitter;
    wxToolBar* m_toolbar;
};
```

### 2b. Simple Dialog with StdDialogButtonSizer (GenerateDlg)

```xml
<node
  class="wxDialog"
  class_name="GenerateDlg"
  title="Generate Code"
  base_file="..\tools\generate_dlg"
  use_derived_class="0"
  wxEVT_INIT_DIALOG="OnInit">
  <node class="wxBoxSizer" orientation="wxVERTICAL" var_name="dlg_sizer">
    <node class="wxStaticText" label="Choose the code you want to generate:" />
    <node class="wxGridSizer" class_access="protected:" var_name="m_grid_sizer" />
    <node class="wxStdDialogButtonSizer" flags="wxEXPAND" />
  </node>
</node>
```

### 2c. Dialog with Splitter, StyledTextCtrl, Events (EditHtmlDialogBase)

```xml
<node
  class="wxDialog"
  class_name="EditHtmlDialogBase"
  persist="1"
  style="wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER"
  size="1000,1000"
  base_file="edit_html_dialog_base"
  derived_class_name="EditHtmlDialog"
  wxEVT_INIT_DIALOG="OnInit">
  <node class="wxBoxSizer" orientation="wxVERTICAL" var_name="parent_sizer">
    <node class="wxSplitterWindow" min_pane_size="250"
          flags="wxEXPAND" proportion="1">
      <node class="wxStyledTextCtrl"
            additional_carets_blink="1" indentation_guides="forward"
            lexer="HTML" use_tabs="0" wrap_mode="word"
            font="large" flags="wxEXPAND" proportion="1"
            wxEVT_STC_CHANGE="OnTextChange" />
      <node class="wxHtmlWindow" />
    </node>
    <node class="wxStdDialogButtonSizer" var_name="stdBtn_2"
          flags="wxEXPAND" OKButtonClicked="OnOK" />
  </node>
</node>
```

**Generated C++ (edit_html_dialog_base.cpp)**:
```cpp
bool EditHtmlDialogBase::Create(wxWindow* parent, wxWindowID id, ...) {
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    auto* parent_sizer = new wxBoxSizer(wxVERTICAL);

    m_splitter = new wxSplitterWindow(this, wxID_ANY, ...wxSP_3D);
    m_splitter->SetSashGravity(0.0);
    m_splitter->SetMinimumPaneSize(250);
    parent_sizer->Add(m_splitter, wxSizerFlags(1).Expand().Border(wxALL));

    m_scintilla = new wxStyledTextCtrl(m_splitter);
    { /* scintilla setup block */ }

    m_htmlWin = new wxHtmlWindow(m_splitter);
    m_splitter->SplitVertically(m_scintilla, m_htmlWin);

    auto* stdBtn_2 = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    parent_sizer->Add(CreateSeparatedSizer(stdBtn_2), wxSizerFlags().Expand().Border(wxALL));

    // ... size handling code ...
    Centre(wxBOTH);
    wxPersistentRegisterAndRestore(this, "EditHtmlDialogBase");

    // Event handlers
    Bind(wxEVT_BUTTON, &EditHtmlDialogBase::OnOK, this, wxID_OK);
    Bind(wxEVT_INIT_DIALOG, &EditHtmlDialogBase::OnInit, this);
    m_scintilla->Bind(wxEVT_STC_CHANGE, &EditHtmlDialogBase::OnTextChange, this);

    return true;
}
```

### 2d. Dialog with TextCtrl, Validator, Hidden Control (EditStringDialogBase)

```xml
<node
  class="wxDialog"
  class_name="EditStringDialogBase"
  style="wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER"
  base_file="editstringdialog_base"
  derived_class_name="EditStringDialog">
  <node class="wxBoxSizer" orientation="wxVERTICAL" var_name="parent_sizer">
    <node class="wxStaticText" label="" var_name="m_static_hdr_text"
          hidden="1" border_size="15" borders="wxTOP|wxRIGHT|wxLEFT" flags="wxEXPAND" />
    <node class="wxTextCtrl" focus="1"
          get_function="GetResults" validator_variable="m_value"
          minimum_size="500,-1" border_size="15" flags="wxEXPAND" />
    <node class="spacer" proportion="1" />
    <node class="wxStdDialogButtonSizer" var_name="stdBtn_2" flags="wxEXPAND" />
  </node>
</node>
```

**Generated C++**:
```cpp
m_static_hdr_text = new wxStaticText(this, wxID_ANY, wxEmptyString);
m_static_hdr_text->Hide();
parent_sizer->Add(m_static_hdr_text, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxTOP, 15));

m_textCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
m_textCtrl->SetValidator(wxTextValidator(wxFILTER_NONE, &m_value));
m_textCtrl->SetMinSize(FromDIP(wxSize(500, -1)));
parent_sizer->Add(m_textCtrl, wxSizerFlags().Expand().TripleBorder(wxALL));

parent_sizer->AddStretchSpacer(1);

auto* stdBtn_2 = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
parent_sizer->Add(CreateSeparatedSizer(stdBtn_2), wxSizerFlags().Expand().Border(wxALL));
```

### 2e. Dialog with Notebook (EventHandlerDlgBase)

```xml
<node
  class="wxDialog"
  class_name="EventHandlerDlgBase"
  persist="1"
  style="wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER"
  title="Event Handler"
  base_file="eventhandler_dlg_base"
  derived_class_name="EventHandlerDlg"
  wxEVT_INIT_DIALOG="OnInit">
  <node class="wxBoxSizer" orientation="wxVERTICAL" var_name="parent_sizer"
        flags="wxEXPAND">
    <node class="wxBoxSizer" orientation="wxVERTICAL" flags="wxEXPAND" proportion="1">
      <node class="wxStaticText" label="..." var_name="m_static_bind_text" wrap="400" />
      <node class="wxNotebook" flags="wxEXPAND"
            wxEVT_NOTEBOOK_PAGE_CHANGED="OnPageChanged">
        <!-- Notebook pages use BookPage class -->
        <node class="BookPage" bitmap="SVG;cpp_logo.svg;[24,24]"
              class_access="protected:" label="C++"
              var_name="m_cpp_bookpage"
              background_colour="wxSYS_COLOUR_BTNFACE"
              window_style="wxTAB_TRAVERSAL">
          <node class="wxBoxSizer" orientation="wxVERTICAL" var_name="page_sizer">
            <!-- StaticRadioBtnBoxSizer = radio button + static box -->
            <node class="StaticRadioBtnBoxSizer" checked="0"
                  class_access="protected:" label="Use function"
                  radiobtn_var_name="m_cpp_radio_use_function"
                  var_name="m_cpp_function_box" flags="wxEXPAND"
                  wxEVT_RADIOBUTTON="OnUseCppFunction">
              <node class="wxBoxSizer" var_name="box_sizer7" flags="wxEXPAND">
                <node class="wxTextCtrl" var_name="m_cpp_text_function"
                      flags="wxEXPAND" proportion="1"
                      wxEVT_TEXT="OnChange" />
                <node class="wxButton" class_access="none" label="Default"
                      var_name="btn" wxEVT_BUTTON="OnDefault" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
</node>
```

### 2f. PanelForm (CodeDisplayBase)

```xml
<node
  class="PanelForm"
  class_name="CodeDisplayBase"
  base_file="codedisplay_base"
  derived_class_name="CodeDisplay">
  <node class="wxBoxSizer" orientation="wxVERTICAL" var_name="parent_sizer">
    <node class="wxStyledTextCtrl"
          indentation_guides="real" read_only="1" symbol_margin="0"
          use_tabs="0" wrap_indent_mode="indent" wrap_mode="whitespace"
          wrap_visual_flag="end" wrap_visual_location="end_text"
          flags="wxEXPAND" proportion="1" />
  </node>
</node>
```

**Generated C++ (PanelForm = wxPanel)**:
```cpp
class CodeDisplayBase : public wxPanel
{
public:
    // ...
    bool Create(wxWindow* parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, long style, const wxString& name);
protected:
    wxStyledTextCtrl* m_scintilla;
};
```

### 2g. PanelForm with Toolbar (DocViewPanel)

```xml
<node
  class="PanelForm"
  class_name="DocViewPanel"
  window_style="0"
  base_file="..\panels\doc_view.cpp"
  private_members="1"
  use_derived_class="0">
  <node class="wxBoxSizer" class_access="protected:" orientation="wxVERTICAL"
        var_name="m_parent_sizer">
    <node class="wxToolBar" style="wxTB_HORIZONTAL|wxTB_TEXT" flags="wxEXPAND">
      <node class="tool" bitmap="Art;wxART_GO_HOME|wxART_TOOLBAR" id="wxID_HOME"
            label="Home" var_name="tool_6" wxEVT_TOOL="OnHome" />
      <node class="tool" bitmap="Art;wxART_GO_BACK|wxART_TOOLBAR" id="wxID_BACKWARD"
            label="Back" var_name="tool_3" wxEVT_TOOL="OnBack"
            wxEVT_UPDATE_UI="OnUpdateBack" />
      <node class="toolSeparator" />
      <node class="tool" bitmap="SVG;cpp_logo.svg;[16,16]" id="ID_CPLUS"
            kind="wxITEM_RADIO" label="C++" wxEVT_TOOL="OnCPlus" />
    </node>
  </node>
</node>
```

### 2h. wxFrame with MenuBar, Toolbar (MsgFrameBase)

```xml
<node
  class="wxFrame"
  class_name="MsgFrameBase"
  title="wxUiEditor Messages"
  base_file="..\internal\msgframe_base"
  derived_class_name="MsgFrame"
  size="-1,-1"
  window_style="wxTAB_TRAVERSAL"
  wxEVT_CLOSE_WINDOW="OnClose">
  <node class="wxMenuBar" class_access="none" var_name="menubar">
    <node class="wxMenu" class_access="none" label="&amp;File" var_name="menu_file">
      <node class="wxMenuItem" bitmap="Art;wxART_FILE_SAVE_AS|wxART_MENU"
            id="wxID_SAVEAS" label="" var_name="menu_item_saveas"
            wxEVT_MENU="OnSaveAs" />
      <node class="separator" />
      <node class="wxMenuItem" bitmap="SVG;hide.svg;[24,24]" id="id_hide"
            label="&amp;Hide" var_name="menu_item_hide" wxEVT_MENU="OnHide" />
    </node>
    <node class="wxMenu" class_access="none" label="&amp;View" var_name="menu_view">
      <node class="wxMenuItem" bitmap="Art;wxART_WARNING|wxART_MENU"
            class_access="protected:" id="id_warning_msgs" kind="wxITEM_CHECK"
            label="Warnings" var_name="m_menu_item_warnings"
            wxEVT_MENU="OnWarnings" />
    </node>
  </node>
  <node class="wxToolBar" var_name="m_tool_bar">
    <node class="tool" bitmap="SVG;saveas.svg;[24,24]" id="wxID_SAVEAS"
          label="" var_name="tool_saveas" wxEVT_TOOL="OnSaveAs" />
    <node class="toolSeparator" />
  </node>
</node>
```

### 2i. Dialog with ColourPicker, Custom Controls, Spacers (ColourPropBase)

```xml
<node
  class="wxDialog"
  class_name="ColourPropBase"
  title="Colour Property"
  base_file="colourprop_base"
  derived_class_name="ColourPropDialog"
  wxEVT_INIT_DIALOG="OnInit">
  <node class="wxBoxSizer" orientation="wxVERTICAL" var_name="dlg_sizer" flags="wxEXPAND">
    <node class="wxBoxSizer" orientation="wxVERTICAL" flags="wxEXPAND">
      <node class="wxBoxSizer" alignment="wxALIGN_CENTER_HORIZONTAL">
        <node class="CustomControl" class_name="ColourRectCtrl"
              header="../custom_ctrls/colour_rect_ctrl.h"
              namespace="wxue_ctrl" var_name="m_colour_rect"
              maximum_size="64,80" />
        <node class="wxStaticText" label="Sample Text"
              var_name="m_static_sample_text"
              alignment="wxALIGN_CENTER_VERTICAL" />
      </node>
    </node>
    <node class="spacer" add_default_border="1" height="5" />
    <node class="wxRadioButton" checked="1" label="Let wxWidgets choose the colour"
          var_name="m_radio_default" border_size="10"
          wxEVT_RADIOBUTTON="OnSetDefault" />
    <node class="spacer" add_default_border="1" height="5" />
    <node class="StaticRadioBtnBoxSizer" checked="0" class_access="protected:"
          disabled="1" label="Custom Colour"
          radiobtn_var_name="m_radio_custom" var_name="m_staticbox_custom"
          flags="wxEXPAND" wxEVT_RADIOBUTTON="OnRadioCustomColour">
      <node class="wxColourPickerCtrl" style="wxCLRP_USE_TEXTCTRL|wxCLRP_SHOW_LABEL"
            disabled="1" subclass="kwColourPickerCtrl"
            subclass_header="../custom_ctrls/kw_color_picker.h"
            window_style="wxWANTS_CHARS"
            wxEVT_COLOURPICKER_CHANGED="OnColourChanged" />
    </node>
    <node class="spacer" add_default_border="1" height="5" />
    <node class="wxStdDialogButtonSizer" flags="wxEXPAND" OKButtonClicked="OnOK" />
  </node>
</node>
```

### 2j. PanelForm with wxRibbonBar (RibbonPanelBase)

```xml
<node
  class="PanelForm"
  class_name="RibbonPanelBase"
  mockup_size="700,100"
  size="-1,-1"
  base_file="ribbonpanel_base"
  generate_ids="0"
  initial_enum_string="START_RIBBON_IDS"
  source_preamble="#include &quot;ribbon_ids.h&quot;@@..."
  derived_class_name="RibbonPanel">
  <node class="wxBoxSizer" class_access="protected:" orientation="wxVERTICAL"
        var_name="parent_sizer">
    <node class="wxRibbonBar" style="wxRIBBON_BAR_SHOW_PAGE_LABELS"
          theme="MSW" flags="wxEXPAND">
      <node class="wxRibbonPage" label="Forms" var_name="page_forms">
        <node class="wxRibbonPanel" label="Windows" var_name="panel_form_windows">
          <node class="wxRibbonToolBar" var_name="forms_bar_windows"
                wxEVT_RIBBONTOOLBAR_CLICKED="OnToolClick">
            <node class="ribbonTool" bitmap="SVG;wxDialog.svg;[24,24]"
                  help="wxDialog" id="CreateNewDialog" />
            <node class="ribbonTool" bitmap="SVG;wxFrame.svg;[24,24]"
                  help="wxFrame" id="CreateNewFrame" />
          </node>
        </node>
      </node>
    </node>
  </node>
</node>
```

### 2k. StartupDlg — Complex Dialog with FlexGridSizer, Hyperlinks, Icons

```xml
<node
  class="wxDialog"
  center="no"
  class_name="StartupDlgBase"
  icon="SVG;wxUiEditor.svg;[16,16]"
  title="Open, Import, or Create Project"
  base_file="..\wxui\startup_dlg_base"
  derived_class_name="StartupDlg"
  private_members="1"
  pure_virtual_functions="1"
  wxEVT_INIT_DIALOG="OnInit">
  <node class="wxBoxSizer" orientation="wxVERTICAL" var_name="dlg_sizer" flags="wxEXPAND">
    <!-- Header row with icon and title -->
    <node class="wxBoxSizer" var_name="box_sizer_6" flags="wxEXPAND" proportion="1">
      <node class="wxStaticBitmap" bitmap="SVG;wxUiEditor.svg;[64,64]" var_name="bmp_4" />
      <node class="spacer" add_default_border="1" width="10" />
      <node class="wxStaticText" label="wxUiEditor" var_name="m_name_version"
            font="extra large" alignment="wxALIGN_CENTER_VERTICAL" proportion="1" />
    </node>
    <!-- Two-column layout -->
    <node class="wxBoxSizer" var_name="box_sizer_8">
      <node class="wxBoxSizer" orientation="wxVERTICAL">
        <node class="wxStaticText" class_access="none" label="Recent"
              var_name="staticText" font="extra large,,semi-bold" />
        <node class="wxStaticText" label="Recent projects..." var_name="m_staticTextRecentProjects"
              wrap="200" />
        <node class="wxFlexGridSizer" class_access="protected:" var_name="m_recent_flex_grid"
              flexible_direction="wxHORIZONTAL" growablecols="1:1" />
      </node>
      <node class="wxStaticLine" style="wxLI_VERTICAL" border_size="20" flags="wxEXPAND" />
      <node class="wxBoxSizer" orientation="wxVERTICAL" border_size="15"
            borders="wxRIGHT|wxLEFT">
        <node class="wxStaticText" class_access="none" label="Start"
              font="extra large,,semi-bold" />
        <!-- Hyperlinks with icons -->
      </node>
    </node>
  </node>
</node>
```

---

## 3. Code Generation Patterns Observed

### 3a. Header File (.h) Pattern

```cpp
// Auto-generation comment block
// clang-format off, NOLINTBEGIN, cppcheck-suppress-begin

#pragma once
#include <wx/...>       // System wx headers

// Forward declarations
class NavigationPanel;

// Optional: extern declarations for embedded images
namespace wxue_img { extern const unsigned char svg[...]; }

class MyDialogBase : public wxDialog   // or wxFrame, wxPanel
{
public:
    MyDialogBase() {}                   // wxDialog/wxPanel: empty braces
    // OR: MainFrameBase() = default;   // wxFrame: = default

    // Constructor delegates to Create()
    MyDialogBase(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& title = "...", ...) { Create(...); }

    bool Create(wxWindow* parent, ...);

protected:
    // Virtual event handlers (default impl calls event.Skip())
    virtual void OnInit(wxInitDialogEvent& event) { event.Skip(); }
    virtual void OnOK(wxCommandEvent& event) { event.Skip(); }

    // Pure virtual when pure_virtual_functions="1"
    virtual void OnImport(wxHyperlinkEvent& event) = 0;

    // Class member variables (alphabetically sorted)
    wxButton* m_okButton;
    wxTextCtrl* m_textCtrl;
};

// End marker block
// ************* End of generated code ***********
// Code below this comment block will be preserved
```

### 3b. Source File (.cpp) Pattern

```cpp
// Auto-generation comment block
#include <wx/...>       // System wx headers (only what's needed)
#include "base_class.h" // Own header

bool MyClass::Create(wxWindow* parent, ...) {
    // 1. Create base window
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    // 2. Create top-level sizer
    auto* parent_sizer = new wxBoxSizer(wxVERTICAL);

    // 3. Create child widgets (parent = this for top-level, or another widget)
    m_textCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    parent_sizer->Add(m_textCtrl, wxSizerFlags().Expand().Border(wxALL));

    // 4. StdDialogButtonSizer
    auto* stdBtn = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    parent_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    // 5. Size handling (DPI-aware)
    if (size == wxDefaultSize)
        SetSizerAndFit(parent_sizer);
    else {
        SetSizer(parent_sizer);
        SetSize(FromDIP(size));
        Layout();
    }

    // 6. Centering (unless center="no")
    Centre(wxBOTH);

    // 7. Persistence (when persist="1")
    wxPersistentRegisterAndRestore(this, "ClassName");

    // 8. Event bindings
    Bind(wxEVT_BUTTON, &MyClass::OnOK, this, wxID_OK);
    Bind(wxEVT_INIT_DIALOG, &MyClass::OnInit, this);
    m_textCtrl->Bind(wxEVT_TEXT, &MyClass::OnTextChanged, this);

    return true;
}
```

### 3c. Sizer Add Patterns

XML attributes → `wxSizerFlags` calls:

| XML Attribute | wxSizerFlags Call |
|---------------|-------------------|
| `proportion="1"` | `wxSizerFlags(1)` |
| `flags="wxEXPAND"` | `.Expand()` |
| `borders="wxALL"` + `border_size="5"` | `.Border(wxALL)` (default=5) |
| `borders="wxALL"` + `border_size="15"` | `.TripleBorder(wxALL)` or `.Border(wxALL, 15)` |
| `borders=""` (empty) | no `.Border()` call |
| `alignment="wxALIGN_CENTER_VERTICAL"` | `.Center()` |
| `borders="wxTOP\|wxRIGHT\|wxLEFT"` + `border_size="15"` | `.Border(wxLEFT\|wxRIGHT\|wxTOP, 15)` |

Spacers: `<node class="spacer" proportion="1" />` → `parent_sizer->AddStretchSpacer(1);`

### 3d. Event Binding Patterns

**Named function handler** (most common):
```xml
wxEVT_BUTTON="OnMyClick"
```
→ `Bind(wxEVT_BUTTON, &MyClass::OnMyClick, this, controlId);`

**Lambda handler** (inline code):
```xml
wxEVT_MENU="[](wxCommandEvent&amp;)@@{@@Project.NewProject(true);@@}"
```
→ `Bind(wxEVT_MENU, [](wxCommandEvent&) { Project.NewProject(true); }, id);`

**StdDialogButtonSizer events**:
```xml
<node class="wxStdDialogButtonSizer" OKButtonClicked="OnOK" />
```
→ `Bind(wxEVT_BUTTON, &MyClass::OnOK, this, wxID_OK);`

**Window-level events** (on the form node directly):
```xml
<node class="wxDialog" wxEVT_INIT_DIALOG="OnInit" wxEVT_CLOSE_WINDOW="OnClose">
```
→ `Bind(wxEVT_INIT_DIALOG, &MyClass::OnInit, this);`

### 3e. class_access Attribute

Controls member visibility in generated class:

| Value | Effect |
|-------|--------|
| (default) `protected:` | Normal protected member variable |
| `protected:` | Explicit protected member |
| `none` | Local variable only (not a class member) |
| (setter available for `public:`) | Public member |

---

## 4. Parent-Child Containment Rules

### Top-Level (Children of Project)
- `wxFrame`, `wxDialog`, `PanelForm`, `wxWizard`, `wxPropertySheetDialog`
- `folder` (organizational only, contains forms)
- `Images` (embedded resources)
- `Data` (data files)

### wxFrame Children
- Exactly ONE sizer (usually `wxBoxSizer`)
- `wxMenuBar` (optional, sibling of the sizer)
- `wxToolBar` (can be inside the sizer, or child of frame)
- `wxStatusBar` (optional)

### wxDialog Children
- Exactly ONE sizer (usually `wxBoxSizer`)

### PanelForm Children
- Same as `wxPanel` — exactly ONE sizer

### Sizer Children
- Widgets (`wxButton`, `wxTextCtrl`, `wxStaticText`, etc.)
- Nested sizers (`wxBoxSizer`, `wxFlexGridSizer`, etc.)
- `spacer` nodes
- Container widgets (`wxSplitterWindow`, `wxNotebook`, etc.)

### wxNotebook / Book Controls
- `BookPage` nodes (each wraps a `wxPanel`-like page)
- BookPage contains one sizer

### wxSplitterWindow
- Exactly TWO child panels/widgets

### wxToolBar Children
- `tool`, `toolSeparator`, `toolStretchable`

### wxMenuBar Children
- `wxMenu` nodes

### wxMenu Children
- `wxMenuItem`, `separator`, `submenu`

---

## 5. Important Patterns from Existing Docs

### From 01-xml-structure.md

1. **Properties as attributes**: All widget properties are XML attributes on `<node>`, not child elements
2. **Events as attributes OR child elements**: Events can be node attributes (`wxEVT_BUTTON="OnClick"`) — this is the actual format used (not child `<event>` elements as in the conceptual examples in 06)
3. **Only non-default values stored**: Compact files — no redundant defaults
4. **data_version**: Currently 19-20
5. **class_name** for forms, **var_name** for widgets
6. **base_file**: Output path for generated base class (no extension)
7. **derived_class_name** + **derived_file**: For the user-editable derived class

### Property Types
- `size="1000,1000"` — width,height
- `style="wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER"` — pipe-separated flags
- `bitmap="SVG;filename.svg;[24,24]"` — type;file;[w,h]
- `bitmap="Art;wxART_CUT|wxART_TOOLBAR"` — art provider
- `font="extra large,,semi-bold"` — symbolic size, style, weight
- `borders="wxTOP|wxRIGHT|wxLEFT"` — which sides get borders
- `growablecols="1:1"` — column:proportion for FlexGridSizer

### From 06-examples.md

The doc uses `<object>` tags (XRC-style) in examples but **actual .wxui files use `<node>` tags with attribute-based properties** (as shown above). The conceptual mappings remain valid:

- **wxStdDialogButtonSizer** → `CreateStdDialogButtonSizer(wxOK|wxCANCEL)` wrapped in `CreateSeparatedSizer()`
- **wxFlexGridSizer** with `growablecols` for form layouts
- **wxNotebook** → `BookPage` children, each with a panel
- **Spacers** → `AddStretchSpacer()` or `AddSpacer()`

---

## 6. Complete File Inventory (src/wxui/)

### Generated Base Class Files (.h + .cpp pairs)
| Base Class | Description |
|------------|-------------|
| `codedisplay_base` | PanelForm with wxStyledTextCtrl |
| `code_compare_base` | Code comparison dialog |
| `colourprop_base` | Colour property editor dialog |
| `edit_html_dialog_base` | HTML editor dialog with splitter |
| `editcodedialog_base` | Code editor dialog |
| `editstringdialog_base` | Simple string editor dialog |
| `eventhandler_dlg_base` | Event handler editor with notebook |
| `fontpropdlg_base` | Font property dialog |
| `gridbag_item_base` | Grid bag sizer item editor |
| `import_base` | Import project dialog |
| `mainframe_base` | Main frame (largest, 1345 lines .cpp) |
| `new_mdi_base` | MDI child frame dialog |
| `ribbonpanel_base` | Ribbon bar panel |
| `startup_dlg_base` | Startup/project-open dialog |

### Non-Base Generated Files
| File | Description |
|------|-------------|
| `dlg_gen_results.h/.cpp` | Generation results dialog |
| `grid_property_dlg.h/.cpp` | Grid property dialog |
| `insert_widget.h/.cpp` | Widget insertion dialog |
| `optionsdlg.h/.cpp` | Options/preferences dialog |
| `menu_*.h/.cpp` | Various context menu classes (auibar, bartools, button, checkbox, combobox, datactrl, listbox, ribbontype, spin, staticsizer) |
| `menubutton.h/.cpp` | Button context menu |

### Resource/Support Files
| File | Description |
|------|-------------|
| `ui_images.h/.cpp` | Embedded image data and bundle functions |
| `wxue_data.h/.cpp` | Embedded XML data |
| `ribbon_ids.h` | Ribbon bar ID enum |
| `wxui_code.cmake` | Auto-generated CMake include |
| `wxUiEditor.wxui` | The project file itself |

---

## 7. Key Attribute Quick Reference

### Form-Level Attributes
| Attribute | Purpose | Example |
|-----------|---------|---------|
| `class` | Widget type | `"wxDialog"`, `"PanelForm"` |
| `class_name` | Generated class name | `"MainFrameBase"` |
| `base_file` | Output file (no ext) | `"mainframe_base"` |
| `derived_class_name` | User class name | `"MainFrame"` |
| `derived_file` | User class file | `"mainframe"` |
| `title` | Window title | `"Event Handler"` |
| `size` | Initial size | `"1000,1000"` |
| `minimum_size` | Min size | `"800,800"` |
| `style` | Window style flags | `"wxDEFAULT_DIALOG_STYLE\|wxRESIZE_BORDER"` |
| `persist` | Remember position/size | `"1"` |
| `use_derived_class` | Generate derived class | `"0"` = no |
| `private_members` | Members are private | `"1"` |
| `pure_virtual_functions` | Virtual = 0 | `"1"` |
| `center` | Center on screen | `"no"` to disable |
| `icon` | Window icon | `"SVG;file.svg;[16,16]"` |
| `generate_ids` | Auto-generate enum IDs | `"0"` to disable |
| `local_src_includes` | Extra includes (source) | `"mainframe.h;project_handler.h"` |
| `system_hdr_includes` | System includes (header) | `"wx/checkbox.h;wx/sizer.h"` |
| `source_preamble` | Code at top of .cpp | `"#include ..."` (use `@@` for newlines) |
| `inserted_hdr_code` | Code inserted in .h | Custom members/methods |
| `class_members` | Extra member declarations | Quoted C++ code |

### Widget Attributes
| Attribute | Purpose | Example |
|-----------|---------|---------|
| `var_name` | Member variable name | `"m_textCtrl"` |
| `class_access` | Visibility | `"protected:"`, `"none"` |
| `label` | Display text | `"Click Me"` |
| `id` | Widget ID | `"wxID_OK"`, `"id_Custom"` |
| `disabled` | Initially disabled | `"1"` |
| `hidden` | Initially hidden | `"1"` |
| `focus` | Gets initial focus | `"1"` |
| `tooltip` | Tooltip text | `"Save current project"` |
| `subclass` | Custom class name | `"NavigationPanel"` |
| `subclass_header` | Header for subclass | `"../panels/nav_panel.h"` |

### Sizer Item Attributes
| Attribute | Purpose | Example |
|-----------|---------|---------|
| `flags` | Sizer flags | `"wxEXPAND"` |
| `proportion` | Growth weight | `"1"` |
| `borders` | Border sides | `"wxALL"`, `"wxTOP\|wxLEFT"` |
| `border_size` | Border pixels | `"5"`, `"10"`, `"15"` |
| `alignment` | Alignment in sizer | `"wxALIGN_CENTER_VERTICAL"` |
