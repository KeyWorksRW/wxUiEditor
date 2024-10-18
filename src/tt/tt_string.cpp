/////////////////////////////////////////////////////////////////////////////
// Purpose:   std::string with additional methods
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/string.h>    // wxString class

#include <cstdarg>  // for va_list
#include <cstring>
#include <sstream>  // for std::stringstream

#include "tt_string.h"

namespace fs = std::filesystem;

#ifdef __cpp_lib_char8_t
    #define CHAR8_T_CAST (char8_t const*)
#else
    #define CHAR8_T_CAST (char const*)
#endif

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
    wxString str = wxString::FromUTF8(data());
    str.MakeLower();
    *this = str.utf8_string();
    return *this;
}

tt_string& tt_string::MakeUpper()
{
    wxString str = wxString::FromUTF8(data());
    str.MakeUpper();
    *this = str.utf8_string();
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

tt_string& tt_string::assign_path(std::filesystem::path path)
{
#ifdef _WIN32
    clear();
    tt::utf16to8(path.wstring(), *this);
#else
    assign(path.string());
#endif
    return *this;
}

std::filesystem::path tt_string::make_path() const
{
    return fs::path(CHAR8_T_CAST c_str());
}

tt_string& tt_string::assignCwd()
{
    return assign_path(fs::current_path());
}

tt_string& tt_string::make_relative(tt_string_view relative_to)
{
    if (empty())
        return *this;

    try
    {
        auto original = fs::absolute(fs::path(CHAR8_T_CAST c_str()));
        auto relative = fs::absolute(fs::path(CHAR8_T_CAST relative_to.ToStdString().c_str()));

        return assign_path(fs::relative(original, relative, tt::error_code));
    }
    catch (const std::exception& /* e */)
    {
    }
    return *this;
}

tt_string& tt_string::make_absolute()
{
    if (!empty())
    {
        try
        {
            fs::path path(CHAR8_T_CAST c_str());
            return assign_path(fs::absolute(path, tt::error_code));
        }
        catch (const std::exception& /* e */)
        {
        }
    }
    return *this;
}

bool tt_string::file_exists() const
{
    if (empty())
        return false;

    try
    {
        fs::path path(CHAR8_T_CAST c_str());
        auto file = fs::directory_entry(path, tt::error_code);
        if (tt::error_code)
            return false;
        return (file.exists() && !file.is_directory());
    }
    catch (const std::exception& /* e */)
    {
    }
    return false;
}

bool tt_string::dir_exists() const
{
    if (empty())
        return false;

    try
    {
        fs::path path(CHAR8_T_CAST c_str());
        auto file = fs::directory_entry(path, tt::error_code);
        if (tt::error_code)
            return false;
        return (file.exists() && file.is_directory());
    }
    catch (const std::exception& /* e */)
    {
    }
    return false;
}

std::filesystem::file_time_type tt_string::last_write_time() const
{
    fs::path path(CHAR8_T_CAST c_str());
    return fs::last_write_time(path, tt::error_code);
}

std::uintmax_t tt_string::file_size() const
{
    fs::path path(CHAR8_T_CAST c_str());
    return fs::file_size(path, tt::error_code);
}

bool tt_string::ChangeDir(bool is_dir) const
{
    if (empty())
        return false;

    try
    {
        fs::path path(CHAR8_T_CAST c_str());
        if (is_dir)
        {
            auto dir = std::filesystem::directory_entry(path, tt::error_code);
            if (dir.exists())
            {
                fs::current_path(dir, tt::error_code);
                return (!tt::error_code);
            }
        }
        else
        {
            path.remove_filename();
            auto dir = std::filesystem::directory_entry(path, tt::error_code);
            if (dir.exists())
            {
                fs::current_path(dir, tt::error_code);
                return (!tt::error_code);
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

#if defined(_WIN32)
std::wstring tt_string::to_utf16() const
{
    std::wstring str16;
    tt::utf8to16(*this, str16);
    return str16;
}
#endif

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

    try
    {
        fs::path dir_path(CHAR8_T_CAST path.c_str());

        if (recursive)
            return fs::create_directories(dir_path);
        else
            return fs::create_directory(dir_path);
    }
    catch (const std::exception& /* e */)
    {
    }
    return false;
}
