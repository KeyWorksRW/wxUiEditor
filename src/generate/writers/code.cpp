/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*

    Notes:

    The Eol() function will automatically append tabs if m_indent is greater than 0. That means you
    should *not* append tabs using += '/t', and you should be very cautious about using += '\n'
    instead of Eol() since the Eol() call will automatically append tabs if needed.

    code_add.cpp contains the Code::Add...() functions
    code_gen.cpp contains the Code::Gen...() functions
    code_pos_style.cpp contains the Code::Pos()/Style() functions
    code_size_pos.cpp contains WxSize() and WxPoint() functions
    code_string.cpp contains QuotedString() and string escaping functions

*/

#include <frozen/map.h>
#include <frozen/set.h>

#include <cstddef>

#include "code.h"

#include "image_gen.h"                          // Functions for generating embedded images
#include "node.h"                               // Node class
#include "project_handler.h"                    // ProjectHandler class
#include "utils.h"                              // Miscellaneous utilities
#include "wxue_namespace/wxue_string.h"         // wxue::string, wxue::is_whitespace
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

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

const view_map g_map_perl_prefix
{
};

static const view_map s_short_perl_map
{
};

// clang-format on

constexpr size_t MIN_VALID_LINE_LENGTH_INIT = 50;  // NOLINT (magic number)
constexpr size_t XRC_LINE_LENGTH = 500;            // NOLINT (magic number)
constexpr size_t DEFAULT_LINE_LENGTH = 90;         // NOLINT (magic number)

[[nodiscard]] auto Code::GetLineBreakLength(GenLang language) -> size_t
{
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            return Project.as_size_t(prop_cpp_line_length);
        case GEN_LANG_PERL:
            return Project.as_size_t(prop_perl_line_length);
        case GEN_LANG_PYTHON:
            return Project.as_size_t(prop_python_line_length);
        case GEN_LANG_RUBY:
            return Project.as_size_t(prop_ruby_line_length);
        case GEN_LANG_XRC:
            return XRC_LINE_LENGTH;
        default:
            return DEFAULT_LINE_LENGTH;
    }
}

[[nodiscard]] auto Code::GetLanguagePrefixForInit(GenLang language) -> std::string_view
{
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
        case GEN_LANG_XRC:
            return "wx";
        case GEN_LANG_PERL:
        case GEN_LANG_RUBY:
            return "Wx::";
        case GEN_LANG_PYTHON:
            return "wx.";
        default:
            return "wx";
    }
}

[[nodiscard]] auto Code::GetIndentSize(GenLang language) -> int
{
    switch (language)
    {
        case GEN_LANG_PERL:
            return 4;
        case GEN_LANG_RUBY:
            return 2;
        default:
            return 4;
    }
}

[[nodiscard]] auto Code::GetLineOffset(GenLang language) -> size_t
{
    switch (language)
    {
        case GEN_LANG_PYTHON:
        case GEN_LANG_RUBY:
            return 2;  // Two tabs
        default:
            return 1;  // One tab
    }
}

auto Code::GetLanguagePrefix(std::string_view candidate, GenLang language) -> std::string_view
{
    // Lambda to find a matching prefix in a map
    auto find_prefix_match = [](const view_map& map,
                                std::string_view candidate) -> std::optional<std::string_view>
    {
        for (const auto& [key, value]: map)
        {
            if (candidate.starts_with(key))
            {
                return value;
            }
        }
        return std::nullopt;
    };

    switch (language)
    {
        case GEN_LANG_PERL:
            if (auto result = find_prefix_match(s_short_perl_map, candidate))
            {
                return *result;
            }
            if (auto result = g_map_perl_prefix.find(candidate); result != g_map_perl_prefix.end())
            {
                return result->second;
            }
            break;

        case GEN_LANG_PYTHON:
            if (auto result = find_prefix_match(s_short_python_map, candidate))
            {
                return *result;
            }
            if (auto result = g_map_python_prefix.find(candidate);
                result != g_map_python_prefix.end())
            {
                return result->second;
            }
            break;

        case GEN_LANG_RUBY:
            if (auto result = find_prefix_match(s_short_ruby_map, candidate))
            {
                return *result;
            }
            if (auto result = g_map_ruby_prefix.find(candidate); result != g_map_ruby_prefix.end())
            {
                return result->second;
            }
            break;

        case GEN_LANG_CPLUSPLUS:
            FAIL_MSG("Don't call GetLanguagePrefix() for C++ code!");
            return {};

        default:
            FAIL_MSG("Unknown language");
            return {};
    }

    return {};
}

