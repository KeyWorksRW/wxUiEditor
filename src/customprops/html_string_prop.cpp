/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for HTML
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "html_string_prop.h"  // auto-generated: wxui/edit_html_dialog_base.h and wxui/edit_html_dialog_base.cpp

#include "../nodes/node_prop.h"  // NodeProperty class
#include "lambdas.h"             // Functions for formatting and storage of lamda events
#include "mainframe.h"           // MainFrame -- Main window frame
#include "utils.h"               // Miscellaneous utility functions

// Defined in base_panel.cpp
extern const char* g_u8_cpp_keywords;

EditHtmlProperty::EditHtmlProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

EditHtmlDialog::EditHtmlDialog(wxWindow* parent, NodeProperty* prop) : EditHtmlDialogBase(parent)
{
    SetTitle(tt_string() << prop->declName() << " property editor");
    m_value = prop->as_wxString();

    SetStcColors(m_scintilla, GEN_LANG_XML, false, false);
};

void EditHtmlDialog::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    m_scintilla->AddText(m_value);
}

void EditHtmlDialog::OnOK(wxCommandEvent& event)
{
    m_value = m_scintilla->GetText();

    event.Skip();
}

void EditHtmlDialog::OnTextChange(wxStyledTextEvent& WXUNUSED(event))
{
    auto content = m_scintilla->GetText();
    m_htmlWin->SetPage(content);
}

bool EditHtmlDialogAdapter::DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property))
{
    EditHtmlDialog dlg(wxGetFrame().getWindow(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
