/////////////////////////////////////////////////////////////////////////////
// Name:      ttcstr.cpp
// Purpose:   Class for handling zero-terminated char strings.
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <locale>
#include <sstream>

#include "ttcstr.h"
#include "ttlibspace.h"

using namespace ttlib;
using namespace tt;

namespace fs = std::filesystem;

bool cstr::is_sameas(std::string_view str, CASE checkcase) const
{
    if (size() != str.size())
        return false;

    if (empty())
        return str.empty();

    // if both strings have the same length and are non-empty, then we can compare as a prefix.
    return is_sameprefix(str, checkcase);
}

bool cstr::is_sameprefix(std::string_view str, CASE checkcase) const
{
    if (str.empty())
        return empty();

    if (empty() || length() < str.length())
        return false;

    if (checkcase == CASE::exact)
    {
        auto iterMain = begin();
        for (auto iterSub: str)
        {
            if (*iterMain++ != iterSub)
                return false;
        }
        return true;
    }
    else if (checkcase == CASE::either)
    {
        auto iterMain = begin();
        for (auto iterSub: str)
        {
            if (std::tolower(*iterMain++) != std::tolower(iterSub))
                return false;
        }
        return true;
    }
    else if (checkcase == CASE::utf8)
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
    assert(!"Unknown CASE value");
    return false;
}

int cstr::comparei(std::string_view str) const
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

cstr& cstr::trim(tt::TRIM where)
{
    if (empty())
        return *this;
    if (where == tt::TRIM::right || where == tt::TRIM::both)
    {
        auto len = length();
        for (--len; len != std::string::npos; --len)
        {
            // char ch = at(len);
            char ch = c_str()[len];
            if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' && ch != '\f')
                break;
        }

        if (len + 1 < length())
            erase(len + 1, length() - len);
    }

    // If trim(right) was called above, the string may now be empty -- front() fails on an empty string
    if (!empty() && (where == tt::TRIM::left || where == tt::TRIM::both))
    {
        // Assume that most strings won't start with whitespace, so return as quickly as possible if that is the
        // case.
        if (!ttlib::is_whitespace(front()))
            return *this;

        size_t pos;
        for (pos = 1; pos < length(); ++pos)
        {
            if (!ttlib::is_whitespace(c_str()[pos]))
                break;
        }
        replace(0, length(), substr(pos, length() - pos));
    }

    return *this;
}

/**
 * @param chBegin -- character that prefixes the string
 * @param chEnd -- character that terminates the string.
 */
std::string_view cstr::view_substr(size_t offset, char chBegin, char chEnd)
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
            if (at(offset) == '\\' && (chBegin == '"' || chBegin == '\'') && offset + 1 < size() &&
                (at(offset + 1) == chEnd))
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

/**
 * @param src -- string to parse
 * @param chBegin -- character that prefixes the string
 * @param chEnd -- character that terminates the string.
 */
size_t cstr::AssignSubString(std::string_view src, char chBegin, char chEnd)
{
    if (src.empty())
    {
        assign(ttlib::emptystring);
        return npos;
    }

    size_t pos = 0;
    // step over any leading whitespace unless chBegin is a whitespace character
    if (!ttlib::is_whitespace(chBegin))
    {
        while (ttlib::is_whitespace(src[pos]))
            ++pos;
    }

    if (src[pos] == chBegin)
    {
        ++pos;
        auto start = pos;
        while (pos < src.length() && src[pos] != chEnd)
        {
            // REVIEW: [KeyWorks - 01-26-2020] '\"' is also valid for the C compiler, though the slash
            // is unnecessary. Should we support it?

            // only check quotes and backslashes -- a slash is ignored before other character pairs.
            if (src[pos] == '\\' && pos + 1 < src.length())
            {
                if (src[pos + 1] == '\\' || ((chBegin == '"' || chBegin == '\'') && src[pos + 1] == chEnd))
                {
                    // step over an escaped quote if the string to fetch is within a quote
                    pos += 2;
                    continue;
                }
            }
            ++pos;
        }
        insert(0, src.substr(start, pos - start));
    }
    else
    {
        // if the string didn't start with chBegin, just copy the string
        *this = src;
        return src.length();
    }
    return pos;
}

/**
 * @param str -- the string to extract from
 * @param start -- the offset into the string to begin extraction from -- this should either be to leading
 *                 whitespace, or to the character that marks the beginning of the sub string.
 * @return The offset to the character that terminated extraction, or **npos** if a terminating
 *         character could not be found.
 */
