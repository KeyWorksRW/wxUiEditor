/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/msgdlg.h>       // common header and base class for wxMessageDialog

#include "code_display.h"  // auto-generated: wxui/codedisplay_base.h and wxui/codedisplay_base.cpp

#include "base_panel.h"      // BasePanel -- Code generation panel
#include "mainframe.h"       // MainFrame -- Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator -- Class used to create nodes
#include "node_event.h"      // NodeEvent and NodeEventInfo classes
#include "propgrid_panel.h"  // PropGridPanel -- PropertyGrid class for node properties and events

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

extern const char* g_u8_cpp_keywords;

const int node_marker = 1;

// XRC Keywords are defined in gen_xrc_utils.cpp so they can easily be updated as XRC generators support more XRC controls.
extern const char* g_xrc_keywords;

// clang-format off

const char* g_python_keywords =
    "False None True and as assert async break class continue def del elif else except finally for from global if import in is lambda "
    "nonlocal not or pass raise return try while with yield";

// clang-format on

const char* g_ruby_keywords =
    "ENCODING LINE FILE BEGIN END alias and begin break case class def defined do else elsif end ensure false for if in "
    "module next nil not or redo require rescue retry return self super then true undef unless until when while yield";

CodeDisplay::CodeDisplay(wxWindow* parent, int panel_type) : CodeDisplayBase(parent), m_panel_type(panel_type)
{
    if (panel_type == GEN_LANG_XRC)
    {
        m_scintilla->SetLexer(wxSTC_LEX_XML);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);

        m_scintilla->StyleSetBold(wxSTC_H_TAG, true);
        m_scintilla->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour("#FF00FF"));
        m_scintilla->StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
        m_scintilla->StyleSetForeground(wxSTC_H_COMMENT, wxColour(0, 128, 0));
        m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, *wxRED);
        m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
        m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour(0, 128, 0));
        m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour(0, 128, 0));
    }
    else if (panel_type == GEN_LANG_PYTHON)
    {
        m_scintilla->SetLexer(wxSTC_LEX_PYTHON);
        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_python_keywords);

        tt_string wxPython_keywords("ToolBar MenuBar BitmapBundle Bitmap MemoryInputStream Window");

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx") || iter->declName().is_sameas("wxContextMenuEvent"))
                continue;
            // wxPython_keywords << " wx." << iter->declName().subview(2);
            wxPython_keywords << ' ' << iter->declName().subview(2);
        }

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) wxPython_keywords.c_str());

        m_scintilla->StyleSetForeground(wxSTC_P_WORD, *wxBLUE);
        m_scintilla->StyleSetForeground(wxSTC_P_WORD2, wxColour("#FF00FF"));
        m_scintilla->StyleSetForeground(wxSTC_P_STRING, wxColour(0, 128, 0));
        m_scintilla->StyleSetForeground(wxSTC_P_STRINGEOL, wxColour(0, 128, 0));
        m_scintilla->StyleSetForeground(wxSTC_P_COMMENTLINE, wxColour(0, 128, 0));
        m_scintilla->StyleSetForeground(wxSTC_P_NUMBER, *wxRED);
    }
    else if (panel_type == GEN_LANG_RUBY)
    {
        m_scintilla->SetLexer(wxSTC_LEX_RUBY);

        tt_string wxRuby_keywords("ToolBar MenuBar BitmapBundle Bitmap Window Wx");

        // clang-format on

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx"))
                continue;
            else if (iter->declName().is_sameas("wxContextMenuEvent") || iter->declName() == "wxTreeCtrlBase" ||
                     iter->declName().starts_with("wxRuby") || iter->declName().starts_with("wxPython"))
                continue;
            wxRuby_keywords << ' ' << iter->declName().subview(2);
        }

        // Unfortunately, RUBY_LEXER only supports one set of keywords so we have to combine the regular keywords with
        // the wxWidgets keywords.

        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) wxRuby_keywords.c_str());

        m_scintilla->StyleSetForeground(wxSTC_RB_WORD, "#FF00FF");
        m_scintilla->StyleSetForeground(wxSTC_RB_STRING, wxColour(0, 128, 0));
        m_scintilla->StyleSetForeground(wxSTC_RB_COMMENTLINE, wxColour(0, 128, 0));
        m_scintilla->StyleSetForeground(wxSTC_RB_NUMBER, *wxRED);
    }
    else  // C++
    {
        // On Windows, this saves converting the UTF16 characters to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_u8_cpp_keywords);

        // Add regular classes that have different generator class names

        tt_string widget_keywords(
            "wxToolBar wxMenuBar wxBitmapBundle wxBitmap wxImage wxMemoryInputStream wxVector wxWindow");

        for (auto iter: NodeCreation.getNodeDeclarationArray())
        {
            if (!iter)
            {
                // This will happen if there is an enumerated value but no generator for it
                continue;
            }

            if (!iter->declName().starts_with("wx") || iter->declName().is_sameas("wxContextMenuEvent"))
                continue;
            widget_keywords << ' ' << iter->declName();
        }

        // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
        m_scintilla->SendMsg(SCI_SETKEYWORDS, 1, (wxIntPtr) widget_keywords.c_str());
        m_scintilla->StyleSetBold(wxSTC_C_WORD, true);
        m_scintilla->StyleSetForeground(wxSTC_C_WORD, *wxBLUE);
        m_scintilla->StyleSetForeground(wxSTC_C_WORD2, wxColour("#FF00FF"));
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

void CodeDisplay::doWrite(tt_string_view code)
{
    m_view.GetBuffer() << code;
}

void CodeDisplay::CodeGenerationComplete()
{
    m_scintilla->AddTextRaw(m_view.GetBuffer().data(), (to_int) m_view.GetBuffer().size());
    m_scintilla->SetReadOnly(true);

    // Find doesn't work correctly unless there's a selection to start the search from.
    m_scintilla->SetEmptySelection(0);

    m_view.ParseBuffer();
}

void CodeDisplay::OnNodeSelected(Node* node)
{
    if (!node->hasProp(prop_var_name) && m_panel_type != GEN_LANG_XRC)
    {
        return;  // probably a form, spacer, or image
    }

    auto is_event = wxGetFrame().getPropPanel()->IsEventPageShowing();
    PANEL_PAGE page = wxGetFrame().getGeneratedPanel()->GetPanelPage();

    if (m_panel_type != GEN_LANG_CPLUSPLUS && page != CPP_PANEL)
        return;  // Nothing to search for in secondary pages of non-C++ languages

    int line = -1;

    tt_string name(" ");

    if (page == CPP_PANEL)
    {
        if (m_panel_type == GEN_LANG_PYTHON && !node->isLocal())
            name << "self.";
        name << node->as_string(prop_var_name);
    }

    if (is_event)
    {
        if (page == CPP_PANEL)
        {
            name << "->Bind";
            line = (to_int) m_view.FindLineContaining(name);
            if (!tt::is_found(line))
            {
                name.Replace("->Bind", " = ");
                line = (to_int) m_view.FindLineContaining(name);
            }
        }
        else
        {
            auto map_events = node->getMapEvents();
            for (auto& iter: map_events)
            {
                auto value = iter.second.get_value();
                if (value.empty())
                    continue;

                line = (to_int) m_view.FindLineContaining(value);
                if (tt::is_found(line))
                    break;
            }
        }
    }
    else if (m_panel_type == GEN_LANG_XRC)
    {
        tt_string search("name=\"");
        if (node->hasProp(prop_id) && node->as_string(prop_id) != "wxID_ANY")
        {
            search << node->getPropId();
        }
        else if (node->hasValue(prop_var_name))
        {
            search << node->as_string(prop_var_name);
        }
        else
        {
            search << node->as_string(prop_class_name);
        }
        line = (to_int) m_view.FindLineContaining(search);
    }
    else
    {
        if (page == CPP_PANEL)
        {
            name << " = ";
            line = (to_int) m_view.FindLineContaining(name);
        }
        else
        {
            if (node->getForm()->as_bool(prop_generate_translation_unit))
            {
                name << node->as_string(prop_var_name) << ";";
                line = (to_int) m_view.FindLineContaining(name);
            }
            else
            {
                name << node->as_string(prop_var_name) << " = ";
                line = (to_int) m_view.FindLineContaining(name);
            }
        }
    }

    if (!tt::is_found(line))
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
