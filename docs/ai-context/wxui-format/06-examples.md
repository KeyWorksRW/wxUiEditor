# Complete Working Examples

Real-world .wxui examples with comprehensive annotations showing complete dialog implementations.

## Example 1: Code Diff Viewer Dialog

This example demonstrates a complete resizable dialog with choice controls, buttons, and text display areas. Based on the DiffViewer dialog from wxUiEditor's internal compare functionality.

### Original C++ Code Pattern

```cpp
class DiffViewer : public wxDialog
{
public:
    DiffViewer(wxWindow* parent)
        : wxDialog(parent, wxID_ANY, "Code Differences",
                   wxDefaultPosition, wxSize(1200,800),
                   wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX)
    {
        CreateControls();
        Centre();
    }

private:
    void CreateControls()
    {
        auto* main_sizer = new wxBoxSizer(wxVERTICAL);

        // Top section: file selection and navigation
        auto* top_sizer = new wxBoxSizer(wxHORIZONTAL);
        auto* file_label = new wxStaticText(this, wxID_ANY, "File:");
        top_sizer->Add(file_label, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_file_choice = new wxChoice(this, wxID_ANY);
        top_sizer->Add(m_file_choice, 1, wxEXPAND|wxALL, 5);

        m_prev_button = new wxButton(this, wxID_ANY, "< Previous");
        top_sizer->Add(m_prev_button, 0, wxALL, 5);

        m_next_button = new wxButton(this, wxID_ANY, "Next >");
        top_sizer->Add(m_next_button, 0, wxALL, 5);

        main_sizer->Add(top_sizer, 0, wxEXPAND|wxALL, 5);

        // Middle section: side-by-side text display
        auto* text_sizer = new wxBoxSizer(wxHORIZONTAL);

        m_left_text = new wxStyledTextCtrl(this, wxID_ANY);
        SetupTextControl(m_left_text);
        text_sizer->Add(m_left_text, 1, wxEXPAND|wxALL, 5);

        m_right_text = new wxStyledTextCtrl(this, wxID_ANY);
        SetupTextControl(m_right_text);
        text_sizer->Add(m_right_text, 1, wxEXPAND|wxALL, 5);

        main_sizer->Add(text_sizer, 1, wxEXPAND);

        // Bottom section: Close button
        auto* button_sizer = new wxBoxSizer(wxHORIZONTAL);
        button_sizer->AddStretchSpacer();

        m_close_button = new wxButton(this, wxID_CLOSE, "Close");
        button_sizer->Add(m_close_button, 0, wxALL, 10);

        main_sizer->Add(button_sizer, 0, wxEXPAND|wxALL, 5);

        SetSizer(main_sizer);

        // Bind events
        m_file_choice->Bind(wxEVT_CHOICE, &DiffViewer::OnFileSelected, this);
        m_prev_button->Bind(wxEVT_BUTTON, &DiffViewer::OnPrevious, this);
        m_next_button->Bind(wxEVT_BUTTON, &DiffViewer::OnNext, this);
        m_close_button->Bind(wxEVT_BUTTON, &DiffViewer::OnClose, this);
    }

    wxChoice* m_file_choice;
    wxButton* m_prev_button;
    wxButton* m_next_button;
    wxStyledTextCtrl* m_left_text;
    wxStyledTextCtrl* m_right_text;
    wxButton* m_close_button;
};
```

### Equivalent .wxui File

