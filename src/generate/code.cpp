/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*

    Notes:

    The Eol() function will automatically append tabs if m_indent is greater than 0. That means you should *not* append tabs
   using += '/t', and you should be very cautious about using += '\n' instead of Eol() since the Eol() call will
   automatically append tabs if needed.

*/

#include <array>
#include <charconv>  // for std::to_chars()
#include <map>

#include <frozen/set.h>

#include "code.h"

#include "gen_common.h"       // Common component functions
#include "image_gen.h"        // Functions for generating embedded images
#include "mainapp.h"          // App class
#include "mainframe.h"        // MainFrame class
#include "node.h"             // Node class
#include "preferences.h"      // Prefs -- Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Miscellaneous utilities

using namespace code;

// clang-format off

// This maps just the prefix
static const view_map s_short_python_map
{
    { "wxAUI_", "wx.aui."},
    { "wxCAL_", "wx.adv."},
    { "wxEL_", "wx.adv."},
    { "wxDATAVIEW_", "wx.dataview."},
    { "wxDV_", "wx.dataview."},
    { "wxPG_", "wx.propgrid."},
    { "wxRE_", "wx.richtext."},
    { "wxRIBBON", "wx.ribbon."},
    { "wxSTC_", "wx.stc."},
    { "wxWIZARD_", "wx.adv."},
    { "wxGRID_", "wx.grid."},

    { "wxEVT_DATAVIEW_", "wx.dataview."},
    { "wxEVT_TREELIST_", "wx.dataview." },
    { "wxTL_", "wx.dataview." },

    { "wxEVT_DATE_", "wx.adv."},
    { "wxEVT_GRID_", "wx.grid." },
    { "wxEVT_RIBBON", "wx.ribbon." },
    { "wxEVT_STC_", "wx.stc." },
    { "wxEVT_TIME_", "wx.adv."},
    { "wxEVT_WIZARD_", "wx.adv."},

};

const view_map g_map_python_prefix
{
    { "wxAnimationCtrl",        "wx.adv."},
    { "wxAuiNotebook",          "wx.aui."},
    { "wxAuiToolBar",           "wx.aui."},
    { "wxAuiToolBarItem",       "wx.aui."},
    { "wxBannerWindow",         "wx.adv."},
    { "wxCalendarCtrl",         "wx.adv."},
    { "wxCommandLinkButton",    "wx.adv."},
    { "wxDatePickerCtrl",       "wx.adv."},
    { "wxEditableListBox",      "wx.adv."},
    { "wxGenericAnimationCtrl", "wx.adv."},
    { "wxHtmlWindow",           "wx.html."},
    { "wxSimpleHtmlListBox",    "wx.html."},
    { "wxHyperlinkCtrl",        "wx.adv."},
    { "wxRichTextCtrl",         "wx.richtext."},
    { "wxStyledTextCtrl",       "wx.stc."},
    { "wxTimePickerCtrl",       "wx.adv."},
    { "wxStyledTextCtrl",       "wx.stc."},
    { "wxWebView",              "wx.html2."},
    { "wxWizard",               "wx.adv."},
    { "wxWizardPageSimple",     "wx.adv."},
    { "wxRibbonBar",            "wx.ribbon."},
    { "wxRibbonButtonBar",      "wx.ribbon."},
    { "wxRibbonPage",           "wx.ribbon."},
    { "wxRibbonPanel",          "wx.ribbon."},
    { "wxRibbonToolBar",        "wx.ribbon."},
    { "wxRibbonGallery",        "wx.ribbon."},
    { "wxBitmapComboBox",       "wx.adv."},
    { "wxDataViewCtrl",         "wx.dataview."},
    { "wxDataViewListCtrl",     "wx.dataview."},
    { "wxDataViewTreeCtrl",     "wx.dataview."},
    { "wxTreeListCtrl",         "wx.dataview."},
    { "wxGrid",                 "wx.grid."},
    { "wxPropertyGridManager",  "wx.propgrid."},
    { "wxPropertyGrid",         "wx.propgrid."},
    { "wxPropertySheetDialog",  "wx.adv."},
    { "wxNullAnimation",        "wx.adv."},

    { "wxEL_ALLOW_NEW",         "wx.adv."},
    { "wxEL_ALLOW_EDIT",        "wx.adv."},
    { "wxEL_ALLOW_DELETE",      "wx.adv."},
    { "wxEL_NO_REORDER",        "wx.adv."},
    { "wxHL_DEFAULT_STYLE",     "wx.adv."},
    { "wxHL_ALIGN_LEFT",        "wx.adv."},
    { "wxHL_ALIGN_RIGHT",       "wx.adv."},
    { "wxHL_ALIGN_CENTRE",      "wx.adv."},
    { "wxHL_CONTEXTMENU",       "wx.adv."},
    { "wxAC_DEFAULT_STYLE",     "wx.adv."},
    { "wxAC_NO_AUTORESIZE",     "wx.adv."},

    { "wxHLB_DEFAULT_STYLE", "wx.html."},
    { "wxHLB_MULTIPLE", "wx.html."},

    { "wxHW_SCROLLBAR_AUTO", "wx.html."},
    { "wxHW_NO_SELECTION", "wx.html."},
    { "wxHW_NO_SELECTION", "wx.html."},

    { "wxEVT_PG_CHANGED", "wx.propgrid." },
    { "wxEVT_PG_CHANGING", "wx.propgrid." },
    { "wxEVT_PG_SELECTED", "wx.propgrid." },
    { "wxEVT_PG_HIGHLIGHTED", "wx.propgrid." },
    { "wxEVT_PG_RIGHT_CLICK", "wx.propgrid." },
    { "wxEVT_PG_DOUBLE_CLICK", "wx.propgrid." },
    { "wxEVT_PG_ITEM_COLLAPSED", "wx.propgrid." },
    { "wxEVT_PG_ITEM_EXPANDED", "wx.propgrid." },
    { "wxEVT_PG_LABEL_EDIT_BEGIN", "wx.propgrid." },
    { "wxEVT_PG_LABEL_EDIT_ENDING", "wx.propgrid." },
    { "wxEVT_PG_COL_BEGIN_DRAG", "wx.propgrid." },
    { "wxEVT_PG_COL_DRAGGING", "wx.propgrid." },
    { "wxEVT_PG_COL_END_DRAG", "wx.propgrid." },

    { "wxPropertyCategory", "wx.propgrid." },
    { "wxBoolProperty", "wx.propgrid." },
    { "wxColourProperty", "wx.propgrid." },
    { "wxCursorProperty", "wx.propgrid." },
    { "wxDateProperty", "wx.propgrid." },
    { "wxDirProperty", "wx.propgrid." },
    { "wxEditEnumProperty", "wx.propgrid." },
    { "wxEnumProperty", "wx.propgrid." },
    { "wxFileProperty", "wx.propgrid." },
    { "wxFlagsProperty", "wx.propgrid." },
    { "wxFloatProperty", "wx.propgrid." },
    { "wxFontProperty", "wx.propgrid." },
    { "wxImageFileProperty", "wx.propgrid." },
    { "wxIntProperty", "wx.propgrid." },
    { "wxLongStringProperty", "wx.propgrid." },
    { "wxMultiChoiceProperty", "wx.propgrid." },
    { "wxStringProperty", "wx.propgrid." },
    { "wxSystemColourProperty", "wx.propgrid." },
    { "wxUIntProperty", "wx.propgrid." },

    { "wxRibbonAUIArtProvider", "wx.ribbon." },
    { "wxRibbonDefaultArtProvider", "wx.ribbon." },
    { "wxRibbonMSWArtProvider", "wx.ribbon." },

    // This doesn't get created as a class, so we have to add it as if it was a constant.
    { "wxWebView", "wx.html2."},
    { "wxWebViewBackendDefault", "wx.html2."},


};

static const view_map s_short_ruby_map
{
    { "wxAUI_", "Wx::AUI::"},
    { "wxPG_", "Wx::PG::"},
    { "wxRE_", "Wx::RTC::"},
    { "wxRIBBON", "Wx::RBN::"},
    { "wxSTC_", "Wx::STC::"},
    { "wxGRID_", "Wx::GRID::"},

};

const view_map g_map_ruby_prefix
{
    { "wxAuiNotebook",    "Wx::AUI::" },
    { "wxAuiToolBar",     "Wx::AUI::" },
    { "wxAuiToolBarItem", "Wx::AUI::" },

    { "wxGrid", "Wx::GRID::"},

    { "wxSimpleHtmlListBox", "Wx::HTML::"},
    { "wxHtmlWindow",        "Wx::HTML::"},

    { "wxEVT_PG_CHANGED",           "Wx::PG::" },
    { "wxEVT_PG_CHANGING",          "Wx::PG::" },
    { "wxEVT_PG_SELECTED",          "Wx::PG::" },
    { "wxEVT_PG_HIGHLIGHTED",       "Wx::PG::" },
    { "wxEVT_PG_RIGHT_CLICK",       "Wx::PG::" },
    { "wxEVT_PG_DOUBLE_CLICK",      "Wx::PG::" },
    { "wxEVT_PG_ITEM_COLLAPSED",    "Wx::PG::" },
    { "wxEVT_PG_ITEM_EXPANDED",     "Wx::PG::" },
    { "wxEVT_PG_LABEL_EDIT_BEGIN",  "Wx::PG::" },
    { "wxEVT_PG_LABEL_EDIT_ENDING", "Wx::PG::" },
    { "wxEVT_PG_COL_BEGIN_DRAG",    "Wx::PG::" },
    { "wxEVT_PG_COL_DRAGGING",      "Wx::PG::" },
    { "wxEVT_PG_COL_END_DRAG",      "Wx::PG::" },

    { "wxPropertyCategory",     "Wx::PG::" },
    { "wxBoolProperty",         "Wx::PG::" },
    { "wxColourProperty",       "Wx::PG::" },
    { "wxCursorProperty",       "Wx::PG::" },
    { "wxDateProperty",         "Wx::PG::" },
    { "wxDirProperty",          "Wx::PG::" },
    { "wxEditEnumProperty",     "Wx::PG::" },
    { "wxEnumProperty",         "Wx::PG::" },
    { "wxFileProperty",         "Wx::PG::" },
    { "wxFlagsProperty",        "Wx::PG::" },
    { "wxFloatProperty",        "Wx::PG::" },
    { "wxFontProperty",         "Wx::PG::" },
    { "wxImageFileProperty",    "Wx::PG::" },
    { "wxIntProperty",          "Wx::PG::" },
    { "wxLongStringProperty",   "Wx::PG::" },
    { "wxMultiChoiceProperty",  "Wx::PG::" },
    { "wxStringProperty",       "Wx::PG::" },
    { "wxSystemColourProperty", "Wx::PG::" },
    { "wxUIntProperty",         "Wx::PG::" },

    { "wxRibbonBar",                "Wx::RBN::"},
    { "wxRibbonButtonBar",          "Wx::RBN::"},
    { "wxRibbonPage",               "Wx::RBN::"},
    { "wxRibbonPanel",              "Wx::RBN::"},
    { "wxRibbonToolBar",            "Wx::RBN::"},
    { "wxRibbonGallery",            "Wx::RBN::"},
    { "wxRibbonAUIArtProvider",     "Wx::RBN::"},
    { "wxRibbonArtProvider",        "Wx::RBN::"},
    { "wxRibbonMSWArtProvider",     "Wx::RBN::"},
    { "wxRibbonDefaultArtProvider", "Wx::RBN::"},

    { "wxRichTextCtrl",   "Wx::RTC::"},
    { "wxStyledTextCtrl", "Wx::STC::"},

};

