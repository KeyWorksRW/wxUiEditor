/////////////////////////////////////////////////////////////////////////////
// Purpose:   Helper class for generating code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-03-2026]

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

#include <cctype>
#include <cstddef>

#include "code.h"

#include "ffi_ui.h"                             // kwxFFI UI descriptor + role resolver
#include "image_gen.h"                          // Functions for generating embedded images
#include "node.h"                               // Node class
#include "project_handler.h"                    // ProjectHandler class
#include "utils.h"                              // Miscellaneous utilities
#include "wxue_namespace/wxue_string.h"         // wxue::string, wxue::is_whitespace
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

using namespace code;

// clang-format off

// This maps just the prefix
const view_map g_map_python_const_prefix
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
    { "wxGeneriRcAnimationCtrl", "wx.adv."},
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

// clang-format on

constexpr size_t MIN_VALID_LINE_LENGTH_INIT = 50;  // NOLINT (magic number)
constexpr size_t XRC_LINE_LENGTH = 500;            // NOLINT (magic number)
constexpr size_t DEFAULT_LINE_LENGTH = 90;         // NOLINT (magic number)

[[nodiscard]] size_t Code::GetLineBreakLength(GenLang language)
{
    switch (language)
    {
        case GenLang::cplusplus:
            return Project.as_size_t(prop_cpp_line_length);

        case GenLang::python:
            return Project.as_size_t(prop_python_line_length);
        case GenLang::ruby:
            return Project.as_size_t(prop_ruby_line_length);
        case GenLang::fortran:
            return Project.as_size_t(prop_fortran_line_length);
        case GenLang::go:
            return Project.as_size_t(prop_go_line_length);
        case GenLang::julia:
            return Project.as_size_t(prop_julia_line_length);
        case GenLang::luajit:
            return Project.as_size_t(prop_lua_line_length);
        case GenLang::rust:
            return Project.as_size_t(prop_rust_line_length);
        case GenLang::typescript:
            return Project.as_size_t(prop_typescript_line_length);
        case GenLang::xrc:
            return XRC_LINE_LENGTH;
        default:
            return DEFAULT_LINE_LENGTH;
    }
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
    m_traits = GetLanguageTraits(language);

    if (m_traits)
    {
        m_language_wxPrefix = m_traits->wx_prefix;

        // Class/type names may use a different prefix than constants -- TypeScript emits bare
        // constants ("") but keeps the "wx" prefix on wrapper class names.
        m_language_classPrefix =
            m_traits->class_prefix.empty() ? m_traits->wx_prefix : m_traits->class_prefix;
    }

    m_indent_size = GetLanguageIndentSize(language);
    m_method_case = GetLanguageMethodCase(language);

    m_break_length = GetLineBreakLength(language);

    const size_t line_offset = m_traits ? m_traits->line_offset : 1;
    m_break_length -= (static_cast<size_t>(m_indent_size) * line_offset);

    if (m_break_length < MIN_VALID_LINE_LENGTH_INIT)
    {
        FAIL_MSG("Configured line length is too short");
    }

    m_break_at = m_break_length;

    // Reserve large enough for multiple lines -- goal is to avoid multiple reallocations
    reserve(256);  // NOLINT (magic number) // cppcheck-suppress constLiteral

    // Go has no line-length limit and gofmt owns wrapping; an automatic break inserted inside
    // an expression produces a layout gofmt rewrites, so the generated file fails `gofmt -l`.
    // Fortran free form cannot break a statement without a trailing `&`, so an inserted break
    // would split one statement into two invalid lines.
    // Lua has no line-length limit either, and a break inserted directly after `local x =`
    // leaves the assignment expression on the following line -- legal but unreadable, and it
    // makes every emitted statement span several lines.
    if (language == GenLang::go || language == GenLang::fortran || language == GenLang::luajit)
    {
        m_auto_break = false;
    }
}

Code& Code::CheckLineLength(size_t next_str_size)
{
    if (m_indent)
    {
        next_str_size += (static_cast<size_t>(m_indent * m_indent_size));
    }

    if (m_auto_break && size() > m_minimum_length && size() + next_str_size > m_break_at)
    {
        if (back() == ' ')
        {
            pop_back();
        }
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
        if (!empty())
        {
            *this += '\n';
        }
    }
    else if (flag == eol_if_needed)
    {
        if (!empty() && back() != '\n')
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
        if (!empty() && back() == ' ')
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
        m_minimum_length = size() + MIN_BREAK_LENGTH;
    }
    return *this;
}

Code& Code::OpenBrace(bool all_languages)
{
    if (!all_languages && !is_cpp())
    {
        return *this;
    }

    if (is_cpp())
    {
        // Place the brace on the following line for C++ code to follow
        // the project's guideline.
        Eol(eol_if_needed);
        *this += "{";
        Indent();
        Eol();
    }
    else
    {
        // Python and Ruby have no block delimiters, so the block is expressed purely by
        // indentation: the body must start on its own line, one level deeper. Indent before
        // Eol() so that the tab for the deeper level is emitted for the new body line.
        Indent();
        Eol(eol_if_needed);
    }

    return *this;
}