```xml
<?xml version="1.0" encoding="UTF-8"?>
<object class="Project" name="Project">
  <code_preference>C++</code_preference>
  <python_pref>python3</python_pref>
  <ruby_pref>wxRuby3</ruby_pref>
  <base_class>wxDialog</base_class>
  <class_name>DiffViewerBase</class_name>
  <derived_class_name>DiffViewer</derived_class_name>
  <file_cpp>diff_viewer_base</file_cpp>
  <file_derived_cpp>diff_viewer</file_derived_cpp>

  <!-- Dialog definition -->
  <object class="wxDialog" name="DiffViewerBase">
    <title>Code Differences</title>
    <size>1200,800</size>
    <style>wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX</style>
    <centered>1</centered>

    <!-- Main vertical sizer -->
    <object class="wxBoxSizer">
      <orient>wxVERTICAL</orient>

      <!-- Top section: file selection and navigation -->
      <object class="sizeritem">
        <object class="wxBoxSizer" name="top_sizer">
          <orient>wxHORIZONTAL</orient>

          <!-- "File:" label -->
          <object class="sizeritem">
            <object class="wxStaticText">
              <label>File:</label>
            </object>
            <flag>wxALIGN_CENTER_VERTICAL|wxALL</flag>
            <border>5</border>
          </object>

          <!-- File selection choice control -->
          <object class="sizeritem">
            <object class="wxChoice" name="m_file_choice">
              <event name="wxEVT_CHOICE">OnFileSelected</event>
            </object>
            <option>1</option>  <!-- proportion=1, grows horizontally -->
            <flag>wxEXPAND|wxALL</flag>
            <border>5</border>
          </object>

          <!-- Previous button -->
          <object class="sizeritem">
            <object class="wxButton" name="m_prev_button">
              <label>&lt; Previous</label>
              <event name="wxEVT_BUTTON">OnPrevious</event>
            </object>
            <flag>wxALL</flag>
            <border>5</border>
          </object>

          <!-- Next button -->
          <object class="sizeritem">
            <object class="wxButton" name="m_next_button">
              <label>Next &gt;</label>
              <event name="wxEVT_BUTTON">OnNext</event>
            </object>
            <flag>wxALL</flag>
            <border>5</border>
          </object>
        </object>
        <flag>wxEXPAND|wxALL</flag>
        <border>5</border>
      </object>

      <!-- Middle section: side-by-side text display -->
      <object class="sizeritem">
        <object class="wxBoxSizer" name="text_sizer">
          <orient>wxHORIZONTAL</orient>

          <!-- Left text control -->
          <object class="sizeritem">
            <object class="wxStyledTextCtrl" name="m_left_text">
              <style>wxBORDER_SUNKEN</style>
            </object>
            <option>1</option>  <!-- grows horizontally -->
            <flag>wxEXPAND|wxALL</flag>
            <border>5</border>
          </object>

          <!-- Right text control -->
          <object class="sizeritem">
            <object class="wxStyledTextCtrl" name="m_right_text">
              <style>wxBORDER_SUNKEN</style>
            </object>
            <option>1</option>  <!-- grows horizontally -->
            <flag>wxEXPAND|wxALL</flag>
            <border>5</border>
          </object>
        </object>
        <option>1</option>  <!-- proportion=1, grows vertically -->
        <flag>wxEXPAND</flag>
      </object>

      <!-- Bottom section: Close button -->
      <object class="sizeritem">
        <object class="wxBoxSizer" name="button_sizer">
          <orient>wxHORIZONTAL</orient>

          <!-- Stretchy spacer to push button right -->
          <object class="spacer">
            <option>1</option>
          </object>

          <!-- Close button -->
          <object class="sizeritem">
            <object class="wxButton" name="m_close_button">
              <id>wxID_CLOSE</id>
              <label>Close</label>
              <event name="wxEVT_BUTTON">OnClose</event>
            </object>
            <flag>wxALL</flag>
            <border>10</border>
          </object>
        </object>
        <flag>wxEXPAND|wxALL</flag>
        <border>5</border>
      </object>
    </object>
  </object>
</object>
```

### Key Conversion Points

1. **Dialog Properties:**
   - `wxDialog(parent, wxID_ANY, "Code Differences", ...)` → `<title>Code Differences</title>`
   - `wxSize(1200,800)` → `<size>1200,800</size>`
   - Style flags → `<style>wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX</style>`
   - `Centre()` → `<centered>1</centered>`

2. **Sizer Hierarchy:**
   - Three-level nesting: main_sizer → (top_sizer, text_sizer, button_sizer)
   - Nested sizers wrapped in `<sizeritem>` with their own flags/borders

3. **Proportions:**
   - `m_file_choice` has `<option>1</option>` to grow horizontally in top_sizer
   - `text_sizer` has `<option>1</option>` to grow vertically in main_sizer
   - Both text controls have `<option>1</option>` to share horizontal space equally

4. **Spacers:**
   - `AddStretchSpacer()` → `<spacer><option>1</option></spacer>`

5. **Event Bindings:**
   - `Bind(wxEVT_CHOICE, &DiffViewer::OnFileSelected, this)` → `<event name="wxEVT_CHOICE">OnFileSelected</event>`

## Example 2: Simple Login Dialog

