/////////////////////////////////////////////////////////////////////////////
// Purpose:   std::string_view with additional methods
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include <cctype>
#include <cstring>
#include <locale>

#include "wxue_string.h"

namespace wxue
{

    bool string_view::is_sameas(std::string_view str, wxue::CASE checkcase) const
    {
        if (size() != str.size())
        {
            return false;
        }

        if (empty())
        {
            return str.empty();
        }

        // if both strings have the same length, then we can compare as a prefix.
        return is_sameprefix(str, checkcase);
    }

    bool string_view::is_sameprefix(std::string_view str, wxue::CASE checkcase) const
    {
        if (str.empty())
        {
            return empty();
        }

        if (empty() || length() < str.length())
        {
            return false;
        }

        if (checkcase == wxue::CASE::exact)
        {
            auto iterMain = begin();
            for (auto iterSub: str)
            {
                if (*iterMain++ != iterSub)
                {
                    return false;
                }
            }
            return true;
        }

        if (checkcase == wxue::CASE::either)
        {
            auto iterMain = begin();
            for (auto iterSub: str)
            {
                if (std::tolower(*iterMain++) != std::tolower(iterSub))
                {
                    return false;
                }
            }
            return true;
        }

        if (checkcase == wxue::CASE::utf8)
        {
            auto utf8locale = std::locale("en_US.utf8");
            auto iterMain = begin();
            for (auto iterSub: str)
            {
                if (std::tolower(*iterMain++, utf8locale) != std::tolower(iterSub, utf8locale))
                {
                    return false;
                }
            }
            return true;
        }
        FAIL_MSG("Unknown wxue::CASE value");
        return false;
    }

    size_t string_view::locate(std::string_view needle, size_t posStart, wxue::CASE checkcase) const
    {
        if (needle.empty() || posStart >= size())
        {
            return npos;
        }

        if (checkcase == wxue::CASE::exact)
        {
            return find(needle, posStart);
        }

        if (checkcase == wxue::CASE::either)
        {
            auto chLower = std::tolower(needle[0]);
            for (auto pos = posStart; pos < length(); ++pos)
            {
                if (std::tolower(at(pos)) == chLower)
                {
                    size_t posSub;
                    for (posSub = 1; posSub < needle.length(); ++posSub)
                    {
                        if (pos + posSub >= length())
                        {
                            return npos;
                        }
                        if (std::tolower(at(pos + posSub)) != std::tolower(needle.at(posSub)))
                        {
                            break;
                        }
                    }
                    if (posSub >= needle.length())
                    {
                        return pos;
                    }
                }
            }
        }
        else
        {
            // For UTF-8, use wxString which properly handles multi-byte sequences
            wxString wxHaystack = wxString::FromUTF8(data(), length());
            wxString wxNeedle = wxString::FromUTF8(needle.data(), needle.length());
            wxHaystack.MakeLower();
            wxNeedle.MakeLower();

            auto wxPos = wxHaystack.find(wxNeedle, posStart);
            if (wxPos == wxString::npos)
            {
                return npos;
            }
            return wxPos;
        }
        return npos;
    }

    bool string_view::moveto_space() noexcept
    {
        if (empty())
        {
            return false;
        }
        size_t pos;
        for (pos = 0; pos < length(); ++pos)
        {
            if (wxue::is_whitespace(at(pos)))
            {
                break;
            }
        }
        if (pos >= length())
        {
            return false;
        }

        remove_prefix(pos);
        return true;
    }

    bool string_view::moveto_nonspace() noexcept
    {
        if (empty())
        {
            return false;
        }
        size_t pos;
        for (pos = 0; pos < length(); ++pos)
        {
            if (!wxue::is_whitespace(at(pos)))
            {
                break;
            }
        }
        if (pos >= length())
        {
            return false;
        }

        remove_prefix(pos);
        return true;
    }

    bool string_view::moveto_nextword() noexcept
    {
        if (empty())
        {
            return false;
        }
        size_t pos;
        for (pos = 0; pos < length(); ++pos)
        {
            if (wxue::is_whitespace(at(pos)))
            {
                break;
            }
        }
        if (pos >= length())
        {
            return false;
        }

        // whitespace found, look for non-whitespace
        for (++pos; pos < length(); ++pos)
        {
            if (!wxue::is_whitespace(at(pos)))
            {
                break;
            }
        }
        if (pos >= length())
        {
            return false;
        }

        remove_prefix(pos);
        return true;
    }

    string_view string_view::view_digit(size_t start) const
    {
        for (; start < length(); ++start)
        {
            if (wxue::is_digit(at(start)))
            {
                return subview(start);
            }
        }

        return subview(length());
    }

    string_view string_view::view_nondigit(size_t start) const
    {
        for (; start < length(); ++start)
        {
            if (!wxue::is_digit(at(start)))
            {
                return subview(start);
            }
        }

        return subview(length());
    }

