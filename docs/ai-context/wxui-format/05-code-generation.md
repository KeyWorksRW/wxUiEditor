# Code Generation Mapping

Comprehensive guide to how .wxui XML elements map to generated code in C++, Python, Ruby, and Perl.

## Base/Derived Class Pattern

wxUiEditor generates **base classes** that are regenerated whenever the .wxui file changes, and separate **derived classes** for user implementation that are never overwritten.

```
MyDialog.wxui
  ↓ generates
MyDialogBase.cpp/h     ← Regenerated (contains UI construction)
MyDialog.cpp/h         ← User edits (contains event implementations)
```

**Base Class Responsibilities:**
- Member variable declarations for all widgets
- UI construction code (CreateControls())
- Virtual event handler declarations (with default `event.Skip()` implementation)
- Sizer hierarchy construction
- Widget property configuration

**Derived Class Responsibilities:**
- Override virtual event handlers
- Add custom member variables/methods
- Implement business logic

## File Naming Conventions

### C++
- Base class: `<FormName>Base.cpp` / `<FormName>Base.h`
- Derived class: `<FormName>.cpp` / `<FormName>.h`
- Example: `MyDialogBase.cpp`, `MyDialog.cpp`

### Python
- Base class: `<form_name>_base.py`
- Derived class: `<form_name>.py`
- Example: `my_dialog_base.py`, `my_dialog.py`

### Ruby
- Base class: `<form_name>_base.rb`
- Derived class: `<form_name>.rb`
- Example: `my_dialog_base.rb`, `my_dialog.rb`

### Perl
- Base class: `<FormName>Base.pm`
- Derived class: `<FormName>.pm`
- Example: `MyDialogBase.pm`, `MyDialog.pm`

## Protected Code Sections

Base class files have auto-generated sections marked with comments:

```cpp
// Do not edit any code above this line
// ... (auto-generated code) ...
// End of generated code
```

**CRITICAL:** Never edit code between these markers - changes will be lost on next regeneration. Add custom code after "End of generated code" marker.

## XML to C++ Mapping

### Dialog Definition

**.wxui:**
```xml
<object class="wxDialog" name="DiffViewer">
  <title>Code Differences</title>
  <size>1200,800</size>
  <style>wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX</style>
  <centered>1</centered>
</object>
```

**Generated C++ Base Class (.h):**
```cpp
class DiffViewerBase : public wxDialog
{
private:
    void CreateControls();

protected:
    DiffViewerBase( wxWindow* parent, wxWindowID id = wxID_ANY,
                    const wxString& title = "Code Differences",
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxSize(1200,800),
                    long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX );
};
```

**Generated C++ Base Class (.cpp):**
```cpp
DiffViewerBase::DiffViewerBase( wxWindow* parent, wxWindowID id,
                                const wxString& title, const wxPoint& pos,
                                const wxSize& size, long style )
    : wxDialog( parent, id, title, pos, size, style )
{
    if (GetSizer())
    {
        GetSizer()->Fit(this);
    }
    if (parent)
    {
        Centre();  // centered=1
    }

    CreateControls();
}
```

### Widget Member Variables

**.wxui:**
```xml
<object class="wxButton" name="m_ok_button">
  <label>OK</label>
  <default>1</default>
</object>
```

**Generated C++ (.h):**
```cpp
protected:
    wxButton* m_ok_button;
```

**Generated C++ (.cpp CreateControls()):**
```cpp
m_ok_button = new wxButton(this, wxID_ANY, "OK");
m_ok_button->SetDefault();
```

### Sizer Construction

**.wxui:**
```xml
<object class="wxBoxSizer" name="main_sizer">
  <orient>wxVERTICAL</orient>
  <object class="sizeritem">
    <object class="wxStaticText">
      <label>Choose file:</label>
    </object>
    <flag>wxALL|wxALIGN_CENTER_VERTICAL</flag>
    <border>5</border>
  </object>
  <object class="sizeritem">
    <object class="wxChoice" name="m_file_choice"/>
    <flag>wxEXPAND|wxALL</flag>
    <border>5</border>
    <option>1</option>
  </object>
</object>
```

**Generated C++ (.cpp):**
```cpp
auto* main_sizer = new wxBoxSizer(wxVERTICAL);

auto* static_text = new wxStaticText(this, wxID_ANY, "Choose file:");
main_sizer->Add(static_text, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

m_file_choice = new wxChoice(this, wxID_ANY);
main_sizer->Add(m_file_choice, 1, wxEXPAND|wxALL, 5);

SetSizer(main_sizer);
```

### Event Handler Generation

**.wxui:**
```xml
<object class="wxChoice" name="m_file_choice">
  <event name="wxEVT_CHOICE">OnFileSelected</event>
</object>
```

