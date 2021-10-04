/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/fdrepdlg.h>  // wxFindReplaceDialog class
#include <wx/msgdlg.h>    // common header and base class for wxMessageDialog
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/stc/stc.h>   // A wxWidgets implementation of Scintilla.  This class is the

#include "code_display.h"

CodeDisplay::CodeDisplay(wxWindow* parent, int id) : wxPanel(parent, id)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    m_code = new wxStyledTextCtrl(this);
    sizer->Add(m_code, wxSizerFlags(1).Expand().Border());
    SetSizer(sizer);

    Bind(wxEVT_FIND, &CodeDisplay::OnFind, this);
    Bind(wxEVT_FIND_NEXT, &CodeDisplay::OnFind, this);
}

void CodeDisplay::OnFind(wxFindDialogEvent& event)
{
    auto wxflags = event.GetFlags();
    int sciflags = 0;

    if (wxflags & wxFR_WHOLEWORD)
    {
        sciflags |= wxSTC_FIND_WHOLEWORD;
    }
    if (wxflags & wxFR_MATCHCASE)
    {
        sciflags |= wxSTC_FIND_MATCHCASE;
    }

    int result;
    if (wxflags & wxFR_DOWN)
    {
        m_code->SetSelectionStart(m_code->GetSelectionEnd());
        m_code->SearchAnchor();
        result = m_code->SearchNext(sciflags, event.GetFindString());
    }
    else
    {
        m_code->SetSelectionEnd(m_code->GetSelectionStart());
        m_code->SearchAnchor();
        result = m_code->SearchPrev(sciflags, event.GetFindString());
    }

    if (result == wxSTC_INVALID_POSITION)
    {
        wxMessageBox(wxString() << event.GetFindString() << " not found.", "Not Found", wxICON_ERROR,
                     wxStaticCast(event.GetClientData(), wxWindow));
    }
    else
    {
        m_code->EnsureCaretVisible();
    }
}

void CodeDisplay::FindItemName(const wxString& name)
{
    m_code->SetSelectionStart(m_code->GetSelectionEnd());
    m_code->SearchAnchor();
    if (m_code->SearchNext(wxSTC_FIND_WHOLEWORD | wxSTC_FIND_MATCHCASE, name) != wxSTC_INVALID_POSITION)
        m_code->EnsureCaretVisible();
}
