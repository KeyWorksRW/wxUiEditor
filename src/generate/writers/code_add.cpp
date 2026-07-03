/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code class Add functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-29-2026]

#include <frozen/map.h>
#include <frozen/set.h>

#include "code.h"

#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Miscellaneous utilities

#include "wxue_namespace/wxue_view_vector.h"  // wxue::ViewVector

// clang-format off

// clang-format on

void Code::AddClassNameForLanguage(const std::string& class_name)
{
    if (is_cpp())
    {
        *this += class_name;
        return;
    }

    // Handle non-C++ languages
    if (class_name.starts_with("wx"))
    {
        *this << m_language_wxPrefix << class_name.substr(2);
    }
    else
    {
        *this += class_name;
    }

    // Add language-specific instantiation suffix (e.g., ".new" for Ruby)
    ASSERT(m_traits);
    if (!m_traits->construction_suffix.empty())
    {
        *this += m_traits->construction_suffix;
    }
}

void Code::AddSubclassParams()
{
    if (m_node->HasValue(prop_subclass_params))
    {
        *this += m_node->as_string(prop_subclass_params);
        RightTrim();
        if (back() != ',')
        {
            Comma();
        }
        else
        {
            *this += ' ';
        }
    }
}

void Code::AddFunctionNoOperatorWithWx(wxue::string_view text)
{
    if (is_ruby())
    {
        *this << "Wx::" << ConvertToSnakeCase(text.substr(sizeof("wx") - 1));
    }
    else if (is_fortran() || is_julia() || is_luajit())
    {
        // wxGetStockLabel( → wx_get_stock_label(
        *this << "wx_" << ConvertToSnakeCase(text.substr(sizeof("wx") - 1));
    }
    else
    {
        // Python, Go, TypeScript: wx.GetStockLabel(
        *this << "wx." << text.substr(sizeof("wx") - 1);
    }
}

void Code::AddFunctionWithOperatorRuby(wxue::string_view text)
{
    // Check for a preceding empty "()" and remove it if found
    if (ends_with("())"))
    {
        resize(size() - 2);
    }

    *this << '.';
    if (text.is_sameprefix("wx"))
    {
        *this << "Wx::" << text.substr(sizeof("wx") - 1);
    }
    else
    {
        *this += ConvertToSnakeCase(text);
    }
}

void Code::AddFunctionWithOperatorPython(wxue::string_view text)
{
    *this << '.';
    if (text.is_sameprefix("wx"))
    {
        *this << "wx." << text.substr(sizeof("wx") - 1);
    }
    else
    {
        *this += text;
    }
}

void Code::AddFunctionWithOperatorFortran(wxue::string_view text)
{
    // Fortran uses the C-wrapper calling convention: wx_button_set_label(button, ...)
    // The underscore connects to the preceding variable name (e.g., wx_button +
    // _set_label = wx_button_set_label)
    *this << '_' << ConvertToSnakeCase(text);
}

void Code::AddFunctionWithOperatorGo(wxue::string_view text)
{
    // Go uses .PascalCase method calls: button.SetLabel(...)
    *this << '.' << text;
}

void Code::AddFunctionWithOperatorJulia(wxue::string_view text)
{
    // Julia uses standalone snake_case functions with object as first argument:
    // set_status_text(frame, "Ready"). No operator prefix — the function name
    // stands alone.
    *this += ConvertToSnakeCase(text);
}

void Code::AddFunctionWithOperatorLua(wxue::string_view text)
{
    // LuaJIT uses : operator with snake_case: button:set_label(...)
    *this << ':' << ConvertToSnakeCase(text);
}

void Code::AddFunctionWithOperatorTypeScript(wxue::string_view text)
{
    // TypeScript uses .PascalCase method calls: button.SetLabel(...)
    *this << '.' << text;
}

Code& Code::Add(wxue::string_view text)
{
    bool old_linebreak = m_auto_break;
    // Ruby changes the prefix to "Wx::", and Python changes it to "wx."
    // C++ uses the constant unmodified.
    //
    // "wx" is the shortest string that could be changed -- no single letter will ever be changed by
    // this function.
    if (is_cpp() || text.size() < (sizeof("wx") - 1))
    {
        CheckLineLength(text.size());
        *this += text;
        return *this;
    }

    // Handle language-specific transformations for Ruby and Python

    if (is_ruby())
    {
        if (!text.empty())
        {
            // Ruby doesn't like breaking the parenthesis for a function call onto the next
            // line, or the .new function
            if (text.front() == '.' || text.front() == '(')
            {
                old_linebreak = m_auto_break;
                m_auto_break = false;
            }
        }

        // Check for Ruby-specific constant mappings
        if (AddRubyConstant(text))
        {
            return *this;
        }
    }

    // Handle combined values separated by pipes
    if (text.find('|') != wxue::npos)
    {
        AddCombinedValues(text);
    }
    // Handle single wx-prefixed constants
    else if (text.is_sameprefix("wx") && !is_cpp())
    {
        AddWxPrefixedConstant(text);
    }
    else
    {
        CheckLineLength(text.size());
        *this += text;
    }

    // In case linebreak was shut off
    m_auto_break = old_linebreak;

    return *this;
}