**Generated C++ Base Class (.h):**
```cpp
protected:
    wxChoice* m_file_choice;

    virtual void OnFileSelected( wxCommandEvent& event ) { event.Skip(); }
```

**Generated C++ Base Class (.cpp):**
```cpp
m_file_choice = new wxChoice(this, wxID_ANY);
m_file_choice->Bind(wxEVT_CHOICE, &DiffViewerBase::OnFileSelected, this);
```

**User Derived Class (.h):**
```cpp
class DiffViewer : public DiffViewerBase
{
protected:
    void OnFileSelected( wxCommandEvent& event ) override;
};
```

**User Derived Class (.cpp):**
```cpp
void DiffViewer::OnFileSelected( wxCommandEvent& event )
{
    // User implementation
    int selection = m_file_choice->GetSelection();
    LoadFile(m_file_choice->GetString(selection));
}
```

## XML to Python Mapping

### Dialog Definition

**.wxui → Python Base Class:**
```python
class MyDialogBase(wx.Dialog):
    def __init__(self, parent, id=wx.ID_ANY, title="Code Differences",
                 pos=wx.DefaultPosition, size=wx.Size(1200,800),
                 style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER|wx.MAXIMIZE_BOX):
        wx.Dialog.__init__(self, parent, id, title, pos, size, style)

        if self.GetSizer():
            self.GetSizer().Fit(self)
        if parent:
            self.Centre()

        self._create_controls()
```

### Widget and Sizer Creation

```python
def _create_controls(self):
    main_sizer = wx.BoxSizer(wx.VERTICAL)

    static_text = wx.StaticText(self, wx.ID_ANY, "Choose file:")
    main_sizer.Add(static_text, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5)

    self.m_file_choice = wx.Choice(self, wx.ID_ANY)
    main_sizer.Add(self.m_file_choice, 1, wx.EXPAND|wx.ALL, 5)

    self.SetSizer(main_sizer)
```

### Event Binding

```python
# In _create_controls():
self.m_file_choice.Bind(wx.EVT_CHOICE, self.OnFileSelected)

# Virtual handler
def OnFileSelected(self, event):
    event.Skip()
```

**User Derived Class:**
```python
class MyDialog(MyDialogBase):
    def OnFileSelected(self, event):
        # User implementation
        selection = self.m_file_choice.GetSelection()
        self.load_file(self.m_file_choice.GetString(selection))
```

## XML to Ruby Mapping

### Dialog Definition

```ruby
class MyDialogBase < Wx::Dialog
  def initialize(parent, id=Wx::ID_ANY, title="Code Differences",
                 pos=Wx::DEFAULT_POSITION, size=Wx::Size.new(1200,800),
                 style=Wx::DEFAULT_DIALOG_STYLE|Wx::RESIZE_BORDER|Wx::MAXIMIZE_BOX)
    super(parent, id, title, pos, size, style)

    if get_sizer
      get_sizer.fit(self)
    end
    if parent
      centre
    end

    create_controls
  end
end
```

### Widget and Sizer Creation

```ruby
def create_controls
  main_sizer = Wx::BoxSizer.new(Wx::VERTICAL)

  static_text = Wx::StaticText.new(self, Wx::ID_ANY, "Choose file:")
  main_sizer.add(static_text, 0, Wx::ALL|Wx::ALIGN_CENTER_VERTICAL, 5)

  @m_file_choice = Wx::Choice.new(self, Wx::ID_ANY)
  main_sizer.add(@m_file_choice, 1, Wx::EXPAND|Wx::ALL, 5)

  set_sizer(main_sizer)
end
```

### Event Binding

```ruby
# In create_controls:
@m_file_choice.evt_choice { |_event| on_file_selected(_event) }

# Virtual handler
def on_file_selected(_event)
  # Default implementation
end
```

**User Derived Class:**
```ruby
class MyDialog < MyDialogBase
  def on_file_selected(_event)
    # User implementation
    selection = @m_file_choice.get_selection
    load_file(@m_file_choice.get_string(selection))
  end
end
```

## XML to Perl Mapping

### Dialog Definition

```perl
package MyDialogBase;
use strict;
use warnings;
use Wx qw[:everything];
use base qw(Wx::Dialog);

sub new {
    my ($class, $parent, $id, $title, $pos, $size, $style) = @_;
    $id //= wxID_ANY;
    $title //= "Code Differences";
    $pos //= wxDefaultPosition;
    $size //= Wx::Size->new(1200, 800);
    $style //= wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX;

    my $self = $class->SUPER::new($parent, $id, $title, $pos, $size, $style);

    if ($self->GetSizer()) {
        $self->GetSizer()->Fit($self);
    }
    if ($parent) {
        $self->Centre();
    }

    $self->_create_controls();
    return $self;
}
```

