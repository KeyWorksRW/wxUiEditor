/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for colour
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "wx/dc.h"                 // wxDC class
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "custom_colour_prop.h"

#include "gen_enums.h"  // Enumerations for generators
#include "node.h"       // Node class
#include "node_prop.h"  // NodeProperty class
#include "utils.h"      // Utility functions that work with properties

#include "../custom_ctrls/colour_rect_ctrl.h"  // ColourRectCtrl -- Control that displays a solid color

#include "wxui/colourprop_base.h"  // auto-generated: wxui/colourprop_base.h wxui/colourprop_base.cpp

EditColourProperty::EditColourProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

void EditColourProperty::OnCustomPaint(wxDC& dc, const wxRect& rect, wxPGPaintData& /* paintdata */)
{
    if (m_prop->HasValue())
    {
        if (auto clr = m_prop->as_color(); clr.IsOk())
        {
            dc.SetBrush(clr);
            dc.DrawRectangle(rect);
        }
    }
}

// We declare the class here so that propgrid_panel doesn't have a dependency on colourprop_base.h and all the header files
// that pulls in.

class EditColourDialog : public ColourPropBase
{
public:
    EditColourDialog(wxWindow* parent, NodeProperty* prop);

    wxString GetResults();

protected:
    void OnColourChanged(wxColourPickerEvent&) override;
    void OnSetDefault(wxCommandEvent&) override;
    void OnSystemColourChange(wxCommandEvent&) override;
    void OnRadioCustomColour(wxCommandEvent& event) override;
    void OnRadioSystemColour(wxCommandEvent& event) override;

    void SetSampleColour(wxColor clr);

private:
    Node* m_node;
    wxColour m_value;
    GenEnum::PropName m_prop_name;
    wxColour m_foreground;
    wxColour m_background;
};