    bool string_view::moveto_digit() noexcept
    {
        if (empty())
        {
            return false;
        }
        size_t pos;
        for (pos = 0; pos < length(); ++pos)
        {
            if (wxue::is_digit(at(pos)))
            {
                break;
            }
        }
        if (pos >= length())
        {
            return false;
        }

        remove_prefix(pos);
        return true;
    }

    bool string_view::moveto_nondigit() noexcept
    {
        if (empty())
        {
            return false;
        }
        size_t pos;
        for (pos = 0; pos < length(); ++pos)
        {
            if (!wxue::is_digit(at(pos)))
            {
                break;
            }
        }
        if (pos >= length())
        {
            return false;
        }

        remove_prefix(pos);
        return true;
    }

    bool string_view::moveto_extension() noexcept
    {
        if (empty())
        {
            return false;
        }

        auto pos = rfind('.');
        if (pos == npos)
        {
            return false;
        }
        if (pos + 1 >= length())  // . by itself is a folder
        {
            return false;
        }
        if (pos < 2 && (at(pos + 1) == '.'))
        {
            return false;
        }

        remove_prefix(pos);
        return true;
    }

    bool string_view::moveto_filename() noexcept
    {
        if (empty())
        {
            return false;
        }

        auto pos = find_last_of('/');

#ifdef _WIN32
        // Windows filenames can contain both forward and back slashes, so check for a backslash as
        // well.
        auto back = find_last_of('\\');
        if (back != npos)
        {
            // If there is no forward slash, or the backslash appears after the forward slash, then
            // use it's position.
            if (pos == npos || back > pos)
            {
                pos = back;
            }
        }
#endif
        if (pos == npos)
        {
            pos = find_last_of(':');
            if (pos == npos)
            {
                return false;
            }
        }

        remove_prefix(pos + 1);
        return true;
    }

    bool string_view::moveto_substr(std::string_view substr, bool StepOverIfFound) noexcept
    {
        auto pos = find(substr);
        if (!wxue::is_found(pos))
        {
            return false;
        }

        if (StepOverIfFound)
        {
            auto stepover_pos = find_nonspace(pos + substr.size());
            if (wxue::is_found(pos))
            {
                pos = stepover_pos;
            }
        }
        remove_prefix(pos);
        return true;
    }

    string_view string_view::extension() const noexcept
    {
        if (empty())
        {
            return "";
        }

        auto pos = find_last_of('.');
        if (pos == npos)
        {
            return "";
        }

        // . by itself is a folder
        if (pos + 1 >= length())
        {
            return "";
        }
        // .. is not a valid extension (it's usually part of a folder as in "../dir/")
        if (data()[pos + 1] == '.')
        {
            return "";
        }

        return { data() + pos, length() - pos };
    }

    string_view string_view::filename() const noexcept
    {
        if (empty())
        {
            return "";
        }

        auto pos = find_last_of('/');

#ifdef _WIN32
        // Windows filenames can contain both forward and back slashes, so check for a backslash as
        // well.
        auto back = find_last_of('\\');
        if (back != npos)
        {
            // If there is no forward slash, or the backslash appears after the forward slash, then
            // use it's position.
            if (pos == npos || back > pos)
            {
                pos = back;
            }
        }
#endif
        if (pos == npos)
        {
            pos = find_last_of(':');
            if (pos == npos)
            {
                return { data(), length() };
            }
        }

        return { data() + pos + 1, length() - (pos + 1) };
    }

    bool string_view::file_exists() const
    {
        if (empty())
        {
            return false;
        }

        return wxFileName::FileExists(wx());
    }

    bool string_view::dir_exists() const
    {
        if (empty())
        {
            return false;
        }

        return wxFileName::DirExists(wx());
    }

    size_t string_view::find_oneof(const std::string& set, size_t start) const
    {
        if (set.empty())
        {
            return wxue::npos;
        }

        for (start = 0; start < length(); ++start)
        {
            if (std::strchr(set.c_str(), at(start)))
            {
                break;
            }
        }
        return (start < length() ? start : wxue::npos);
    }

    size_t string_view::find_oneof(string_view set, size_t start) const
    {
        if (set.empty())
        {
            return wxue::npos;
        }
        std::string copy(set);
        return find_oneof(copy, start);
    }

    size_t string_view::find_space(size_t start) const
    {
        for (; start < length(); ++start)
        {
            if (std::strchr(" \t\r\n\f", at(start)))
            {
                break;
            }
        }
        return (start < length() ? start : wxue::npos);
    }

    size_t string_view::find_nonspace(size_t start) const
    {
        for (; start < length(); ++start)
        {
            if (!std::strchr(" \t\r\n\f", at(start)))
            {
                break;
            }
        }
        return (start < length() ? start : wxue::npos);
    }

    size_t string_view::stepover(size_t start) const
    {
        auto pos = find_space(start);
        if (pos != npos)
        {
            pos = find_nonspace(pos);
        }
        return pos;
    }

