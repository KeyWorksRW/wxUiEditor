/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code class Add functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <frozen/map.h>
#include <frozen/set.h>

#include "code.h"

#include "preferences.h"     // Prefs -- Set/Get wxUiEditor preferences
#include "tt_view_vector.h"  // tt_view_vector -- Class for reading and writing line-oriented strings/files
#include "utils.h"           // Miscellaneous utilities

// clang-format off

constexpr auto set_perl_constants = frozen::make_set<std::string_view>({

    "wxALL",
    "wxLEFT",
    "wxRIGHT",
    "wxTOP",
    "wxBOTTOM",

    "wxEXPAND",
    "wxSHAPED",
    "wxFIXED_MINSIZE",
    "wxRESERVE_SPACE_EVEN_IF_HIDDEN",

    "wxALIGN_CENTER_HORIZONTAL",
    "wxALIGN_CENTER_VERTICAL",
    "wxALIGN_LEFT",
    "wxALIGN_RIGHT",
    "wxALIGN_TOP",
    "wxALIGN_BOTTOM",
    "wxALIGN_CENTER",

    "wxITEM_CHECK",
    "wxITEM_DROPDOWN",
    "wxITEM_NORMAL",
    "wxITEM_RADIO",

    "wxNullBitmap",
    "wxID_ANY",

    "wxVERTICAL",
    "wxHORIZONTAL",
    "wxBOTH",

    "wxWINDOW_VARIANT_LARGE",
    "wxWINDOW_VARIANT_SMALL",
    "wxWINDOW_VARIANT_MINI",

});

// clang-format on

Code& Code::Add(tt_string_view text)
{
    bool old_linebreak = m_auto_break;
    // Ruby changes the prefix to "Wx::", and Python changes it to "wx."
    // C++, Perl, and Rust use the constant unmodified.
    //
    // "wx" is the shortest string that could be changed -- no single letter will ever be changed by
    // this function.
    if (is_cpp() || is_perl() || is_rust() || text.size() < (sizeof("wx") - 1))
    {
        if (is_perl() && text == "wxEmptyString")
        {
            // wxPerl doesn't support wxEmptyString
            *this += "\"\"";
        }
        else
        {
            CheckLineLength(text.size());
            *this += text;
        }
    }
    else
    {
        if (is_ruby())
        {
            if (text.size())
            {
                // Ruby doesn't like breaking the parenthesis for a function call onto the next line,
                // or the .new function
                if (text.front() == '.' || text.front() == '(')
                {
                    old_linebreak = m_auto_break;
                    m_auto_break = false;
                }
            }
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
#if 0
                    if (is_perl() && (HasPerlMapConstant(text) || set_perl_constants.contains(text)))
                    {
                        CheckLineLength(text.size());
                        *this += text;
                        initial_combined_value_set = true;
                        continue;
                    }
#endif
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
            return *this;  // no modifier for local variables in Python
        }
        else if (is_ruby())
        {
            return *this;  // no modifier for local variables in Ruby
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

Code& Code::AddConstant(GenEnum::PropName prop_name, tt_string_view short_name)
{
    return Add(m_node->as_constant(prop_name, short_name));
}

// clang-format off
constexpr auto show_effect_map = frozen::make_map<std::string_view, std::string_view>({
    { "wxSHOW_EFFECT_NONE", "0" },
    { "wxSHOW_EFFECT_ROLL_TO_LEFT", "1" },
    { "wxSHOW_EFFECT_ROLL_TO_RIGHT", "2" },
    { "wxSHOW_EFFECT_ROLL_TO_TOP", "3" },
    { "wxSHOW_EFFECT_ROLL_TO_BOTTOM", "4" },
    { "wxSHOW_EFFECT_SLIDE_TO_LEFT", "5" },
    { "wxSHOW_EFFECT_SLIDE_TO_RIGHT", "6" },
    { "wxSHOW_EFFECT_SLIDE_TO_TOP", "7" },
    { "wxSHOW_EFFECT_SLIDE_TO_BOTTOM", "8" },
    { "wxSHOW_EFFECT_BLEND", "9" },
    { "wxSHOW_EFFECT_EXPAND", "10" },
    { "wxSHOW_EFFECT_MAX", "11" }
});
// clang-format on

Code& Code::AddConstant(tt_string_view text)
{
    if (is_cpp())
    {
        CheckLineLength(text.size());
        *this += text;
        return *this;
    }
    else if (is_perl())
    {
        // In some cases, wxPerl doesn't supprt a constant, but if we use the numeric value instead,
        // then it works fine.
        if (text.contains("wxBU_NOTEXT"))
        {
            tt_string new_value(text);
            new_value.Replace("wxBU_NOTEXT", "2");
            CheckLineLength(new_value.size());
            *this += new_value;
        }
        else if (text.contains("wxSHOW_EFFECT"))
        {
            tt_string new_value(text);
            for (auto& iter: show_effect_map)
            {
                if (new_value.Replace(iter.first, iter.second))
                    break;
            }
            CheckLineLength(new_value.size());
            *this += new_value;
        }
        else
        {
            *this += text;
        }
        return *this;
    }
    return Add(text);
}

void Code::AddPublicRubyMembers()
{
    ASSERT(is_ruby());
    std::set<tt_string> public_members;
    auto FindPublicMembers = [&](Node* node, auto&& FindPublicMembers) -> void
    {
        if (node->hasProp(prop_var_name) && node->as_string(prop_class_access) == "public:")
        {
            public_members.insert(tt_string(":") << node->getNodeName(get_language()));
        }
        if (node->getChildCount())
        {
            for (const auto& child: node->getChildNodePtrs())
            {
                FindPublicMembers(child.get(), FindPublicMembers);
            }
        }
    };

    FindPublicMembers(m_node, FindPublicMembers);

    if (public_members.size())
    {
        Indent(1);
        Tab() << "attr_accessor ";
        for (auto iter = public_members.begin(); iter != public_members.end(); ++iter)
        {
            if (iter != public_members.begin())
                *this << ", ";
            *this << *iter;
            CheckLineLength();
        }
        ResetIndent();
        Eol();
    }
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