bool EditColourDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property))
{
    EditColourDialog dlg(propGrid->GetPanel(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}

// clang-format off
std::map<std::string, const char*> g_sys_colour_pair = {

    { "App Workspace", "wxSYS_COLOUR_APPWORKSPACE" },
    { "Active Border", "wxSYS_COLOUR_ACTIVEBORDER" },
    { "Active Caption", "wxSYS_COLOUR_ACTIVECAPTION" },
    { "Button Face", "wxSYS_COLOUR_BTNFACE" },
    { "Button Highlight", "wxSYS_COLOUR_BTNHIGHLIGHT" },
    { "Button Shadow", "wxSYS_COLOUR_BTNSHADOW" },
    { "Button Text", "wxSYS_COLOUR_BTNTEXT" },
    { "Caption Text", "wxSYS_COLOUR_CAPTIONTEXT" },
    { "Control Dark", "wxSYS_COLOUR_3DDKSHADOW" },
    { "Control Light", "wxSYS_COLOUR_3DLIGHT" },
    { "Desktop", "wxSYS_COLOUR_BACKGROUND" },
    { "Gray Text", "wxSYS_COLOUR_GRAYTEXT" },
    { "Highlight", "wxSYS_COLOUR_HIGHLIGHT" },
    { "Highlight Text", "wxSYS_COLOUR_HIGHLIGHTTEXT" },
    { "Inactive Border", "wxSYS_COLOUR_INACTIVEBORDER" },
    { "Inactive Caption", "wxSYS_COLOUR_INACTIVECAPTION" },
    { "Inactive Caption Text", "wxSYS_COLOUR_INACTIVECAPTIONTEXT" },
    { "Menu", "wxSYS_COLOUR_MENU" },
    { "Scrollbar", "wxSYS_COLOUR_SCROLLBAR" },
    { "Tooltip", "wxSYS_COLOUR_INFOBK" },
    { "Tooltip Text", "wxSYS_COLOUR_INFOTEXT" },
    { "Window", "wxSYS_COLOUR_WINDOW" },
    { "Window Frame", "wxSYS_COLOUR_WINDOWFRAME" },
    { "Window Text", "wxSYS_COLOUR_WINDOWTEXT" },

};
// clang-format on

EditColourDialog::EditColourDialog(wxWindow* parent, NodeProperty* prop) : ColourPropBase(parent)
{
    SetTitle(ttlib::cstr() << prop->DeclName() << " property editor");
    m_value = prop->as_color();
    m_node = prop->GetNode();
    m_prop_name = prop->get_name();

    if (m_node->HasValue(prop_foreground_colour))
        m_foreground = m_node->prop_as_wxColour(prop_foreground_colour);
    if (m_node->HasValue(prop_background_colour))
        m_background = m_node->prop_as_wxColour(prop_background_colour);

    wxArrayString tmp_array;

    tmp_array.Add("aquamarine");
    tmp_array.Add("black");
    tmp_array.Add("blue");
    tmp_array.Add("blue violet");
    tmp_array.Add("brown");
    tmp_array.Add("cadet blue");
    tmp_array.Add("coral");
    tmp_array.Add("cornflower blue");
    tmp_array.Add("cyan");
    tmp_array.Add("dark grey");
    tmp_array.Add("dark green");
    tmp_array.Add("dark olive green");
    tmp_array.Add("dark orchid");
    tmp_array.Add("dark slate blue");
    tmp_array.Add("dark slate grey");
    tmp_array.Add("dark turquoise");
    tmp_array.Add("dim grey");
    tmp_array.Add("firebrick");
    tmp_array.Add("forest green");
    tmp_array.Add("gold");
    tmp_array.Add("goldenrod");
    tmp_array.Add("grey");
    tmp_array.Add("green");
    tmp_array.Add("green yellow");
    tmp_array.Add("indian red");
    tmp_array.Add("khaki");
    tmp_array.Add("light blue");
    tmp_array.Add("light grey");
    tmp_array.Add("light steel blue");
    tmp_array.Add("lime green");
    tmp_array.Add("light magenta");
    tmp_array.Add("magenta");
    tmp_array.Add("maroon");
    tmp_array.Add("medium aquamarine");
    tmp_array.Add("medium grey");
    tmp_array.Add("medium blue");
    tmp_array.Add("medium forest green");
    tmp_array.Add("medium goldenrod");
    tmp_array.Add("medium orchid");
    tmp_array.Add("medium sea green");
    tmp_array.Add("medium slate blue");
    tmp_array.Add("medium spring green");
    tmp_array.Add("medium turquoise");
    tmp_array.Add("medium violet red");
    tmp_array.Add("midnight blue");
    tmp_array.Add("navy");
    tmp_array.Add("orange");
    tmp_array.Add("orange red");
    tmp_array.Add("orchid");
    tmp_array.Add("pale green");
    tmp_array.Add("pink");
    tmp_array.Add("plum");
    tmp_array.Add("purple");
    tmp_array.Add("red");
    tmp_array.Add("salmon");
    tmp_array.Add("sea green");
    tmp_array.Add("sienna");
    tmp_array.Add("sky blue");
    tmp_array.Add("slate blue");
    tmp_array.Add("spring green");
    tmp_array.Add("steel blue");
    tmp_array.Add("tan");
    tmp_array.Add("thistle");
    tmp_array.Add("turquoise");
    tmp_array.Add("violet");
    tmp_array.Add("violet red");
    tmp_array.Add("wheat");
    tmp_array.Add("white");
    tmp_array.Add("yellow");
    tmp_array.Add("yellow green");

    m_colourPicker->GetTextCtrl()->AutoComplete(tmp_array);

    for (auto& iter: g_sys_colour_pair)
    {
        m_combo_system->AppendString(wxString(iter.first));
    }

    if (m_foreground.IsOk())
        m_static_sample_text->SetForegroundColour(m_foreground);
    if (m_background.IsOk())
        m_static_sample_text->SetBackgroundColour(m_background);

    if (!prop->HasValue())
    {
        m_colour_rect->SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        m_combo_system->Select(m_combo_system->FindString("WindowText", true));
    }
    else if (prop->as_string().starts_with("wx"))
    {
        m_radio_default->SetValue(false);
        m_radio_custom->SetValue(false);
        m_radio_system->SetValue(true);
        m_colourPicker->Enable(false);
        m_combo_system->Enable(true);

        for (auto& iter: g_sys_colour_pair)
        {
            if (prop->as_string() == iter.second)
            {
                wxColour clr = wxSystemSettings::GetColour(ConvertToSystemColour(iter.second));
                m_colour_rect->SetColour(clr);
                m_colourPicker->SetColour(prop->as_color());
                m_combo_system->Select(m_combo_system->FindString(iter.first, true));
                break;
            }
        }
    }
    else
    {
        m_radio_default->SetValue(false);
        m_radio_custom->SetValue(true);
        m_radio_system->SetValue(false);
        m_colourPicker->Enable(true);
        m_combo_system->Enable(false);
        m_combo_system->Select(m_combo_system->FindString("WindowText", true));

        auto clr = prop->as_color();
        m_colour_rect->SetColour(prop->as_color());
        m_colourPicker->SetColour(prop->as_color());
    }
}

wxString EditColourDialog::GetResults()
{
    wxString result;

    // The property string needs to be empty for the default value, 3 comma-separated numbers if it's a custom colour, and a
    // wxSYS_COLOUR_... string if it's a system colour.

    if (m_radio_default->GetValue())
        return result;
    else if (m_radio_custom->GetValue())
    {
        m_value = m_colourPicker->GetColour();
        result = ConvertColourToString(m_value).c_str();
    }
    else
    {
        auto str = m_combo_system->GetStringSelection().utf8_string();
        for (auto& iter: g_sys_colour_pair)
        {
            if (str == iter.first)
            {
                result = iter.second;
                break;
            }
        }
    }

    return result;
}

void EditColourDialog::OnSystemColourChange(wxCommandEvent& /* event */)
{
    auto str = m_combo_system->GetStringSelection().utf8_string();
    for (auto& iter: g_sys_colour_pair)
    {
        if (str == iter.first)
        {
            wxColour clr = wxSystemSettings::GetColour(ConvertToSystemColour(iter.second));
            m_colour_rect->SetColour(clr);
            SetSampleColour(clr);
            Refresh();
            break;
        }
    }
}

void EditColourDialog::OnColourChanged(wxColourPickerEvent&)
{
    auto clr = m_colourPicker->GetColour();
    m_colour_rect->SetColour(clr);
    SetSampleColour(clr);
    Refresh();
}

void EditColourDialog::OnSetDefault(wxCommandEvent&)
{
    m_colour_rect->SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    m_static_sample_text->SetForegroundColour(*wxBLACK);

    m_radio_system->SetValue(false);
    m_colourPicker->Enable(false);

    m_radio_custom->SetValue(false);
    m_combo_system->Enable(false);
    Refresh();
}

void EditColourDialog::OnRadioCustomColour(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        m_radio_default->SetValue(false);
        m_radio_system->SetValue(false);
        m_colourPicker->Enable(true);
        m_combo_system->Enable(false);

        wxColourPickerEvent dummy;
        OnColourChanged(dummy);
    }
    event.Skip();
}

void EditColourDialog::OnRadioSystemColour(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        m_radio_default->SetValue(false);
        m_radio_custom->SetValue(false);
        m_colourPicker->Enable(false);
        m_combo_system->Enable(true);

        wxCommandEvent dummy;
        OnSystemColourChange(dummy);
    }
    event.Skip();
}

void EditColourDialog::SetSampleColour(wxColor clr)
{
    if (m_prop_name == prop_foreground_colour)
        m_static_sample_text->SetForegroundColour(clr);
    else
        m_static_sample_text->SetBackgroundColour(clr);
}
