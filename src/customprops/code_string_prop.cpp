/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "code_string_prop.h"

#include "../nodes/node_prop.h"  // NodeProperty class
#include "lambdas.h"             // Functions for formatting and storage of lamda events

#include "../ui/editcodedialog_base.h"  // auto-generated: ../ui/editcodedialog_base.cpp

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
    SetTitle(ttlib::cstr() << prop->DeclName() << " property editor");
    m_value = prop->as_wxString();

    m_stc->SetLexer(wxSTC_LEX_CPP);

    // On Windows, this saves converting the UTF16 characters to ANSI.
    m_stc->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);

    m_stc->StyleSetBold(wxSTC_C_WORD, true);
    m_stc->StyleSetForeground(wxSTC_C_WORD, *wxBLUE);
    m_stc->StyleSetForeground(wxSTC_C_STRING, wxColour(0, 128, 0));
    m_stc->StyleSetForeground(wxSTC_C_STRINGEOL, wxColour(0, 128, 0));
    m_stc->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
    m_stc->StyleSetForeground(wxSTC_C_COMMENT, wxColour(0, 128, 0));
    m_stc->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0, 128, 0));
    m_stc->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(0, 128, 0));
    m_stc->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, wxColour(0, 128, 0));
    m_stc->StyleSetForeground(wxSTC_C_NUMBER, *wxRED);
};

void EditCodeDialog::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    ttlib::cstr lamda = m_value.substr();
    ExpandLambda(lamda);

    m_stc->AddTextRaw(lamda.c_str());
}

void EditCodeDialog::OnOK(wxCommandEvent& event)
{
    ttlib::cstr body(m_stc->GetTextRaw().data());
    body.Replace("\r", "", tt::REPLACE::all);  // Remove Windows EOL
    body.Replace("\n", "@@", tt::REPLACE::all);
    body.RightTrim();
    m_value = body.wx_str();

    event.Skip();
}

bool EditCodeDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property))
{
    EditCodeDialog dlg(propGrid->GetPanel(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