    int string_view::comparei(std::string_view str) const
    {
        auto main = begin();
        auto sub = str.begin();
        while (sub != str.end())
        {
            auto diff = std::tolower(main[0]) - std::tolower(sub[0]);
            if (diff != 0)
            {
                return diff;
            }
            ++main;
            ++sub;
            if (main == end())
            {
                return (sub != str.end() ? -1 : 0);
            }
        }

        return (main != end() ? 1 : 0);
    }

    /**
     * @param chBegin -- character that prefixes the string
     * @param chEnd -- character that terminates the string.
     */
    string_view string_view::view_substr(size_t offset, char chBegin, char chEnd) const
    {
        if (empty() || offset >= size())
        {
            return wxue::emptystring;
        }

        // step over any leading whitespace unless chBegin is a whitespace character
        if (!wxue::is_whitespace(chBegin))
        {
            while (wxue::is_whitespace(at(offset)))
            {
                ++offset;
            }
        }

        if (at(offset) == chBegin)
        {
            ++offset;
            auto start = offset;
            while (offset < size() && at(offset) != chEnd)
            {
                // REVIEW: [KeyWorks - 01-26-2020] '\"' is also valid for the C compiler, though the
                // slash is unnecessary. Should we support it?

                // only check quotes -- a slash is valid before other character pairs.
                if (at(offset) == '\\' && (chBegin == '"' || chBegin == '\'') &&
                    offset + 1 < size() && (at(offset + 1) == chEnd))
                {
                    // step over an escaped quote if the string to fetch is within a quote
                    offset += 2;
                    continue;
                }
                ++offset;
            }

            return string_view(data() + start, offset - start);
        }

        // if the string didn't start with chBegin, just copy the string. Note that offset may
        // have changed if chBegin was not whitespace and at(offset) was whitespace.

        return subview(offset);
    }

    string_view string_view::subview(size_t start, size_t len) const
    {
        if (start >= size())
        {
            return string_view(data(), 0);
        }
#ifdef min
        return string_view(data() + start, min(size() - start, len));
#else
        return string_view(data() + start, std::min(size() - start, len));
#endif
    }

    string_view string_view::find_space(std::string_view str) noexcept
    {
        if (str.empty())
        {
            return wxue::emptystring;
        }
        size_t pos;
        for (pos = 0; pos < str.length(); ++pos)
        {
            if (wxue::is_whitespace(str.at(pos)))
            {
                break;
            }
        }
        if (pos >= str.length())
        {
            return wxue::emptystring;
        }
        return string_view(str.data() + pos, str.length() - pos);
    }

    string_view string_view::find_nonspace(std::string_view str) noexcept
    {
        if (str.empty())
        {
            return wxue::emptystring;
        }
        size_t pos;
        for (pos = 0; pos < str.length(); ++pos)
        {
            if (!wxue::is_whitespace(str.at(pos)))
            {
                break;
            }
        }
        if (pos >= str.length())
        {
            return wxue::emptystring;
        }
        return string_view(str.data() + pos, str.length() - pos);
    }

    string_view string_view::stepover(std::string_view str) noexcept
    {
        if (str.empty())
        {
            return wxue::emptystring;
        }

        size_t pos;
        for (pos = 0; pos < str.length(); ++pos)
        {
            if (wxue::is_whitespace(str.at(pos)))
            {
                break;
            }
        }
        if (pos >= str.length())
        {
            return wxue::emptystring;
        }

        for (; pos < str.length(); ++pos)
        {
            if (!wxue::is_whitespace(str.at(pos)))
            {
                break;
            }
        }
        if (pos >= str.length())
        {
            return wxue::emptystring;
        }
        return string_view(str.data() + pos, str.length() - pos);
    }

    string_view& string_view::trim(wxue::TRIM where)
    {
        if (empty())
        {
            return *this;
        }

        if (where == wxue::TRIM::right || where == wxue::TRIM::both)
        {
            auto len = length();
            for (--len; len != std::string::npos; --len)
            {
                // char ch = at(len);
                char chr = data()[len];
                if (chr != ' ' && chr != '\t' && chr != '\r' && chr != '\n' && chr != '\f')
                {
                    ++len;
                    break;
                }
            }

            if (len < length())
            {
                remove_suffix(length() - len);
            }
        }

        // If trim(right) was called above, the string may now be empty -- front() fails on an empty
        // string
        if (!empty() && (where == wxue::TRIM::left || where == wxue::TRIM::both))
        {
            // Assume that most strings won't start with whitespace, so return as quickly as
            // possible if that is the case.
            if (!wxue::is_whitespace(front()))
            {
                return *this;
            }

            size_t pos;
            for (pos = 1; pos < length(); ++pos)
            {
                if (!wxue::is_whitespace(data()[pos]))
                {
                    break;
                }
            }
            remove_prefix(pos);
        }

        return *this;
    }

    string_view& string_view::erase_from(std::string_view sub, wxue::CASE check)
    {
        auto pos = locate(sub, 0, check);
        if (pos != wxue::npos)
        {
            remove_suffix(length() - pos);
            trim();
        }

        return *this;
    }

}  // namespace wxue
