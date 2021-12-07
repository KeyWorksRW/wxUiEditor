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

EditCodeProperty::EditCodeProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

EditCodeDialog::EditCodeDialog(wxWindow* parent, NodeProperty* prop) : EditCodeDialogBase(parent)
{
    SetTitle(ttlib::cstr() << prop->DeclName() << " property editor");
    m_value = prop->as_wxString();

    m_stc->SetLexer(wxSTC_LEX_CPP);

    m_stc->SetKeyWords(0, "alignas alignof and and_eq atomic_cancel atomic_commit atomic_noexcept auto \
                              bitand bitor bool break case catch char char8_t char16_t char32_t \
                              class compl concept const consteval constexpr constinit const_cast \
                              continue co_await co_return co_yield \
	                          decltype default delete do double dynamic_cast else enum explicit \
	                          export extern false float for friend goto if inline int long \
	                          mutable namespace new noexcept not not_eq nullptr operator private or or_eq \
                              private protected public reflexpr register reinterpret_cast requires \
	                          return short signed sizeof static static_assert static_cast \
	                          struct switch synchronized template this thread_local throw true try typedef typeid \
	                          typename union unsigned using virtual void volatile wchar_t \
	                          while xor xor_eq");

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