Code& Code::CloseBrace(bool all_languages, bool close_ruby)
{
    if (!all_languages && !is_cpp())
    {
        return *this;
    }

    // Ensure there are no trailing tabs
    while (!empty() && wxue::is_whitespace(back()))
    {
        pop_back();
    }
    Unindent();

    if (is_cpp())
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
        while (!empty() && wxue::is_whitespace(back()))
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
    m_minimum_length = cur_pos + 10;  // NOLINT (magic number) // cppcheck-suppress magicLiteral
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

Code& Code::TrueFalseIf(GenEnum::PropName prop_name)
{
    if (m_node->as_bool(prop_name))
    {
        return True();
    }
    return False();
}

Code& Code::Function(wxue::string_view text, bool add_operator)
{
    if (!add_operator)
    {
        if (text.is_sameprefix("wx") && !is_cpp())
        {
            AddFunctionNoOperatorWithWx(text);
        }
        else
        {
            *this += text;
        }
    }
    else if (is_cpp())
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
    else if (is_fortran())
    {
        AddFunctionWithOperatorFortran(text);
    }
    else if (is_go())
    {
        AddFunctionWithOperatorGo(text);
    }
    else if (is_julia())
    {
        AddFunctionWithOperatorJulia(text);
    }
    else if (is_luajit())
    {
        AddFunctionWithOperatorLua(text);
    }
    else if (is_typescript())
    {
        AddFunctionWithOperatorTypeScript(text);
    }

    return *this;
}

Code& Code::ClassMethod(wxue::string_view function_name)
{
    ASSERT(m_traits);
    ASSERT_MSG(!function_name.empty(), "function_name must not be empty");
    *this += m_traits->scope_operator;
    switch (m_method_case)
    {
        case LanguageTraits::MethodCase::snake_case:
            *this += ConvertToSnakeCase(function_name);
            break;

        case LanguageTraits::MethodCase::camel_case:
            {
                std::string camel_name(function_name);
                camel_name[0] =
                    static_cast<char>(std::tolower(static_cast<unsigned char>(camel_name[0])));
                *this += camel_name;
            }
            break;

        case LanguageTraits::MethodCase::pascal_case:
            ASSERT_MSG(m_method_case == LanguageTraits::MethodCase::pascal_case,
                       "expected pascal_case");
            *this += function_name;
            break;
    }

    return *this;
}

Code& Code::VariableMethod(wxue::string_view function_name)
{
    ASSERT(m_traits);
    ASSERT_MSG(!function_name.empty(), "function_name must not be empty");
    *this += '.';
    switch (m_method_case)
    {
        case LanguageTraits::MethodCase::snake_case:
            *this += ConvertToSnakeCase(function_name);
            break;

        case LanguageTraits::MethodCase::camel_case:
            {
                std::string camel_name(function_name);
                camel_name[0] =
                    static_cast<char>(std::tolower(static_cast<unsigned char>(camel_name[0])));
                *this += camel_name;
            }
            break;

        case LanguageTraits::MethodCase::pascal_case:
            ASSERT_MSG(m_method_case == LanguageTraits::MethodCase::pascal_case,
                       "expected pascal_case");
            *this += function_name;
            break;
    }

    return *this;
}

Code& Code::FormFunction(wxue::string_view text)
{
    ASSERT(m_traits);
    ASSERT_MSG(!text.empty(), "text must not be empty");
    if (is_python())
    {
        *this += "self.";
    }
    switch (m_method_case)
    {
        case LanguageTraits::MethodCase::snake_case:
            *this += ConvertToSnakeCase(text);
            break;
        case LanguageTraits::MethodCase::camel_case:
            {
                std::string camel_name(text);
                camel_name[0] =
                    static_cast<char>(std::tolower(static_cast<unsigned char>(camel_name[0])));
                *this += camel_name;
            }
            break;
        case LanguageTraits::MethodCase::pascal_case:
            ASSERT_MSG(m_method_case == LanguageTraits::MethodCase::pascal_case,
                       "expected pascal_case");
            *this += text;
            break;
    }

    return *this;
}

Code& Code::FormParent()
{
    ASSERT(m_traits);
    *this += m_traits->self_reference;
    return *this;
}

Code& Code::Class(wxue::string_view text)
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
    else if (is_ruby())
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
    else if (is_ffi())
    {
        if (text.is_sameprefix("wx"))
        {
            *this << m_language_classPrefix << text.substr(2);
        }
        else
        {
            *this += text;
        }
    }

    return *this;
}

