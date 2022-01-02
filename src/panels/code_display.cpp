/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/fdrepdlg.h>  // wxFindReplaceDialog class
#include <wx/msgdlg.h>    // common header and base class for wxMessageDialog

#include "code_display.h"  // auto-generated: ../ui/codedisplay_base.h and ../ui/codedisplay_base.cpp
#include "node_creator.h"  // NodeCreator -- Class used to create nodes

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

extern const char* g_u8_cpp_keywords;

CodeDisplay::CodeDisplay(wxWindow* parent) : CodeDisplayBase(parent)
{
    // On Windows, this saves converting the UTF16 characters to ANSI.
    m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);

    // clang-format off

    // Add regular classes that have different generator class names

    ttlib::cstr widget_keywords("\
        wxToolBar \
        wxMenuBar \
        wxWindow"

        );

    // clang-format on

    for (auto iter: g_NodeCreator.GetNodeDeclarationArray())
    {
        if (!iter)
        {
            // This will happen if there is an enumerated value but no generator for it
            continue;
        }

        if (!iter->DeclName().is_sameprefix("wx") || iter->DeclName().is_sameas("wxContextMenuEvent"))
            continue;
        widget_keywords << ' ' << iter->DeclName();
    }

    // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
    m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());

    // TODO: [KeyWorks - 01-02-2022] We do this because currently font selection uses a facename which is not cross-platform.
    // See issue #597.
    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

    m_scintilla->StyleSetBold(wxSTC_C_WORD, true);
    m_scintilla->StyleSetForeground(wxSTC_C_WORD, *wxBLUE);
    m_scintilla->StyleSetForeground(wxSTC_C_WORD2, wxColour("#E91AFF"));
    m_scintilla->StyleSetForeground(wxSTC_C_STRING, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_C_STRINGEOL, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
    m_scintilla->StyleSetForeground(wxSTC_C_COMMENT, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_C_NUMBER, *wxRED);

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
        m_scintilla->SetSelectionStart(m_scintilla->GetSelectionEnd());
        m_scintilla->SearchAnchor();
        result = m_scintilla->SearchNext(sciflags, event.GetFindString());
    }
    else
    {
        m_scintilla->SetSelectionEnd(m_scintilla->GetSelectionStart());
        m_scintilla->SearchAnchor();
        result = m_scintilla->SearchPrev(sciflags, event.GetFindString());
    }

    if (result == wxSTC_INVALID_POSITION)
    {
        wxMessageBox(wxString() << event.GetFindString() << " not found.", "Not Found", wxICON_ERROR,
                     wxStaticCast(event.GetClientData(), wxWindow));
    }
    else
    {
        m_scintilla->EnsureCaretVisible();
    }
}

void CodeDisplay::FindItemName(const wxString& name)
{
    m_scintilla->SetSelectionStart(m_scintilla->GetSelectionEnd());
    m_scintilla->SearchAnchor();
    if (m_scintilla->SearchNext(wxSTC_FIND_WHOLEWORD | wxSTC_FIND_MATCHCASE, name) != wxSTC_INVALID_POSITION)
        m_scintilla->EnsureCaretVisible();
}
