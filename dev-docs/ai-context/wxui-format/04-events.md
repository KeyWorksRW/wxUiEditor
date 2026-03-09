# Event Handler Binding

Comprehensive reference for binding wxWidgets event handlers in .wxui files and mapping source code event bindings to XML.

## Event Element Syntax

Events are declared as child elements of widgets/forms using the `<event>` element:

```xml
<object class="wxButton" name="m_ok_button">
  <label>OK</label>
  <event name="wxEVT_BUTTON">OnOKClicked</event>
</object>
```

**Generated Code (C++ Base Class):**
```cpp
class MyDialogBase : public wxDialog
{
protected:
    wxButton* m_ok_button;

    // Virtual handler declared
    virtual void OnOKClicked( wxCommandEvent& event ) { event.Skip(); }

private:
    void CreateControls()
    {
        m_ok_button = new wxButton(this, wxID_ANY, "OK");

        // Bind to virtual handler
        m_ok_button->Bind(wxEVT_BUTTON, &MyDialogBase::OnOKClicked, this);
    }
};
```

**User Derived Class:**
```cpp
class MyDialog : public MyDialogBase
{
protected:
    // Override the virtual handler
    void OnOKClicked( wxCommandEvent& event ) override
    {
        // Your implementation
        EndModal(wxID_OK);
    }
};
```

## Event Class Types

Each wxEVT_* event type has an associated event class that determines the handler signature:

| Event Class | Common Events | Handler Signature |
|-------------|---------------|-------------------|
| wxCommandEvent | wxEVT_BUTTON, wxEVT_CHECKBOX, wxEVT_CHOICE, wxEVT_TEXT | `void Handler(wxCommandEvent& event)` |
| wxMouseEvent | wxEVT_LEFT_DOWN, wxEVT_RIGHT_UP, wxEVT_MOTION | `void Handler(wxMouseEvent& event)` |
| wxKeyEvent | wxEVT_CHAR, wxEVT_KEY_DOWN, wxEVT_KEY_UP | `void Handler(wxKeyEvent& event)` |
| wxFocusEvent | wxEVT_SET_FOCUS, wxEVT_KILL_FOCUS | `void Handler(wxFocusEvent& event)` |
| wxSizeEvent | wxEVT_SIZE | `void Handler(wxSizeEvent& event)` |
| wxPaintEvent | wxEVT_PAINT | `void Handler(wxPaintEvent& event)` |
| wxUpdateUIEvent | wxEVT_UPDATE_UI | `void Handler(wxUpdateUIEvent& event)` |
| wxTreeEvent | wxEVT_TREE_SEL_CHANGED, wxEVT_TREE_ITEM_ACTIVATED | `void Handler(wxTreeEvent& event)` |
| wxListEvent | wxEVT_LIST_ITEM_SELECTED, wxEVT_LIST_ITEM_ACTIVATED | `void Handler(wxListEvent& event)` |

## Event Types by Widget Category

### Buttons

**wxButton, wxBitmapButton:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_BUTTON | wxCommandEvent | Button clicked |

**wxCheckBox, Check3State:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_CHECKBOX | wxCommandEvent | Checkbox toggled |

**wxRadioButton:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_RADIOBUTTON | wxCommandEvent | Radio button selected |

**wxToggleButton, wxBitmapToggleButton:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_TOGGLEBUTTON | wxCommandEvent | Toggle button state changed |

**wxSpinCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_SPINCTRL | wxSpinEvent | Spin value changed |
| wxEVT_TEXT | wxCommandEvent | Text edited |
| wxEVT_TEXT_ENTER | wxCommandEvent | Enter key pressed |

**wxSpinCtrlDouble:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_SPINCTRLDOUBLE | wxSpinDoubleEvent | Spin value changed |
| wxEVT_TEXT | wxCommandEvent | Text edited |
| wxEVT_TEXT_ENTER | wxCommandEvent | Enter key pressed |

**wxSpinButton:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_SPIN | wxSpinEvent | Spin value changed |
| wxEVT_SPIN_UP | wxSpinEvent | Up arrow clicked |
| wxEVT_SPIN_DOWN | wxSpinEvent | Down arrow clicked |

### Text Controls

**wxTextCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_TEXT | wxCommandEvent | Text content changed |
| wxEVT_TEXT_ENTER | wxCommandEvent | Enter key pressed (requires wxTE_PROCESS_ENTER) |
| wxEVT_TEXT_URL | wxTextUrlEvent | URL clicked (requires wxTE_AUTO_URL) |
| wxEVT_TEXT_MAXLEN | wxCommandEvent | Max length reached |

**wxRichTextCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_TEXT | wxCommandEvent | Text changed |
| wxEVT_TEXT_ENTER | wxCommandEvent | Enter pressed |
| wxEVT_TEXT_URL | wxTextUrlEvent | URL clicked |
| wxEVT_RICHTEXT_LEFT_CLICK | wxRichTextEvent | Left mouse click |
| wxEVT_RICHTEXT_RIGHT_CLICK | wxRichTextEvent | Right mouse click |
| wxEVT_RICHTEXT_MIDDLE_CLICK | wxRichTextEvent | Middle mouse click |
| wxEVT_RICHTEXT_CHARACTER | wxRichTextEvent | Character typed |
| wxEVT_RICHTEXT_DELETE | wxRichTextEvent | Content deleted |
| wxEVT_RICHTEXT_RETURN | wxRichTextEvent | Return key |
| wxEVT_RICHTEXT_STYLE_CHANGED | wxRichTextEvent | Style changed |
| wxEVT_RICHTEXT_PROPERTIES_CHANGED | wxRichTextEvent | Properties changed |

**wxSearchCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_TEXT | wxCommandEvent | Text changed |
| wxEVT_TEXT_ENTER | wxCommandEvent | Enter pressed |
| wxEVT_SEARCHCTRL_SEARCH_BTN | wxCommandEvent | Search button clicked |
| wxEVT_SEARCHCTRL_CANCEL_BTN | wxCommandEvent | Cancel button clicked |

### Choice Controls

**wxChoice, wxComboBox:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_CHOICE | wxCommandEvent | Selection changed |

**wxListBox, wxCheckListBox:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_LISTBOX | wxCommandEvent | Selection changed |
| wxEVT_LISTBOX_DCLICK | wxCommandEvent | Item double-clicked |
| wxEVT_CHECKLISTBOX | wxCommandEvent | Check state changed (wxCheckListBox only) |

**wxRadioBox:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_RADIOBOX | wxCommandEvent | Selection changed |

### Pickers

**wxFilePickerCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_FILEPICKER_CHANGED | wxFileDirPickerEvent | File selection changed |

**wxDirPickerCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_DIRPICKER_CHANGED | wxFileDirPickerEvent | Directory selection changed |

**wxFontPickerCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_FONTPICKER_CHANGED | wxFontPickerEvent | Font selection changed |

**wxColourPickerCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_COLOURPICKER_CHANGED | wxColourPickerEvent | Color selection confirmed |
| wxEVT_COLOURPICKER_CURRENT_CHANGED | wxColourPickerEvent | Current color changed (preview) |

**wxDatePickerCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_DATE_CHANGED | wxDateEvent | Date selection changed |

**wxTimePickerCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_TIME_CHANGED | wxDateEvent | Time selection changed |

### Sliders and Scrollbars

**wxSlider, wxScrollBar:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_SLIDER / wxEVT_SCROLL_CHANGED | wxCommandEvent / wxScrollEvent | Value changed (after release) |
| wxEVT_SCROLL_TOP | wxScrollEvent | Scrolled to top |
| wxEVT_SCROLL_BOTTOM | wxScrollEvent | Scrolled to bottom |
| wxEVT_SCROLL_LINEUP | wxScrollEvent | Scrolled up one unit |
| wxEVT_SCROLL_LINEDOWN | wxScrollEvent | Scrolled down one unit |
| wxEVT_SCROLL_PAGEUP | wxScrollEvent | Scrolled up one page |
| wxEVT_SCROLL_PAGEDOWN | wxScrollEvent | Scrolled down one page |
| wxEVT_SCROLL_THUMBTRACK | wxScrollEvent | Thumb dragged (continuous updates) |
| wxEVT_SCROLL_THUMBRELEASE | wxScrollEvent | Thumb released |

### Tree and List Controls

**wxTreeCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_TREE_SEL_CHANGED | wxTreeEvent | Selection changed |
| wxEVT_TREE_SEL_CHANGING | wxTreeEvent | Selection changing (vetoable) |
| wxEVT_TREE_ITEM_ACTIVATED | wxTreeEvent | Item activated (double-click/Enter) |
| wxEVT_TREE_ITEM_EXPANDED | wxTreeEvent | Item expanded |
| wxEVT_TREE_ITEM_EXPANDING | wxTreeEvent | Item expanding (vetoable) |
| wxEVT_TREE_ITEM_COLLAPSED | wxTreeEvent | Item collapsed |
| wxEVT_TREE_ITEM_COLLAPSING | wxTreeEvent | Item collapsing (vetoable) |
| wxEVT_TREE_BEGIN_LABEL_EDIT | wxTreeEvent | Label editing started (vetoable) |
| wxEVT_TREE_END_LABEL_EDIT | wxTreeEvent | Label editing finished |
| wxEVT_TREE_BEGIN_DRAG | wxTreeEvent | Drag started (left button) |
| wxEVT_TREE_BEGIN_RDRAG | wxTreeEvent | Drag started (right button) |
| wxEVT_TREE_END_DRAG | wxTreeEvent | Drag ended |
| wxEVT_TREE_ITEM_RIGHT_CLICK | wxTreeEvent | Item right-clicked |
| wxEVT_TREE_ITEM_MIDDLE_CLICK | wxTreeEvent | Item middle-clicked |
| wxEVT_TREE_DELETE_ITEM | wxTreeEvent | Item deleted |
| wxEVT_TREE_KEY_DOWN | wxTreeEvent | Key pressed |
| wxEVT_TREE_ITEM_MENU | wxTreeEvent | Context menu requested |

### Special Widgets

**wxCalendarCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_CALENDAR_SEL_CHANGED | wxCalendarEvent | Date selected |
| wxEVT_CALENDAR_DOUBLECLICKED | wxCalendarEvent | Date double-clicked |
| wxEVT_CALENDAR_DAY_CHANGED | wxCalendarEvent | Day changed |
| wxEVT_CALENDAR_MONTH_CHANGED | wxCalendarEvent | Month changed |
| wxEVT_CALENDAR_YEAR_CHANGED | wxCalendarEvent | Year changed |
| wxEVT_CALENDAR_PAGE_CHANGED | wxCalendarEvent | Page (month/year) changed |
| wxEVT_CALENDAR_WEEK_CLICKED | wxCalendarEvent | Week number clicked |

**wxHtmlWindow:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_HTML_CELL_CLICKED | wxHtmlCellEvent | Cell clicked |
| wxEVT_HTML_CELL_HOVER | wxHtmlCellEvent | Mouse hovering over cell |
| wxEVT_HTML_LINK_CLICKED | wxHtmlLinkEvent | Link clicked |

**wxWebView:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_WEBVIEW_NAVIGATING | wxWebViewEvent | Navigation starting (vetoable) |
| wxEVT_WEBVIEW_NAVIGATED | wxWebViewEvent | Navigation completed |
| wxEVT_WEBVIEW_LOADED | wxWebViewEvent | Page loaded |
| wxEVT_WEBVIEW_ERROR | wxWebViewEvent | Navigation error |
| wxEVT_WEBVIEW_NEWWINDOW | wxWebViewEvent | New window requested |
| wxEVT_WEBVIEW_TITLE_CHANGED | wxWebViewEvent | Page title changed |

**wxGenericDirCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_DIRCTRL_SELECTIONCHANGED | wxCommandEvent | Selection changed |
| wxEVT_DIRCTRL_FILEACTIVATED | wxCommandEvent | File activated |

**wxFileCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_FILECTRL_FILEACTIVATED | wxCommandEvent | File activated |
| wxEVT_FILECTRL_SELECTIONCHANGED | wxCommandEvent | Selection changed |
| wxEVT_FILECTRL_FOLDERCHANGED | wxCommandEvent | Folder changed |

**wxHyperlinkCtrl:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_HYPERLINK | wxHyperlinkEvent | Link clicked |

## Common Events (All Widgets)

### Focus Events
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_SET_FOCUS | wxFocusEvent | Widget gained focus |
| wxEVT_KILL_FOCUS | wxFocusEvent | Widget lost focus |

