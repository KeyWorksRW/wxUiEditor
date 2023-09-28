/////////////////////////////////////////////////////////////////////////////
// Purpose:   std::string with additional methods
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/string.h>    // wxString class

#include <cstdarg>  // for va_list
#include <cstring>
#include <sstream>  // for std::stringstream

#include "tt_string.h"

bool tt_string::is_sameas(std::string_view str, tt::CASE checkcase) const
{
    if (size() != str.size())
        return false;

    if (empty())
        return str.empty();

    // if both strings have the same length and are non-empty, then we can compare as a prefix.
    return is_sameprefix(str, checkcase);
}

bool tt_string::is_sameprefix(std::string_view str, tt::CASE checkcase) const
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

int tt_string::comparei(std::string_view str) const
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

tt_string& tt_string::trim(tt::TRIM where)
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
        if (!tt::is_whitespace(front()))
            return *this;

        size_t pos;
        for (pos = 1; pos < length(); ++pos)
        {
            if (!tt::is_whitespace(c_str()[pos]))
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
tt_string_view tt_string::view_substr(size_t offset, char chBegin, char chEnd) const
{
    if (empty() || offset >= size())
    {
        return tt_string_view(tt::emptystring);
    }

    // step over any leading whitespace unless chBegin is a whitespace character
    if (!tt::is_whitespace(chBegin))
    {
        while (tt::is_whitespace(at(offset)))
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
size_t tt_string::AssignSubString(std::string_view src, char chBegin, char chEnd)
{
    if (src.empty())
    {
        assign(tt::emptystring);
        return npos;
    }

    size_t pos = 0;
    // step over any leading whitespace unless chBegin is a whitespace character
    if (!tt::is_whitespace(chBegin))
    {
        while (tt::is_whitespace(src[pos]))
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
size_t tt_string::ExtractSubString(std::string_view src, size_t start)
{
    if (src.empty())
    {
        assign(tt::emptystring);
        return npos;
    }

    // start by finding the first non-whitespace character
    size_t pos = start;
    while (pos < src.length() && tt::is_whitespace(src[pos]))
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
        chEnd = ']';
    else if (chBegin == '(')
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
size_t tt_string::Replace(std::string_view oldtext, std::string_view newtext, bool replace_all, tt::CASE checkcase)
{
    if (oldtext.empty())
        return false;

    size_t replacements = 0;
    if (auto pos = locate(oldtext, 0, checkcase); tt::is_found(pos))
    {
        do
        {
            if (oldtext.size() == newtext.size())
            {
                for (size_t idx = 0; idx < newtext.size(); ++idx)
                {
                    data()[pos + idx] = newtext[idx];
                }
            }
            else
            {
                erase(pos, oldtext.length());
                insert(pos, newtext);
            }
            ++replacements;
            pos += newtext.length();
            if (pos >= size() || !replace_all)
                break;
            pos = locate(oldtext, pos, checkcase);
        } while (tt::is_found(pos));
    }

    return replacements;
}

size_t tt_string::locate(std::string_view str, size_t posStart, tt::CASE checkcase) const
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

size_t tt_string::get_hash() const noexcept
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

tt_string& tt_string::MakeLower()
{
    auto utf8locale = std::locale("en_US.utf8");
    for (auto iter = begin(); iter != end(); ++iter)
    {
        *iter = std::tolower(*iter, utf8locale);
    }
    return *this;
}

tt_string& tt_string::MakeUpper()
{
    auto utf8locale = std::locale("en_US.utf8");
    for (auto iter = begin(); iter != end(); ++iter)
    {
        *iter = std::toupper(*iter, utf8locale);
    }
    return *this;
}

tt_string& tt_string::backslashestoforward()
{
    for (auto pos = find('\\'); pos != std::string::npos; pos = find('\\'))
    {
        replace(pos, 1, "/");
    }
    return *this;
}

tt_string& tt_string::forwardslashestoback()
{
    for (auto pos = find('/'); pos != std::string::npos; pos = find('/'))
    {
        replace(pos, 1, "\\");
    }
    return *this;
}

tt_string& tt_string::replace_extension(std::string_view newExtension)
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
    if (!tt::is_found(pos_file))
        pos_file = 0;

    if (auto pos = find_last_of('.'); tt::is_found(pos) && pos > pos_file)
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

tt_string_view tt_string::extension() const noexcept
{
    if (empty())
        return "";

    auto pos = find_last_of('.');
    if (!tt::is_found(pos))
        return "";

    // . by itself is a folder
    else if (pos + 1 >= length())
        return "";
    // .. is not a valid extension (it's usually part of a folder as in "../dir/")
    else if (c_str()[pos + 1] == '.')
        return "";

    return { c_str() + pos, length() - pos };
}

tt_string_view tt_string::filename() const noexcept
{
    if (empty())
        return "";

    auto pos = find_last_of('/');

#ifdef _WIN32
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

size_t tt_string::find_filename() const noexcept
{
    if (empty())
        return tt::npos;

    auto pos = find_last_of('/');

#ifdef _WIN32
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

tt_string& tt_string::replace_filename(std::string_view newFilename)
{
    if (empty())
    {
        assign(newFilename);
        return *this;
    }

    auto pos = find_last_of('/');

#ifdef _WIN32
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

tt_string& tt_string::append_filename(std::string_view filename)
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

tt_string& tt_string::assignCwd()
{
#ifdef _WIN32
    clear();
    tt::utf16to8(std::filesystem::current_path().c_str(), *this);
#else
    assign(std::filesystem::current_path().c_str());
#endif
    return *this;
}

tt_string& tt_string::make_relative(tt_string_view relative_to)
{
    if (empty())
        return *this;

    wxFileName file(make_wxString());
    file.MakeRelativeTo(relative_to.make_wxString());
    assign(file.GetFullPath().utf8_string());
    return *this;
}

tt_string& tt_string::make_absolute()
{
    if (!empty())
    {
#ifdef _WIN32
        auto current = std::filesystem::path(to_utf16());
        clear();
        tt::utf16to8(std::filesystem::absolute(current).wstring(), *this);
#else
        auto current = std::filesystem::path(c_str());
        assign(std::filesystem::absolute(current).string());
#endif
    }
    return *this;
}

bool tt_string::file_exists() const
{
    if (empty())
        return false;
#ifdef _WIN32
    auto file = std::filesystem::directory_entry(std::filesystem::path(to_utf16()));
#else
    auto file = std::filesystem::directory_entry(std::filesystem::path(c_str()));
#endif
    return (file.exists() && !file.is_directory());
}

bool tt_string::dir_exists() const
{
    if (empty())
        return false;
#ifdef _WIN32
    auto file = std::filesystem::directory_entry(std::filesystem::path(to_utf16()));
#else
    auto file = std::filesystem::directory_entry(std::filesystem::path(c_str()));
#endif
    return (file.exists() && file.is_directory());
}

bool tt_string::ChangeDir(bool is_dir) const
{
    if (empty())
        return false;
    try
    {
        if (is_dir)
        {
#if defined(_WIN32)

            auto dir = std::filesystem::directory_entry(std::filesystem::path(to_utf16()));
#else
            auto dir = std::filesystem::directory_entry(std::filesystem::path(c_str()));
#endif  // _WIN32
            if (dir.exists())
            {
                std::filesystem::current_path(dir);
                return true;
            }
        }
        else
        {
            tt_string tmp(*this);
            tmp.remove_filename();
            if (tmp.empty())
                return false;
#if defined(_WIN32)
            auto dir = std::filesystem::directory_entry(std::filesystem::path(tmp.to_utf16()));
#else
            auto dir = std::filesystem::directory_entry(std::filesystem::path(tmp.c_str()));
#endif  // _WIN32
            if (dir.exists())
            {
                std::filesystem::current_path(dir);
                return true;
            }
        }
    }
    catch (const std::exception& /* e */)
    {
    }
    return false;
}

size_t tt_string::find_oneof(const char* pszSet) const
{
    if (!pszSet || !*pszSet)
        return npos;
    const char* pszFound = std::strpbrk(c_str(), pszSet);
    if (!pszFound)
        return npos;
    return (static_cast<size_t>(pszFound - c_str()));
}

size_t tt_string::find_oneof(const char* pset, size_t start) const
{
    if (!pset || !*pset || start > size())
        return tt::npos;
    const char* pstart = c_str() + start;
    const char* pfound = std::strpbrk(pstart, pset);
    if (!pfound)
        return tt::npos;
    return (static_cast<size_t>(pfound - pstart));
}

size_t tt_string::find_space(size_t start) const
{
    if (start >= length())
        return npos;
    const char* pszFound = std::strpbrk(c_str() + start, " \t\r\n\f");
    if (!pszFound)
        return npos;
    return (static_cast<size_t>(pszFound - c_str()));
}

size_t tt_string::find_nonspace(size_t start) const
{
    for (; start < length(); ++start)
    {
        if (!std::strchr(" \t\r\n\f", at(start)))
            break;
    }
    return start;
}

size_t tt_string::stepover(size_t start) const
{
    auto pos = find_space(start);
    if (pos != npos)
    {
        pos = find_nonspace(pos);
    }
    return pos;
}

std::wstring tt_string::to_utf16() const
{
    std::wstring str16;
    tt::utf8to16(*this, str16);
    return str16;
}

tt_string_view tt_string::subview(size_t start, size_t len) const
{
    if (start >= size())
        return tt_string_view(tt::emptystring);
#ifdef min
    return std::string_view(c_str() + start, min(size() - start, len));
#else
    return std::string_view(c_str() + start, std::min(size() - start, len));
#endif
}

bool tt_string::assignEnvVar(const char* env_var)
{
    clear();
    if (!env_var || !*env_var)
        return false;
    auto pEnv = std::getenv(env_var);
    if (!pEnv)
        return false;
    assign(pEnv);
    return true;
}

void tt_string::erase_from(char ch)
{
    auto pos = find(ch);
    if (pos != tt::npos)
    {
        erase(pos);
        trim();
    }
}

void tt_string::erase_from(std::string_view sub)
{
    auto pos = find(sub);
    if (pos != tt::npos)
    {
        erase(pos);
        trim();
    }
}

tt_string& tt_string::Format(std::string_view format, ...)
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

            if (tt::is_digit(format.at(pos)))
            {
                auto fieldWidth = tt::atoi(format.substr(pos));
                buffer << std::setw(fieldWidth);
                do
                {
                    ++pos;
                } while (pos < format.length() && tt::is_digit(format.at(pos)));
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
                    tt::utf16to8(str16, str8);
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
                    tt::utf16to8(str16, str8);
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
        assert(!"exception in tt_string.Format()");
    }

    va_end(args);

    this->assign(buffer.str());

    return *this;
}

tt_string_view tt_string::subview(size_t start) const
{
    if (static_cast<ptrdiff_t>(start) == -1)
        start = length();
    assert(start <= length());
    return std::string_view(c_str() + start, length() - start);
}

tt_string_view tt_string::view_space(size_t start) const
{
    return subview(find_space(start));
}

tt_string_view tt_string::view_nonspace(size_t start) const
{
    return subview(find_nonspace(start));
}

tt_string_view tt_string::view_stepover(size_t start) const
{
    return subview(stepover(start));
}

// This is a static function, so no access to the class buffer.
bool tt_string::MkDir(const tt_string& path, bool recursive)
{
    if (path.empty())
        return false;

#ifdef _WIN32
    auto dir_path = std::filesystem::path(path.to_utf16());
    if (recursive)
        return std::filesystem::create_directories(dir_path);
    else
        return std::filesystem::create_directory(dir_path);
#else
    auto dir_path = std::filesystem::path(path.c_str());
    if (recursive)
        return std::filesystem::create_directories(dir_path);
    else
        return std::filesystem::create_directory(dir_path);
#endif
}
