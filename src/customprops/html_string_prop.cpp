/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for HTML
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "html_string_prop.h"  // auto-generated: wxui/edit_html_dialog_base.h and wxui/edit_html_dialog_base.cpp

#include "../nodes/node_prop.h"  // NodeProperty class
#include "lambdas.h"             // Functions for formatting and storage of lambda events
#include "mainframe.h"           // MainFrame -- Main window frame
#include "utils.h"               // Miscellaneous utility functions

#include "keywords.h"

EditHtmlProperty::EditHtmlProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()),
    m_prop(prop)
{
}

EditHtmlDialog::EditHtmlDialog(wxWindow* parent, NodeProperty* prop) : EditHtmlDialogBase(parent)
{
    SetTitle((wxue::string() << prop->get_DeclName() << " property editor").wx());
    m_value = prop->as_wxString();

    SetStcColors(m_scintilla, GenLang::xml, false, false);
};

void EditHtmlDialog::OnInit([[maybe_unused]] wxInitDialogEvent& event)
{
    m_scintilla->AddText(m_value);
}

void EditHtmlDialog::OnOK(wxCommandEvent& event)
{
    m_value = m_scintilla->GetText();

    event.Skip();
}

void EditHtmlDialog::OnTextChange([[maybe_unused]] wxStyledTextEvent& event)
{
    const wxString content = m_scintilla->GetText();
    m_htmlWin->SetPage(content);
}

bool EditHtmlDialogAdapter::DoShowDialog([[maybe_unused]] wxPropertyGrid* propGrid,
                                         [[maybe_unused]] wxPGProperty* property)
{
    EditHtmlDialog html_dialog(wxGetFrame().getWindow(), m_prop);
    if (html_dialog.ShowModal() == wxID_OK)
    {
        SetValue(html_dialog.GetResults());
        return true;
    }

    return false;
}