### Mouse Events
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_LEFT_DOWN | wxMouseEvent | Left button pressed |
| wxEVT_LEFT_UP | wxMouseEvent | Left button released |
| wxEVT_LEFT_DCLICK | wxMouseEvent | Left button double-clicked |
| wxEVT_RIGHT_DOWN | wxMouseEvent | Right button pressed |
| wxEVT_RIGHT_UP | wxMouseEvent | Right button released |
| wxEVT_RIGHT_DCLICK | wxMouseEvent | Right button double-clicked |
| wxEVT_MIDDLE_DOWN | wxMouseEvent | Middle button pressed |
| wxEVT_MIDDLE_UP | wxMouseEvent | Middle button released |
| wxEVT_MIDDLE_DCLICK | wxMouseEvent | Middle button double-clicked |
| wxEVT_AUX1_DOWN | wxMouseEvent | Aux button 1 pressed |
| wxEVT_AUX1_UP | wxMouseEvent | Aux button 1 released |
| wxEVT_AUX1_DCLICK | wxMouseEvent | Aux button 1 double-clicked |
| wxEVT_AUX2_DOWN | wxMouseEvent | Aux button 2 pressed |
| wxEVT_AUX2_UP | wxMouseEvent | Aux button 2 released |
| wxEVT_AUX2_DCLICK | wxMouseEvent | Aux button 2 double-clicked |
| wxEVT_MOTION | wxMouseEvent | Mouse moved |
| wxEVT_MOUSEWHEEL | wxMouseEvent | Mouse wheel scrolled |
| wxEVT_ENTER_WINDOW | wxMouseEvent | Mouse entered widget |
| wxEVT_LEAVE_WINDOW | wxMouseEvent | Mouse left widget |

### Keyboard Events
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_CHAR | wxKeyEvent | Character typed |
| wxEVT_CHAR_HOOK | wxKeyEvent | Character (preprocessed) |
| wxEVT_KEY_DOWN | wxKeyEvent | Key pressed |
| wxEVT_KEY_UP | wxKeyEvent | Key released |

### Other Common Events
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_UPDATE_UI | wxUpdateUIEvent | UI update requested (for enable/disable logic) |
| wxEVT_SIZE | wxSizeEvent | Widget resized |
| wxEVT_PAINT | wxPaintEvent | Widget needs repainting |
| wxEVT_ERASE_BACKGROUND | wxEraseEvent | Background erase requested |
| wxEVT_CONTEXT_MENU | wxContextMenuEvent | Context menu requested |
| wxEVT_HELP | wxHelpEvent | Help requested |

## Window/Form Events

**wxDialog, wxFrame:**
| Event | Class | Description |
|-------|-------|-------------|
| wxEVT_ACTIVATE | wxActivateEvent | Window activated/deactivated |
| wxEVT_CLOSE_WINDOW | wxCloseEvent | Window close requested |
| wxEVT_ICONIZE | wxIconizeEvent | Window minimized/restored |

## Code to XML Conversion

**Recognizing Event Bindings in Source:**

```cpp
// Simple Bind
m_button->Bind(wxEVT_BUTTON, &MyDialog::OnButtonClick, this);
// → <event name="wxEVT_BUTTON">OnButtonClick</event>

// Lambda (convert to named handler)
m_choice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
    UpdateDisplay();
});
// → Create named handler: <event name="wxEVT_CHOICE">OnChoiceChanged</event>
//   Implement: void OnChoiceChanged(wxCommandEvent&) { UpdateDisplay(); }

// Multiple events on same widget
m_text->Bind(wxEVT_TEXT, &MyDialog::OnTextChanged, this);
m_text->Bind(wxEVT_TEXT_ENTER, &MyDialog::OnTextEnter, this);
// → <event name="wxEVT_TEXT">OnTextChanged</event>
//   <event name="wxEVT_TEXT_ENTER">OnTextEnter</event>
```

**Event Handler Naming Conventions:**
- Button click: `OnButtonClick`, `OnOKClicked`, `OnCancelClicked`
- Selection change: `OnChoiceChanged`, `OnListSelected`
- Text change: `OnTextChanged`, `OnTextEnter`
- Tree events: `OnTreeSelChanged`, `OnTreeItemActivated`
- Generic: `On<WidgetName><Action>`

## Multiple Events Example

```xml
<object class="wxTextCtrl" name="m_search_text">
  <style>wxTE_PROCESS_ENTER</style>
  <event name="wxEVT_TEXT">OnSearchTextChanged</event>
  <event name="wxEVT_TEXT_ENTER">OnSearchEnter</event>
  <event name="wxEVT_SET_FOCUS">OnSearchFocusGained</event>
</object>
```

Generated handlers:
```cpp
virtual void OnSearchTextChanged( wxCommandEvent& event ) { event.Skip(); }
virtual void OnSearchEnter( wxCommandEvent& event ) { event.Skip(); }
virtual void OnSearchFocusGained( wxFocusEvent& event ) { event.Skip(); }
```
