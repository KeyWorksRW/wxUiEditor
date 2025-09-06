/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/msgdlg.h>       // common header and base class for wxMessageDialog

#include "code_display.h"  // auto-generated: wxui/codedisplay_base.h and wxui/codedisplay_base.cpp

#include "base_panel.h"      // BasePanel -- Code generation panel
#include "code.h"            // Code -- Helper class for generating code
#include "font_prop.h"       // FontProp -- Font properties
#include "image_handler.h"   // ImageHandler class
#include "mainframe.h"       // MainFrame -- Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator -- Class used to create nodes
#include "node_event.h"      // NodeEvent and NodeEventInfo classes
#include "preferences.h"     // Prefs -- Set/Get wxUiEditor preferences
#include "propgrid_panel.h"  // PropGridPanel -- PropertyGrid class for node properties and events
#include "to_casts.h"        // to_int, to_size_t, and to_char classes
#include "tt_view_vector.h"  // tt_view_vector -- read/write line-oriented strings/files
#include "utils.h"           // Miscellaneous utility functions

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

// For the actual Scintilla constants, see the following file.
// ../../wxWidgets/src/stc/lexilla/include/SciLexer.h

extern const char* g_u8_cpp_keywords;
extern const char* g_python_keywords;
extern const char* g_ruby_keywords;
extern const char* g_perl_keywords;
extern const char* g_rust_keywords;

// XRC Keywords are defined in gen_xrc_utils.cpp so they can easily be updated as XRC
// generators support more XRC controls.
extern const char* g_xrc_keywords;

const int node_marker = 1;

// clang-format off

// These are base wxWidgets classes that may be in generated code, or in member variables
// a user adds.
inline const char* lst_widgets_keywords[] = {

    "wxArrayInt",
    "wxAuiToolBarItem",
    "wxBitmap",
    "wxBitmapBundle",
    "wxBitmapButton",
    "wxColour",
    "wxDocument",
    "wxFileHistory",
    "wxFont",
    "wxGenericAnimationCtrl",
    "wxIcon",
    "wxImage",
    "wxListItem",
    "wxMemoryInputStream",
    "wxMenuBar",
    "wxObject",
    "wxPoint",
    "wxSize",
    "wxSizerFlags",
    "wxString",
    "wxToolBar",
    "wxToolBarToolBase",
    "wxVector",
    "wxWindow",
    "wxZlibInputStream",

};
// clang-format on