// Helper method: Check if text matches a Ruby constant mapping
[[nodiscard]] bool Code::AddRubyConstant(wxue::string_view text)
{
    // Handle Ruby-specific constant mappings
    static constexpr frozen::map<std::string_view, std::string_view, 6> ruby_constant_map =
        frozen::make_map<std::string_view, std::string_view>(
            { { "wxEmptyString", "''" },
              { "wxDefaultCoord", "Wx::DEFAULT_COORD" },
              { "wxDefaultSize", "Wx::DEFAULT_SIZE" },
              { "wxDefaultPosition", "Wx::DEFAULT_POSITION" },
              { "wxNullBitmap", "Wx::NULL_BITMAP" },
              { "wxNullAnimation", "Wx::NULL_ANIMATION" } });

    if (const auto* iter = ruby_constant_map.find(text); iter != ruby_constant_map.end())
    {
        *this += iter->second;
        return true;
    }
    return false;
}

// Helper method: Handle combined values separated by pipes (|)
Code& Code::AddCombinedValues(wxue::string_view text)
{
    bool initial_combined_value_set = false;
    const wxue::ViewVector multistr(text, "|", wxue::TRIM::both);
    for (auto& iter: multistr)
    {
        if (iter.empty())
        {
            continue;
        }
        if (initial_combined_value_set)
        {
            *this += '|';
        }
        if (iter.is_sameprefix("wx") && !is_cpp())
        {
            CheckLineLength(m_language_wxPrefix.size() + iter.size() - 2);
            *this << m_language_wxPrefix << iter.substr(2);
        }
        else
        {
            CheckLineLength(iter.size());
            *this += iter;
        }
        initial_combined_value_set = true;
    }
    return *this;
}

// Helper method: Handle wx-prefixed constants with language transformations
Code& Code::AddWxPrefixedConstant(wxue::string_view text)
{
    CheckLineLength(m_language_wxPrefix.size() + text.size() - 2);
    *this << m_language_wxPrefix << text.substr(2);
    return *this;
}

Code& Code::AddComment(std::string_view comment, bool force)
{
    if (!Project.AddOptionalComments() && !force)
    {
        return *this;
    }
    ASSERT(m_traits);
    Eol(eol_if_needed);
    *this << m_traits->line_comment << comment;
    Eol(eol_if_needed);

    return *this;
}

Code& Code::AddAuto()
{
    ASSERT(m_traits);
    if (is_local_var() && !m_traits->local_var_keyword.empty())
    {
        *this += m_traits->local_var_keyword;
    }
    return *this;
}

Code& Code::AddConditionalAnd()
{
    ASSERT(m_traits);
    *this << m_traits->logical_and;
    return *this;
}

Code& Code::AddConditionalOr()
{
    ASSERT(m_traits);
    *this << m_traits->logical_or;
    return *this;
}

Code& Code::AddConstant(GenEnum::PropName prop_name, wxue::string_view short_name)
{
    return Add(m_node->as_constant(prop_name, short_name));
}

Code& Code::AddConstant(wxue::string_view text)
{
    if (is_cpp())
    {
        CheckLineLength(text.size());
        *this += text;
        return *this;
    }

    return Add(text);
}

void Code::AddPublicRubyMembers()
{
    ASSERT(is_ruby());
    std::set<wxue::string> public_members;
    auto FindPublicMembers = [&](Node* node, auto&& FindPublicMembers) -> void
    {
        if (node->HasProp(prop_var_name) && node->as_string(prop_class_access) == "public:")
        {
            public_members.insert(wxue::string(":") << node->get_NodeName(get_language()));
        }
        if (node->get_ChildCount())
        {
            for (const auto& child: node->get_ChildNodePtrs())
            {
                FindPublicMembers(child.get(), FindPublicMembers);
            }
        }
    };

    FindPublicMembers(m_node, FindPublicMembers);

    if (!public_members.empty())
    {
        Indent(1);
        Tab() << "attr_accessor ";
        bool is_first = true;
        for (const auto& member: public_members)
        {
            if (!is_first)
            {
                *this << ", ";
            }
            *this << member;
            CheckLineLength();
            is_first = false;
        }
        ResetIndent();
        Eol();
    }
}

Code& Code::AddType(wxue::string_view text)
{
    if (is_cpp() || text.size() < 3)
    {
        CheckLineLength(text.size());
        *this += text;
    }
    else if (is_ruby())
    {
        const wxue::string new_text = ConvertToUpperSnakeCase(text.substr(2));
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