Code& Code::Object(wxue::string_view class_name)
{
    if (is_cpp())
    {
        *this += class_name;
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
    }
    else if (is_ffi())
    {
        if (class_name.is_sameprefix("wx"))
        {
            *this << m_language_classPrefix << class_name.substr(2);
        }
        else
        {
            *this += class_name;
        }
    }
    if (m_traits && !m_traits->construction_suffix.empty())
    {
        *this += m_traits->construction_suffix;
    }
    *this << '(';

    return *this;
}

Code& Code::CreateClass(bool use_generic, wxue::string_view override_name, bool assign)
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

    const std::string class_name = DetermineClassName(use_generic, override_name);
    AddClassNameForLanguage(class_name);

    *this += '(';
    AddSubclassParams();

    return *this;
}

// ==================== kwxFFI descriptor-driven primitives ====================

namespace code_detail
{
    // "TextCtrl::as_ptr" -> "as_ptr"; "Frame::new" -> "new".
    [[nodiscard]] std::string StripTypePrefix(std::string_view symbol)
    {
        if (const size_t pos = symbol.rfind("::"); pos != std::string_view::npos)
        {
            return std::string(symbol.substr(pos + 2));
        }
        return std::string(symbol);
    }

    // "Base.bind" -> "bind"; used for Julia's generated helpers.
    [[nodiscard]] std::string StripBasePrefix(std::string_view symbol)
    {
        constexpr std::string_view base_prefix = "Base.";
        if (symbol.starts_with(base_prefix))
        {
            return std::string(symbol.substr(base_prefix.size()));
        }
        return std::string(symbol);
    }

    [[nodiscard]] std::string ClassType(const FfiUiDescriptor& descriptor,
                                        std::string_view wx_class)
    {
        if (const auto iter = descriptor.classes.find(wx_class);
            iter != descriptor.classes.end() && !iter->second.type.empty())
        {
            return iter->second.type;
        }
        return std::string(wx_class);
    }

    // TypeScript wrapper parameters declared `bigint` (the C i64/usize parameters) reject a
    // `number` literal, while the descriptor's constants and sizes are numbers. Wrap the value
    // explicitly. Every other language takes it unchanged.
    [[nodiscard]] std::string CoerceParam(GenLang language, std::string_view param_type,
                                          std::string_view value)
    {
        if (language == GenLang::typescript && param_type == "bigint" && !value.empty() &&
            !std::string_view(value).starts_with("BigInt("))
        {
            return "BigInt(" + std::string(value) + ")";
        }
        if (language == GenLang::fortran && param_type.starts_with("integer(c_long)") &&
            !value.empty() && !std::string_view(value).starts_with("int("))
        {
            // Constants are declared integer(c_int); the wrapper's c_long dummy needs the
            // explicit conversion when the two kinds differ (LP64 platforms).
            return "int(" + std::string(value) + ", kind=c_long)";
        }
        if (language == GenLang::rust && param_type.starts_with('&') && !value.empty() &&
            !std::string_view(value).starts_with('&'))
        {
            // The kwxRust wrapper takes the wrapper object itself (`&impl WxWindow`,
            // `&impl WxBitmap`), not the native handle the pointer-oriented ports pass.
            return "&" + std::string(value);
        }
        return std::string(value);
    }

    [[nodiscard]] std::string CombineArgs(std::string_view first, std::string_view second)
    {
        if (first.empty())
        {
            return std::string(second);
        }
        if (second.empty())
        {
            return std::string(first);
        }
        return std::string(first) + ", " + std::string(second);
    }

    // Renders `<prefix><symbol>(args)` for a callable whose idiom is constructor/builder/
    // procedure. `wx_class` selects the module table LuaJIT prefixes with.
    void AppendConstructorCall(Code& code, GenLang language, const FfiUiDescriptor& descriptor,
                               const FfiUiCall& call, std::string_view wx_class,
                               std::string_view args)
    {
        switch (language)
        {
            case GenLang::fortran:
                code += call.symbol;
                break;
            case GenLang::go:
                code << "wx." << call.symbol;
                break;
            case GenLang::julia:
                code += call.symbol;
                break;
            case GenLang::luajit:
                // The per-class wrapper modules are re-exported by kwxffi_idiomatic_gen, the
                // port's documented master module (`local wx = require(
                // "kwxffi_idiomatic_gen")` -> wx.wxButton.Create(...)). The `wx` module
                // (lua/wx/init.lua) owns constants, event types and the idiomatic method
                // overrides. gen_luajit.cpp emits both requires under exactly these names.
                code << "wx_classes." << ClassType(descriptor, wx_class) << '.' << call.symbol;
                break;
            case GenLang::rust:
                // Every kwxRust wrapper type is a module-level struct (wx::frame::Frame), and
                // the generated form imports each type it uses by name, so the associated
                // function is spelled `Frame::new(...)` -- `wx::Frame::new` names no item.
                code += call.symbol;
                break;
            case GenLang::typescript:
                code += call.symbol;
                break;
            default:
                code += call.symbol;
                break;
        }
        code << '(' << args << ')';
        if (language == GenLang::typescript)
        {
            // Every TypeScript wrapper factory is declared `| null`. The generated form only
            // builds the natives the wrapper always produces, so assert non-null here instead of
            // threading a null check through every subsequent statement.
            code += '!';
        }
    }
}  // namespace code_detail