CodeDisplay::CodeDisplay(wxWindow* parent, GenLang panel_type) :
    CodeDisplayBase(parent), m_panel_type(panel_type)
{
    SetStcColors(m_scintilla, panel_type);

    // TODO: [KeyWorks - 01-02-2022] We do this because currently font selection uses a facename
    // which is not cross-platform. See issue #597.

    FontProperty font_prop(UserPrefs.get_CodeDisplayFont().ToStdView());
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font_prop.GetFont());

    // wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    // m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

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
        wxMessageBox(wxString() << event.GetFindString() << " not found.", "Not Found",
                     wxICON_ERROR, wxStaticCast(event.GetClientData(), wxWindow));
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
    if (node->is_Gen(gen_embedded_image))
    {
        OnEmbedImageSelected(node);
        return;
    }
    else if (node->is_Gen(gen_ribbonTool) || node->is_Gen(gen_ribbonButton))
    {
        OnRibbonToolSelected(node);
        return;
    }

    if (!node->HasProp(prop_var_name) && m_panel_type != GEN_LANG_XRC &&
        !node->is_Gen(gen_ribbonTool) && !node->is_Gen(gen_ribbonButton))
    {
        return;  // probably a form, spacer, or image
    }

    auto is_event = wxGetFrame().get_PropPanel()->IsEventPageShowing();
    PANEL_PAGE page = wxGetFrame().GetCppPanel()->GetPanelPage();

    if (m_panel_type != GEN_LANG_CPLUSPLUS && page != CPP_PANEL)
        return;  // Nothing to search for in secondary pages of non-C++ languages

    int line = -1;

    tt_string name(" ");

    Code code(node, m_panel_type);

    if (page == CPP_PANEL)
    {
        code.NodeName();
        name << code.GetCode();
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
            auto map_events = node->get_MapEvents();
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
        if (node->HasProp(prop_id) && node->as_string(prop_id) != "wxID_ANY")
        {
            search << node->get_PropId();
        }
        else if (node->HasValue(prop_var_name))
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
        if (node->is_Gen(gen_tool) || node->is_Gen(gen_auitool) || node->is_Gen(gen_ribbonTool) ||
            node->is_Gen(gen_ribbonButton))
        {
            if (node->HasValue(prop_bitmap))
            {
                tt_view_vector parts(node->as_string(prop_bitmap), BMP_PROP_SEPARATOR,
                                     tt::TRIM::both);
                if (parts.size() && parts[IndexImage].size())
                {
                    if (auto result = FileNameToVarName(parts[IndexImage]); result)
                    {
                        code.clear();
                        code.Function(node->is_Gen(gen_ribbonButton) ? "AddButton" : "AddTool");
                        line = (to_int) m_view.FindLineContaining(code.GetCode());
                        if (tt::is_found(line))
                        {
                            line = (to_int) m_view.FindLineContaining(*result, line);
                        }
                    }
                }
            }

            if (!tt::is_found(line) && node->HasValue(prop_label))
            {
                code.clear();
                code.Function("AddTool");
                line = (to_int) m_view.FindLineContaining(code.GetCode());
                if (tt::is_found(line))
                {
                    line = (to_int) m_view.FindLineContaining(node->as_string(prop_label), line);
                }
            }
        }

        if (!tt::is_found(line))
        {
            if (page == CPP_PANEL)
            {
                name << " = ";
                line = (to_int) m_view.FindLineContaining(name);
            }
            else
            {
                name << node->as_string(prop_var_name) << ";";
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

void CodeDisplay::OnRibbonToolSelected(Node* node)
{
    tt_string search;
    if (auto parent = node->get_Parent(); parent)
    {
        if (parent->is_Gen(gen_wxRibbonButtonBar))
        {
            search << '"' << node->as_string(prop_label) << '"';
        }
        else if (parent->is_Gen(gen_wxRibbonToolBar))
        {
            search << parent->as_string(prop_var_name) << "->AddTool(" << node->as_string(prop_id)
                   << ",";
            if (m_panel_type == GEN_LANG_PYTHON)
                search.Replace("->", ".");
            else if (m_panel_type == GEN_LANG_RUBY)
                search.Replace("->AddTool(", ".add_tool($");
        }
    }

    if (search.size())
    {
        if (auto line = (to_int) m_view.FindLineContaining(search); line >= 0)
        {
            m_scintilla->MarkerDeleteAll(node_marker);
            m_scintilla->MarkerAdd(line, node_marker);
            m_scintilla->ScrollToLine(line);
        }
        return;
    }
}

void CodeDisplay::OnEmbedImageSelected(Node* node)
{
    if (node->HasValue(prop_bitmap))
    {
        auto func_name = ProjectImages.GetBundleFuncName(node->as_string(prop_bitmap));
        if (func_name.size())
        {
            if (func_name.starts_with("wxue_img::"))
                func_name.erase(0, sizeof("wxue_img::") - 1);
            if (auto pos = func_name.find("("); pos != tt::npos)
                func_name.erase(pos, tt::npos);

            if (auto line = (to_int) m_view.FindLineContaining(func_name); line >= 0)
            {
                m_scintilla->MarkerDeleteAll(node_marker);
                m_scintilla->MarkerAdd(line, node_marker);
                m_scintilla->ScrollToLine(line);
                return;
            }

            // For icons, there is no bundle, just an image_ function
            func_name.Replace("bundle_", "image_");
            if (auto line = (to_int) m_view.FindLineContaining(func_name); line >= 0)
            {
                m_scintilla->MarkerDeleteAll(node_marker);
                m_scintilla->MarkerAdd(line, node_marker);
                m_scintilla->ScrollToLine(line);
                return;
            }

            // If all else fails, try just the name. This will also handle Python and Ruby panels
            func_name.Replace("image_", "");
            if (auto line = (to_int) m_view.FindLineContaining(func_name); line >= 0)
            {
                m_scintilla->MarkerDeleteAll(node_marker);
                m_scintilla->MarkerAdd(line, node_marker);
                m_scintilla->ScrollToLine(line);
                return;
            }
        }
    }
}

void CodeDisplay::SetColor(int style, const wxColour& color)
{
    m_scintilla->StyleSetForeground(style, color);
}

void CodeDisplay::SetCodeFont(const wxFont& font)
{
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
}