### Widget and Sizer Creation

```perl
sub _create_controls {
    my ($self) = @_;

    my $main_sizer = Wx::BoxSizer->new(wxVERTICAL);

    my $static_text = Wx::StaticText->new($self, wxID_ANY, "Choose file:");
    $main_sizer->Add($static_text, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

    $self->{m_file_choice} = Wx::Choice->new($self, wxID_ANY);
    $main_sizer->Add($self->{m_file_choice}, 1, wxEXPAND|wxALL, 5);

    $self->SetSizer($main_sizer);
}
```

### Event Binding

```perl
# In _create_controls:
Wx::Event::EVT_CHOICE($self, $self->{m_file_choice}->GetId(),
                      sub { $self->OnFileSelected($_[1]); });

# Virtual handler
sub OnFileSelected {
    my ($self, $event) = @_;
    $event->Skip();
}
```

## Reverse Mapping: Source Code to .wxui

When converting existing wxWidgets code to .wxui format:

### 1. Identify Form Type and Properties

```cpp
// Constructor signature reveals form class, title, size, style
wxDialog(parent, wxID_ANY, "Code Differences",
         wxDefaultPosition, wxSize(1200,800),
         wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX)
```
→
```xml
<object class="wxDialog" name="FormName">
  <title>Code Differences</title>
  <size>1200,800</size>
  <style>wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX</style>
</object>
```

### 2. Extract Sizer Hierarchy

```cpp
auto* main_sizer = new wxBoxSizer(wxVERTICAL);
auto* button_sizer = new wxBoxSizer(wxHORIZONTAL);
button_sizer->Add(ok_btn, 0, wxALL, 5);
main_sizer->Add(button_sizer, 0, wxALIGN_RIGHT);
```
→
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
    </object>
    <flag>wxALIGN_RIGHT</flag>
  </object>
</object>
```

### 3. Extract Widget Properties

```cpp
m_text = new wxTextCtrl(this, wxID_ANY, "default value",
                        wxDefaultPosition, wxDefaultSize,
                        wxTE_MULTILINE|wxTE_READONLY);
m_text->SetMaxLength(100);
```
→
```xml
<object class="wxTextCtrl" name="m_text">
  <value>default value</value>
  <style>wxTE_MULTILINE|wxTE_READONLY</style>
  <maxlength>100</maxlength>
</object>
```

### 4. Extract Event Bindings

```cpp
m_button->Bind(wxEVT_BUTTON, &MyDialog::OnButtonClick, this);
m_choice->Bind(wxEVT_CHOICE, &MyDialog::OnChoiceChanged, this);
```
→
```xml
<object class="wxButton" name="m_button">
  <event name="wxEVT_BUTTON">OnButtonClick</event>
</object>
<object class="wxChoice" name="m_choice">
  <event name="wxEVT_CHOICE">OnChoiceChanged</event>
</object>
```

### 5. Handle Lambda Event Bindings

```cpp
// Convert lambda to named handler
m_slider->Bind(wxEVT_SLIDER, [this](wxCommandEvent& event) {
    m_label->SetLabel(std::to_string(m_slider->GetValue()));
});
```
→ Create named handler:
```xml
<object class="wxSlider" name="m_slider">
  <event name="wxEVT_SLIDER">OnSliderChanged</event>
</object>
```
And implement in derived class:
```cpp
void OnSliderChanged(wxCommandEvent& event) {
    m_label->SetLabel(std::to_string(m_slider->GetValue()));
}
```

## Property Name Mapping

XML properties often have different names than C++ method calls:

| XML Property | C++ Setter | Example |
|--------------|------------|---------|
| `label` | `SetLabel()` | `<label>OK</label>` → `button->SetLabel("OK")` |
| `value` | `SetValue()` | `<value>Text</value>` → `text->SetValue("Text")` |
| `checked` | `SetValue(true)` | `<checked>1</checked>` → `checkbox->SetValue(true)` |
| `selection` | `SetSelection()` | `<selection>0</selection>` → `choice->SetSelection(0)` |
| `tooltip` | `SetToolTip()` | `<tooltip>Tip</tooltip>` → `SetToolTip("Tip")` |
| `enabled` | `Enable()` | `<enabled>0</enabled>` → `Enable(false)` |
| `hidden` | `Show()` | `<hidden>1</hidden>` → `Show(false)` |
| `font` | `SetFont()` | Complex font descriptor |
| `fg` | `SetForegroundColour()` | `<fg>#FF0000</fg>` |
| `bg` | `SetBackgroundColour()` | `<bg>#FFFFFF</bg>` |
