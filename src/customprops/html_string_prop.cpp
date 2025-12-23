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
    SetTitle(tt_string() << prop->get_DeclName() << " property editor");
    m_value = prop->as_wxString();

    SetStcColors(m_scintilla, GEN_LANG_XML, false, false);
};

auto EditHtmlDialog::OnInit([[maybe_unused]] wxInitDialogEvent& event) -> void
{
    m_scintilla->AddText(m_value);
}

auto EditHtmlDialog::OnOK(wxCommandEvent& event) -> void
{
    m_value = m_scintilla->GetText();

    event.Skip();
}

auto EditHtmlDialog::OnTextChange([[maybe_unused]] wxStyledTextEvent& event) -> void
{
    auto content = m_scintilla->GetText();
    m_htmlWin->SetPage(content);
}

auto EditHtmlDialogAdapter::DoShowDialog([[maybe_unused]] wxPropertyGrid* propGrid,
                                         [[maybe_unused]] wxPGProperty* property) -> bool
{
    EditHtmlDialog dlg(wxGetFrame().getWindow(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