using namespace code_detail;

// The C1 contract specifies a nullable probe, so the pointer return is intentional.
// NOLINTNEXTLINE(modernize-use-expected)
const FfiUiDescriptor* Code::FfiDescriptor() const
{
    if (!is_ffi())
    {
        return nullptr;
    }
    return GetFfiUiDescriptor(m_language);
}

bool Code::FfiResolve(std::string_view wx_class, std::string_view role, FfiUiCall& out) const
{
    const FfiUiDescriptor* descriptor = FfiDescriptor();
    if (!descriptor)
    {
        return false;
    }

    if (ResolveRole(*descriptor, wx_class, role, out))
    {
        return true;
    }

    ReportUnsupportedRole(*descriptor, wx_class, role);
    return false;
}

bool Code::FfiCtor(std::string_view wx_class, std::string_view args, std::string_view receiver)
{
    FfiUiCall call;
    if (!FfiResolve(wx_class, "ctor", call))
    {
        return false;
    }

    const FfiUiDescriptor* descriptor = FfiDescriptor();
    if (!descriptor)
    {
        return false;
    }

    if (!receiver.empty())
    {
        *this += receiver;
        *this += " = ";
    }

    AppendConstructorCall(*this, m_language, *descriptor, call, wx_class, args);
    return true;
}

bool Code::FfiMethod(std::string_view wx_class, std::string_view role, std::string_view receiver,
                     std::string_view args)
{
    FfiUiCall call;
    if (!FfiResolve(wx_class, role, call))
    {
        return false;
    }

    if (call.idiom == "constructor" || call.idiom == "builder")
    {
        return FfiCtor(wx_class, args);
    }

    switch (m_language)
    {
        case GenLang::fortran:
            // A module subroutine must be invoked with CALL; a module function must not.
            if (call.returns.empty())
            {
                *this << "call ";
            }
            *this << call.symbol << '(' << CombineArgs(receiver, args) << ')';
            break;
        case GenLang::julia:
            // The Julia idiomatic layer is a set of snake_case functions taking the object as
            // its first argument (`set_title!(obj, text)`); there is no method-on-self syntax.
            // An empty receiver (a `procedure` role such as `bitmap.from_base64`) therefore
            // becomes a plain free-function call rather than a leading `.symbol(...)`.
            *this << StripBasePrefix(call.symbol) << '(' << CombineArgs(receiver, args) << ')';
            break;
        case GenLang::luajit:
            *this << receiver << ':' << call.symbol << '(' << args << ')';
            break;
        case GenLang::rust:
            *this << receiver << '.' << StripTypePrefix(call.symbol) << '(' << args << ')';
            break;
        default:
            *this << receiver << '.' << call.symbol << '(' << args << ')';
            break;
    }

    return true;
}

bool Code::FfiParentMethod(std::string_view wx_class, std::string_view role,
                           std::string_view parent, std::string_view args)
{
    FfiUiCall call;
    if (!FfiResolve(wx_class, role, call))
    {
        return false;
    }

    if (call.idiom == "constructor" || call.idiom == "builder")
    {
        return FfiCtor(wx_class, args);
    }

    switch (m_language)
    {
        case GenLang::fortran:
            // A module subroutine must be invoked with CALL; a module function must not.
            if (call.returns.empty())
            {
                *this << "call ";
            }
            *this << call.symbol << '(' << CombineArgs(parent, args) << ')';
            break;
        case GenLang::julia:
            *this << call.symbol << '(' << CombineArgs(parent, args) << ')';
            break;
        case GenLang::luajit:
            *this << parent << ':' << call.symbol << '(' << args << ')';
            break;
        case GenLang::rust:
            *this << parent << '.' << StripTypePrefix(call.symbol) << '(' << args << ')';
            break;
        default:
            *this << parent << '.' << call.symbol << '(' << args << ')';
            break;
    }

    if (m_language == GenLang::typescript && !call.returns.empty())
    {
        // The wrapper's Create* methods return the new object or null.
        *this += '!';
    }

    return true;
}

bool Code::FfiBind(std::string_view wx_class, std::string_view receiver, std::string_view args)
{
    return FfiMethod(wx_class, "bind", receiver, args);
}

