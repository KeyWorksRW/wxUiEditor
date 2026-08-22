/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Property editor for pop_custom_mockup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "../nodes/node_prop.h"  // NodeProperty class
#include "mainframe.h"           // MainFrame -- Main window frame

#include "edit_custom_mockup.h"

#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

EditCustomMockupProperty::EditCustomMockupProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()),
    m_prop(prop)
{
}

EditCustomMockup::EditCustomMockup(wxWindow* parent, NodeProperty* prop) :
    EditCustomMockupBase(parent)
{
    m_result = prop->as_wxString();
}

bool EditCustomMockupDialogAdapter::DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                                                 wxPGProperty* /* property unused */)
{
    EditCustomMockup mockup_dialog(wxGetFrame().getWindow(), m_prop);
    if (mockup_dialog.ShowModal() == wxID_OK)
    {
        SetValue(mockup_dialog.GetResults());
        return true;
    }

    return false;
}

void EditCustomMockup::OnInit(wxInitDialogEvent& event)
{
    wxue::StringVector parts(m_result.ToStdString(), ';');
    if (parts.size() == 3)
    {
        m_widget_type = parts[0];
        m_width = wxue::atoi(parts[1]);
        m_height = parts[2];

        if (m_widget_type.starts_with("wxStaticText"))
        {
            if (auto pos = m_widget_type.find('('); pos != std::string::npos)
            {
                wxue::StringVector options(m_widget_type.Mid(pos + 1).utf8_string(), ",");
                m_text_static->SetValue(options[0]);
                if (options.size() > 1)
                {
                    m_check_centered->SetValue(options[1].contains("1"));
                }
            }

            m_widget_types->SetStringSelection("wxStaticText");
            m_static_box->GetStaticBox()->Show(true);
            Fit();
        }
    }

    event.Skip();
}

void EditCustomMockup::OnSelect(wxCommandEvent& /* event unused */)
{
    const wxString widget_type = m_widget_types->GetStringSelection();
    if (widget_type.StartsWith("wxStaticText"))
    {
        m_static_box->GetStaticBox()->Show(true);
    }
    else
    {
        m_static_box->GetStaticBox()->Show(false);
    }
    Fit();
}

void EditCustomMockup::OnOK(wxCommandEvent& event)
{
    if (!Validate() || !TransferDataFromWindow())
    {
        return;
    }

    m_result.clear();
    m_result << m_widget_type;
    if (m_widget_type.StartsWith("wxStaticText"))
    {
        m_result << "(" << m_text_static->GetValue();
        m_result << (m_check_centered->GetValue() ? ", 1" : ", 0");
        m_result << ")";
    }

    // wxBitmap is the default, and currently we don't allow any parameters for it

    m_result << ";" << m_width << ";" << m_height;
    event.Skip();
}