size_t cstr::ExtractSubString(std::string_view src, size_t start)
{
    if (src.empty())
    {
        assign(ttlib::emptystring);
        return npos;
    }

    // start by finding the first non-whitespace character
    size_t pos = start;
    while (pos < src.length() && ttlib::is_whitespace(src[pos]))
    {
        ++pos;
    }

    if (pos >= src.length())
        return npos;

    // based on the opening character, determine what the matching end character should be
    char chEnd;
    char chBegin = src[pos];
    if (chBegin == '`' || chBegin == '\'')
        chEnd = '\'';
    else if (chBegin == '"')
        chEnd = '"';
    else if (chBegin == '<')
        chEnd = '>';
    else if (chBegin == '[')
        chEnd = '>';
    else if (chBegin == ']')
        chEnd = ')';
    else
        return npos;

    // now find the substring between chBegin and chEnd
    auto startPos = pos + 1;
    for (++pos; pos < src.length(); ++pos)
    {
        // only check quotes -- a slash is valid before other character pairs.
        if (src[pos] == '\\' && (chBegin == '"' || chBegin == '\'') && pos + 1 < src.length() && (src[pos + 1] == chEnd))
        {
            // step over an escaped quote if the string to fetch is within a quote
            ++pos;
            continue;
        }
        else if (src[pos] == chEnd)
        {
            break;
        }
    }

    if (pos >= src.length())
        return npos;

    assign(src.substr(startPos, pos - startPos));
    return pos;
}

/**
 * @param oldtext -- the text to search for
 * @param newtext -- the text to replace it with
 * @param replace_all -- replace all occurrences or just the first one
 * @param CaseSensitive -- indicates whether or not to use a case-insensitive search
 * @return Number of replacements made
 */
size_t cstr::Replace(std::string_view oldtext, std::string_view newtext, bool replace_all, tt::CASE checkcase)
{
    if (oldtext.empty())
        return false;

    size_t replacements = 0;
    if (auto pos = locate(oldtext, 0, checkcase); ttlib::is_found(pos))
    {
        do
        {
            erase(pos, oldtext.length());
            insert(pos, newtext);
            ++replacements;
            pos += newtext.length();
            if (pos >= size() || !replace_all)
                break;
            pos = locate(oldtext, pos, checkcase);
        } while (ttlib::is_found(pos));
    }

    return replacements;
}

size_t cstr::locate(std::string_view str, size_t posStart, CASE checkcase) const
{
    if (str.empty() || posStart >= size())
        return npos;

    if (checkcase == CASE::exact)
        return find(str, posStart);

    if (checkcase == CASE::either)
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

size_t cstr::get_hash() const noexcept
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

cstr& cstr::MakeLower()
{
    auto utf8locale = std::locale("en_US.utf8");
    for (auto iter = begin(); iter != end(); ++iter)
    {
        *iter = std::tolower(*iter, utf8locale);
    }
    return *this;
}

cstr& cstr::MakeUpper()
{
    auto utf8locale = std::locale("en_US.utf8");
    for (auto iter = begin(); iter != end(); ++iter)
    {
        *iter = std::toupper(*iter, utf8locale);
    }
    return *this;
}

/// Converts all backslashes in the string to forward slashes.
///
/// Note: Windows handles paths that use forward slashes, so backslashes are normally
/// unnecessary.
cstr& cstr::backslashestoforward()
{
    for (auto pos = find('\\'); pos != std::string::npos; pos = find('\\'))
    {
        replace(pos, 1, "/");
    }
    return *this;
}

cstr& cstr::replace_extension(std::string_view newExtension)
{
    if (empty())
    {
        if (newExtension.empty())
            return *this;
        if (newExtension.at(0) != '.')
            push_back('.');
        append(newExtension);
        return *this;
    }

    auto pos_file = find_filename();
    if (ttlib::is_error(pos_file))
        pos_file = 0;

    if (auto pos = find_last_of('.'); ttlib::is_found(pos) && pos > pos_file)
    {
        // If the string only contains . or .. then it is a folder
        if (pos == 0 || (pos == 1 && at(0) != '.'))
            return *this;  // can't add an extension if it isn't a valid filename

        if (newExtension.empty())
        {
            // If the new extension is empty, then just erase the old extension.
            erase(pos);
        }
        else
        {
            // If the new extension doesn't start with '.', then keep our own '.' prefix.
            if (newExtension.at(0) != '.')
                ++pos;
            replace(pos, length() - pos, newExtension);
        }
    }
    else if (newExtension.size())
    {
        // Current filename doesn't have an extension, so append the new one
        if (newExtension.at(0) != '.')
            push_back('.');
        append(newExtension);
    }

    return *this;
}

ttlib::cview cstr::extension() const noexcept
{
    if (empty())
        return "";

    auto pos = find_last_of('.');
    if (!ttlib::is_found(pos))
        return "";

    // . by itself is a folder
    else if (pos + 1 >= length())
        return "";
    // .. is not a valid extension (it's usually part of a folder as in "../dir/")
    else if (c_str()[pos + 1] == '.')
        return "";

    return { c_str() + pos, length() - pos };
}

ttlib::cview cstr::filename() const noexcept
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

size_t cstr::find_filename() const noexcept
{
    if (empty())
        return tt::npos;

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
            return tt::npos;
    }

    return pos + 1;
}