Code::Code(Node* node, GenLang language)  // NOLINT (doesn't initialize all members) //
                                          // cppcheck-suppress uninitMemberVar
{
    Init(node, language);
}

void Code::Init(Node* node, GenLang language)
{
    m_node = node;
    m_language = language;

    const auto indent_size = GetIndentSize(language);
    if (indent_size != 4)
    {
        m_indent_size = indent_size;
    }

    m_language_wxPrefix = GetLanguagePrefixForInit(language);
    m_break_length = GetLineBreakLength(language);

    const auto line_offset = GetLineOffset(language);
    m_break_length -= (static_cast<size_t>(m_indent_size * line_offset));

    if (m_break_length < MIN_VALID_LINE_LENGTH_INIT)
    {
        FAIL_MSG("Configured line length is too short");
    }

    m_break_at = m_break_length;

    // Reserve large enough for multiple lines -- goal is to avoid multiple reallocations
    reserve(256);  // NOLINT (magic number) // cppcheck-suppress constLiteral
}

auto Code::CheckLineLength(size_t next_str_size) -> Code&
{
    if (m_indent)
    {
        next_str_size += (static_cast<size_t>(m_indent * m_indent_size));
    }

    if (m_auto_break && size() > m_minium_length && size() + next_str_size > m_break_at)
    {
        if (back() == ' ')
        {
            pop_back();
        }
        Eol().Tab();
    }
    return *this;
}

auto Code::CheckLineLength(GenEnum::PropName next_prop_name) -> Code&
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
        {
            pop_back();
        }
        *this += '\n';
    }

#if 0  // NOLINT (always false) // cppcheck-suppress constConditionTrueFalse
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
        m_minium_length = size() + MIN_BREAK_LENGTH;
    }
    return *this;
}

