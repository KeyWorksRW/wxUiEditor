/////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel to display original imported file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/fdrepdlg.h>  // wxFindReplaceDialog class
#include <wx/scrolwin.h>  // wxScrolledWindow, wxScrolledControl and wxScrollHelper
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/stc/stc.h>   // A wxWidgets implementation of Scintilla.

#include "import_panel.h"

#include "cstm_event.h"  // CustomEvent -- Custom Event class
#include "mainapp.h"     // App -- Main application class
#include "mainframe.h"   // MainFrame -- Main window frame

#include "../panels/propgrid_panel.h"  // PropGridPanel -- PropertyGrid class for node properties and events
#include "node.h"                      // Node class
#include "utils.h"                     // Utility functions that work with properties

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

// Keywords are defined in gen_xrc_utils.cpp so they can easily be updated as XRC generators
// support more XRC controls.
extern const char* g_xrc_keywords;

const int node_marker = 1;

ImportPanel::ImportPanel(wxWindow* parent) : wxScrolled<wxPanel>(parent)
{
    auto parent_sizer = new wxBoxSizer(wxVERTICAL);
    m_scintilla = new wxStyledTextCtrl(this, wxID_ANY);

    // TODO: [KeyWorks - 01-02-2022] We do this because currently font selection uses a facename which is not
    // cross-platform. See issue #597.
    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

    // These are settings used in codedisplay_base
    m_scintilla->SetWrapMode(wxSTC_WRAP_WHITESPACE);
    m_scintilla->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_END);
    m_scintilla->SetWrapVisualFlagsLocation(wxSTC_WRAPVISUALFLAGLOC_END_BY_TEXT);
    m_scintilla->SetWrapIndentMode(wxSTC_WRAPINDENT_INDENT);
    // Sets text margin scaled appropriately for the current DPI on Windows,
    // 5 on wxGTK or wxOSX
    m_scintilla->SetMarginLeft(wxSizerFlags::GetDefaultBorder());
    m_scintilla->SetMarginRight(wxSizerFlags::GetDefaultBorder());
    m_scintilla->SetMarginWidth(1, 0);  // Remove default margin
    m_scintilla->SetMarginWidth(0, 16);
    m_scintilla->SetMarginType(0, wxSTC_MARGIN_SYMBOL);
    m_scintilla->SetMarginMask(0, ~wxSTC_MASK_FOLDERS);
    m_scintilla->SetMarginSensitive(0, false);
    m_scintilla->SetIndentationGuides(wxSTC_IV_REAL);
    m_scintilla->SetUseTabs(false);
    m_scintilla->SetTabWidth(4);
    m_scintilla->SetBackSpaceUnIndents(true);

    m_scintilla->MarkerDefine(node_marker, wxSTC_MARK_BOOKMARK, wxNullColour, *wxGREEN);
    parent_sizer->Add(m_scintilla, wxSizerFlags(1).Expand().Border(wxALL));

    Bind(wxEVT_FIND, &ImportPanel::OnFind, this);
    Bind(wxEVT_FIND_NEXT, &ImportPanel::OnFind, this);

    SetSizerAndFit(parent_sizer);
}

void ImportPanel::SetImportFile(const ttlib::cstr& file, int lexer)
{
    m_view.clear();
    m_view.GetBuffer().clear();
    if (!m_view.ReadFile(file))
    {
        FAIL_MSG(ttlib::cstr("Can't read ") << file);
        return;
    }

    m_lexer = lexer;
    m_scintilla->SetLexer(lexer);
    m_scintilla->SetTabWidth(4);

    switch (lexer)
    {
        case wxSTC_LEX_XML:
            // This uses the XRC keywords, which will generally be fine for XRC, wxSMith, and wxGlade. wxFormBuilder
            // could probably use some extra keywords...

            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);

            m_scintilla->StyleSetBold(wxSTC_H_TAG, true);
            m_scintilla->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour("#E91AFF"));
            m_scintilla->StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
            m_scintilla->StyleSetForeground(wxSTC_H_COMMENT, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, *wxRED);
            m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
            m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour(0, 128, 0));
            break;

        case wxSTC_LEX_CPP:
            // TODO: [Randalphwa - 06-17-2022] wxSTC_LEX_CPP is used for Windows Resource files, so RC keywords are needed...

            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            // m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);

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
            break;

        case wxSTC_LEX_JSON:
            // TODO: [Randalphwa - 06-17-2022] Keywords specific to wxCrafter would probably be useful....

            // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
            m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);

            m_scintilla->StyleSetBold(wxSTC_H_TAG, true);
            m_scintilla->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour("#E91AFF"));
            m_scintilla->StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
            m_scintilla->StyleSetForeground(wxSTC_H_COMMENT, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, *wxRED);
            m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
            m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour(0, 128, 0));
            m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour(0, 128, 0));
            break;

        default:
            FAIL_MSG(ttlib::cstr("Unsupported lexer: ") << lexer);
            break;
    }

    m_scintilla->SetReadOnly(false);
    m_scintilla->ClearAll();
    m_scintilla->AddTextRaw(m_view.GetBuffer().data(), (to_int) m_view.GetBuffer().size());
    m_scintilla->SetReadOnly(true);

    // Find doesn't work correctly unless there's a selection to start the search from.
    m_scintilla->SetEmptySelection(0);
}

void ImportPanel::OnFind(wxFindDialogEvent& event)
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

void ImportPanel::Clear()
{
    m_view.clear();
    m_view.GetBuffer().clear();

    m_scintilla->SetReadOnly(false);
    m_scintilla->ClearAll();
}

void ImportPanel::OnNodeSelected(Node* node)
{
    // Find where the node is created.
    ttlib::cstr name(" ");
    name << node->prop_as_string(prop_var_name);
    int line = 0;

    ttlib::cstr search(m_lexer != wxSTC_LEX_JSON ? "name=\"" : "\"");
    if (node->HasProp(prop_id) && node->prop_as_string(prop_id) != "wxID_ANY")
    {
        search << node->prop_as_string(prop_id);
    }
    else if (node->HasValue(prop_var_name))
    {
        search << node->prop_as_string(prop_var_name);
    }
    else
    {
        search << node->prop_as_string(prop_class_name);
    }
    line = (to_int) m_view.FindLineContaining(search);

    if (!ttlib::is_found(line))
        return;

    m_scintilla->MarkerDeleteAll(node_marker);
    m_scintilla->MarkerAdd(line, node_marker);

    // Unlike GetLineVisible(), this function does ensure that the line is visible.
    m_scintilla->ScrollToLine(line);
}