cstr& cstr::replace_filename(std::string_view newFilename)
{
    if (empty())
    {
        assign(newFilename);
        return *this;
    }

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
        {
            // If we get here, we think the entire current string is a filename.
            assign(newFilename);
            return *this;
        }
    }

    replace(pos + 1, length() - (pos + 1), newFilename);
    return *this;
}

cstr& cstr::append_filename(std::string_view filename)
{
    if (filename.empty())
        return *this;
    if (empty())
    {
        assign(filename);
        return *this;
    }

    auto last = back();
    if (last != '/' && last != '\\')
        push_back('/');
    append(filename);
    return *this;
}

cstr& cstr::assignCwd()
{
    assign(std::filesystem::absolute(".").u8string());
    return *this;
}

cstr& cstr::make_relative(ttlib::cview relative_to)
{
    if (empty())
        return *this;

    auto current = fs::absolute(fs::u8path(c_str()));
    auto rel_to = fs::absolute(fs::u8path(relative_to.c_str()));
    assign(current.lexically_relative(rel_to).u8string());
    return *this;
}

cstr& cstr::make_absolute()
{
    if (!empty())
    {
        auto current = std::filesystem::u8path(c_str());
        assign(std::filesystem::absolute(current).u8string());
    }
    return *this;
}

bool cstr::file_exists() const
{
    if (empty())
        return false;
    auto file = std::filesystem::directory_entry(std::filesystem::u8path(c_str()));
    return (file.exists() && !file.is_directory());
}

bool cstr::dir_exists() const
{
    if (empty())
        return false;
    auto file = std::filesystem::directory_entry(std::filesystem::u8path(c_str()));
    return (file.exists() && file.is_directory());
}

size_t cstr::find_oneof(const char* pszSet) const
{
    if (!pszSet || !*pszSet)
        return npos;
    const char* pszFound = std::strpbrk(c_str(), pszSet);
    if (!pszFound)
        return npos;
    return (static_cast<size_t>(pszFound - c_str()));
}

size_t cstr::find_oneof(cview set, size_t start) const
{
    if (set.empty())
        return tt::npos;
    auto view_start = subview(start);
    const char* pszFound = std::strpbrk(view_start, set);
    if (!pszFound)
        return tt::npos;
    return (static_cast<size_t>(pszFound - view_start.c_str()));
}

size_t cstr::find_space(size_t start) const
{
    if (start >= length())
        return npos;
    const char* pszFound = std::strpbrk(c_str() + start, " \t\r\n\f");
    if (!pszFound)
        return npos;
    return (static_cast<size_t>(pszFound - c_str()));
}

size_t cstr::find_nonspace(size_t start) const
{
    for (; start < length(); ++start)
    {
        if (!std::strchr(" \t\r\n\f", at(start)))
            break;
    }
    return start;
}

size_t cstr::stepover(size_t start) const
{
    auto pos = find_space(start);
    if (pos != npos)
    {
        pos = find_nonspace(pos);
    }
    return pos;
}

std::wstring cstr::to_utf16() const
{
    std::wstring str16;
    ttlib::utf8to16(*this, str16);
    return str16;
}

std::string_view cstr::subview(size_t start, size_t len) const
{
    if (start >= size())
        return {};
#ifdef min
    return std::string_view(c_str() + start, min(size() - start, len));
#else
    return std::string_view(c_str() + start, std::min(size() - start, len));
#endif
}

bool cstr::assignEnvVar(ttlib::cview env_var)
{
    clear();
    if (env_var.empty())
        return false;
    char* pszEnv = std::getenv(env_var);
    if (!pszEnv)
        return false;
    assign(pszEnv);
    return true;
}

void cstr::erase_from(char ch)
{
    auto pos = find(ch);
    if (pos != tt::npos)
    {
        erase(pos);
        trim();
    }
}

