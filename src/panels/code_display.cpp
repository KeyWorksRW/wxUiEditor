/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-01-2026]

#include <cctype>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/msgdlg.h>       // common header and base class for wxMessageDialog
#include <wx/stc/stc.h>      // wxStyledTextEvent

#include "code_display.h"  // auto-generated: wxui/codedisplay_base.h and wxui/codedisplay_base.cpp

#include "base_panel.h"        // BasePanel -- Code generation panel
#include "code.h"              // Code -- Helper class for generating code
#include "font_prop.h"         // FontProp -- Font properties
#include "image_handler.h"     // ImageHandler class
#include "language_traits.h"   // LanguageTraits, GetLanguageTraits()
#include "mainframe.h"         // MainFrame -- Main window frame
#include "node.h"              // Node class
#include "node_creator.h"      // NodeCreator -- Class used to create nodes
#include "node_event.h"        // NodeEvent and NodeEventInfo classes
#include "preferences.h"       // Prefs -- Set/Get wxUiEditor preferences
#include "propgrid_panel.h"    // PropGridPanel -- PropertyGrid class for node properties and events
#include "to_casts.h"          // to_int, to_size_t, and to_char classes
#include "utils.h"             // Miscellaneous utility functions
#include "wxue_view_vector.h"  // wxue::ViewVector

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
#endif

// For the actual Scintilla constants, see the following file.
// ../../wxWidgets/src/stc/lexilla/include/SciLexer.h

#include "keywords.h"

// XRC Keywords are defined in gen_xrc_utils.cpp so they can easily be updated as XRC
// generators support more XRC controls.
extern const char* g_xrc_keywords;

const int node_marker = 1;

CodeDisplay::CodeDisplay(wxWindow* parent, GenLang panel_type) :
    CodeDisplayBase(parent),
    m_panel_type(panel_type)
{
    SetStcColors(m_scintilla, panel_type);

    // TODO: [KeyWorks - 01-02-2022] We do this because currently font selection uses a facename
    // which is not cross-platform. See issue #597.

    const FontProperty font_prop(UserPrefs.get_CodeDisplayFont().ToStdView());
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font_prop.GetFont());

    // wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    // m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

    m_scintilla->MarkerDefine(node_marker, wxSTC_MARK_BOOKMARK, wxNullColour, *wxGREEN);

    Bind(wxEVT_FIND, &CodeDisplay::OnFind, this);
    Bind(wxEVT_FIND_NEXT, &CodeDisplay::OnFind, this);

    m_scintilla->Bind(wxEVT_STC_HOTSPOT_CLICK, &CodeDisplay::OnHotSpotClick, this);
}

void CodeDisplay::OnFind(wxFindDialogEvent& event)
{
    const int wxflags = event.GetFlags();
    int sciflags = 0;

    if (wxflags & wxFR_WHOLEWORD)
    {
        sciflags |= wxSTC_FIND_WHOLEWORD;
    }
    if (wxflags & wxFR_MATCHCASE)
    {
        sciflags |= wxSTC_FIND_MATCHCASE;
    }

    int result {};
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
    m_hotspots.clear();

    m_scintilla->SetReadOnly(false);
    m_scintilla->ClearAll();
}

void CodeDisplay::doWrite(std::string_view code)
{
    m_view.GetBuffer() += code;
}

void CodeDisplay::CodeGenerationComplete()
{
    m_scintilla->AddTextRaw(m_view.GetBuffer().data(), (to_int) m_view.GetBuffer().size());
    m_scintilla->SetReadOnly(true);

    // Find doesn't work correctly unless there's a selection to start the search from.
    m_scintilla->SetEmptySelection(0);

    m_view.ParseBuffer();

    ApplyClassHotspots();
}