Code& Code::OpenBrace(bool all_languages)
{
    if (!all_languages && !is_cpp() && !is_perl())
    {
        return *this;
    }

    if (is_cpp() || is_perl())
    {
        // Perl place the brace at the end of the function. wxUiEditor
        // follows CppCoreGuidelines and places the brace on the next line for
        // C++ code.
        if (is_cpp())
        {
            Eol(eol_if_needed);
        }
        else if (is_perl())
        {
            if (size() && back() != ' ')
            {
                *this += ' ';
            }
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

auto Code::CloseBrace(bool all_languages, bool close_ruby) -> Code&
{
    if (!all_languages && !is_cpp() && !is_perl())
    {
        return *this;
    }

    // Ensure there are no trailing tabs
    while (size() && wxue::is_whitespace(back()))
    {
        pop_back();
    }
    Unindent();

    if (is_cpp() || is_perl())
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
    if (is_cpp() || is_perl())
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
    if (is_cpp() || is_perl())
    {
        while (size() && wxue::is_whitespace(back()))
        {
            pop_back();
        }
        Unindent();
        m_within_font_braces = false;
        Eol().Str("}").Eol();
    }
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
    m_minium_length = cur_pos + 10;  // NOLINT (magic number) // cppcheck-suppress magicLiteral
}

auto Code::Tab(int tabs) -> Code&
{
    while (tabs)
    {
        *this += '\t';
        --tabs;
    }
    return *this;
}

auto Code::TrueFalseIf(GenEnum::PropName prop_name) -> Code&
{
    if (m_node->as_bool(prop_name))
    {
        return True();
    }
    return False();
}

auto Code::Function(wxue::string_view text, bool add_operator) -> Code&
{
    if (!add_operator)
    {
        if (text.is_sameprefix("wx") && (is_python() || is_ruby() || is_perl()))
        {
            AddFunctionNoOperatorWithWx(text);
        }
        else
        {
            *this += text;
        }
    }
    else if (is_cpp() || is_perl())
    {
        *this << "->" << text;
    }
    else if (is_ruby())
    {
        AddFunctionWithOperatorRuby(text);
    }
    else if (is_python())
    {
        AddFunctionWithOperatorPython(text);
    }

    return *this;
}

auto Code::ClassMethod(wxue::string_view function_name) -> Code&
{
    if (is_cpp() || is_perl())
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

auto Code::VariableMethod(wxue::string_view function_name) -> Code&
{
    if (is_perl())
    {
        *this += "->";
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

auto Code::FormFunction(wxue::string_view text) -> Code&
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

    *this += text;
    return *this;
}

auto Code::FormParent() -> Code&
{
    if (is_cpp())
    {
        *this += "this";
    }
    else if (is_python() || is_ruby())
    {
        *this += "self";
    }
    else if (is_perl())
    {
        *this += "$self";
    }
    else
    {
        MSG_WARNING("unknown language");
    }
    return *this;
}

auto Code::Class(wxue::string_view text) -> Code&
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

    return *this;
}

auto Code::Object(wxue::string_view class_name) -> Code&
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

auto Code::CreateClass(bool use_generic, wxue::string_view override_name, bool assign) -> Code&
{
    if (assign)
    {
        *this += " = ";
    }

    if (is_cpp())
    {
        *this += "new ";
        if (HandleCppSubclass())
        {
            return *this;
        }
    }

    auto class_name = DetermineClassName(use_generic, override_name);
    AddClassNameForLanguage(class_name);

    *this += '(';
    AddSubclassParams();

    return *this;
}

auto Code::HandleCppSubclass() -> bool
{
    if (m_node->HasValue(prop_subclass) &&
        !m_node->as_string(prop_subclass).starts_with("wxGeneric"))
    {
        *this += m_node->as_string(prop_subclass);
        *this += '(';
        AddSubclassParams();
        return true;
    }
    return false;
}

auto Code::DetermineClassName(bool use_generic, wxue::string_view override_name) const
    -> std::string
{
    wxue::string class_name;
    if (override_name.empty())
    {
        class_name = m_node->get_DeclName();
    }
    else
    {
        class_name = override_name;
    }

    if (use_generic)
    {
        class_name.Replace("wx", "wxGeneric");
        return std::string(class_name);
    }

    if (m_node->is_Gen(gen_BookPage))
    {
        return "wxPanel";
    }

    return std::string(class_name);
}

auto Code::Assign(wxue::string_view class_name) -> Code&
{
    *this += " = ";
    if (class_name.empty())
    {
        return *this;
    }

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

auto Code::EndFunction() -> Code&
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

    if (is_cpp() || is_perl())
    {
        *this += ';';
    }
    return *this;
}

auto Code::NodeName(Node* node) -> Code&
{
    if (!node)
    {
        node = m_node;
    }
    auto node_name = node->get_NodeName(get_language());
    if (is_python())
    {
        if (!node->is_Form() && node->as_string(prop_class_access) != "none" &&
            node->as_string(prop_class_access) != "public")
        {
            *this += "self.";
        }
    }
    else if (is_ruby() && !node->is_Form() && !node->is_Local() && node_name[0] != '@')
    {
        *this += "@";
    }
    else if (is_perl() && !node->is_Form())
    {
        if (node->is_Local())
        {
            if (!node_name.starts_with("$") && (size() < 1 || back() != '$'))
            {
                *this += "$";
            }
            *this += node_name;
            return *this;
        }

        if (node_name.starts_with("$self->"))
        {
            *this += node_name;
            return *this;
        }
        *this += "$self->{";
        if (node_name.starts_with("$"))
        {
            *this += node_name.substr(1);
        }
        else
        {
            *this += node_name;
        }
        *this += "}";
        return *this;
    }

    *this += node_name;
    return *this;
}

auto Code::VarName(wxue::string_view var_name, bool class_access) -> Code&
{
    if (is_cpp())
    {
        // If a Ruby user added this, then it must be removed for valid C++ code
        if (var_name.is_sameprefix("@"))
        {
            *this += var_name.subview(1);
        }
        else
        {
            *this += var_name;
        }
        return *this;
    }

    if (class_access)
    {
        if (is_python())
        {
            *this += "self.";
        }
        else if (is_ruby())
        {
            *this += "@";
        }
        else if (is_perl())
        {
            Str("$self->{");
            if (var_name.is_sameprefix("m_"))
            {
                *this += var_name.subview(2);
            }
            else
            {
                *this += var_name;
            }
            *this += "}";
            return *this;
        }
    }

    else if (is_perl())
    {
        *this += "$";
    }

    if (var_name.is_sameprefix("m_"))
    {
        *this += var_name.subview(2);
    }
    else
    {
        *this += var_name;
    }

    return *this;
}

auto Code::ParentName() -> Code&
{
    NodeName(m_node->get_Parent());
    return *this;
}

[[nodiscard]] auto Code::is_local_var() const -> bool
{
    return m_node->is_Local();
}

[[nodiscard]] auto Code::HasValue(GenEnum::PropName prop_name) const -> bool
{
    return m_node->HasValue(prop_name);
}

[[nodiscard]] auto Code::IntValue(GenEnum::PropName prop_name) const -> int
{
    return m_node->as_int(prop_name);
}

[[nodiscard]] auto Code::PropContains(GenEnum::PropName prop_name, wxue::string_view text) const
    -> bool
{
    return m_node->as_string(prop_name).contains(text);
}

[[nodiscard]] auto Code::PropSize(GenEnum::PropName prop_name) const -> size_t
{
    return m_node->as_string(prop_name).size();
}

// List of valid component parent types
constexpr auto s_GenParentTypes = std::to_array<GenType>(
    { type_aui_toolbar, type_auinotebook, type_bookpage, type_choicebook, type_container,
      type_listbook, type_notebook, type_panel, type_propgridpage, type_ribbonbar,
      type_ribbonbar_form, type_ribbonpanel, type_simplebook, type_splitter, type_toolbar,
      type_wizardpagesimple });

void Code::AddFormParentName()
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
}

auto Code::ValidParentName() -> Code&
{
    auto* parent = m_node->get_Parent();
    while (parent)
    {
        if (parent->is_Sizer() && parent->is_StaticBoxSizer())
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

        if (parent->is_Form())
        {
            AddFormParentName();
            return *this;
        }

        for (auto iter: s_GenParentTypes)
        {
            if (parent->is_Type(iter))
            {
                NodeName(parent);
                if (parent->is_Gen(gen_wxCollapsiblePane))
                {
                    Function("GetPane()");
                }
                return *this;
            }
        }
        parent = parent->get_Parent();
    }

    ASSERT_MSG(parent, wxue::string() << m_node->get_NodeName() << " has no parent!");
    return *this;
}

[[nodiscard]] auto Code::IsDefaultPosSizeFlags(wxue::string_view def_style) const -> bool
{
    if (m_node->HasValue(prop_window_name))
    {
        return false;
    }

    if ((m_node->HasValue(prop_style) && m_node->as_string(prop_style) != def_style))
    {
        return false;
    }
    if (m_node->HasValue(prop_window_style))
    {
        return false;
    }
    if (m_node->HasValue(prop_orientation) &&
        !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL") &&
        !m_node->as_string(prop_orientation).is_sameas("wxSL_HORIZONTAL"))
    {
        return false;
    }
    if (m_node->HasValue(prop_tab_position) &&
        !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        return false;
    }
    if (m_node->is_Gen(gen_wxRichTextCtrl) || m_node->is_Gen(gen_wxListView))
    {
        return false;
    }

    if (m_node->as_wxPoint(prop_pos) != wxDefaultPosition)
    {
        return false;
    }
    if (m_node->as_wxSize(prop_size) != wxDefaultSize)
    {
        return false;  // These always need non-default values
    }

    return true;
}

[[nodiscard]] auto Code::WhatParamsNeeded(wxue::string_view default_style) const -> int
{
    if (m_node->HasValue(prop_window_name))
    {
        return (pos_needed | size_needed | style_needed | window_name_needed);
    }

    // This could be done as a single if statement, but it is easier to read this way.
    if ((m_node->HasValue(prop_style) && m_node->as_string(prop_style) != default_style))
    {
        return (pos_needed | size_needed | style_needed);
    }
    if (m_node->HasValue(prop_window_style))
    {
        return (pos_needed | size_needed | style_needed);
    }
    if (m_node->HasValue(prop_orientation) &&
        !m_node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL") &&
        !m_node->as_string(prop_orientation).is_sameas("wxSL_HORIZONTAL"))
    {
        return (pos_needed | size_needed | style_needed);
    }
    if (m_node->HasValue(prop_tab_position) &&
        !m_node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        return (pos_needed | size_needed | style_needed);
    }
    if (m_node->is_Gen(gen_wxRichTextCtrl) || m_node->is_Gen(gen_wxListView))
    {
        return (pos_needed | size_needed | style_needed);
    }

    if (m_node->as_wxSize(prop_size) != wxDefaultSize)
    {
        return (pos_needed | size_needed);
    }
    if (m_node->as_wxPoint(prop_pos) != wxDefaultPosition)
    {
        return pos_needed;
    }

    return nothing_needed;
}

auto Code::SizerFlagsFunction(wxue::string_view function_name) -> Code&
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

constexpr int DEFAULT_BORDER_SIZE = 5;

auto Code::BorderSize(GenEnum::PropName prop_name) -> Code&
{
    int border_size = m_node->as_int(prop_name);
    bool is_scalable_border =
        (border_size > 0 && border_size != DEFAULT_BORDER_SIZE &&
         border_size != DEFAULT_BORDER_SIZE * 2 && border_size != DEFAULT_BORDER_SIZE * 3);
    if ((prop_name == prop_border_size && m_node->as_bool(prop_scale_border_size)) &&
        is_scalable_border)
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

auto Code::ColourCode(GenEnum::PropName prop_name) -> Code&
{
    if (!HasValue(prop_name))
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

auto Code::is_ScalingEnabled(GenEnum::PropName prop_name, int enable_dpi_scaling) const -> bool
{
    if (enable_dpi_scaling == code::no_dpi_scaling ||
        wxue::contains(m_node->as_string(prop_name), 'n', wxue::CASE::either))
    {
        return false;
    }
#if !PERL_FROM_DIP
    // REVIEW: [Randalphwa - 03-02-2025] As far as I have been able to determine, wxPerl does not
    // have a FromDIP function. So we need to disable DPI scaling for Perl.
    if (m_language == GEN_LANG_PERL)
    {
        return false;
    }
#endif

    if (enable_dpi_scaling == code::conditional_scaling && m_node->is_Form())
    {
        return false;
    }

    return true;
}

auto Code::BeginConditional() -> Code&
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

auto Code::EndConditional() -> Code&
{
    if (is_cpp() || is_perl())
    {
        *this << ')';
    }
    else if (is_python())
    {
        *this << ':';
    }

    // Ruby doesn't need anything to complete the conditional statement

    return *this;
}

auto Code::True() -> Code&
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

auto Code::False() -> Code&
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

auto Code::ExpandEventLambda(wxue::string lambda) -> Code&
{
    lambda.LeftTrim();
    lambda.Replace("@@", "\n", wxue::REPLACE::all);
    lambda.RightTrim();

    if (is_cpp())
    {
        Indent(1);
        Eol();
        wxue::StringVector lines(lambda, '\n');

        for (auto& line: lines)
        {
            if (line.contains("}"))
            {
                Unindent(1);
                if (back() == '\t')
                {
                    pop_back();
                }
                Str(line);
            }
            else if (line.contains("{"))
            {
                Str(line);
                Indent();
            }
            else
            {
                Str(line);
            }
            Eol();
        }
        Unindent(1);

        // The caller will be adding a comma, which should appear after the closing brace
        while (back() == '\t')
        {
            pop_back();
        }
        while (back() == '\n')
        {
            pop_back();
        }
    }

    return *this;
}
