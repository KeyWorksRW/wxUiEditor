/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/fdrepdlg.h>  // wxFindReplaceDialog class
#include <wx/msgdlg.h>    // common header and base class for wxMessageDialog

#include "code_display.h"  // auto-generated: wxui/codedisplay_base.h and wxui/codedisplay_base.cpp

#include "mainframe.h"       // MainFrame -- Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator -- Class used to create nodes
#include "propgrid_panel.h"  // PropGridPanel -- PropertyGrid class for node properties and events

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

extern const char* g_u8_cpp_keywords;

const int node_marker = 1;

const char* g_xrc_keywords =
    "animation bg bitmap border centered class default default_size direction effectduration enabled exstyle fields flag fg "
    "focused font hideeffect gradient-end gradient-start help icon inactive-bitmap label linesize max min message name "
    "object option orient pagesize pos range resource selmax selmin showeffect size stock_client stock_id style styles "
    "thumb tick tickfreq title tooltip value variant widths";

CodeDisplay::CodeDisplay(wxWindow* parent, bool is_XML) : CodeDisplayBase(parent), m_isXML(is_XML)
{
    if (m_isXML)
    {
        m_scintilla->SetLexer(wxSTC_LEX_XML);
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
    }
    else
    {
        // On Windows, this saves converting the UTF16 characters to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);

        // clang-format off

    // Add regular classes that have different generator class names

    ttlib::cstr widget_keywords("\
        wxToolBar \
        wxMenuBar \
        wxBitmapBundle \
        wxBitmap \
        wxImage \
        wxMemoryInputStream \
        wxVector \
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

            if (!iter->DeclName().starts_with("wx") || iter->DeclName().is_sameas("wxContextMenuEvent"))
                continue;
            widget_keywords << ' ' << iter->DeclName();
        }

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
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
    }

    // TODO: [KeyWorks - 01-02-2022] We do this because currently font selection uses a facename which is not cross-platform.
    // See issue #597.
    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

    m_scintilla->MarkerDefine(node_marker, wxSTC_MARK_BOOKMARK, wxNullColour, *wxGREEN);

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

void CodeDisplay::Clear()
{
    m_view.clear();
    m_view.GetBuffer().clear();

    m_scintilla->SetReadOnly(false);
    m_scintilla->ClearAll();
}

void CodeDisplay::doWrite(ttlib::sview code) { m_view.GetBuffer() << code; }

void CodeDisplay::CodeGenerationComplete()
{
    m_scintilla->AddTextRaw(m_view.GetBuffer().data(), (to_int) m_view.GetBuffer().size());
    m_scintilla->SetReadOnly(true);

    m_view.ParseBuffer();
}

void CodeDisplay::OnNodeSelected(Node* node)
{
    if (!node->HasProp(prop_var_name) && !m_isXML)
    {
        return;  // probably a form, spacer, or image
    }

    auto is_event = wxGetFrame().GetPropPanel()->IsEventPageShowing();

    // Find where the node is created.

    ttlib::cstr name(" ");
    name << node->prop_as_string(prop_var_name);
    int line = 0;
    if (is_event)
    {
        name << "->Bind";
        line = (to_int) m_view.FindLineContaining(name);
        if (!ttlib::is_found(line))
        {
            name.Replace("->Bind", " = ");
            line = (to_int) m_view.FindLineContaining(name);
        }
    }
    else if (m_isXML)
    {
        ttlib::cstr search("name=\"");
        if (node->HasValue(prop_var_name))
        {
            search << node->prop_as_string(prop_var_name);
        }
        else
        {
            search << node->prop_as_string(prop_class_name);
        }
        line = (to_int) m_view.FindLineContaining(search);
    }
    else
    {
        name << " = ";
        line = (to_int) m_view.FindLineContaining(name);
    }

    if (!ttlib::is_found(line))
        return;

    m_scintilla->MarkerDeleteAll(node_marker);
    m_scintilla->MarkerAdd(line, node_marker);

#if 0
    // REVIEW: [KeyWorks - 01-20-2022] This would be great if it worked, but GetLineVisible() is returning true even if
    // the line is not visible.
    if (!m_scintilla->GetLineVisible(line))
    {
        m_scintilla->ScrollToLine(line);
    }
#endif

    // Unlike GetLineVisible(), this function does ensure that the line is visible.
    m_scintilla->ScrollToLine(line);
}
