/////////////////////////////////////////////////////////////////////////////
// Purpose:   String and quote handling for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-29-2026]

#include "code.h"

#include "project_handler.h"  // ProjectHandler class

using namespace code;

Code& Code::as_string(PropName prop_name)
{
    if (prop_name == prop_id)
    {
        wxue::string result = m_node->get_PropId();
        CheckLineLength(result.size());

        // For Ruby, if it doesn't start with 'wx' then assume it is a global with a '$' prefix
        if (is_ruby() && !result.is_sameprefix("wx"))
        {
            *this << '$' << result;
            return *this;
        }
        if (!is_cpp())
        {
            std::ignore = result.Replace("wx", m_language_wxPrefix);
        }
        *this += result;
        return *this;
    }

    return Add(m_node->as_string(prop_name));
}

Code& Code::QuotedString(GenEnum::PropName prop_name)
{
    if (!m_node->HasValue(prop_name))
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

    // A window name is an internal identifier, not user-visible text, so it must never be wrapped
    // in a translation function. Translating it would make GetName() and FindWindowByName()
    // locale-dependent. See issue #1876.
    if (prop_name == prop_window_name)
    {
        return AddQuotedText(m_node->as_string(prop_name), false);
    }

    return QuotedString(m_node->as_string(prop_name));
}

void Code::ProcessEscapedChar(char char_val, bool& has_escape)
{
    switch (char_val)
    {
        case '"':
            *this += "\\\"";
            has_escape = true;
            break;

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
            *this += char_val;
            break;
    }
}

[[nodiscard]] bool Code::HasUtf8Char(wxue::string_view text)
{
    return std::ranges::any_of(text,
                               [](auto iter)
                               {
                                   return iter < 0;
                               });
}

void Code::AddQuoteClosing(bool has_escape, size_t begin_quote, bool has_utf_char)
{
    if (is_ruby() && has_escape)
    {
        at(begin_quote) = '"';
        *this += '"';
        return;
    }

    if (is_ruby())
    {
        *this += '\'';
    }
    else
    {
        *this += '"';
    }

    if (has_utf_char)
    {
        *this += ')';
    }
}

Code& Code::QuotedString(wxue::string_view text)
{
    return AddQuotedText(text, true);
}

Code& Code::AddQuotedText(wxue::string_view text, bool translate)
{
    const size_t cur_pos = this->size();

    const bool internationalize =
        translate && Project.as_bool(prop_internationalize) && wxue::has_alpha(text);
    const bool has_utf_char = is_cpp() && HasUtf8Char(text);
    if (internationalize)
    {
        if (is_cpp())
        {
            // wxWidgets 3.3's _() only accepts a string literal, so a string that also needs
            // wxString::FromUTF8() must be translated with wxGetTranslation() instead.
            if (has_utf_char)
            {
                *this += "wxGetTranslation(";
            }
            else
            {
                *this += "_(";
            }
        }
        else
        {
            Function("wxGetTranslation");
        }
    }

    if (has_utf_char)
    {
        *this += "wxString::FromUTF8(";
    }

    const size_t begin_quote = this->size();
    bool has_escape = false;

    if (is_ruby())
    {
        *this += '\'';
    }
    else
    {
        *this += '"';
    }

    for (auto char_val: text)
    {
        ProcessEscapedChar(char_val, has_escape);
    }

    AddQuoteClosing(has_escape, begin_quote, has_utf_char);

    if (internationalize)
    {
        *this += ')';
    }

    if (m_auto_break && size() > m_break_at)
    {
        InsertLineBreak(cur_pos);
    }

    return *this;
}
