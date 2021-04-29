/////////////////////////////////////////////////////////////////////////////
// Name:      ttcview.cpp
// Purpose:   string_view functionality on a zero-terminated char string.
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <cassert>
#include <cctype>
#include <locale>

#include "ttcview.h"

using namespace ttlib;

bool cview::is_sameas(std::string_view str, tt::CASE checkcase) const
{
    if (size() != str.size())
        return false;

    if (empty())
        return str.empty();

    // if both strings have the same length, then we can compare as a prefix.
    return is_sameprefix(str, checkcase);
}

bool cview::is_sameprefix(std::string_view str, tt::CASE checkcase) const
{
    if (str.empty())
        return empty();

    if (empty() || length() < str.length())
        return false;

    if (checkcase == tt::CASE::exact)
    {
        auto iterMain = begin();
        for (auto iterSub: str)
        {
            if (*iterMain++ != iterSub)
                return false;
        }
        return true;
    }
    else if (checkcase == tt::CASE::either)
    {
        auto iterMain = begin();
        for (auto iterSub: str)
        {
            if (std::tolower(*iterMain++) != std::tolower(iterSub))
                return false;
        }
        return true;
    }
    else if (checkcase == tt::CASE::utf8)
    {
        auto utf8locale = std::locale("en_US.utf8");
        auto iterMain = begin();
        for (auto iterSub: str)
        {
            if (std::tolower(*iterMain++, utf8locale) != std::tolower(iterSub, utf8locale))
                return false;
        }
        return true;
    }
    assert(!"Unknown tt::CASE value");
    return false;
}

size_t cview::locate(std::string_view str, size_t posStart, tt::CASE checkcase) const
{
    if (str.empty() || posStart >= size())
        return npos;

    if (checkcase == tt::CASE::exact)
        return find(str, posStart);

    if (checkcase == tt::CASE::either)
    {
        auto chLower = std::tolower(str[0]);
        for (auto pos = posStart; pos < length(); ++pos)
        {
            if (std::tolower(at(pos)) == chLower)
            {
                size_t posSub;
                for (posSub = 1; posSub < str.length(); ++posSub)
                {
                    if (pos + posSub >= length())
                        return npos;
                    if (std::tolower(at(pos + posSub)) != std::tolower(str.at(posSub)))
                        break;
                }
                if (posSub >= str.length())
                    return pos;
            }
        }
    }
    else
    {
        auto utf8locale = std::locale("en_US.utf8");
        auto chLower = std::tolower(str[0], utf8locale);
        for (auto pos = posStart; pos < length(); ++pos)
        {
            if (std::tolower(at(pos), utf8locale) == chLower)
            {
                size_t posSub;
                for (posSub = 1; posSub < str.length(); ++posSub)
                {
                    if (pos + posSub >= length())
                        return npos;
                    if (std::tolower(at(pos + posSub), utf8locale) != std::tolower(str.at(posSub), utf8locale))
                        break;
                }
                if (posSub >= str.length())
                    return pos;
            }
        }
    }
    return npos;
}

bool cview::moveto_space() noexcept
{
    if (empty())
        return false;
    size_t pos;
    for (pos = 0; pos < length(); ++pos)
    {
        if (ttlib::is_whitespace(at(pos)))
            break;
    }
    if (pos >= length())
        return false;
    else
    {
        remove_prefix(pos);
        return true;
    }
}

bool cview::moveto_nonspace() noexcept
{
    if (empty())
        return false;
    size_t pos;
    for (pos = 0; pos < length(); ++pos)
    {
        if (!ttlib::is_whitespace(at(pos)))
            break;
    }
    if (pos >= length())
        return false;
    else
    {
        remove_prefix(pos);
        return true;
    }
}

bool cview::moveto_nextword() noexcept
{
    if (empty())
        return false;
    size_t pos;
    for (pos = 0; pos < length(); ++pos)
    {
        if (ttlib::is_whitespace(at(pos)))
            break;
    }
    if (pos >= length())
        return false;

    // whitespace found, look for non-whitespace
    else
    {
        for (++pos; pos < length(); ++pos)
        {
            if (!ttlib::is_whitespace(at(pos)))
                break;
        }
        if (pos >= length())
            return false;

        remove_prefix(pos);
        return true;
    }
}

cview cview::view_digit(size_t start) const
{
    for (; start < length(); ++start)
    {
        if (ttlib::is_digit(at(start)))
            return subview(start);
    }

    return subview(length());
}

cview cview::view_nondigit(size_t start) const
{
    for (; start < length(); ++start)
    {
        if (!ttlib::is_digit(at(start)))
            return subview(start);
    }

    return subview(length());
}

bool cview::moveto_digit() noexcept
{
    if (empty())
        return false;
    size_t pos;
    for (pos = 0; pos < length(); ++pos)
    {
        if (ttlib::is_digit(at(pos)))
            break;
    }
    if (pos >= length())
        return false;
    else
    {
        remove_prefix(pos);
        return true;
    }
}

bool cview::moveto_nondigit() noexcept
{
    if (empty())
        return false;
    size_t pos;
    for (pos = 0; pos < length(); ++pos)
    {
        if (!ttlib::is_digit(at(pos)))
            break;
    }
    if (pos >= length())
        return false;
    else
    {
        remove_prefix(pos);
        return true;
    }
}

bool cview::moveto_extension() noexcept
{
    if (empty())
        return false;

    auto pos = rfind('.');
    if (pos == npos)
        return false;
    else if (pos + 1 >= length())  // . by itself is a folder
        return false;
    else if (pos < 2 && (at(pos + 1) == '.'))
        return false;

    remove_prefix(pos);
    return true;
}