const view_map g_map_fortran_prefix
{
};

const view_map g_map_haskell_prefix
{
};

const view_map g_map_lua_prefix
{
};

const view_map g_map_perl_prefix
{
};

const view_map g_map_rust_prefix
{
};

static const view_map s_short_fortran_map
{
};

static const view_map s_short_haskell_map
{
};

static const view_map s_short_lua_map
{
};

static const view_map s_short_perl_map
{
};

static const view_map s_short_rust_map
{
};

constexpr auto set_perl_constants = frozen::make_set<std::string_view>({

    "wxNullBitmap",
    "wxITEM_CHECK",
    "wxITEM_DROPDOWN",
    "wxITEM_NORMAL",
    "wxITEM_RADIO",
    "wxID_ANY",
    "wxVERTICAL",
    "wxHORIZONTAL",
    "wxBOTH",

});

// clang-format on

std::string_view GetLanguagePrefix(tt_string_view candidate, GenLang language)
{
    const view_map* prefix_list;
    const view_map* global_list;

    switch (language)
    {
        case GEN_LANG_PERL:
            prefix_list = &s_short_perl_map;
            global_list = &g_map_perl_prefix;
            break;

        case GEN_LANG_PYTHON:
            prefix_list = &s_short_python_map;
            global_list = &g_map_python_prefix;
            break;

        case GEN_LANG_RUBY:
            prefix_list = &s_short_ruby_map;
            global_list = &g_map_ruby_prefix;
            break;

        case GEN_LANG_RUST:
            return "wx::";

        case GEN_LANG_CPLUSPLUS:
            FAIL_MSG("Don't call GetLanguagePrefix() for C++ code!");
            return {};

#if GENERATE_NEW_LANG_CODE
        case GEN_LANG_FORTRAN:
            prefix_list = &s_short_fortran_map;
            global_list = &g_map_fortran_prefix;
            break;

        case GEN_LANG_HASKELL:
            prefix_list = &s_short_haskell_map;
            global_list = &g_map_haskell_prefix;
            break;

        case GEN_LANG_LUA:
            prefix_list = &s_short_lua_map;
            global_list = &g_map_lua_prefix;
            break;
#endif  // GENERATE_NEW_LANG_CODE

        default:
            FAIL_MSG("Unknown language");
            return {};
    }

    for (auto& iter_prefix: *prefix_list)
    {
        if (candidate.starts_with(iter_prefix.first))
        {
            return iter_prefix.second;
        }
    }

    if (auto result = global_list->find(candidate); result != global_list->end())
    {
        return result->second;
    }

    return {};
}

Code::Code(Node* node, GenLang language)
{
    Init(node, language);
}

void Code::Init(Node* node, GenLang language)
{
    m_node = node;
    m_language = language;
    if (language == GEN_LANG_CPLUSPLUS)
    {
        m_language_wxPrefix = "wx";
        m_break_length = Project.as_size_t(prop_cpp_line_length);
        // Always assume C++ code has one tab at the beginning of the line
        m_break_length -= m_indent_size;
    }
    else if (language == GEN_LANG_PERL)
    {
        m_indent_size = 4;
        m_language_wxPrefix = "Wx::";
        m_break_length = Project.as_size_t(prop_perl_line_length);
        // Always assume Perl code has one tab at the beginning of the line
        m_break_length -= m_indent_size;
    }
    else if (language == GEN_LANG_PYTHON)
    {
        m_language_wxPrefix = "wx.";
        m_break_length = Project.as_size_t(prop_python_line_length);
        // Always assume Python code has two tabs at the beginning of the line
        m_break_length -= (m_indent_size * 2);
    }
    else if (language == GEN_LANG_RUBY)
    {
        m_indent_size = 2;
        m_language_wxPrefix = "Wx::";
        m_break_length = Project.as_size_t(prop_ruby_line_length);
        // Always assume Ruby code has two tabs at the beginning of the line
        m_break_length -= (m_indent_size * 2);
    }
    else if (language == GEN_LANG_RUST)
    {
        m_language_wxPrefix = "wx.";
        m_break_length = 100;
        // Always assume Rust code has one tab at the beginning of the line
        m_break_length -= m_indent_size;
    }

#if GENERATE_NEW_LANG_CODE
    else if (language == GEN_LANG_FORTRAN)
    {
        // REVIEW: [Randalphwa - 11-24-2024] wxFortran3 doesn't exist yet, but I'm guessing that
        // there will be a wx derived type with members accessed using %.
        m_language_wxPrefix = "wx%";
        m_break_length = Project.as_size_t(prop_fortran_line_length);
        m_break_length -= m_indent_size;
    }
    else if (language == GEN_LANG_HASKELL)
    {
        m_language_wxPrefix = "wx";  // wxHaskell doesn't change wxWidgets naming
        m_break_length = Project.as_size_t(prop_haskell_line_length);
        m_break_length -= m_indent_size;
    }
    else if (language == GEN_LANG_LUA)
    {
        // Lua simply uses a "wx." prefix before the normal wxWidgets "wx" prefix
        m_language_wxPrefix = "wx.wx";
        m_break_length = Project.as_size_t(prop_lua_line_length);
        m_break_length -= m_indent_size;
    }
#endif  // GENERATE_NEW_LANG_CODE

    else
    {
        FAIL_MSG("Unknown language");
        m_language_wxPrefix = "wx";
        m_break_length = 90;
        // Always assume code has one tab at the beginning of the line
        m_break_length -= m_indent_size;
    }
    m_break_at = m_break_length;

    // Reserve large enough for multiple lines -- goal is to avoid multiple reallocations
    reserve(256);
}

Code& Code::CheckLineLength(size_t next_str_size)
{
    if (m_indent)
        next_str_size += (m_indent * m_indent_size);

    if (m_auto_break && size() > m_minium_length && size() + next_str_size > m_break_at)
    {
        if (back() == ' ')
            pop_back();
        Eol().Tab();
    }
    return *this;
}

Code& Code::CheckLineLength(GenEnum::PropName next_prop_name)
{
    return CheckLineLength(m_node->as_string(next_prop_name).size());
}

Code& Code::Eol(int flag)
{
    if (flag == eol_if_empty)
    {
        if (size())
        {
            *this += '\n';
        }
    }
    else if (flag == eol_if_needed)
    {
        if (size() && back() != '\n')
        {
            // Check for single and nested indents
            if (!ends_with("\n\t") && !ends_with("\n\t\t"))
            {
                return Eol();
            }
        }
    }
    else
    {
        if (size() && back() == ' ')
            pop_back();
        *this += '\n';
    }

#if 0
    // REVIEW: [Randalphwa - 08-26-2024] m_within_braces is no longer set to true in OpenBrace()
    if (m_within_braces && is_cpp() && size() && back() != '\t')
    {
        *this += '\t';
        if (m_within_font_braces)
            *this += '\t';
    }
    else if (m_indent > 0 && (empty() || back() != '\t'))
    {
        Tab(m_indent);
    }
#else
    if (m_indent > 0 && (empty() || back() != '\t'))
    {
        Tab(m_indent);
    }
#endif

    if (m_auto_break)
    {
        m_break_at = size() + m_break_length;
        m_minium_length = size() + 10;
    }
    return *this;
}

Code& Code::OpenBrace(bool all_languages)
{
    if (!all_languages && !is_cpp() && !is_perl() && !is_rust())
    {
        return *this;
    }

    if (is_cpp() || is_perl() || is_rust())
    {
        // Perl and Rust place the brace at the end of the function. wxUiEditor
        // follows CppCoreGuidelines and places the brace on the next line for
        // C++ code.
        if (is_cpp())
        {
            Eol(eol_if_needed);
        }
        else if (is_perl())
        {
            if (size() && back() != ' ')
                *this += ' ';
        }
        *this += "{";
        Indent();
        Eol();
        // m_within_braces = true;
    }
    else
    {
        Indent();
        Eol(eol_if_needed);
    }

    return *this;
}

Code& Code::CloseBrace(bool all_languages, bool close_ruby)
{
    if (!all_languages && !is_cpp() && !is_perl() && !is_rust())
    {
        return *this;
    }

    // Ensure there are no trailing tabs
    while (size() && tt::is_whitespace(back()))
        pop_back();
    Unindent();

    if (is_cpp() || is_perl() || is_rust())
    {
        m_within_braces = false;
        Eol();
        *this += "}";
    }
    if (all_languages && is_ruby() && close_ruby)
    {
        Eol();
        *this += "end";
    }

    return *this;
}

void Code::OpenFontBrace()
{
    // REVIEW: [Randalphwa - 09-26-2024] Will this be needed for wxPerl as well?
    if (is_cpp())
    {
        m_within_font_braces = true;
        Eol(eol_if_needed);
        *this += '{';
        Indent();
        Eol();
    }
}

void Code::CloseFontBrace()
{
    if (is_cpp())
    {
        while (size() && tt::is_whitespace(back()))
            pop_back();
        Unindent();
        m_within_font_braces = false;
        Eol().Str("}").Eol();
    }
}