void CodeDisplay::OnNodeSelected(Node* node)
{
    if (node->is_Gen(gen_embedded_image))
    {
        OnEmbedImageSelected(node);
        return;
    }
    if (node->is_Gen(gen_ribbonTool) || node->is_Gen(gen_ribbonButton))
    {
        OnRibbonToolSelected(node);
        return;
    }

    if (!node->HasProp(prop_var_name) && m_panel_type != GenLang::xrc)
    {
        return;  // probably a form, spacer, or image
    }

    const bool is_event = wxGetFrame().get_PropPanel()->IsEventPageShowing();
    const PANEL_PAGE page = wxGetFrame().GetCppPanel()->GetPanelPage();

    if (m_panel_type != GenLang::cplusplus && page != PANEL_PAGE::SOURCE_PANEL)
    {
        return;  // Nothing to search for in secondary pages of non-C++ languages
    }

    int line = -1;

    wxue::string name(" ");

    Code code(node, m_panel_type);

    if (page == PANEL_PAGE::SOURCE_PANEL)
    {
        code.NodeName();
        name << code.GetCode();
    }

    if (is_event)
    {
        if (page == PANEL_PAGE::SOURCE_PANEL)
        {
            name << "->Bind";
            line = (to_int) m_view.FindLineContaining(name);
            if (!wxue::is_found(line))
            {
                std::ignore = name.Replace("->Bind", " = ");
                line = (to_int) m_view.FindLineContaining(name);
            }
        }
        else
        {
            const NodeMapEvents map_events = node->get_MapEvents();
            for (auto& iter: map_events)
            {
                const std::string value = iter.second.get_value();
                if (value.empty())
                {
                    continue;
                }

                line = (to_int) m_view.FindLineContaining(value);
                if (wxue::is_found(line))
                {
                    break;
                }
            }
        }
    }
    if (m_panel_type == GenLang::xrc)
    {
        wxue::string search("name=\"");
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
                wxue::ViewVector parts(node->as_string(prop_bitmap), BMP_PROP_SEPARATOR,
                                       wxue::TRIM::both);
                if (!parts.empty() && !parts[IndexImage].empty())
                {
                    if (auto result = FileNameToVarName(parts[IndexImage]); result)
                    {
                        code.clear();
                        code.Function(node->is_Gen(gen_ribbonButton) ? "AddButton" : "AddTool");
                        line = (to_int) m_view.FindLineContaining(code.GetCode());
                        if (wxue::is_found(line))
                        {
                            line = (to_int) m_view.FindLineContaining(*result, line);
                        }
                    }
                }
            }

            if (!wxue::is_found(line) && node->HasValue(prop_label))
            {
                code.clear();
                code.Function("AddTool");
                line = (to_int) m_view.FindLineContaining(code.GetCode());
                if (wxue::is_found(line))
                {
                    line = (to_int) m_view.FindLineContaining(node->as_string(prop_label), line);
                }
            }
        }

        if (!wxue::is_found(line))
        {
            if (page == PANEL_PAGE::SOURCE_PANEL)
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

    if (!wxue::is_found(line))
    {
        return;
    }

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
    wxue::string search;
    if (auto* parent = node->get_Parent(); parent)
    {
        if (parent->is_Gen(gen_wxRibbonButtonBar))
        {
            search << '"' << node->as_string(prop_label) << '"';
        }
        if (parent->is_Gen(gen_wxRibbonToolBar))
        {
            search << parent->as_string(prop_var_name) << "->AddTool(" << node->as_string(prop_id)
                   << ",";
            if (m_panel_type == GenLang::python)
            {
                std::ignore = search.Replace("->", ".");
            }
            if (m_panel_type == GenLang::ruby)
            {
                std::ignore = search.Replace("->AddTool(", ".add_tool($");
            }
        }
    }

    if (!search.empty())
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
        if (!func_name.empty())
        {
            if (func_name.starts_with("wxue_img::"))
            {
                func_name.erase(0, sizeof("wxue_img::") - 1);
            }
            if (auto pos = func_name.find("("); pos != wxue::npos)
            {
                func_name.erase(pos, wxue::npos);
            }

            if (auto line = (to_int) m_view.FindLineContaining(func_name); line >= 0)
            {
                m_scintilla->MarkerDeleteAll(node_marker);
                m_scintilla->MarkerAdd(line, node_marker);
                m_scintilla->ScrollToLine(line);
                return;
            }

            // For icons, there is no bundle, just an image_ function
            std::ignore = func_name.Replace("bundle_", "image_");
            if (auto line = (to_int) m_view.FindLineContaining(func_name); line >= 0)
            {
                m_scintilla->MarkerDeleteAll(node_marker);
                m_scintilla->MarkerAdd(line, node_marker);
                m_scintilla->ScrollToLine(line);
                return;
            }

            // If all else fails, try just the name. This will also handle Python and Ruby panels
            std::ignore = func_name.Replace("image_", "");
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
    m_scintilla->StyleSetFont(wxSTC_STYLE_MAX, font);
}

// Builds a map from the spelling that appears in generated code to the wxWidgets class
// name. This mirrors Code::AddClassNameForLanguage() in code_add.cpp -- it must not use
// LanguageStrategy::MapClassName() because that adds module prefixes (e.g. "wx.adv.")
// that the generated code does not contain.
static std::unordered_map<std::string, std::string> BuildHotspotClassNameMap(GenLang language)
{
    std::unordered_map<std::string, std::string> class_map;

    const LanguageTraits* traits = GetLanguageTraits(language);
    if (traits == nullptr)
    {
        return class_map;
    }

    const std::string_view class_prefix = traits->class_prefix;

    for (const NodeDeclaration* decl: NodeCreation.get_NodeDeclarationArray())
    {
        if (decl == nullptr)
        {
            continue;
        }

        const std::string_view decl_name = decl->get_DeclName();
        if (!decl_name.starts_with("wx"))
        {
            continue;
        }

        std::string key(class_prefix);
        key += decl_name.substr(2);
        class_map.emplace(std::move(key), std::string(decl_name));
    }

    return class_map;
}

static const std::unordered_map<std::string, std::string>& GetHotspotClassNameMap(GenLang language)
{
    static const std::unordered_map<std::string, std::string> cpp_map =
        BuildHotspotClassNameMap(GenLang::cplusplus);
    static const std::unordered_map<std::string, std::string> python_map =
        BuildHotspotClassNameMap(GenLang::python);
    static const std::unordered_map<std::string, std::string> ruby_map =
        BuildHotspotClassNameMap(GenLang::ruby);

    switch (language)
    {
        case GenLang::python:
            return python_map;

        case GenLang::ruby:
            return ruby_map;

        default:
            return cpp_map;
    }
}

void CodeDisplay::ApplyClassHotspots()
{
    m_hotspots.clear();

    if (m_panel_type != GenLang::cplusplus && m_panel_type != GenLang::python &&
        m_panel_type != GenLang::ruby)
    {
        return;
    }

    const std::unordered_map<std::string, std::string>& class_map =
        GetHotspotClassNameMap(m_panel_type);
    if (class_map.empty())
    {
        return;
    }

    const std::string& buffer = m_view.GetBuffer();

    const bool is_python = (m_panel_type == GenLang::python);
    const bool is_ruby = (m_panel_type == GenLang::ruby);

    // Python class names contain a '.', Ruby class names contain ':', so those characters are
    // part of a token in those languages. Tokens never start with either character.
    auto is_token_char = [is_python, is_ruby](char ch) -> bool
    {
        if (ch == '_' || std::isalnum(static_cast<unsigned char>(ch)) != 0)
        {
            return true;
        }
        if (is_python)
        {
            return ch == '.';
        }
        if (is_ruby)
        {
            return ch == ':';
        }
        return false;
    };

    size_t pos = 0;
    const size_t buffer_size = buffer.size();
    while (pos < buffer_size)
    {
        const char start_ch = buffer[pos];
        if (start_ch != '_' && std::isalpha(static_cast<unsigned char>(start_ch)) == 0)
        {
            ++pos;
            continue;
        }

        const size_t token_start = pos;
        while (pos < buffer_size && is_token_char(buffer[pos]))
        {
            ++pos;
        }

        std::string_view candidate(buffer.data() + token_start, pos - token_start);

        // Cheap rejection of tokens that cannot be a wxWidgets class.
        if (is_ruby ? !candidate.starts_with("Wx") : !candidate.starts_with("wx"))
        {
            continue;
        }

        while (!candidate.empty())
        {
            const std::string lookup(candidate);
            if (const auto iter = class_map.find(lookup); iter != class_map.end())
            {
                m_hotspots.push_back(
                    Hotspot { .start = static_cast<int>(token_start),
                              .end = static_cast<int>(token_start + candidate.size()),
                              .class_name = iter->second });
                break;
            }

            if (m_panel_type == GenLang::cplusplus)
            {
                break;
            }

            // Strip the trailing member so that "wx.Button.GetValue" matches "wx.Button".
            const size_t sep_pos = is_ruby ? candidate.rfind("::") : candidate.rfind('.');
            if (sep_pos == std::string_view::npos)
            {
                break;
            }
            candidate = candidate.substr(0, sep_pos);
        }
    }

    if (m_hotspots.empty())
    {
        return;
    }

    // wxSTC_STYLE_MAX (255) is a free style -- the last predefined style is 39.
    const int hotspot_style = wxSTC_STYLE_MAX;
    m_scintilla->StyleSetFont(hotspot_style, m_scintilla->StyleGetFont(wxSTC_STYLE_DEFAULT));
    m_scintilla->StyleSetForeground(hotspot_style, *wxBLUE);
    m_scintilla->StyleSetUnderline(hotspot_style, true);
    m_scintilla->StyleSetHotSpot(hotspot_style, true);

    for (const auto& hotspot: m_hotspots)
    {
        m_scintilla->StartStyling(hotspot.start);
        m_scintilla->SetStyling(hotspot.end - hotspot.start, hotspot_style);
    }
}

void CodeDisplay::OnHotSpotClick(wxStyledTextEvent& event)
{
    const int pos = event.GetPosition();
    for (const auto& hotspot: m_hotspots)
    {
        if (pos >= hotspot.start && pos < hotspot.end)
        {
            wxGetFrame().ShowClassDocs(hotspot.class_name, m_panel_type);
            return;
        }
    }
}
