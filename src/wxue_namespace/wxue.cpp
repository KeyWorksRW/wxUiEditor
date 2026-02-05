/////////////////////////////////////////////////////////////////////////////
// Purpose:   ttwx namespace functions and declarations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filename.h>
#include <wx/string.h>

#include <algorithm>
// #include <cctype>
#include <charconv>
// #include <locale>

#include "wxue_string.h"

auto wxue::find_oneof(const wxString& src, const std::string& group, size_t src_start) -> size_t
{
    if (group.empty())
    {
        return std::string::npos;
    }

    for (; src_start < src.size(); ++src_start)
    {
        if (group.find(src[src_start]) != std::string::npos)
        {
            break;
        }
    }
    return (src_start < src.size() ? src_start : std::string::npos);
}

auto wxue::stepover(std::string_view str) noexcept -> std::string_view
{
    if (str.empty())
    {
        return {};
    }

    // Find first whitespace character
    size_t pos = 0;
    for (; pos < str.size(); ++pos)
    {
        if (is_whitespace(str.at(pos)))
        {
            break;
        }
    }
    if (pos >= str.size())
    {
        return {};
    }

    // Find first non-whitespace after the whitespace
    for (; pos < str.size(); ++pos)
    {
        if (!is_whitespace(str.at(pos)))
        {
            break;
        }
    }
    if (pos >= str.size())
    {
        return {};
    }

    return str.substr(pos);
}

auto wxue::find_nonspace(std::string_view str) noexcept -> std::string_view
{
    if (!str.empty())
    {
        auto iter = std::ranges::find_if(str,
                                         [](auto chr)
                                         {
                                             return !is_whitespace(chr);
                                         });
        if (iter != str.end())
        {
            return str.substr(std::distance(str.begin(), iter));
        }
    }

    return {};
}

auto wxue::find_extension(std::string_view str) -> std::string_view
{
    auto pos = str.rfind('.');
    if (pos == std::string_view::npos)
    {
        return {};
    }

    if (pos == 0)
    {  // If the file or folder name starts with a . then it is not an extension
        return {};
    }

    if (pos + 1 >= str.size())
    {  // . by itself is a folder
        return {};
    }

    if (pos < 2 && (str.at(pos + 1) == '.'))
    {
        return {};
    }

    return str.substr(pos);
}

void wxue::replace_extension(wxString& str, std::string_view new_extension)
{
    if (new_extension.empty())
    {
        wxFileName fname(str);
        fname.ClearExt();
        str = fname.GetFullPath();
        return;
    }

    wxString ext(new_extension);
    if (ext.at(0) != '.')
    {
        ext.Prepend('.');
    }

    wxFileName fname(str);
    wxString current_ext = fname.GetExt();
    if (current_ext.empty())
    {
        str.append(ext);
        return;
    }

    if (current_ext.at(0) != '.')
    {
        current_ext.Prepend('.');
    }

    if (ext != current_ext)
    {
        auto pos = str.rfind(current_ext);
        if (pos != wxString::npos)
        {
            str.erase(pos, current_ext.length());
            str.append(ext);
        }
    }
}

auto wxue::find_filename(std::string_view str) noexcept -> std::string_view
{
    if (str.empty())
    {
        return "";
    }

    auto pos = str.find_last_of('/');

#ifdef _WIN32
    // Windows filenames can contain both forward and back slashes, so check for a backslash as
    // well.
    auto back = str.find_last_of('\\');
    if (back != std::string_view::npos)
    {
        // If there is no forward slash, or the backslash appears after the forward slash, then use
        // it's position.
        if (pos == std::string_view::npos || back > pos)
        {
            pos = back;
        }
    }
#endif
    if (pos == std::string_view::npos)
    {
        pos = str.find_last_of(':');
        if (pos == std::string_view::npos)
        {
            return { str.data(), str.size() };
        }
    }

    return str.substr(pos + 1);
}

auto wxue::append_filename(wxString& path, const wxString& filename) -> wxString&
{
    if (!filename.IsEmpty())
    {
        if (path.IsEmpty())
        {
            path = filename;
            return path;
        }

        auto last = path.ToStdString().back();
        if (last != '/' && last != '\\')
        {
            // wxWidgets prefers to use the native path separator, however Windows handles a
            // forward slash as a separator. Using a forward slash means the path can be saved in a
            // format that works on all platforms, unlike using the wxWidgets native separator which
            // would fail if the filename were then used on a non-Windows platform.
            path.Append('/');
        }
        path.Append(filename);
    }
    return path;
}

auto wxue::append_folder_name(wxString& path, const wxString& folder_name) -> wxString&
{
    if (!folder_name.IsEmpty())
    {
        if (path.IsEmpty())
        {
            path = folder_name;
            add_trailing_slash(path, false);
            return path;
        }

        // Ensure that the string looks like it ends with a folder name
        add_trailing_slash(path, false);
        path.Append(folder_name);
        add_trailing_slash(path, false);
    }
    return path;
}

/**
 * @param str -- the string to extract from
 * @param start -- the offset into the string to begin extraction from -- this should either be to
 * leading whitespace, or to the character that marks the beginning of the sub string.
 * @return The offset to the character that terminated extraction, or **npos** if a terminating
 *         character could not be found.
 */