bool Code::FfiSizerItem(std::string_view sizer_name, std::string_view item_name, bool item_is_sizer,
                        int proportion, std::string_view flag_expr, int border,
                        std::string_view flags_ptr)
{
    const std::string_view role = item_is_sizer ? "add_sizer" : "add_window";
    FfiUiCall call;
    if (!FfiResolve("wxBoxSizer", role, call))
    {
        return false;
    }

    // Fill each argument from the descriptor's parameter name, so a language whose add callable
    // takes a built wxSizerFlags handle and one that takes a numeric mask both work.
    std::string args;
    for (const FfiUiParam& param: call.params)
    {
        if (param.name == "self")
        {
            // method-on-self passes the receiver separately.
            continue;
        }

        std::string value = "0";
        if (param.name == "sizer" || param.name == "window")
        {
            value = std::string(item_name);
        }
        else if (param.name == "option" || param.name == "proportion")
        {
            value = std::to_string(proportion);
        }
        else if (param.name == "flags" && !flags_ptr.empty())
        {
            value = std::string(flags_ptr);
        }
        else if (param.name == "flag")
        {
            value = std::string(flag_expr);
        }
        else if (param.name == "border")
        {
            value = std::to_string(border);
        }
        else if (param.name == "userData" || param.name == "userdata" || param.name == "user_data")
        {
            value = FfiNullLiteral();
        }

        value = CoerceParam(m_language, param.type, value);

        if (!args.empty())
        {
            args += ", ";
        }
        args += value;
    }

    return FfiMethod("wxBoxSizer", role, sizer_name, args);
}

bool Code::FfiFlags(std::string_view receiver, int proportion, bool expand,
                    std::string_view border_expr)
{
    const FfiUiDescriptor* descriptor = FfiDescriptor();
    if (!descriptor)
    {
        return false;
    }

    // The sizer-flags callables live in the descriptor's `sizers.flags` block -- ResolveRole()
    // does not route the flags constructor through `classes`, so look it up directly.
    const FfiUiCallMap::const_iterator ctor_iter = descriptor->sizers.find("flags.ctor");
    if (ctor_iter == descriptor->sizers.end() || !ctor_iter->second.IsUsable())
    {
        ReportUnsupportedRole(*descriptor, "wxSizerFlags", "ctor");
        return false;
    }

    const FfiUiCall& ctor = ctor_iter->second;
    switch (m_language)
    {
        case GenLang::go:
            *this << receiver << " := wx." << ctor.symbol << '(' << proportion << ')';
            break;
        case GenLang::typescript:
        case GenLang::julia:
        case GenLang::fortran:
            *this << receiver << " = " << ctor.symbol << '(' << proportion << ')';
            break;
        case GenLang::luajit:
            *this << "local " << receiver << " = wx." << ctor.symbol << '(' << proportion << ')';
            break;
        default:
            *this << receiver << " = wx::" << ctor.symbol << '(' << proportion << ')';
            break;
    }

    // `border_expr` is rendered by the caller with the language spelling of the wrapper
    // constants; each step stays a separate statement because the wrapper's builder methods
    // return void in Go, LuaJIT and Fortran.
    if (expand)
    {
        *this += '\n';
        std::ignore = FfiMethod("wxSizerFlags", "expand", receiver);
    }
    if (!border_expr.empty())
    {
        *this += '\n';
        std::ignore = FfiMethod("wxSizerFlags", "border", receiver, border_expr);
    }

    return true;
}

bool Code::FfiStockButton(std::string_view parent_name, std::string_view button_name,
                          std::string_view flags_expr, bool flags_used)
{
    FfiUiCall create_call;
    if (!FfiResolve("wxStdDialogButtonSizer", "create", create_call))
    {
        return false;
    }

    // The mask parameter is named `flags`; a method-on-parent create also carries the receiver
    // as its first parameter, so the first parameter is not necessarily the mask.
    std::string args;
    if (flags_used)
    {
        std::string flags_type;
        for (const FfiUiParam& param: create_call.params)
        {
            if (param.name == "flags")
            {
                flags_type = param.type;
                break;
            }
        }
        if (flags_type.empty() && create_call.params.size() == 1)
        {
            flags_type = create_call.params.front().type;
        }
        args = CoerceParam(m_language, flags_type, flags_expr);
    }

    // Emits the create expression only: the caller owns the assignment prefix (the languages
    // disagree on `=`, `:=`, `local`, `let`) and emits `realize` as its own statement.
    if (create_call.idiom == "method-on-parent")
    {
        return FfiParentMethod("wxStdDialogButtonSizer", "create", parent_name, args);
    }

    if (m_language == GenLang::rust && create_call.idiom == "builder" && create_call.params.empty())
    {
        // The descriptor's std_buttons.create is the port's zero-argument factory, which cannot
        // express the button mask (a kwxgen descriptor defect, filed in kwxFfiIssues.md). The
        // kwxRust dialog exposes `Dialog::create_std_button_sizer(flags)` -- the same shape the
        // Go, TypeScript, Lua and Julia ports use -- and Realize() runs inside it. The wrapper
        // returns Option, so the assignment unwraps it where the value is produced.
        *this << parent_name << ".create_std_button_sizer(" << args
              << ").expect(\"CreateStdDialogButtonSizer returned null\")";
        return true;
    }

    if (m_language == GenLang::julia && create_call.params.empty())
    {
        // The Julia descriptor records wxStdDialogButtonSizer's empty factory, which cannot
        // express the button mask (a kwxgen descriptor defect, filed in kwxFfiIssues.md). The
        // Julia idiomatic layer realizes the buttons through wxDialog's own method instead --
        // `create_std_dialog_button_sizer(obj, flags)` -- and Realize() runs inside it, which
        // is the same shape the Go, TypeScript and Lua ports expose.
        *this << "create_std_dialog_button_sizer(" << parent_name << ", " << args << ')';
        return true;
    }

    const FfiUiDescriptor* descriptor = FfiDescriptor();
    if (!descriptor)
    {
        return false;
    }

    // Constructor/builder/procedure: prefix the std_buttons.create symbol the way the language
    // wraps a free function. Do not delegate to FfiCtor(), which resolves `classes.ctor` and
    // would report a different (and wrong) role.
    std::ignore = button_name;
    AppendConstructorCall(*this, m_language, *descriptor, create_call, "wxStdDialogButtonSizer",
                          args);
    return true;
}