Code& Code::AddAuto()
{
    if (is_local_var())
    {
        if (is_cpp())
        {
            *this += "auto* ";
        }
        else if (is_perl())
        {
            *this += "my $";
        }
        else if (is_python())
        {
            *this += "self.";
        }
        else if (is_ruby())
        {
            *this += "@";
        }
        else if (is_rust())
        {
            *this += "let ";
        }
#if GENERATE_NEW_LANG_CODE

        else if (is_fortran())
        {
            *this += "type(";
        }
        else if (is_haskell())
        {
            *this += "let ";
        }
        else if (is_lua())
        {
            *this += "local ";
        }
#endif  // GENERATE_NEW_LANG_CODE
    }
    return *this;
}

void Code::InsertLineBreak(size_t cur_pos)
{
    ASSERT(cur_pos > 1 && cur_pos <= size());
    if (at(cur_pos - 1) == ' ')
    {
        at(cur_pos - 1) = '\n';
        insert(cur_pos, "\t");
    }
    else
    {
        insert(cur_pos, "\n");
    }
    m_break_at = cur_pos + m_break_length;
    m_minium_length = cur_pos + 10;
}

Code& Code::Tab(int tabs)
{
    while (tabs)
    {
        *this += '\t';
        --tabs;
    }
    return *this;
}

Code& Code::as_string(PropName prop_name)
{
    if (prop_name == prop_id)
    {
        auto result = m_node->getPropId();
        CheckLineLength(result.size());

        // For Ruby, if it doesn't start with 'wx' then assume it is a global with a '$' prefix
        if (is_ruby() && !result.is_sameprefix("wx"))
        {
            *this << '$' << result;
            return *this;
        }
        else if (!is_cpp() && !is_perl())
        {
            result.Replace("wx", m_language_wxPrefix);
        }
        *this += result;
        return *this;
    }

    return Add(m_node->as_string(prop_name));
}

Code& Code::AddType(tt_string_view text)
{
    if (is_cpp() || text.size() < 3)
    {
        CheckLineLength(text.size());
        *this += text;
    }
    else if (is_ruby())
    {
        auto new_text = ConvertToUpperSnakeCase(text.substr(2));
        CheckLineLength(sizeof("Wx::") + new_text.size());
        *this << "Wx::" << new_text;
    }
    else
    {
        CheckLineLength(m_language_wxPrefix.size() + text.size() - 2);
        *this << m_language_wxPrefix << text.substr(2);
    }

    return *this;
}

Code& Code::AddConstant(tt_string_view text)
{
    if (is_cpp() || is_perl())
    {
        CheckLineLength(text.size());
        *this += text;
        return *this;
    }
    return Add(text);
}

Code& Code::Add(tt_string_view text)
{
    bool old_linebreak = m_auto_break;
    if (is_ruby() && text.size())
    {
        // Ruby doesn't like breaking the parenthesis for a function call onto the next line,
        // or the .new function
        if (text.front() == '.' || text.front() == '(')
        {
            old_linebreak = m_auto_break;
            m_auto_break = false;
        }
    }

    if (is_cpp() || text.size() < 3)
    {
        CheckLineLength(text.size());
        *this += text;
    }
    else
    {
        if (is_ruby())
        {
            if (text == "wxEmptyString")
            {
                // wxRuby prefers ('') for an empty string instead of the expected Wx::empty_string
                *this += "('')";
                return *this;
            }
            else if (text == "wxDefaultCoord")
            {
                *this += "Wx::DEFAULT_COORD";
                return *this;
            }
            else if (text == "wxDefaultSize")
            {
                *this += "Wx::DEFAULT_SIZE";
                return *this;
            }
            else if (text == "wxDefaultPosition")
            {
                *this += "Wx::DEFAULT_POSITION";
                return *this;
            }
            else if (text == "wxNullBitmap")
            {
                *this += "Wx::NULL_BITMAP";
                return *this;
            }
            else if (text == "wxNullAnimation")
            {
                *this += "Wx::NULL_ANIMATION";
                return *this;
            }
        }

        if (text.find('|') != tt::npos)
        {
            bool initial_combined_value_set = false;
            tt_view_vector multistr(text, "|", tt::TRIM::both);
            for (auto& iter: multistr)
            {
                if (iter.empty())
                    continue;
                if (initial_combined_value_set)
                    *this += '|';
                if (iter.is_sameprefix("wx") && !is_cpp())
                {
                    if (is_perl() && (HasPerlMapConstant(text) || set_perl_constants.contains(text)))
                    {
                        CheckLineLength(text.size());
                        *this += text;
                        initial_combined_value_set = true;
                        continue;
                    }

                    if (std::string_view language_prefix = GetLanguagePrefix(text, m_language); language_prefix.size())
                    {
                        // Some languages will have a module added after their standard prefix.
                        CheckLineLength(language_prefix.size() + iter.size() - 2);
                        *this << language_prefix << iter.substr(2);
                    }
                    else
                    {
                        // If there was no sub-language module added (e.g., wx.aui. for
                        // Python), then use the default language prefix.
                        CheckLineLength(m_language_wxPrefix.size() + iter.size() - 2);
                        *this << m_language_wxPrefix << iter.substr(2);
                    }
                }
                else
                {
                    CheckLineLength(iter.size());
                    *this += iter;
                }
                initial_combined_value_set = true;
            }
        }
        else if (text.is_sameprefix("wx") && !is_cpp())
        {
            if (is_perl())
            {
                if (HasPerlMapConstant(text) || set_perl_constants.contains(text))
                {
                    CheckLineLength(text.size());
                    *this += text;
                    return *this;
                }
                else if (text == "wxEmptyString")
                {
                    *this << "\"\"";
                    return *this;
                }

                if (std::string_view language_prefix = GetLanguagePrefix(text, m_language); language_prefix.size())
                {
                    CheckLineLength(language_prefix.size() + text.size() - 2);
                    *this << language_prefix << text.substr(2);
                }
                else
                {
                    CheckLineLength(m_language_wxPrefix.size() + text.size() - 2);
                    *this << m_language_wxPrefix << text.substr(2);
                }
            }
            else if (std::string_view language_prefix = GetLanguagePrefix(text, m_language); language_prefix.size())
            {
                CheckLineLength(language_prefix.size() + text.size() - 2);
                *this << language_prefix << text.substr(2);
            }
            else
            {
                CheckLineLength(m_language_wxPrefix.size() + text.size() - 2);
                *this << m_language_wxPrefix << text.substr(2);
            }
        }
        else
        {
            CheckLineLength(text.size());
            *this += text;
        }
    }

    // In case linebreak was shut off
    m_auto_break = old_linebreak;

    return *this;
}

Code& Code::TrueFalseIf(GenEnum::PropName prop_name)
{
    if (m_node->as_bool(prop_name))
        return True();
    else
        return False();
}

Code& Code::AddConstant(GenEnum::PropName prop_name, tt_string_view short_name)
{
    return Add(m_node->as_constant(prop_name, short_name));
}

Code& Code::Function(tt_string_view text, bool add_operator)
{
    if (!add_operator)
    {
        if (text.is_sameprefix("wx") && (is_python() || is_ruby() || is_perl()))
        {
            if (is_ruby())
                *this << m_language_wxPrefix << ConvertToSnakeCase(text.substr(sizeof("wx") - 1));
            else
                *this << m_language_wxPrefix << text.substr(sizeof("wx") - 1);
        }
        else
        {
            *this += text;
        }
    }
    else
    {
        if (is_cpp())
        {
            *this << "->" << text;
        }
        else if (is_ruby())
        {
            // Check for a preceeding empty "()" and remove it if found
            if (ends_with("())"))
            {
                resize(size() - 2);
            }

            *this << '.';
            if (text.is_sameprefix("wx"))
            {
                *this << m_language_wxPrefix << text.substr(sizeof("wx") - 1);
            }
            else
            {
                *this += ConvertToSnakeCase(text);
            }
        }
        else if (is_python())
        {
            *this << '.';
            if (text.is_sameprefix("wx"))
            {
                *this << m_language_wxPrefix << text.substr(sizeof("wx") - 1);
            }
            else
            {
                *this += text;
            }
        }
#if GENERATE_NEW_LANG_CODE
        else if (is_lua())
        {
            *this << '.' << text;
        }
#endif
        else
        {
            *this << "->" << text;
        }
    }
    return *this;
}

Code& Code::ClassMethod(tt_string_view function_name)
{
    if (is_cpp())
    {
        *this += "::";
    }
    else
    {
        *this += '.';
    }
    if (is_ruby())
    {
        *this += ConvertToSnakeCase(function_name);
    }
    else
    {
        *this += function_name;
    }

    return *this;
}

Code& Code::VariableMethod(tt_string_view function_name)
{
    *this += '.';
    if (is_ruby())
    {
        *this += ConvertToSnakeCase(function_name);
    }
    else
    {
        *this += function_name;
    }

    return *this;
}

Code& Code::FormFunction(tt_string_view text)
{
    if (is_python())
    {
        *this += "self.";
    }
    else if (is_ruby())
    {
        *this += ConvertToSnakeCase(text);
        return *this;
    }
    else if (is_perl())
    {
        *this += "$self->";
    }
#if GENERATE_NEW_LANG_CODE

    else if (is_lua())
    {
        *this += "this:";
    }
#endif

    *this += text;
    return *this;
}

Code& Code::Class(tt_string_view text)
{
    if (is_cpp())
    {
        *this += text;
    }
    else if (is_python())
    {
        if (text.is_sameprefix("wx"))
        {
            *this << "wx." << text.substr(2);
        }
        else
        {
            *this += text;
        }
    }
    else if (is_ruby() || is_perl())
    {
        if (text.is_sameprefix("wx"))
        {
            *this << "Wx::" << text.substr(2);
        }
        else
        {
            *this += text;
        }
    }
#if GENERATE_NEW_LANG_CODE

    else if (is_fortran())
    {
        if (text.is_sameprefix("wx"))
        {
            *this << "wx%" << text;
        }
        else
        {
            *this += text;
        }
    }
    else if (is_lua())
    {
        if (text.is_sameprefix("wx"))
        {
            *this << "wx." << text;
        }
        else
        {
            *this += text;
        }
    }
    else if (is_haskell())
    {
        *this += text;
    }
#endif

    return *this;
}