auto wxue::extract_substring(std::string_view src, wxString& dest, size_t start) -> size_t
{
    if (src.empty())
    {
        dest = wxEmptyString;
        return std::string::npos;
    }

    // start by finding the first non-whitespace character
    size_t pos = start;
    while (pos < src.length() && wxue::is_whitespace(src[pos]))
    {
        ++pos;
    }

    if (pos >= src.length())
    {
        return pos;
    }

    // based on the opening character, determine what the matching end character should be
    char chEnd = 0;
    char chBegin = src[pos];
    if (chBegin == '\'')
    {
        chEnd = '\'';
    }
    else if (chBegin == '`')
    {
        chEnd = '`';
    }
    else if (chBegin == '"')
    {
        chEnd = '"';
    }
    else if (chBegin == '<')
    {
        chEnd = '>';
    }
    else if (chBegin == '[')
    {
        chEnd = ']';
    }
    else if (chBegin == '(')
    {
        chEnd = ')';
    }
    else
    {
        return std::string::npos;
    }

    // now find the substring between chBegin and chEnd
    auto startPos = pos + 1;
    for (++pos; pos < src.length(); ++pos)
    {
        // only check quotes -- a slash is valid before other character pairs.
        if (src[pos] == '\\' && (chBegin == '"' || chBegin == '\'') && pos + 1 < src.length() &&
            (src[pos + 1] == chEnd))
        {
            // step over an escaped quote if the string to fetch is within a quote
            ++pos;
            continue;
        }

        if (src[pos] == chEnd)
        {
            break;
        }
    }

    if (pos >= src.length())
    {
        return std::string::npos;
    }

    dest = (src.substr(startPos, pos - startPos));
    return pos;
}

// Only use for non-UTF-8 strings -- otherwise use wxString::MakeLower()
auto wxue::MakeLower(std::string& str) -> std::string&
{
    std::ranges::transform(str, str.begin(),
                           [](unsigned char character)
                           {
                               return static_cast<char>(std::tolower(character));
                           });
    return str;
}

constexpr int hex_base = 16;
constexpr int dec_base = 10;

// Unlike std::stoi, this accepts a std::string_view, returns 0 instead of throwing exceptions, and
// handles hexadecimal numbers beginning with 0x or 0X.
auto wxue::atoi(std::string_view str) noexcept -> int
{
    if (str.empty())
    {
        return 0;
    }

#if defined(_DEBUG)
    std::string_view original = str;
    (void) original;
#endif  // _DEBUG
    str = find_nonspace(str);

    if (str.empty())
    {
        return 0;
    }
    bool negative = false;
    if (str[0] == '-')
    {
        negative = true;
        str.remove_prefix(1);
    }
    else if (str[0] == '+')
    {
        // we always default to positive, but handle the + sign if it is present
        str.remove_prefix(1);
    }

    int base = dec_base;
    if (str.size() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        base = hex_base;
        str.remove_prefix(2);
    }

    int value = 0;
    const char* begin = str.data();
    // Suppress warning about pointer arithmetic -- we need it for std::from_chars()
    const char* end =
        begin + str.size();  // NOLINT (pointer-arithmetic) // cppcheck-suppress pointerArithmetic
    auto result = std::from_chars(begin, end, value, base);
    if (result.ec == std::errc())  // means that at least some conversion was done
    {
        return (negative ? -value : value);
    }

    return 0;
}

auto wxue::contains(std::string_view haystack, char character, CASE checkcase) -> bool
{
    if (checkcase == CASE::exact)
    {
        if (auto pos = haystack.find(character); pos < haystack.length())
        {
            return true;
        }

        return false;
    }

    auto chLower = std::tolower(character);
    return std::ranges::any_of(haystack,
                               [chLower](char chr)
                               {
                                   return std::tolower(chr) == chLower;
                               });
}

auto wxue::is_sameas(std::string_view str1, std::string_view str2, CASE checkcase) -> bool
{
    if (str1.size() != str2.size())
    {
        return false;
    }

    if (str1.empty())
    {
        return str2.empty();
    }

    if (checkcase == CASE::exact)
    {
        return (str1.compare(str2) == 0);
    }
    if (checkcase == CASE::either)
    {
        auto main = str1.begin();
        auto sub = str2.begin();
        while (sub != str2.end())
        {
            auto diff = std::tolower(main[0]) - std::tolower(sub[0]);
            if (diff != 0)
            {
                return false;
            }
            ++main;
            ++sub;
            if (main == str1.end())
            {
                return (sub != str2.end() ? false : true);
            }
        }
        return (main != str1.end() ? false : true);
    }
    if (checkcase == CASE::utf8)
    {
        // For UTF-8, use wxString which properly handles multi-byte sequences
        wxString wxStr1 = wxString::FromUTF8(str1);
        wxString wxStr2 = wxString::FromUTF8(str2);
        wxStr1.MakeLower();
        wxStr2.MakeLower();
        return (wxStr1 == wxStr2);
    }
    ASSERT_MSG(false, "Unknown CASE value");
    return false;
}

auto wxue::is_sameprefix(std::string_view strMain, std::string_view strSub, CASE checkcase) -> bool
{
    if (strSub.empty())
    {
        return strMain.empty();
    }

    if (strMain.empty() || strMain.size() < strSub.size())
    {
        return false;
    }

    if (checkcase == CASE::exact)
    {
        auto iterMain = strMain.begin();
        for (auto iterSub: strSub)
        {
            if (*iterMain++ != iterSub)
            {
                return false;
            }
        }
        return true;
    }
    if (checkcase == CASE::either)
    {
        auto iterMain = strMain.begin();
        for (auto iterSub: strSub)
        {
            if (std::tolower(*iterMain++) != std::tolower(iterSub))
            {
                return false;
            }
        }
        return true;
    }
    if (checkcase == CASE::utf8)
    {
        // For UTF-8, use wxString which properly handles multi-byte sequences
        wxString wxMain = wxString::FromUTF8(strMain);
        wxString wxSub = wxString::FromUTF8(strSub);
        wxMain.MakeLower();
        wxSub.MakeLower();

        return wxMain.StartsWith(wxSub);
    }
    ASSERT_MSG(false, "Unknown CASE value");
    return false;
}