```xml
<?xml version="1.0" encoding="UTF-8"?>
<object class="Project">
  <object class="wxDialog" name="LoginDialogBase">
    <title>Login</title>
    <size>400,200</size>
    <centered>1</centered>

    <object class="wxBoxSizer">
      <orient>wxVERTICAL</orient>

      <!-- Form section: username and password -->
      <object class="sizeritem">
        <object class="wxFlexGridSizer">
          <rows>0</rows>
          <cols>2</cols>
          <vgap>10</vgap>
          <hgap>10</hgap>
          <growablecols>1</growablecols>

          <!-- Username label -->
          <object class="sizeritem">
            <object class="wxStaticText">
              <label>Username:</label>
            </object>
            <flag>wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT</flag>
          </object>

          <!-- Username input -->
          <object class="sizeritem">
            <object class="wxTextCtrl" name="m_username">
              <event name="wxEVT_TEXT_ENTER">OnUsernameEnter</event>
            </object>
            <flag>wxEXPAND</flag>
          </object>

          <!-- Password label -->
          <object class="sizeritem">
            <object class="wxStaticText">
              <label>Password:</label>
            </object>
            <flag>wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT</flag>
          </object>

          <!-- Password input -->
          <object class="sizeritem">
            <object class="wxTextCtrl" name="m_password">
              <style>wxTE_PASSWORD|wxTE_PROCESS_ENTER</style>
              <event name="wxEVT_TEXT_ENTER">OnPasswordEnter</event>
            </object>
            <flag>wxEXPAND</flag>
          </object>
        </object>
        <flag>wxEXPAND|wxALL</flag>
        <border>20</border>
      </object>

      <!-- Standard dialog buttons -->
      <object class="sizeritem">
        <object class="wxStdDialogButtonSizer">
          <OK>1</OK>
          <Cancel>1</Cancel>
          <default_button>OK</default_button>
          <event name="OKButtonClicked">OnOKClicked</event>
          <event name="CancelButtonClicked">OnCancelClicked</event>
        </object>
        <flag>wxEXPAND|wxALL</flag>
        <border>10</border>
      </object>
    </object>
  </object>
</object>
```

**Key Features:**
- wxFlexGridSizer for 2-column form layout (labels + inputs)
- Column 1 (index 1) set as growable for input fields to expand
- wxTE_PASSWORD style for password field
- wxTE_PROCESS_ENTER enables wxEVT_TEXT_ENTER events
- wxStdDialogButtonSizer for platform-native button ordering

## Example 3: Settings Dialog with Tabs

```xml
<?xml version="1.0" encoding="UTF-8"?>
<object class="Project">
  <object class="wxDialog" name="SettingsDialogBase">
    <title>Preferences</title>
    <size>600,400</size>
    <style>wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER</style>

    <object class="wxBoxSizer">
      <orient>wxVERTICAL</orient>

      <!-- Notebook for tabbed interface -->
      <object class="sizeritem">
        <object class="wxNotebook" name="m_notebook">

          <!-- General Settings Tab -->
          <object class="notebookpage">
            <label>General</label>
            <object class="wxPanel">
              <object class="wxBoxSizer">
                <orient>wxVERTICAL</orient>

                <object class="sizeritem">
                  <object class="wxCheckBox" name="m_auto_save">
                    <label>Enable auto-save</label>
                    <checked>1</checked>
                    <event name="wxEVT_CHECKBOX">OnAutoSaveChanged</event>
                  </object>
                  <flag>wxALL</flag>
                  <border>10</border>
                </object>

                <object class="sizeritem">
                  <object class="wxBoxSizer">
                    <orient>wxHORIZONTAL</orient>

                    <object class="sizeritem">
                      <object class="wxStaticText">
                        <label>Auto-save interval (minutes):</label>
                      </object>
                      <flag>wxALIGN_CENTER_VERTICAL|wxALL</flag>
                      <border>10</border>
                    </object>

                    <object class="sizeritem">
                      <object class="wxSpinCtrl" name="m_interval">
                        <value>5</value>
                        <min>1</min>
                        <max>60</max>
                      </object>
                      <flag>wxALL</flag>
                      <border>10</border>
                    </object>
                  </object>
                </object>
              </object>
            </object>
          </object>

          <!-- Appearance Tab -->
          <object class="notebookpage">
            <label>Appearance</label>
            <object class="wxPanel">
              <object class="wxStaticBoxSizer">
                <orient>wxVERTICAL</orient>
                <label>Color Scheme</label>

                <object class="sizeritem">
                  <object class="wxRadioButton" name="m_light_theme">
                    <label>Light theme</label>
                    <value>1</value>
                  </object>
                  <flag>wxALL</flag>
                  <border>5</border>
                </object>

                <object class="sizeritem">
                  <object class="wxRadioButton" name="m_dark_theme">
                    <label>Dark theme</label>
                  </object>
                  <flag>wxALL</flag>
                  <border>5</border>
                </object>
              </object>
            </object>
          </object>
        </object>
        <option>1</option>
        <flag>wxEXPAND|wxALL</flag>
        <border>10</border>
      </object>

      <!-- Buttons -->
      <object class="sizeritem">
        <object class="wxStdDialogButtonSizer">
          <OK>1</OK>
          <Apply>1</Apply>
          <Cancel>1</Cancel>
          <event name="OKButtonClicked">OnOK</event>
          <event name="ApplyButtonClicked">OnApply</event>
        </object>
        <flag>wxEXPAND|wxALL</flag>
        <border>10</border>
      </object>
    </object>
  </object>
</object>
```

**Key Features:**
- wxNotebook for multi-tab interface
- Each tab is a `<notebookpage>` containing a wxPanel
- wxStaticBoxSizer creates labeled grouping box
- wxRadioButton with `<value>1</value>` is initially selected
- Apply button in standard dialog button sizer