Code& Code::Object(tt_string_view class_name)
{
    if (is_cpp())
    {
        *this += class_name;
    }
    else if (is_perl())
    {
        if (class_name.is_sameprefix("wx"))
        {
            *this << "Wx::" << class_name.substr(2);
        }
        else
        {
            *this += class_name;
        }
        *this << "->new";
    }
    else if (is_python())
    {
        if (class_name.is_sameprefix("wx"))
        {
            *this << "wx." << class_name.substr(2);
        }
        else
        {
            *this += class_name;
        }
    }
    else if (is_ruby())
    {
        if (class_name.is_sameprefix("wx"))
        {
            *this << "Wx::" << class_name.substr(2);
        }
        else
        {
            *this += class_name;
        }
        *this << ".new";
    }
    *this << '(';

    return *this;
}

Code& Code::CreateClass(bool use_generic, tt_string_view override_name, bool assign)
{
    if (assign)
    {
        *this += " = ";
    }
    if (is_cpp())
    {
        *this += "new ";
        if (m_node->hasValue(prop_subclass) && !m_node->as_string(prop_subclass).starts_with("wxGeneric"))
        {
            *this += m_node->as_string(prop_subclass);
            *this += '(';
            if (m_node->hasValue(prop_subclass_params))
            {
                *this += m_node->as_string(prop_subclass_params);
                RightTrim();
                if (back() != ',')
                    Comma();
                else
                    *this += ' ';
            }
            return *this;
        }
    }

    tt_string class_name;
    if (override_name.empty())
        class_name = m_node->declName();
    else
        class_name = override_name;
    if (use_generic)
    {
        class_name.Replace("wx", "wxGeneric");
    }
    else if (m_node->isGen(gen_BookPage))
    {
        class_name = "wxPanel";
    }

    if (is_cpp())
        *this += class_name;
    else
    {
        if (class_name.is_sameprefix("wx") && !is_cpp())
        {
            if (std::string_view language_prefix = GetLanguagePrefix(class_name, m_language); language_prefix.size())
            {
                *this << language_prefix << class_name.substr(2);
            }
            else
            {
                *this << m_language_wxPrefix << class_name.substr(2);
            }
        }
        else
        {
            *this += class_name;
        }

        if (is_perl())
        {
            *this += "->new";
        }
        else if (is_ruby())
        {
            *this += ".new";
        }
        else if (is_rust())
        {
            *this += "::new";
        }
    }

    *this += '(';
    if (m_node->hasValue(prop_subclass_params))
    {
        *this += m_node->as_string(prop_subclass_params);
        RightTrim();
        if (back() != ',')
            Comma();
        else
            *this += ' ';
    }
    return *this;
}

Code& Code::Assign(tt_string_view class_name)
{
    *this += " = ";
    if (class_name.empty())
        return *this;

    if (is_cpp())
    {
        *this << "new " << class_name << ';';
    }
    else
    {
        *this << m_language_wxPrefix << class_name.substr(2);
        if (is_ruby())
        {
            *this << ".new";
        }
    }

    return *this;
}

Code& Code::EndFunction()
{
    if (is_ruby() && back() == '(')
    {
        // Ruby style guidelines recommend not using empty parentheses
        pop_back();
    }
    else
    {
        *this += ')';
    }

    if (is_cpp() || is_perl() || is_rust())
    {
        *this += ';';
    }
    return *this;
}

Code& Code::NodeName(Node* node)
{
    if (!node)
        node = m_node;
    auto node_name = node->getNodeName(get_language());
    if (is_python() && !node->isForm() && !node->isLocal() && !node_name.starts_with("self."))
    {
        *this += "self.";
    }
    else if (is_ruby() && !node->isForm() && !node->isLocal() && node_name[0] != '@')
    {
        *this += "@";
    }
    else if (is_perl() && !node->isForm())
    {
        if (node->isLocal())
        {
            if (!node_name.starts_with("$") && (size() < 1 || back() != '$'))
            {
                *this += "$";
            }
            *this += node_name;
            return *this;
        }
        else
        {
            if (node_name.starts_with("$self->"))
            {
                *this += node_name;
                return *this;
            }
            *this += "$self->{";
            if (node_name.is_sameprefix("$"))
            {
                *this += node_name.subview(1);
            }
            else
            {
                *this += node_name;
            }
            *this += "}";
            return *this;
        }
    }
#if GENERATE_NEW_LANG_CODE
    else if (is_lua() && !node->isForm() && !node->isLocal() && !node_name.starts_with("self."))
    {
        *this += "self.";
    }
#endif

    *this += node_name;
    return *this;
}

Code& Code::VarName(tt_string_view var_name, bool class_access)
{
    if (is_cpp())
    {
        // If a Ruby user added this, then it must be removed for valid C++ code
        if (var_name.is_sameprefix("@"))
            *this += var_name.subview(1);
        else
            *this += var_name;
        return *this;
    }

    if (class_access)
    {
        if (is_python())
            *this += "self.";
        else if (is_ruby())
            *this += "@";
        else if (is_perl())
        {
            Str("$self->{");
            if (var_name.is_sameprefix("m_"))
                *this += var_name.subview(2);
            else
                *this += var_name;
            *this += "}";
            return *this;
        }
    }

    else if (is_perl())
    {
        *this += "$";
    }

    if (var_name.is_sameprefix("m_"))
        *this += var_name.subview(2);
    else
        *this += var_name;

    return *this;
}

Code& Code::ParentName()
{
    NodeName(m_node->getParent());
    return *this;
}

bool Code::is_local_var() const
{
    return m_node->isLocal();
}

bool Code::hasValue(GenEnum::PropName prop_name) const
{
    return m_node->hasValue(prop_name);
}

int Code::IntValue(GenEnum::PropName prop_name) const
{
    return m_node->as_int(prop_name);
}

bool Code::PropContains(GenEnum::PropName prop_name, tt_string_view text) const
{
    return m_node->as_string(prop_name).contains(text);
}

size_t Code::PropSize(GenEnum::PropName prop_name) const
{
    return m_node->as_string(prop_name).size();
}

// clang-format off

// List of valid component parent types
static constexpr GenType s_GenParentTypes[] = {

    type_aui_toolbar,
    type_auinotebook,
    type_bookpage,
    type_choicebook,
    type_container,
    type_listbook,
    type_notebook,
    type_panel,
    type_propgridpage,
    type_ribbonpanel,
    type_simplebook,
    type_splitter,
    type_toolbar,
    type_wizardpagesimple,

};

// clang-format on

Code& Code::ValidParentName()
{
    auto parent = m_node->getParent();
    while (parent)
    {
        if (parent->isSizer())
        {
            if (parent->isStaticBoxSizer())
            {
                NodeName(parent);
                if (is_ruby())
                {
                    Function("GetStaticBox");
                }
                else
                {
                    Function("GetStaticBox()");
                }
                return *this;
            }
        }
        else if (parent->isForm())
        {
            if (is_cpp())
            {
                *this += "this";
            }
            else if (is_perl())
            {
                *this += "$self";
            }
            else
            {
                *this += "self";
            }
            return *this;
        }

        for (auto iter: s_GenParentTypes)
        {
            if (parent->isType(iter))
            {
                NodeName(parent);
                if (parent->isGen(gen_wxCollapsiblePane))
                {
                    Function("GetPane()");
                }
                return *this;
            }
        }
        parent = parent->getParent();
    }

    ASSERT_MSG(parent, tt_string() << m_node->getNodeName() << " has no parent!");
    return *this;
}

Code& Code::QuotedString(GenEnum::PropName prop_name)
{
    if (!m_node->hasValue(prop_name))
    {
        if (is_cpp())
        {
            CheckLineLength(sizeof("wxEmptyString"));
            *this += "wxEmptyString";
        }
        else if (is_ruby())
        {
            *this += "''";
        }
        else
        {
            *this += "\"\"";
        }
        return *this;
    }
    else
    {
        return QuotedString(m_node->as_string(prop_name));
    }
}

Code& Code::QuotedString(tt_string_view text)
{
    auto cur_pos = this->size();

    if (Project.as_bool(prop_internationalize))
    {
        if (is_cpp())
        {
            *this += "_(";
        }
        else
        {
            // REVIEW: [Randalphwa - 08-12-2023] Need to verify this for each language. I'm
            // dubious this is correct for all languages.
            Function("wxGetTranslation");
        }
    }

    // This is only used by C++, but we need to know if it was set in order to generate closing parenthesis.
    bool has_utf_char = false;
    if (is_cpp())
    {
        for (auto iter: text)
        {
            if (iter < 0)
            {
                has_utf_char = true;
                break;
            }
        }

        if (has_utf_char)
        {
            *this += "wxString::FromUTF8(";
        }
    }

    // bool text_has_single_quote = (text.find('\'') != tt::npos);

    auto begin_quote = this->size();
    bool has_escape = false;

    if (is_ruby())
        *this += '\'';
    else if (is_perl())
    {
        // Perl should use single-quotes if there are no escape characters, otherwise it should use
        // double-quotes.
        *this += '\'';
    }
    else
        *this += '"';
    for (auto c: text)
    {
        switch (c)
        {
            case '"':
                *this += "\\\"";
                has_escape = true;
                break;

            // This generally isn't needed for C++, but is needed for other languages
            case '\'':
                *this += "\\'";
                has_escape = true;
                break;

            case '\\':
                *this += "\\\\";
                has_escape = true;
                break;

            case '\t':
                *this += "\\t";
                has_escape = true;
                break;

            case '\n':
                *this += "\\n";
                has_escape = true;
                break;

            case '\r':
                *this += "\\r";
                has_escape = true;
                break;

            default:
                *this += c;
                break;
        }
    }
    if (is_ruby())
        *this += '\'';
    else if (is_perl())
    {
        if (has_escape)
        {
            *this += '"';
            at(begin_quote) = '"';
        }
        else
        {
            *this += '\'';
        }
    }
    else
        *this += '"';

    if (has_utf_char)
    {
        *this += ')';
    }
    if (Project.as_bool(prop_internationalize))
    {
        *this += ')';
    }

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}
Code& Code::WxSize(GenEnum::PropName prop_name, int enable_dpi_scaling)
{
    return WxSize(m_node->as_wxSize(prop_name), enable_dpi_scaling);
}