bool Code::FfiBitmap(std::string_view role, std::string_view receiver, std::string_view args)
{
    FfiUiCall call;
    if (!FfiResolve("wxBitmap", role, call))
    {
        return false;
    }

    if (call.idiom == "constructor")
    {
        const FfiUiDescriptor* descriptor = FfiDescriptor();
        if (!descriptor)
        {
            return false;
        }
        if (!receiver.empty())
        {
            *this += receiver;
            *this += " = ";
        }
        AppendConstructorCall(*this, m_language, *descriptor, call, "wxBitmap", args);
        return true;
    }

    return FfiMethod("wxBitmap", role, receiver, args);
}

bool Code::FfiFont(std::string_view receiver, std::string_view args)
{
    FfiUiCall call;
    if (!FfiResolve("wxFont", "create", call))
    {
        return false;
    }

    const FfiUiDescriptor* descriptor = FfiDescriptor();
    if (!descriptor)
    {
        return false;
    }

    if (!receiver.empty())
    {
        *this += receiver;
        *this += " = ";
    }
    AppendConstructorCall(*this, m_language, *descriptor, call, "wxFont", args);
    return true;
}

bool Code::FfiPtr(std::string_view wx_class, std::string_view receiver)
{
    if (m_language == GenLang::typescript)
    {
        // The TypeScript wrapper exposes the native pointer as the read-only `ptr` getter, not
        // as a method -- selecting the spelling from the role keeps the descriptor authoritative.
        FfiUiCall call;
        if (!FfiResolve(wx_class, "ptr", call))
        {
            return false;
        }
        *this << receiver << '.' << call.symbol;
        return true;
    }

    return FfiMethod(wx_class, "ptr", receiver);
}
std::string Code::FfiPointerAccessor() const
{
    const FfiUiDescriptor* descriptor = FfiDescriptor();
    if (!descriptor)
    {
        return {};
    }
    const FfiUiStringMap::const_iterator iter = descriptor->naming.find("pointer_accessor");
    return iter == descriptor->naming.end() ? std::string() : iter->second;
}

bool Code::FfiDelete(std::string_view wx_class, std::string_view receiver)
{
    return FfiMethod(wx_class, "delete", receiver);
}

std::string Code::FfiConstant(std::string_view name) const
{
    switch (m_language)
    {
        case GenLang::go:
        case GenLang::luajit:
            return "wx." + std::string(name);
        case GenLang::typescript:
            // kwx_constants_gen.ts exports the bare constant name (EXPAND, ID_ANY, ...).
            return std::string(name);
        case GenLang::julia:
            // Constants live in the KwxFFI submodule (the wrapper exports the module, not the
            // 800-odd constant functions); samples spell them `KwxFFI.ID_ANY()`.
            return "KwxFFI." + std::string(name) + "()";
        case GenLang::fortran:
            return "wx" + std::string(name) + "()";
        case GenLang::rust:
            // Events live in `wx::events`; every other constant in `wx::constants`. Both
            // modules expose accessor functions rather than consts, so the call needs `()`.
            if (std::string_view(name).starts_with("EVT_"))
            {
                return "events::" + std::string(name) + "()";
            }
            return "constants::" + std::string(name) + "()";
        default:
            return "wx" + std::string(name);
    }
}

std::string Code::FfiCoerceParam(std::string_view param_type, std::string_view value) const
{
    return CoerceParam(m_language, param_type, value);
}

std::string Code::FfiStringLiteral(std::string_view text) const
{
    std::string result("\"");
    for (const char character: text)
    {
        if (character == '\\' || character == '"')
        {
            result += '\\';
        }
        result += character;
    }
    result += '"';
    return result;
}

bool Code::FfiStringTakesPointer() const
{
    return m_language == GenLang::typescript;
}