## Example 4: Data Entry Form with Validation

```xml
<object class="wxDialog" name="ContactFormBase">
  <title>Add Contact</title>
  <size>500,350</size>

  <object class="wxBoxSizer">
    <orient>wxVERTICAL</orient>

    <!-- Input fields in grid -->
    <object class="sizeritem">
      <object class="wxFlexGridSizer">
        <cols>2</cols>
        <vgap>8</vgap>
        <hgap>10</hgap>
        <growablecols>1</growablecols>

        <!-- Name -->
        <object class="sizeritem">
          <object class="wxStaticText">
            <label>Name *</label>  <!-- * indicates required -->
          </object>
          <flag>wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT</flag>
        </object>
        <object class="sizeritem">
          <object class="wxTextCtrl" name="m_name">
            <event name="wxEVT_TEXT">OnNameChanged</event>
          </object>
          <flag>wxEXPAND</flag>
        </object>

        <!-- Email -->
        <object class="sizeritem">
          <object class="wxStaticText">
            <label>Email *</label>
          </object>
          <flag>wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT</flag>
        </object>
        <object class="sizeritem">
          <object class="wxTextCtrl" name="m_email">
            <event name="wxEVT_TEXT">OnEmailChanged</event>
          </object>
          <flag>wxEXPAND</flag>
        </object>

        <!-- Phone (optional) -->
        <object class="sizeritem">
          <object class="wxStaticText">
            <label>Phone</label>
          </object>
          <flag>wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT</flag>
        </object>
        <object class="sizeritem">
          <object class="wxTextCtrl" name="m_phone"/>
          <flag>wxEXPAND</flag>
        </object>

        <!-- Category -->
        <object class="sizeritem">
          <object class="wxStaticText">
            <label>Category</label>
          </object>
          <flag>wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT</flag>
        </object>
        <object class="sizeritem">
          <object class="wxChoice" name="m_category">
            <contents>Personal|Business|Other</contents>
            <selection>0</selection>
          </object>
          <flag>wxEXPAND</flag>
        </object>

        <!-- Notes -->
        <object class="sizeritem">
          <object class="wxStaticText">
            <label>Notes</label>
          </object>
          <flag>wxALIGN_TOP|wxALIGN_RIGHT</flag>
        </object>
        <object class="sizeritem">
          <object class="wxTextCtrl" name="m_notes">
            <style>wxTE_MULTILINE</style>
            <size>-1,80</size>
          </object>
          <flag>wxEXPAND</flag>
        </object>
      </object>
      <flag>wxEXPAND|wxALL</flag>
      <border>15</border>
    </object>

    <!-- Validation message -->
    <object class="sizeritem">
      <object class="wxStaticText" name="m_validation_msg">
        <label></label>
        <fg>#FF0000</fg>  <!-- Red text for errors -->
      </object>
      <flag>wxALL</flag>
      <border>10</border>
    </object>

    <!-- Buttons -->
    <object class="sizeritem">
      <object class="wxStdDialogButtonSizer">
        <Save>1</Save>
        <Cancel>1</Cancel>
        <default_button>Save</default_button>
        <event name="SaveButtonClicked">OnSave</event>
      </object>
      <flag>wxEXPAND|wxALL</flag>
      <border>10</border>
    </object>
  </object>
</object>
```

**Key Features:**
- wxFlexGridSizer with labels right-aligned, inputs left-aligned
- `<contents>` property populates wxChoice with pipe-separated values
- Multiline wxTextCtrl with explicit height
- Validation message with red foreground color
- wxEVT_TEXT events for real-time validation
- Save button as default (Enter key activates)

## Conversion Checklist

When converting existing wxWidgets code to .wxui:

**Dialog/Frame:**
- [ ] Extract title, size, style flags from constructor
- [ ] Check for `Centre()` call → `<centered>1</centered>`
- [ ] Identify base form class (wxDialog/wxFrame/wxPanel)

**Sizers:**
- [ ] Map sizer types (wxBoxSizer → `<wxBoxSizer>`)
- [ ] Extract orientation (wxVERTICAL/wxHORIZONTAL)
- [ ] Note nesting structure
- [ ] Capture all Add() calls with proportion, flags, border

**Widgets:**
- [ ] Extract widget class and variable name
- [ ] Capture constructor parameters (label, value, style, etc.)
- [ ] Note any SetXXX() calls after construction
- [ ] Map to XML property names

**Events:**
- [ ] List all Bind() calls
- [ ] Convert lambdas to named handlers
- [ ] Extract handler names for virtual functions

**Special Cases:**
- [ ] Spacers: AddStretchSpacer() → `<spacer><option>1</option></spacer>`
- [ ] Static box sizers with labels
- [ ] Standard dialog button sizers
- [ ] Grid bag sizer cell positions