Code& Code::WxSize(wxSize size, int enable_dpi_scaling)
{
    auto cur_pos = this->size();
    auto size_scaling = is_ScalingEnabled(prop_size, enable_dpi_scaling);

    if (is_ruby())
    {
        if (size == wxDefaultSize)
        {
            CheckLineLength((sizeof("Wx::DEFAULT_SIZE") - 1));
            *this += "Wx::DEFAULT_SIZE";
            return *this;
        }

        if (size_scaling)
        {
            CheckLineLength(sizeof(", from_DIP(Wx::Size.new(999, 999))"));
        }
        else
        {
            CheckLineLength(sizeof("Wx::Size.new(999, 999)"));
        }

        if (size_scaling)
        {
            FormFunction("FromDIP(");
            Class("Wx::Size.new(").itoa(size.x).Comma().itoa(size.y) << ')';
            *this += ')';
        }
        else
        {
            Class("Wx::Size.new(").itoa(size.x).Comma().itoa(size.y) << ')';
        }

        if (m_auto_break && this->size() > m_break_at)
        {
            InsertLineBreak(cur_pos);
        }

        return *this;
    }
    else if (is_perl())
    {
        if (size == wxDefaultSize)
        {
            CheckLineLength((sizeof("wxDefaultSize") - 1));
            *this += "wxDefaultSize";
            return *this;
        }

        if (size_scaling)
        {
            CheckLineLength(sizeof(", $self->FromDIP->new(Wx::Size->new(999, 999))"));
            FormFunction("FromDIP->new(");
            Class("Wx::Size->new(").itoa(size.x).Comma().itoa(size.y) << "))";
        }
        else
        {
            CheckLineLength(sizeof("Wx::Size->new(999, 999)"));
            Class("Wx::Size->new(").itoa(size.x).Comma().itoa(size.y) << ')';
        }

        if (m_auto_break && this->size() > m_break_at)
        {
            InsertLineBreak(cur_pos);
        }

        return *this;
    }

    // The following code is for non-Ruby languages

    if (size == wxDefaultSize)
    {
        CheckLineLength((sizeof("DefaultSize") - 1) + m_language_wxPrefix.size());
        if (is_perl())
            *this << "wxDefaultSize";
        else
            *this << m_language_wxPrefix << "DefaultSize";
        return *this;
    }

    if (size_scaling)
    {
        if (is_cpp())
        {
            if (Project.is_wxWidgets31())
            {
                CheckLineLength(sizeof("wxSize(999, 999)"));
                Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';
            }
            else
            {
                CheckLineLength(sizeof("FromDIP(wxSize(999, 999))"));
                FormFunction("FromDIP(");
                Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';
                *this += ')';
            }
        }
        else if (is_python())
        {
            CheckLineLength(sizeof("self.FromDIP(wxSize(999, 999))"));
            FormFunction("FromDIP(");
            Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';
            *this += ')';
        }
#if GENERATE_NEW_LANG_CODE
        else if (is_lua())
        {
            CheckLineLength(sizeof("wx.wxSize(999, 999)"));
            Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';
        }
#endif
    }
    else
    {
        CheckLineLength(sizeof("wxSize(999, 999)"));
        Class("wxSize(").itoa(size.x).Comma().itoa(size.y) << ')';
    }

    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::WxPoint(wxPoint position, int enable_dpi_scaling)
{
    auto cur_pos = this->size();
    auto pos_scaling = is_ScalingEnabled(prop_pos, enable_dpi_scaling);

    if (is_ruby())
    {
        if (position == wxDefaultPosition)
        {
            CheckLineLength((sizeof("Wx::DEFAULT_POSITION") - 1));
            *this += "Wx::DEFAULT_POSITION";
            return *this;
        }

        if (pos_scaling)
        {
            CheckLineLength(sizeof(", from_DIP(Wx::Point.new(999, 999))"));
        }
        else
        {
            CheckLineLength(sizeof("Wx::Point.new(999, 999)"));
        }

        if (pos_scaling)
        {
            FormFunction("FromDIP(");
            Class("Wx::Point.new(").itoa(position.x).Comma().itoa(position.y) << ')';
            *this += ')';
        }
        else
        {
            Class("Wx::Point.new(").itoa(position.x).Comma().itoa(position.y) << ')';
        }

        if (m_auto_break && this->size() > m_break_at)
        {
            InsertLineBreak(cur_pos);
        }

        return *this;
    }

    // The following code is for non-Ruby languages

    if (position == wxDefaultPosition)
    {
        CheckLineLength((sizeof("DefaultPosition") - 1) + m_language_wxPrefix.size());
        if (is_perl())
            *this << "wxDefaultPosition";
        else
            *this << m_language_wxPrefix << "DefaultPosition";
        return *this;
    }

    if (pos_scaling)
    {
        if (is_cpp())
        {
            if (Project.is_wxWidgets31())
            {
                CheckLineLength(sizeof("wxPoint(999, 999)"));
                Class("wxPoint(").itoa(position.x).Comma().itoa(position.y) << ')';
            }
            else
            {
                CheckLineLength(sizeof("FromDIP(wxPoint(999, 999))"));
                FormFunction("FromDIP(");
                Class("wxPoint(").itoa(position.x).Comma().itoa(position.y) << ')';
                *this += ')';
            }
        }
        else if (is_python())
        {
            CheckLineLength(sizeof("self.FromDIP(wxPoint(999, 999))"));
            FormFunction("FromDIP(");
            Class("wxPoint(").itoa(position.x).Comma().itoa(position.y) << ')';
            *this += ')';
        }
#if GENERATE_NEW_LANG_CODE
        else if (is_lua())
        {
            CheckLineLength(sizeof("wx.wxPoint(999, 999)"));
            Class("wxPoint(").itoa(size.x).Comma().itoa(size.y) << ')';
        }
#endif
    }
    else
    {
        CheckLineLength(sizeof("wxPoint(999, 999)"));
        Class("wxPoint(").itoa(position.x).Comma().itoa(position.y) << ')';
    }

    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::Pos(GenEnum::PropName prop_name, int enable_dpi_scaling)
{
    auto cur_pos = size();
    auto point = m_node->as_wxPoint(prop_name);
    auto pos_scaling = is_ScalingEnabled(prop_pos, enable_dpi_scaling);

    if (m_node->as_string(prop_name).contains("d", tt::CASE::either))
    {
        FAIL_MSG("Pos() should not be used with a string that contains 'd'");
        point = wxGetMainFrame()->getWindow()->ConvertDialogToPixels(point);
    }

    if (is_ruby())
    {
        if (m_node->as_wxPoint(prop_name) == wxDefaultPosition)
        {
            CheckLineLength((sizeof("pos=Wx::DEFAULT_POSITION") - 1));
            *this += "Wx::DEFAULT_POSITION";
            return *this;
        }

        if (pos_scaling)
        {
            CheckLineLength(sizeof(", from_DIP(Wx::Point.new(999, 999))"));
            FormFunction("FromDIP(");
            Class("Wx::Point.new(").itoa(point.x).Comma().itoa(point.y) << ')';
            *this += ')';
        }
        else
        {
            CheckLineLength(sizeof("Wx::Point.new(999, 999)"));
            Class("Wx::Point.new(").itoa(point.x).Comma().itoa(point.y) << ')';
        }

        if (m_auto_break && this->size() > m_break_at)
        {
            InsertLineBreak(cur_pos);
        }

        return *this;
    }

    // The following code is for non-Ruby languages

    if (m_node->as_wxPoint(prop_name) == wxDefaultPosition)
    {
        CheckLineLength((sizeof("DefaultPosition") - 1) + m_language_wxPrefix.size());
        if (is_perl())
            *this << "wxDefaultPosition";
        else
            *this << m_language_wxPrefix << "DefaultPosition";
        return *this;
    }

    if (pos_scaling)
    {
        if (is_cpp())
        {
            CheckLineLength(sizeof("FromDIP(wxPoint(999, 999))"));
        }
        else if (is_python())
        {
            CheckLineLength(sizeof("self.FromDIP(wxPoint(999, 999))"));
        }
        FormFunction("FromDIP(");
        Class("wxPoint(").itoa(point.x).Comma().itoa(point.y) << ')';
        *this += ')';
    }
    else
    {
        CheckLineLength(sizeof("wxPoint(999, 999)"));
        Class("wxPoint(").itoa(point.x).Comma().itoa(point.y) << ')';
    }
    if (m_auto_break && this->size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::Style(const char* prefix, tt_string_view force_style)
{
    bool style_set = false;
    if (force_style.size())
    {
        Add(force_style);
        style_set = true;
    }

    if (m_node->hasValue(prop_tab_position) && !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        as_string(prop_tab_position);
    }
    if (m_node->hasValue(prop_orientation) && !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL"))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        as_string(prop_orientation);
    }

    // Note that as_string() may break the line, so recalculate any time as_string() is called
    auto cur_pos = size();

    if (m_node->isGen(gen_wxRichTextCtrl))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        AddConstant("wxRE_MULTILINE");
    }

    if (m_node->hasValue(prop_style))
    {
        if (style_set)
            *this += '|';
        if (prefix)
        {
            if (is_cpp())
            {
                *this += m_node->as_constant(prop_style, prefix);
            }
            else
            {
                tt_view_vector multistr(m_node->as_constant(prop_style, prefix), "|", tt::TRIM::both);
                for (auto& iter: multistr)
                {
                    if (iter.empty())
                        continue;
                    if (style_set)
                        *this += '|';
                    if (iter.is_sameprefix("wx"))
                    {
                        if (std::string_view language_prefix = GetLanguagePrefix(iter, m_language); language_prefix.size())
                        {
                            // Some languages will have a module added after their standard prefix.
                            CheckLineLength(language_prefix.size() + iter.size() - 2);
                            *this << language_prefix << iter.substr(2);
                        }
                        else
                        {
                            // If there was no sub-language module added (e.g., wx.aui. for
                            // Python), then use the default language prefix.
                            CheckLineLength(m_language_wxPrefix.size() + iter.size() - 2);
                            *this << m_language_wxPrefix << iter.substr(2);
                        }
                    }
                    else
                        *this += iter;
                    style_set = true;
                }
            }
        }
        else
        {
            AddConstant(m_node->as_string(prop_style));
            cur_pos = size();
        }
        style_set = true;
    }

    if (m_node->hasValue(prop_window_style))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        AddConstant(m_node->as_string(prop_window_style));
        cur_pos = size();
    }

    if (m_node->isGen(gen_wxListView))
    {
        if (style_set)
            *this += '|';
        style_set = true;
        as_string(prop_mode);
        cur_pos = size();
    }

    if (!style_set)
    {
        *this += "0";
    }

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::PosSizeFlags(ScalingType enable_dpi_scaling, bool uses_def_validator, tt_string_view def_style)
{
    auto pos_scaling = is_ScalingEnabled(prop_pos, enable_dpi_scaling);
    auto size_scaling = is_ScalingEnabled(prop_size, enable_dpi_scaling);

    if (m_node->hasValue(prop_window_name))
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        Comma();
        Pos(prop_pos, pos_scaling).Comma().WxSize(prop_size, size_scaling).Comma();
        Style();
        if (uses_def_validator)
            Comma().Add("wxDefaultValidator");
        Comma();
        if (is_ruby())
        {
            *this += "name=";
        }
        QuotedString(prop_window_name).EndFunction();
        return *this;
    }

    // This could be done as a single if statement, but it is easier to read this way.
    bool style_needed = false;
    if ((m_node->hasValue(prop_style) && m_node->as_string(prop_style) != def_style))
        style_needed = true;
    else if (m_node->hasValue(prop_window_style))
        style_needed = true;
    else if (m_node->hasValue(prop_orientation) && !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL") &&
             !m_node->as_string(prop_orientation).is_sameas("wxSL_HORIZONTAL"))
        style_needed = true;
    else if (m_node->hasValue(prop_tab_position) && !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
        style_needed = true;
    else if (m_node->isGen(gen_wxRichTextCtrl) || m_node->isGen(gen_wxListView))
        style_needed = true;

    // Do we need a style and/or a default validator?
    if (style_needed)
    {
        Comma();
        Pos(prop_pos, pos_scaling).Comma().WxSize(prop_size, size_scaling).Comma().Style();
        if (def_style.size() && ends_with(def_style))
        {
            erase(size() - def_style.size());
            if (back() == '|')
                pop_back();
        }
    }
    else if (m_node->as_wxSize(prop_size) != wxDefaultSize)
    {
        Comma();
        Pos(prop_pos, pos_scaling).Comma().WxSize(prop_size, size_scaling);
    }
    else if (m_node->as_wxPoint(prop_pos) != wxDefaultPosition)
    {
        Comma();
        Pos(prop_pos, pos_scaling);
    }
    EndFunction();
    return *this;
}

bool Code::IsDefaultPosSizeFlags(tt_string_view def_style) const
{
    if (m_node->hasValue(prop_window_name))
        return false;

    if ((m_node->hasValue(prop_style) && m_node->as_string(prop_style) != def_style))
        return false;
    if (m_node->hasValue(prop_window_style))
        return false;
    if (m_node->hasValue(prop_orientation) && !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL") &&
        !m_node->as_string(prop_orientation).is_sameas("wxSL_HORIZONTAL"))
        return false;
    if (m_node->hasValue(prop_tab_position) && !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
        return false;
    if (m_node->isGen(gen_wxRichTextCtrl) || m_node->isGen(gen_wxListView))
        return false;

    if (m_node->as_wxPoint(prop_pos) != wxDefaultPosition)
        return false;
    if (m_node->as_wxSize(prop_size) != wxDefaultSize)
        return false;  // These always need non-default values

    return true;
}

int Code::WhatParamsNeeded(tt_string_view default_style) const
{
    if (m_node->hasValue(prop_window_name))
    {
        return (pos_needed | size_needed | style_needed | window_name_needed);
    }

    // This could be done as a single if statement, but it is easier to read this way.
    if ((m_node->hasValue(prop_style) && m_node->as_string(prop_style) != default_style))
        return (pos_needed | size_needed | style_needed);
    else if (m_node->hasValue(prop_window_style))
        return (pos_needed | size_needed | style_needed);
    else if (m_node->hasValue(prop_orientation) && !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL") &&
             !m_node->as_string(prop_orientation).is_sameas("wxSL_HORIZONTAL"))
        return (pos_needed | size_needed | style_needed);
    else if (m_node->hasValue(prop_tab_position) && !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
        return (pos_needed | size_needed | style_needed);
    else if (m_node->isGen(gen_wxRichTextCtrl) || m_node->isGen(gen_wxListView))
        return (pos_needed | size_needed | style_needed);

    if (m_node->as_wxSize(prop_size) != wxDefaultSize)
        return (pos_needed | size_needed);
    else if (m_node->as_wxPoint(prop_pos) != wxDefaultPosition)
        return pos_needed;

    return nothing_needed;
}

Code& Code::PosSizeForceStyle(tt_string_view force_style, bool uses_def_validator)
{
    if (m_node->hasValue(prop_window_name))
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        Comma();
        Pos().Comma().WxSize().Comma();
        Style(nullptr, force_style);
        if (uses_def_validator)
            Comma().Add("wxDefaultValidator");
        Comma();
        QuotedString(prop_window_name).EndFunction();
        return *this;
    }
    else
    {
        Comma();
        Pos().Comma().WxSize().Comma().Style(nullptr, force_style);
    }
    EndFunction();
    return *this;
}

Code& Code::SizerFlagsFunction(tt_string_view function_name)
{
    *this += '.';
    if (is_ruby())
    {
        *this += ConvertToSnakeCase(function_name);
    }
    else
    {
        *this += function_name;
    }
    *this += '(';
    return *this;
}

Code& Code::GenSizerFlags()
{
    // wxSizerFlags functions are chained together, so we don't want to break them. Instead,
    // shut off auto_break and then restore it when we are done, after which we can check whether
    // or note the entire wxSizerFlags() statement needs to be broken.

    bool save_auto_break = m_auto_break;
    m_auto_break = false;
    auto cur_pos = size();

    Add("wxSizerFlags");
    if (is_ruby())
    {
        Add(".new");
    }

    if (auto& prop = m_node->as_string(prop_proportion); prop != "0")
    {
        *this << '(' << prop << ')';
    }
    else if (!is_ruby())  // Don't use empty () for Ruby
    {
        *this << "()";
    }

    if (auto& prop = m_node->as_string(prop_alignment); prop.size())
    {
        if (prop.contains("wxALIGN_CENTER_HORIZONTAL") &&
            (m_node->getParent()->isGen(gen_wxGridSizer) || m_node->getParent()->isGen(gen_wxFlexGridSizer) ||
             m_node->getParent()->isGen(gen_wxGridBagSizer)))
        {
            SizerFlagsFunction("CenterHorizontal") += ')';
        }
        else if (prop.contains("wxALIGN_CENTER_VERTICAL") &&
                 (m_node->getParent()->isGen(gen_wxGridSizer) || m_node->getParent()->isGen(gen_wxFlexGridSizer) ||
                  m_node->getParent()->isGen(gen_wxGridBagSizer)))

        {
            SizerFlagsFunction("CenterVertical") += ')';
        }
        else if (prop.contains("wxALIGN_CENTER"))
        {
            // Note that CenterHorizontal() and CenterVertical() require wxWidgets 3.1 or higher. Their advantage is
            // generating an assert if you try to use one that is invalid if the sizer parent's orientation doesn't
            // support it. Center() just works without the assertion check.
            SizerFlagsFunction("Center") += ')';
        }

        if (prop.contains("wxALIGN_LEFT"))
        {
            SizerFlagsFunction("Left") += ')';
        }
        else if (prop.contains("wxALIGN_RIGHT"))
        {
            SizerFlagsFunction("Right") += ')';
        }

        if (prop.contains("wxALIGN_TOP"))
        {
            SizerFlagsFunction("Top") += ')';
        }
        else if (prop.contains("wxALIGN_BOTTOM"))
        {
            SizerFlagsFunction("Bottom") += ')';
        }
        if (is_ruby())
        {
            // Ruby style guidelines are to eliminate empty parenthesis
            pop_back();
            pop_back();
        }
    }

    if (auto& prop = m_node->as_string(prop_flags); prop.size())
    {
        if (prop.contains("wxEXPAND"))
        {
            SizerFlagsFunction("Expand") += ')';
        }
        if (prop.contains("wxSHAPED"))
        {
            SizerFlagsFunction("Shaped") += ')';
        }
        if (prop.contains("wxFIXED_MINSIZE"))
        {
            SizerFlagsFunction("FixedMinSize") += ')';
        }
        if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
        {
            SizerFlagsFunction("ReserveSpaceEvenIfHidden") += ')';
        }
        if (is_ruby())
        {
            // Ruby style guidelines are to eliminate empty parenthesis
            pop_back();
            pop_back();
        }
    }

    if (auto& prop = m_node->as_string(prop_borders); prop.size())
    {
        auto border_size = m_node->as_int(prop_border_size);
        if (prop.contains("wxALL"))
        {
            if (border_size == 5)
                SizerFlagsFunction("Border").Add("wxALL)");
            else if (border_size == 10)
                SizerFlagsFunction("DoubleBorder").Add("wxALL)");
            else if (border_size == 15)
                SizerFlagsFunction("TripleBorder").Add("wxALL)");
            else
            {
                SizerFlagsFunction("Border").Add("wxALL, ");
                BorderSize() += ')';
            }
        }
        else
        {
            SizerFlagsFunction("Border");
            tt_string border_flags;

            if (prop.contains("wxLEFT"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << m_language_wxPrefix << "LEFT";
            }
            if (prop.contains("wxRIGHT"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << m_language_wxPrefix << "RIGHT";
            }
            if (prop.contains("wxTOP"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << m_language_wxPrefix << "TOP";
            }
            if (prop.contains("wxBOTTOM"))
            {
                if (border_flags.size())
                    border_flags << '|';
                border_flags << m_language_wxPrefix << "BOTTOM";
            }
            if (border_flags.empty())
                border_flags = "0";

            *this << border_flags << ", ";
            if (border_size == 5)
            {
                if (is_cpp())
                    *this += "wxSizerFlags::GetDefaultBorder())";
                else if (is_ruby())
                    *this += "Wx::SizerFlags.get_default_border)";
                else
                    *this << m_language_wxPrefix << "SizerFlags.GetDefaultBorder())";
            }
            else
            {
                BorderSize() += ')';
            }
        }
    }

    m_auto_break = save_auto_break;

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}

Code& Code::BorderSize(GenEnum::PropName prop_name)
{
    int border_size = m_node->as_int(prop_name);
    bool is_scalable_border = (border_size > 0 && border_size != 5 && border_size != 10 && border_size != 15);
    if ((prop_name == prop_border_size && m_node->as_bool(prop_scale_border_size)) && is_scalable_border)
    {
        if (is_ruby())
        {
            Str("from_dip(").Add("wxSize.new");
        }
        else
        {
            FormFunction("FromDIP(").Add("wxSize");
        }
        Str("(").itoa(border_size).Comma().Str("-1)).x");
    }
    else
    {
        *this += std::to_string(border_size);
    }
    return *this;
}

void Code::GenWindowSettings()
{
    if (hasValue(prop_window_extra_style))
    {
        if (m_node->isForm())
        {
            FormFunction("SetExtraStyle(");
        }
        else
        {
            NodeName().Function("SetExtraStyle(");
        }

        Add(prop_window_extra_style).EndFunction();
    }

    if (IsTrue(prop_disabled))
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("Enable(").False().EndFunction();
        }
        else
        {
            FormFunction("Enable(").False().EndFunction();
        }
    }

    if (IsTrue(prop_hidden))
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("Hide(").EndFunction();
        }
        else
        {
            FormFunction("Hide(").EndFunction();
        }
        if (is_ruby())
        {
            // Ruby style guidelines are to eliminate empty parenthesis
            pop_back();
            pop_back();
        }
    }

    bool allow_minmax { true };
    if (m_node->isForm() && !m_node->isGen(gen_PanelForm) && !m_node->isGen(gen_wxToolBar))
        allow_minmax = false;

    if (allow_minmax && m_node->as_wxSize(prop_minimum_size) != wxDefaultSize)
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("SetMinSize(");
        }
        else
        {
            FormFunction("SetMinSize(");
        }
        WxSize(prop_minimum_size).EndFunction();
    }

    if (allow_minmax && m_node->as_wxSize(prop_maximum_size) != wxDefaultSize)
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("SetMaxSize(");
        }
        else
        {
            FormFunction("SetMaxSize(");
        }
        WxSize(prop_maximum_size).EndFunction();
    }

    if (!m_node->isForm() && !m_node->isPropValue(prop_variant, "normal"))
    {
        Eol(eol_if_empty).NodeName().Function("SetWindowVariant(");
        if (m_node->isPropValue(prop_variant, "small"))
            Add("wxWINDOW_VARIANT_SMALL");
        else if (m_node->isPropValue(prop_variant, "mini"))
            Add("wxWINDOW_VARIANT_MINI");
        else
            Add("wxWINDOW_VARIANT_LARGE");

        EndFunction();
    }

    // wxAuiNotebook uses page tooltips for the tabs, so it should be ignored when generating
    // the page cade.
    if (hasValue(prop_tooltip) && !m_node->getParent()->isGen(gen_wxAuiNotebook))
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
        {
            NodeName().Function("SetToolTip(");
        }
        else
        {
            FormFunction("SetToolTip(");
        }
        QuotedString(prop_tooltip).EndFunction();
    }

    if (hasValue(prop_context_help))
    {
        Eol(eol_if_empty);
        if (!m_node->isForm())
            NodeName().Function("SetHelpText(");
        else
            FormFunction("SetHelpText(");
        QuotedString(prop_context_help).EndFunction();
    }

    GenFontColourSettings();
}