bool cview::moveto_filename() noexcept
{
    if (empty())
        return false;

    auto pos = find_last_of('/');

#if defined(_WIN32)
    // Windows filenames can contain both forward and back slashes, so check for a backslash as well.
    auto back = find_last_of('\\');
    if (back != npos)
    {
        // If there is no forward slash, or the backslash appears after the forward slash, then use it's position.
        if (pos == npos || back > pos)
            pos = back;
    }
#endif
    if (pos == npos)
    {
        pos = find_last_of(':');
        if (pos == npos)
            return false;
    }

    remove_prefix(pos + 1);
    return true;
}

ttlib::cview cview::extension() const noexcept
{
    if (empty())
        return "";

    auto pos = find_last_of('.');
    if (pos == npos)
        return "";

    // . by itself is a folder
    else if (pos + 1 >= length())
        return "";
    // .. is not a valid extension (it's usually part of a folder as in "../dir/")
    else if (c_str()[pos + 1] == '.')
        return "";

    return { c_str() + pos, length() - pos };
}

ttlib::cview cview::filename() const noexcept
{
    if (empty())
        return "";

    auto pos = find_last_of('/');

#if defined(_WIN32)
    // Windows filenames can contain both forward and back slashes, so check for a backslash as well.
    auto back = find_last_of('\\');
    if (back != npos)
    {
        // If there is no forward slash, or the backslash appears after the forward slash, then use it's position.
        if (pos == npos || back > pos)
            pos = back;
    }
#endif
    if (pos == npos)
    {
        pos = find_last_of(':');
        if (pos == npos)
            return { c_str(), length() };
    }

    return { c_str() + pos + 1, length() - (pos + 1) };
}

bool cview::file_exists() const
{
    if (empty())
        return false;
    auto file = std::filesystem::directory_entry(std::filesystem::u8path(c_str()));
    return (file.exists() && !file.is_directory());
}

bool cview::dir_exists() const
{
    if (empty())
        return false;
    auto file = std::filesystem::directory_entry(std::filesystem::u8path(c_str()));
    return (file.exists() && file.is_directory());
}

size_t cview::get_hash() const noexcept
{
    if (empty())
        return 0;

    // djb2 hash algorithm

    size_t hash = 5381;

    for (auto iter: *this)
    {
        hash = ((hash << 5) + hash) ^ iter;
    }

    return hash;
}

size_t cview::find_oneof(const std::string& set) const
{
    if (set.empty())
        return tt::npos;
    const char* pszFound = std::strpbrk(c_str(), set.c_str());
    if (!pszFound)
        return tt::npos;
    return (static_cast<size_t>(pszFound - c_str()));
}

size_t cview::find_oneof(cview set, size_t start) const
{
    if (set.empty())
        return tt::npos;
    auto view_start = subview(start);
    const char* pszFound = std::strpbrk(view_start, set);
    if (!pszFound)
        return tt::npos;
    return (static_cast<size_t>(pszFound - view_start.c_str()));
}

size_t cview::find_space(size_t start) const
{
    if (start >= length())
        return npos;
    const char* pszFound = std::strpbrk(c_str() + start, " \t\r\n\f");
    if (!pszFound)
        return npos;
    return (static_cast<size_t>(pszFound - c_str()));
}

size_t cview::find_nonspace(size_t start) const
{
    for (; start < length(); ++start)
    {
        if (!std::strchr(" \t\r\n\f", at(start)))
            break;
    }
    return start;
}

size_t cview::stepover(size_t start) const
{
    auto pos = find_space(start);
    if (pos != npos)
    {
        pos = find_nonspace(pos);
    }
    return pos;
}

std::wstring cview::to_utf16() const
{
    std::wstring str16;
    ttlib::utf8to16(*this, str16);
    return str16;
}

int cview::comparei(std::string_view str) const
{
    auto main = begin();
    auto sub = str.begin();
    while (sub != str.end())
    {
        auto diff = std::tolower(main[0]) - std::tolower(sub[0]);
        if (diff != 0)
            return diff;
        ++main;
        ++sub;
        if (main == end())
            return (sub != str.end() ? -1 : 0);
    }

    return (main != end() ? 1 : 0);
}

std::string_view cview::subview(size_t start, size_t len) const
{
    if (start >= size())
        return {};
#ifdef min
    return std::string_view(c_str() + start, min(size() - start, len));
#else
    return std::string_view(c_str() + start, std::min(size() - start, len));
#endif
}

/**
 * @param chBegin -- character that prefixes the string
 * @param chEnd -- character that terminates the string.
 */
std::string_view cview::view_substr(size_t offset, char chBegin, char chEnd)
{
    if (empty() || offset >= size())
    {
        return {};
    }

    // step over any leading whitespace unless chBegin is a whitespace character
    if (!ttlib::is_whitespace(chBegin))
    {
        while (ttlib::is_whitespace(at(offset)))
            ++offset;
    }

    if (at(offset) == chBegin)
    {
        ++offset;
        auto start = offset;
        while (offset < size() && at(offset) != chEnd)
        {
            // REVIEW: [KeyWorks - 01-26-2020] '\"' is also valid for the C compiler, though the slash
            // is unnecessary. Should we support it?

            // only check quotes -- a slash is valid before other character pairs.
            if (at(offset) == '\\' && (chBegin == '"' || chBegin == '\'') && offset + 1 < size() && (at(offset + 1) == chEnd))
            {
                // step over an escaped quote if the string to fetch is within a quote
                offset += 2;
                continue;
            }
            ++offset;
        }

        return subview(start, offset - start);
    }
    else
    {
        // if the string didn't start with chBegin, just copy the string. Note that offset may have changed if
        // chBegin was not whitespace and at(offset) was whitespace.

        return subview(offset);
    }
}
