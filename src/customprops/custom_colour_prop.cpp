/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for colour
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
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
#include "../custom_ctrls/kw_color_picker.h"   // Modified version of wxColourPickerCtrl

#include "wxui/colourprop_base.h"  // auto-generated: wxui/colourprop_base.h wxui/colourprop_base.cpp

EditColourProperty::EditColourProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

void EditColourProperty::OnCustomPaint(wxDC& dc, const wxRect& rect, wxPGPaintData& /* paintdata */)
{
    if (m_prop->hasValue())
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
    void OnRadioCustomColour(wxCommandEvent& event) override;

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

EditColourDialog::EditColourDialog(wxWindow* parent, NodeProperty* prop) : ColourPropBase(parent)
{
    SetTitle(tt_string() << prop->declName() << " property editor");
    m_value = prop->as_color();
    m_node = prop->getNode();
    m_prop_name = prop->get_name();

    if (m_node->hasValue(prop_foreground_colour))
        m_foreground = m_node->as_wxColour(prop_foreground_colour);
    if (m_node->hasValue(prop_background_colour))
        m_background = m_node->as_wxColour(prop_background_colour);

    if (m_foreground.IsOk())
        m_static_sample_text->SetForegroundColour(m_foreground);
    if (m_background.IsOk())
        m_static_sample_text->SetBackgroundColour(m_background);

    if (!prop->hasValue())
    {
        m_colour_rect->SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    }
    else
    {
        m_radio_default->SetValue(false);
        m_radio_custom->SetValue(true);
        m_colourPicker->Enable(true);

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

    if (!m_radio_default->GetValue())
    {
        result = m_colourPicker->GetColour().GetAsString(wxC2S_HTML_SYNTAX);
    }

    return result;
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

    m_colourPicker->Enable(false);
    m_radio_custom->SetValue(false);
    Refresh();
}

void EditColourDialog::OnRadioCustomColour(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        m_radio_default->SetValue(false);
        m_colourPicker->Enable(true);

        wxColourPickerEvent dummy;
        OnColourChanged(dummy);
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