Code& Code::GenFont(GenEnum::PropName prop_name, tt_string_view font_function)
{
    FontProperty fontprop(m_node->getPropPtr(prop_name));
    if (is_perl())
    {
        // REVIEW: [Randalphwa - 01-07-2025] As of wx-3.005, wxPerl doesn't support wxFontinfo
        return *this;
    }
    if (fontprop.isDefGuiFont())
    {
        OpenFontBrace();
        if (is_cpp())
        {
            Add("wxFont font(");
        }
        else
        {
            Add("font").CreateClass(false, "wxFont");
        }
        Add("wxSystemSettings").ClassMethod("GetFont(").Add("wxSYS_DEFAULT_GUI_FONT").Str(")");
        EndFunction();

        if (fontprop.GetSymbolSize() != wxFONTSIZE_MEDIUM)
            Eol()
                .Str("font")
                .VariableMethod("SetSymbolicSize(")
                .Add(font_symbol_pairs.GetValue(fontprop.GetSymbolSize()))
                .EndFunction();
        if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
            Eol().Str("font").VariableMethod("SetStyle(").Add(font_style_pairs.GetValue(fontprop.GetStyle())).EndFunction();
        if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
            Eol()
                .Str("font")
                .VariableMethod("SetWeight(")
                .Add(font_weight_pairs.GetValue(fontprop.GetWeight()))
                .EndFunction();
        if (fontprop.IsUnderlined())
            Eol().Str("font").VariableMethod("SetUnderlined(").True().EndFunction();
        if (fontprop.IsStrikethrough())
            Eol().Str("font").VariableMethod("SetStrikethrough(").True().EndFunction();
        Eol();

        if (m_node->isForm())
        {
            if (m_node->isGen(gen_wxPropertySheetDialog))
            {
                FormFunction("GetBookCtrl()").Function("SetFont(").Add("font").EndFunction();
            }
            else
            {
                FormFunction("SetFont(font").EndFunction();
            }
            CloseFontBrace();
        }
        else if (m_node->isGen(gen_wxStyledTextCtrl))
        {
            NodeName().Function("StyleSetFont(").Add("wxSTC_STYLE_DEFAULT");
            Comma().Str("font").EndFunction();
            CloseFontBrace();
        }
        else
        {
            NodeName().Function(font_function).Str("font").EndFunction();
            CloseFontBrace();
        }
    }
    else  // not isDefGuiFont()
    {
        bool more_than_pointsize =
            ((fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default") ||
             fontprop.GetFamily() != wxFONTFAMILY_DEFAULT || fontprop.GetStyle() != wxFONTSTYLE_NORMAL ||
             fontprop.GetWeight() != wxFONTWEIGHT_NORMAL || fontprop.IsUnderlined() || fontprop.IsStrikethrough());

        const auto point_size = fontprop.GetFractionalPointSize();
        if (is_cpp())
        {
            OpenFontBrace();
            Str("wxFontInfo font_info(");
        }
        else
        {
            Eol(eol_if_needed);
            if (is_perl())
            {
                *this += "my $";
            }
            Add("font_info").CreateClass(false, "wxFontInfo");
        }

        if (point_size != static_cast<int>(point_size))  // is there a fractional value?
        {
            if (is_cpp() && Project.is_wxWidgets31())
            {
                Eol().Str("#if !wxCHECK_VERSION(3, 1, 2)").Eol().Tab();
                {
                    if (point_size <= 0)
                    {
                        Add("wxSystemSettings").ClassMethod("GetFont(").Add("wxSYS_DEFAULT_GUI_FONT").Str(")");
                        VariableMethod("GetPointSize()").EndFunction();
                    }
                    else
                    {
                        // GetPointSize() will round the result rather than truncating the decimal
                        itoa(fontprop.GetPointSize()).EndFunction();
                    }
                }
                Eol().Str("#else // fractional point sizes are new to wxWidgets 3.1.2").Eol().Tab();
                {
                    std::array<char, 10> float_str;
                    if (auto [ptr, ec] = std::to_chars(float_str.data(), float_str.data() + float_str.size(), point_size);
                        ec == std::errc())
                    {
                        Str(std::string_view(float_str.data(), ptr - float_str.data())).EndFunction();
                    }
                }
                Eol().Str("#endif");
            }
            else
            {
                std::array<char, 10> float_str;
                if (auto [ptr, ec] = std::to_chars(float_str.data(), float_str.data() + float_str.size(), point_size);
                    ec == std::errc())
                {
                    Str(std::string_view(float_str.data(), ptr - float_str.data())).EndFunction();
                }
            }
        }
        else
        {
            if (point_size <= 0)
            {
                Add("wxSystemSettings").ClassMethod("GetFont(").Add("wxSYS_DEFAULT_GUI_FONT").Str(")");
                VariableMethod("GetPointSize()").EndFunction();
                if (!is_cpp() && more_than_pointsize)
                {
                    Eol().Str("font_info");
                }
            }
            else
            {
                // GetPointSize() will round the result rather than truncating the decimal
                itoa(fontprop.GetPointSize()).EndFunction();
            }
        }

        if (is_cpp())
        {
            Eol();
            if (more_than_pointsize)
            {
                Str("font_info");
            }
        }

        if (is_perl())
        {
            if (fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default")
            {
                Eol().Str("$font_info->").Str("FaceName = ");
                QuotedString(tt_string() << fontprop.GetFaceName().utf8_string()) += ";";
            }
            if (fontprop.GetFamily() != wxFONTFAMILY_DEFAULT)
            {
                Eol().Str("$font_info->").Str("Family = ");
                Add(font_family_pairs.GetValue(fontprop.GetFamily())) += ";";
            }
            if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
            {
                Eol().Str("$font_info->").Str("Style = ");
                Add(font_style_pairs.GetValue(fontprop.GetStyle())) += ";";
            }
        }
        else
        {
            if (fontprop.GetFaceName().size() && fontprop.GetFaceName() != "default")
                VariableMethod("FaceName(").QuotedString(tt_string() << fontprop.GetFaceName().utf8_string()) += ")";
            if (fontprop.GetFamily() != wxFONTFAMILY_DEFAULT)
                VariableMethod("Family(").Add(font_family_pairs.GetValue(fontprop.GetFamily())) += ")";
            if (fontprop.GetStyle() != wxFONTSTYLE_NORMAL)
                VariableMethod("Style(").Add(font_style_pairs.GetValue(fontprop.GetStyle())) += ")";
            if (fontprop.GetWeight() != wxFONTWEIGHT_NORMAL)
            {
                if (is_cpp() && Project.is_wxWidgets31())
                {
                    Eol().Str("#if !wxCHECK_VERSION(3, 1, 2)").Eol().Tab();
                    {
                        // wxFontInfo::SetFlag() would have worked around this, unfortunately it is a private: function
                        bool is_code_added = false;
                        if (fontprop.GetWeight() == wxFONTWEIGHT_LIGHT)
                        {
                            VariableMethod("Light();");
                            is_code_added = true;
                        }
                        else if (fontprop.GetWeight() == wxFONTWEIGHT_BOLD)
                        {
                            VariableMethod("Bold()");
                            is_code_added = true;
                        }

                        if (!is_code_added)
                        {
                            Str("// Only Bold and Light are supported in wxWidgets 3.1.1 and earlier");
                        }
                    }
                    Eol().Str("#else // Weight() is new to wxWidgets 3.1.2").Eol().Tab();
                    {
                        VariableMethod("Weight(").Add(font_weight_pairs.GetValue(fontprop.GetWeight())) += ")";
                    }
                    Eol().Str("#endif").Eol();
                    if (!fontprop.IsUnderlined() && !fontprop.IsStrikethrough())
                    {
                        Str(";").Eol();
                    }
                }
                else
                {
                    VariableMethod("Weight(").Add(font_weight_pairs.GetValue(fontprop.GetWeight())) += ")";
                }
            }
            if (fontprop.IsUnderlined())
                VariableMethod("Underlined()");
            if (fontprop.IsStrikethrough())
                VariableMethod("Strikethrough()");
        }
        if (back() == '.')
        {
            pop_back();
        }
        if (is_cpp())
        {
            while (back() == '\t')
                pop_back();
            if (back() != '\n')
                *this += ';';
        }
        Eol(eol_if_needed);

        if (m_node->isForm())
        {
            if (m_node->isGen(gen_wxPropertySheetDialog))
            {
                FormFunction("GetBookCtrl()")
                    .Function(font_function)
                    .Object("wxFont")
                    .Str("font_info")
                    .Str(")")
                    .EndFunction();
            }
            else
            {
                FormFunction(font_function).Object("wxFont").VarName("font_info", false).Str(")").EndFunction();
            }
        }
        else
        {
            NodeName().Function(font_function).Object("wxFont").VarName("font_info", false).Str(")").EndFunction();
        }
        CloseFontBrace();
    }

    return *this;
}

void Code::GenFontColourSettings()
{
    auto* node = m_node;
    if (hasValue(prop_font))
    {
        GenFont();
    }

    if (auto& fg_clr = node->as_string(prop_foreground_colour); fg_clr.size())
    {
        Eol(eol_if_needed);
        if (node->isForm())
        {
            if (m_node->isGen(gen_wxPropertySheetDialog))
            {
                FormFunction("GetBookCtrl()").Function("SetForegroundColour(");
            }
            else
            {
                FormFunction("SetForegroundColour(");
            }
        }
        else
        {
            NodeName().Function("SetForegroundColour(");
        }
        if (fg_clr.contains("wx"))
        {
            Add("wxSystemSettings").ClassMethod("GetColour(").Add(fg_clr) += ")";
        }
        else
        {
            if (fg_clr.starts_with('#'))
            {
                Object("wxColour").QuotedString(fg_clr) += ')';
            }
            else
            {
                // This handles older project versions, and hand-edited project files
                const auto colour = m_node->as_wxColour(prop_foreground_colour);
                Object("wxColour").QuotedString(colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString()) += ')';
            }
        }
        EndFunction();
    }

    if (auto& bg_clr = node->as_string(prop_background_colour); bg_clr.size())
    {
        Eol(eol_if_needed);
        if (node->isForm())
        {
            FormFunction("SetBackgroundColour(");
        }
        else
        {
            NodeName().Function("SetBackgroundColour(");
        }
        if (bg_clr.contains("wx"))
        {
            Add("wxSystemSettings").ClassMethod("GetColour(").Add(bg_clr) += ")";
        }
        else
        {
            if (bg_clr.starts_with('#'))
            {
#if GENERATE_NEW_LANG_CODE
                if (is_lua())
                {
                    // Lua 3.2 doesn't allow passing a string to SetBackgroundColour
                    Class("wxColour(").QuotedString(bg_clr) += ')';
                }
                else
#endif
                {
                    Object("wxColour").QuotedString(bg_clr) += ')';
                }
            }
            else
            {
                // This handles older project versions, and hand-edited project files
                const auto colour = m_node->as_wxColour(prop_background_colour);
#if GENERATE_NEW_LANG_CODE
                if (is_lua())
                {
                    // Lua 3.2 doesn't allow passing a string to SetBackgroundColour
                    Class("wxColour(").QuotedString(colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString()) += ')';
                }
                else
#endif
                {
                    Object("wxColour").QuotedString(colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString()) += ')';
                }
            }
        }

        EndFunction();

        // For background color, set both the background of the dialog and the background of the book control
        if (m_node->isGen(gen_wxPropertySheetDialog))
        {
            FormFunction("GetBookCtrl()").Function("SetBackgroundColour(");
            if (bg_clr.contains("wx"))
            {
                Add("wxSystemSettings").ClassMethod("GetColour(").Add(bg_clr) += ")";
            }
            else
            {
                if (bg_clr.starts_with('#'))
                {
                    if (is_perl())
                    {
                        Class("wxColour->new(").QuotedString(bg_clr) += ")";
                    }
                    else
                    {
                        Object("wxColour").QuotedString(bg_clr) += ')';
                    }
                }
                else
                {
                    // This handles older project versions, and hand-edited project files
                    const auto colour = m_node->as_wxColour(prop_background_colour);
                    Object("wxColour").QuotedString(colour.GetAsString(wxC2S_HTML_SYNTAX).ToStdString()) += ')';
                }
            }

            EndFunction();
        }
    }
}

Code& Code::ColourCode(GenEnum::PropName prop_name)
{
    if (!hasValue(prop_name))
    {
        Add("wxNullColour");
    }
    else
    {
        auto colour = m_node->as_wxColour(prop_name);
        Object("wxColour").QuotedString(colour) += ')';
    }

    return *this;
}

Code& Code::Bundle(GenEnum::PropName prop_name)
{
    tt_string_vector parts(m_node->as_string(prop_name), BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts.size() <= 1 || parts[IndexImage].empty())
    {
        return Add("wxNullBitmap");
    }

    if (parts[IndexType].contains("SVG"))
    {
        GenerateBundleParameter(*this, parts);
    }

    else
    {
        switch (m_language)
        {
            case GEN_LANG_PYTHON:
                PythonBundleCode(*this, prop_name);
                break;

            case GEN_LANG_RUBY:
                RubyBundleCode(*this, prop_name);
                break;
            case GEN_LANG_PERL:
                // PerlBundleCode(*this, prop_name);
                break;

            case GEN_LANG_RUST:
                // RustBundleCode(*this, prop_name);
                break;

#if GENERATE_NEW_LANG_CODE
            case GEN_LANG_HASKELL:
                // HaskellBundleCode(*this, prop_name);
                break;

            case GEN_LANG_LUA:
                // LuaBundleCode(*this, prop_name);
                break;
#endif

            default:
                break;
        }
    }

    return *this;
}

bool Code::is_ScalingEnabled(GenEnum::PropName prop_name, int enable_dpi_scaling) const
{
    if (enable_dpi_scaling == code::no_dpi_scaling ||
        tt::contains(m_node->as_string(prop_name), 'n', tt::CASE::either) == true)
    {
        return false;
    }
    else if (m_language == GEN_LANG_CPLUSPLUS && Project.is_wxWidgets31())
    {
        return false;
    }
#if !PERL_FROM_DIP
    // REVIEW: [Randalphwa - 03-02-2025] As far as I have been able to determine, wxPerl does not
    // have a FromDIP function. So we need to disable DPI scaling for Perl.
    else if (m_language == GEN_LANG_PERL)
        return false;
#endif

    if (enable_dpi_scaling == code::conditional_scaling && m_node->isForm())
        return false;

    return true;
}

Code& Code::AddComment(std::string_view comment, bool force)
{
    if (!UserPrefs.is_AddComments() && !force)
        return *this;
    Eol(eol_if_needed);
    switch (m_language)
    {
        case GEN_LANG_CPLUSPLUS:
            *this << "// " << comment;
            break;
        case GEN_LANG_PYTHON:
        case GEN_LANG_RUBY:
        case GEN_LANG_PERL:
            *this << "# " << comment;
            break;
        default:
            *this << "# " << comment;
            break;
    }

    Eol(eol_if_needed);

    return *this;
}

Code& Code::BeginConditional()
{
    if (is_cpp() || is_perl())
    {
        *this << "if (";
    }
    else
    {
        *this << "if ";
    }
    return *this;
}

Code& Code::AddConditionalAnd()
{
    if (is_cpp() || is_ruby() || is_perl() || is_rust())
    {
        *this << " && ";
    }
    else if (is_python())
    {
        *this << " and ";
    }
#if GENERATE_NEW_LANG_CODE
    else if (is_fortran())
    {
        *this << " .AND. ";
    }
    else if (is_haskell())
    {
        *this << " && ";
    }
    else if (is_lua())
    {
        *this << " and ";
    }
#endif

    else
    {
        MSG_WARNING("unknown language");
    }

    return *this;
}
Code& Code::AddConditionalOr()
{
    if (is_cpp() || is_ruby() || is_perl() || is_rust())
    {
        *this << " || ";
    }
    else if (is_python())
    {
        *this << " or ";
    }
#if GENERATE_NEW_LANG_CODE
    else if (is_fortran())
    {
        *this << " .OR. ";
    }
    else if (is_haskell())
    {
        *this << " || ";
    }
    else if (is_lua())
    {
        *this << " or ";
    }
#endif
    else
    {
        MSG_WARNING("unknown language");
    }

    return *this;
}

Code& Code::EndConditional()
{
    if (is_cpp() || is_perl())

    {
        *this << ')';
    }
    else if (is_python())
    {
        *this << ':';
    }
#if GENERATE_NEW_LANG_CODE

    else if (is_lua())
    {
        *this << ':';
    }
#endif

    // Ruby doesn't need anything to complete the conditional statement

    return *this;
}

Code& Code::True()
{
    if (is_python())
    {
        *this << "True";
    }
    else if (is_perl())
    {
        *this << "1";
    }
    else
    {
        *this << "true";
    }

    return *this;
}

Code& Code::False()
{
    if (is_python())
    {
        *this << "False";
    }
    else if (is_perl())
    {
        *this << "0";
    }
    else
    {
        *this << "false";
    }

    return *this;
}
