/////////////////////////////////////////////////////////////////////////////
// Purpose:   String and quote handling for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "code.h"

#include "project_handler.h"  // ProjectHandler class

using namespace code;

auto Code::as_string(PropName prop_name) -> Code&
{
    if (prop_name == prop_id)
    {
        auto result = m_node->get_PropId();
        CheckLineLength(result.size());

        // For Ruby, if it doesn't start with 'wx' then assume it is a global with a '$' prefix
        if (is_ruby() && !result.is_sameprefix("wx"))
        {
            *this << '$' << result;
            return *this;
        }
        if (!is_cpp() && !is_perl())
        {
            result.Replace("wx", m_language_wxPrefix);
        }
        *this += result;
        return *this;
    }

    return Add(m_node->as_string(prop_name));
}

auto Code::QuotedString(GenEnum::PropName prop_name) -> Code&
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

    return QuotedString(m_node->as_string(prop_name));
}

void Code::ProcessEscapedChar(char chr, bool& has_escape)
{
    switch (chr)
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
            *this += chr;
            break;
    }
}

[[nodiscard]] auto Code::HasUtf8Char(wxue::string_view text) -> bool
{
    return std::ranges::any_of(text,
                               [](auto iter)
                               {
                                   return iter < 0;
                               });
}

void Code::AddQuoteClosing(bool has_escape, size_t begin_quote, bool has_utf_char)
{
    if (is_ruby())
    {
        *this += '\'';
    }
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
    {
        *this += '"';
    }

    if (has_utf_char)
    {
        *this += ')';
    }
}

auto Code::QuotedString(wxue::string_view text) -> Code&
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
            Function("wxGetTranslation");
        }
    }

    bool has_utf_char = is_cpp() && HasUtf8Char(text);
    if (has_utf_char)
    {
        *this += "wxString::FromUTF8(";
    }

    auto begin_quote = this->size();
    bool has_escape = false;

    if (is_ruby() || is_perl())
    {
        *this += '\'';
    }
    else
    {
        *this += '"';
    }

    for (auto chr: text)
    {
        ProcessEscapedChar(chr, has_escape);
    }

    AddQuoteClosing(has_escape, begin_quote, has_utf_char);

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