void cstr::erase_from(std::string_view sub)
{
    auto pos = find(sub);
    if (pos != tt::npos)
    {
        erase(pos);
        trim();
    }
}

cstr& cdecl cstr::Format(std::string_view format, ...)
{
    enum WIDTH : size_t
    {
        WIDTH_NONE,
        WIDTH_CHAR,
        WIDTH_SHORT,
        WIDTH_LONG,
        WIDTH_LONG_LONG,
        WIDTH_INT_MAX,
        WIDTH_SIZE_T,
        WIDTH_PTRDIFF,
        WIDTH_LONG_DOUBLE
    };

    std::va_list args;
    va_start(args, format);

    std::stringstream buffer;

    // Place this outside the try block so that we can refer to it in the catch block
    size_t pos = 0;

    try
    {
        for (pos = 0; pos < format.length(); ++pos)
        {
            // Use .at(pos) not .[pos] to throw an exception if invalid format string is used
            if (format.at(pos) != '%')
            {
                buffer << format.at(pos);
                continue;
            }
            ++pos;

            if (format.at(pos) == '%')
            {
                buffer << format.at(pos);
                continue;
            }

            bool kflag = false;
            if (format.at(pos) == 'k')
            {
                kflag = true;
                ++pos;
            }

            WIDTH width = WIDTH_NONE;

            if (format.at(pos) == 'h')
            {
                ++pos;
                if (format.at(pos) == 'h')
                {
                    ++pos;
                    width = WIDTH_SHORT;
                }
                else
                {
                    width = WIDTH_CHAR;
                }
            }

            else if (format.at(pos) == 'l')
            {
                ++pos;
                if (format.at(pos) == 'l')
                {
                    ++pos;
                    width = WIDTH_LONG_LONG;
                }
                else
                {
                    width = WIDTH_LONG;
                }
            }

            else if (format.at(pos) == 'j')
            {
                ++pos;
                width = WIDTH_INT_MAX;
            }
            else if (format.at(pos) == 'z')
            {
                ++pos;
                width = WIDTH_SIZE_T;
            }
            else if (format.at(pos) == 't')
            {
                ++pos;
                width = WIDTH_PTRDIFF;
            }
            else if (format.at(pos) == 'L')
            {
                ++pos;
                width = WIDTH_LONG_DOUBLE;
            }

            bool LeftFieldWidth = false;
            if (format.at(pos) == '-')
            {
                buffer << std::left;
                LeftFieldWidth = true;
                ++pos;
            }

            if (ttlib::is_digit(format.at(pos)))
            {
                auto fieldWidth = ttlib::atoi(format.substr(pos));
                buffer << std::setw(fieldWidth);
                do
                {
                    ++pos;
                } while (pos < format.length() && ttlib::is_digit(format.at(pos)));
            }

            // For both %lc and %ls we assume a UTF16 string and convert it to UTF8.

            if (format.at(pos) == 'c')
            {
                if (width != WIDTH_LONG)
                    buffer << va_arg(args, char);
                else
                {
                    std::wstring str16;
                    str16 += va_arg(args, wchar_t);
                    std::string str8;
                    ttlib::utf16to8(str16, str8);
                    buffer << str8;
                }
            }
            else if (format.at(pos) == 's')
            {
                if (width != WIDTH_LONG)
                {
                    if (kflag)
                        buffer << std::quoted(va_arg(args, const char*));
                    else
                        buffer << va_arg(args, const char*);
                }
                else
                {
                    std::wstring str16;
                    str16 += va_arg(args, const wchar_t*);
                    std::string str8;
                    ttlib::utf16to8(str16, str8);
                    if (kflag)
                        buffer << std::quoted(str8);
                    else
                        buffer << str8;
                }
            }
            else if (format.at(pos) == 'v')
            {
                if (width != WIDTH_LONG)
                {
                    if (kflag)
                        buffer << std::quoted(va_arg(args, std::string_view));
                    else
                        buffer << va_arg(args, std::string_view);
                }
                else
                {
                    std::wstring str16;
                    str16 += va_arg(args, std::wstring_view);
                    std::string str8;
                    ttlib::utf16to8(str16, str8);
                    if (kflag)
                        buffer << std::quoted(str8);
                    else
                        buffer << str8;
                }
            }
            else if (format.at(pos) == 'd' || format.at(pos) == 'i')
            {
                std::locale previous;
                if (kflag)
                    previous = buffer.imbue(std::locale(""));

                switch (width)
                {
                    default:
                    case WIDTH_NONE:
                        buffer << va_arg(args, int);
                        break;

                    case WIDTH_CHAR:
                        buffer << va_arg(args, signed char);
                        break;

                    case WIDTH_SHORT:
                        buffer << va_arg(args, short);
                        break;

                    case WIDTH_LONG:
                        buffer << va_arg(args, long);
                        break;

                    case WIDTH_LONG_LONG:
                        buffer << va_arg(args, long long);
                        break;

                    case WIDTH_INT_MAX:
                        buffer << va_arg(args, intmax_t);
                        break;

                    case WIDTH_SIZE_T:
                        // REVIEW: [KeyWorks - 02-14-2020] The spec says this should be signed. Knowing that a
                        // size_t is set to -1 is valuable, but any other value is likely to be unsigned. We break
                        // from the spec and special case a value of -1, otherwise we format the value as unsigned.
                        {
                            size_t val = va_arg(args, size_t);
                            if (val == static_cast<size_t>(-1))
                                buffer << "-1";
                            else
                                buffer << val;
                        }
                        break;

                    case WIDTH_PTRDIFF:
                        buffer << va_arg(args, ptrdiff_t);
                        break;
                }
                if (kflag)
                    previous = buffer.imbue(previous);
            }
            else if (format.at(pos) == 'u' || format.at(pos) == 'o' || format.at(pos) == 'x' || format.at(pos) == 'X')
            {
                std::locale previous;
                if (kflag)
                    previous = buffer.imbue(std::locale(""));

                if (format.at(pos) == 'o')
                    buffer << std::oct;
                else if (format.at(pos) == 'x')
                {
                    buffer << std::hex;
                }
                else if (format.at(pos) == 'X')
                {
                    buffer << std::uppercase;
                    buffer << std::hex;
                }

                switch (width)
                {
                    default:
                    case WIDTH_NONE:
                        buffer << va_arg(args, unsigned int);
                        break;

                    case WIDTH_CHAR:
                        buffer << va_arg(args, unsigned char);
                        break;

                    case WIDTH_SHORT:
                        buffer << va_arg(args, unsigned short);
                        break;

                    case WIDTH_LONG:
                        buffer << va_arg(args, unsigned long);
                        break;

                    case WIDTH_LONG_LONG:
                        buffer << va_arg(args, unsigned long long);
                        break;

                    case WIDTH_INT_MAX:
                        buffer << va_arg(args, uintmax_t);
                        break;

                    case WIDTH_SIZE_T:
                        // REVIEW: [KeyWorks - 02-14-2020] The spec says this should be signed. Knowing that a
                        // size_t is set to -1 is valuable, but any other value is likely to be unsigned. We
                        // break from the spec and special case a value of -1, otherwise we format the value as
                        // unsigned.
                        {
                            size_t val = va_arg(args, size_t);
                            if (val == static_cast<size_t>(-1))
                                buffer << "-1";
                            else
                                buffer << val;
                        }
                        break;

                    case WIDTH_PTRDIFF:
                        // REVIEW: [KeyWorks - 02-14-2020] There is no unsigned ptrdiff_t, so we use size_t
                        buffer << va_arg(args, size_t);
                        break;
                }

                if (kflag)
                    previous = buffer.imbue(previous);
                buffer << std::dec;
                buffer << std::nouppercase;
            }

            if (LeftFieldWidth)
                buffer << std::right;
        }
    }
    catch (const std::exception& /* e */)
    {
        assert(!"exception in ttlib::cstr.Format()");
    }

    va_end(args);

    this->assign(buffer.str());

    return *this;
}

/////////////////// The following section is only built when building with wxWidgets header files ///////////////////

// clang-format off
#if defined(_WX_DEFS_H_)

#include <wx/string.h>  // wxString class

#if defined(_WIN32)
    // This ctor is only available on Windows builds where wxString is UTF16. For non-Windows builds,
    // a ctor for std::wstring_view is used insted.

    cstr::cstr(const wxString& str)
    {
    #if defined(_WIN32)
        utf16to8(str.wx_str(), *this);
    #else
        *this = str.c_str();
    #endif
}
#endif  // _WIN32

cstr& cstr::assign_wx(const wxString& str)
{
#if defined(_WIN32)
    clear();
    utf16to8(str.wx_str(), *this);
#else
    *this = str.c_str();
#endif
    return *this;
}

cstr& cstr::append_wx(const wxString& str)
{
#if defined(_WIN32)
    utf16to8(str.wx_str(), *this);
#else
    *this += str.c_str();
#endif
    return *this;
}
// clang-format on

#endif  // _WX_DEFS_H_