std::string Code::FfiNullLiteral() const
{
    switch (m_language)
    {
        case GenLang::go:
        case GenLang::luajit:
            return "nil";
        case GenLang::typescript:
            return "null";
        case GenLang::julia:
            return "C_NULL";
        case GenLang::rust:
            return "std::ptr::null_mut()";
        case GenLang::fortran:
            return "c_null_ptr";
        default:
            return "nullptr";
    }
}

std::string Code::FfiBoolLiteral(bool value) const
{
    if (m_language == GenLang::fortran)
    {
        return value ? ".true." : ".false.";
    }
    return value ? "true" : "false";
}

bool Code::HandleCppSubclass()
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

std::string Code::DetermineClassName(bool use_generic, wxue::string_view override_name) const
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
        std::ignore = class_name.Replace("wx", "wxGeneric");
        return std::string(class_name);
    }

    if (m_node->is_Gen(gen_BookPage) || m_node->is_Gen(gen_TreePage))
    {
        return "wxPanel";
    }

    return std::string(class_name);
}

Code& Code::Assign(wxue::string_view class_name)
{
    *this += " = ";
    if (class_name.empty())
    {
        return *this;
    }

    ASSERT(m_traits);
    if (m_traits->is_cpp_family())
    {
        *this << "new " << class_name << ';';
    }
    else
    {
        *this << m_language_classPrefix << class_name.substr(2);
        if (!m_traits->construction_suffix.empty())
        {
            *this += m_traits->construction_suffix;
        }
    }

    return *this;
}

Code& Code::EndFunction()
{
    ASSERT(m_traits);
    if (m_traits->removes_empty_parens && back() == '(')
    {
        // Ruby style guidelines recommend not using empty parentheses
        pop_back();
    }
    else
    {
        *this += ')';
    }

    if (!m_traits->stmt_end.empty())
    {
        *this += m_traits->stmt_end;
    }
    return *this;
}

Code& Code::NodeName(Node* node)
{
    if (!node)
    {
        node = m_node;
    }
    const std::string_view node_name = node->get_NodeName(get_language());
    if (is_python())
    {
        if (!node->is_Form() && node->as_string(prop_class_access) != "none" &&
            node->as_string(prop_class_access) != "public")
        {
            *this += "self.";
        }
    }
    else if (is_ruby() && node->is_Form())
    {
        // A Ruby class name must be a constant. A project class name such as `kwxMainFrame`
        // would otherwise emit `class kwxMainFrame < Wx::Frame`, which is a SyntaxError.
        if (!node_name.empty() && node_name.front() >= 'a' && node_name.front() <= 'z')
        {
            *this += static_cast<char>(std::toupper(static_cast<unsigned char>(node_name.front())));
            *this += node_name.substr(1);
            return *this;
        }
    }
    else if (is_ruby() && !node->is_Local() && node_name[0] != '@')
    {
        *this += "@";
    }

    *this += node_name;
    return *this;
}

Code& Code::VarName(wxue::string_view var_name, bool class_access)
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

Code& Code::ParentName()
{
    NodeName(m_node->get_Parent());
    return *this;
}

[[nodiscard]] bool Code::is_local_var() const
{
    return m_node->is_Local();
}

[[nodiscard]] bool Code::HasValue(GenEnum::PropName prop_name) const
{
    return m_node->HasValue(prop_name);
}

[[nodiscard]] int Code::IntValue(GenEnum::PropName prop_name) const
{
    return m_node->as_int(prop_name);
}

[[nodiscard]] bool Code::PropContains(GenEnum::PropName prop_name, wxue::string_view text) const
{
    return m_node->as_string(prop_name).contains(text);
}

[[nodiscard]] size_t Code::PropSize(GenEnum::PropName prop_name) const
{
    return m_node->as_string(prop_name).size();
}

// List of valid component parent types
constexpr auto s_GenParentTypes = std::to_array<GenType>(
    { type_aui_toolbar, type_auinotebook, type_bookpage, type_choicebook, type_container,
      type_listbook, type_notebook, type_panel, type_propgridpage, type_ribbonbar,
      type_ribbonbar_form, type_ribbonpanel, type_simplebook, type_splitter, type_statusbar,
      type_toolbar, type_treepage, type_wizardpagesimple });

void Code::AddFormParentName()
{
    ASSERT(m_traits);
    *this += m_traits->self_reference;
}

