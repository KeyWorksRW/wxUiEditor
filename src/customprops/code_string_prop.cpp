/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "code_string_prop.h"

#include "../nodes/node_prop.h"  // NodeProperty class
#include "lambdas.h"             // Functions for formatting and storage of lamda events
#include "mainframe.h"           // MainFrame -- Main window frame

#include "utils.h"                     // Miscellaneous utility functions
#include "wxui/editcodedialog_base.h"  // auto-generated: wxui/editcodedialog_base.cpp

// Defined in base_panel.cpp
extern const char* g_u8_cpp_keywords;

EditCodeProperty::EditCodeProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

EditCodeDialog::EditCodeDialog(wxWindow* parent, NodeProperty* prop) : EditCodeDialogBase(parent)
{
    SetTitle(tt_string() << prop->declName() << " property editor");
    m_value = prop->as_wxString();
    SetStcColors(m_stc, GEN_LANG_CPLUSPLUS);
};

void EditCodeDialog::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    tt_string lamda = m_value.substr();
    ExpandLambda(lamda);

    m_stc->AddTextRaw(lamda.c_str());
}

void EditCodeDialog::OnOK(wxCommandEvent& event)
{
    // We use \r\n because it allows us to convert them in place to @@
    m_stc->ConvertEOLs(wxSTC_EOL_CRLF);

    m_value = m_stc->GetText();
    m_value.Replace(" \r", "");  // trim trailing space in lines
    m_value.Replace("\r\n", "@@");
    m_value.Trim();

    event.Skip();
}

bool EditCodeDialogAdapter::DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property))
{
    EditCodeDialog dlg(wxGetFrame().getWindow(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