Code& Code::ValidParentName()
{
    Node* parent = m_node->get_Parent();
    while (parent)
    {
        if (parent->is_Sizer() && parent->is_StaticBoxSizer())
        {
            NodeName(parent);
            if (m_traits && m_traits->removes_empty_parens)
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

    FAIL_MSG(wxue::string() << m_node->get_NodeName() << " has no parent!");
    return *this;
}

[[nodiscard]] bool Code::IsDefaultPosSizeFlags(wxue::string_view def_style) const
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

[[nodiscard]] int Code::WhatParamsNeeded(wxue::string_view default_style) const
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

Code& Code::SizerFlagsFunction(wxue::string_view function_name)
{
    *this += '.';
    switch (m_method_case)
    {
        case LanguageTraits::MethodCase::snake_case:
            *this += ConvertToSnakeCase(function_name);
            break;
        case LanguageTraits::MethodCase::camel_case:
            {
                std::string camel_name(function_name);
                if (!camel_name.empty())
                {
                    camel_name[0] =
                        static_cast<char>(std::tolower(static_cast<unsigned char>(camel_name[0])));
                }
                *this += camel_name;
            }
            break;
        default:
            *this += function_name;
            break;
    }
    *this += '(';
    return *this;
}

constexpr int DEFAULT_BORDER_SIZE = 5;

Code& Code::BorderSize(GenEnum::PropName prop_name)
{
    const int border_size = m_node->as_int(prop_name);
    const bool is_scalable_border =
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

Code& Code::ColourCode(GenEnum::PropName prop_name)
{
    if (!HasValue(prop_name))
    {
        Add("wxNullColour");
    }
    else
    {
        const wxColour colour = m_node->as_wxColour(prop_name);
        Object("wxColour").QuotedString(colour) += ')';
    }

    return *this;
}

bool Code::is_ScalingEnabled(GenEnum::PropName prop_name, int enable_dpi_scaling) const
{
    if (enable_dpi_scaling == code::no_dpi_scaling ||
        wxue::contains(m_node->as_string(prop_name), 'n', wxue::CASE::either))
    {
        return false;
    }

    if (enable_dpi_scaling == code::conditional_scaling && m_node->is_Form())
    {
        return false;
    }

    return true;
}

Code& Code::BeginConditional()
{
    ASSERT(m_traits);
    *this << m_traits->conditional_begin;
    return *this;
}

Code& Code::EndConditional()
{
    ASSERT(m_traits);
    *this << m_traits->conditional_end;
    return *this;
}

Code& Code::True()
{
    ASSERT(m_traits);
    *this << m_traits->true_literal;
    return *this;
}

Code& Code::False()
{
    ASSERT(m_traits);
    *this << m_traits->false_literal;
    return *this;
}

// Returns the number of leading space or tab characters in the line.
static size_t GetLeadingWhitespace(const wxue::string& line)
{
    size_t count = 0;
    while (count < line.size() && (line[count] == ' ' || line[count] == '\t'))
    {
        ++count;
    }
    return count;
}

// Removes the indentation that is common to every non-blank line of a lambda function's body. Any
// additional indentation is preserved so that nested code stays readable.
static void RemoveCommonIndentation(wxue::string& lambda)
{
    wxue::StringVector lines(lambda, '\n');

    size_t open_brace = lines.size();
    size_t close_brace = 0;
    for (size_t pos = 0; pos < lines.size(); ++pos)
    {
        if (lines[pos].contains("{"))
        {
            // The first line containing an opening brace starts the lambda body
            open_brace = pos;
            break;
        }
    }
    for (size_t pos = 0; pos < lines.size(); ++pos)
    {
        if (lines[pos].contains("}"))
        {
            // The last line containing a closing brace ends the lambda body
            close_brace = pos;
        }
    }

    if (open_brace == lines.size() || open_brace + 1 >= close_brace)
    {
        // Single-line lambda, or a body with no lines to de-indent
        return;
    }

    size_t common = 0;
    bool first_line = true;
    for (size_t pos = open_brace + 1; pos < close_brace; ++pos)
    {
        const size_t leading = GetLeadingWhitespace(lines[pos]);
        if (leading == lines[pos].size())
        {
            // Blank line -- ignore it when calculating the common indentation
            continue;
        }
        if (first_line)
        {
            common = leading;
            first_line = false;
        }
        else if (leading < common)
        {
            common = leading;
        }
    }

    if (first_line || common == 0)
    {
        return;
    }

    for (size_t pos = open_brace + 1; pos < close_brace; ++pos)
    {
        lines[pos].erase(0, common);
    }

    lambda.clear();
    for (size_t pos = 0; pos < lines.size(); ++pos)
    {
        if (pos > 0)
        {
            lambda += '\n';
        }
        lambda += lines[pos];
    }
}

Code& Code::ExpandEventLambda(wxue::string lambda)
{
    lambda.LeftTrim();
    std::ignore = lambda.Replace("@@", "\n", wxue::REPLACE::all);
    lambda.RightTrim();

    if (is_cpp())
    {
        RemoveCommonIndentation(lambda);

        // WriteLambdaEventLines() now preserves the leading whitespace of every line, so no tabs
        // may be added here. The string handed to it must contain only the user's own
        // (de-indented) whitespace.
        const int saved_indent = get_Indentation();
        ResetIndent();
        Eol();
        const wxue::StringVector lines(lambda, '\n');

        for (auto& line: lines)
        {
            Str(line);
            Eol();
        }
        Indent(saved_indent);

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
