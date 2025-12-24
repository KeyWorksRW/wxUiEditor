/////////////////////////////////////////////////////////////////////////////
// Purpose:   ttwx namespace functions and declarations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filename.h>
#include <wx/string.h>

#include <algorithm>
#include <charconv>

// Place this *before* including <string> and <string_view> in order
// to report an error if the compiler doesn't support C++17.
#include "ttwx.h"

auto ttwx::find_oneof(const wxString& src, const std::string& group, size_t src_start) -> size_t
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

auto ttwx::stepover(std::string_view str) noexcept -> std::string_view
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

auto ttwx::find_nonspace(std::string_view str) noexcept -> std::string_view
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

auto ttwx::find_extension(std::string_view str) -> std::string_view
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

void ttwx::replace_extension(wxString& str, std::string_view new_extension)
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

auto ttwx::find_filename(std::string_view str) noexcept -> std::string_view
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

auto ttwx::append_filename(wxString& path, const wxString& filename) -> wxString&
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

auto ttwx::append_folder_name(wxString& path, const wxString& folder_name) -> wxString&
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
auto ttwx::extract_substring(std::string_view src, wxString& dest, size_t start) -> size_t
{
    if (src.empty())
    {
        dest = wxEmptyString;
        return std::string::npos;
    }

    // start by finding the first non-whitespace character
    size_t pos = start;
    while (pos < src.length() && ttwx::is_whitespace(src[pos]))
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
auto ttwx::MakeLower(std::string& str) -> std::string&
{
    std::ranges::transform(str, str.begin(),
                           [](unsigned char character)
                           {
                               return static_cast<char>(std::tolower(character));
                           });
    return str;
}

auto ttwx::Replace(std::string& original, std::string_view oldtext, std::string_view newtext,
                   bool replace_all, bool case_sensitive) -> bool
{
    if (original.empty() || oldtext.empty())
    {
        return false;
    }

    bool replaced = false;
    size_t pos = 0;

    while (true)
    {
        size_t found_pos =
            locate(original, oldtext, pos, case_sensitive ? ttwx::CASE::exact : ttwx::CASE::either);

        if (found_pos == std::string::npos)
        {
            break;
        }

        original.replace(found_pos, oldtext.size(), newtext);
        replaced = true;

        if (!replace_all)
        {
            break;
        }

        // Continue searching after the replaced text
        pos = found_pos + newtext.size();

        // Prevent infinite loop if newtext contains oldtext
        if (pos >= original.size())
        {
            break;
        }
    }

    return replaced;
}

constexpr int hex_base = 16;
constexpr int dec_base = 10;

// Unlike std::stoi, this accepts a std::string_view, returns 0 instead of throwing exceptions, and
// handles hexadecimal numbers beginning with 0x or 0X.
auto ttwx::atoi(std::string_view str) noexcept -> int
{
    if (str.empty())
    {
        return 0;
    }

#ifdef _DEBUG
    std::string_view original = str;
    (void) original;
#endif  // _DEBUG
    str = find_nonspace(str);
    // ASSERT_MSG(!str.empty(), "non-empty string that doesn't have non-empty spaces -- shouldn't be
    // possible");

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

auto ttwx::locate(std::string_view haystack, std::string_view needle, size_t posStart,
                  ttwx::CASE checkcase) -> size_t
{
    if (needle.empty() || posStart >= haystack.size())
    {
        return std::string::npos;
    }

    if (checkcase == ttwx::CASE::exact)
    {
        return haystack.find(needle, posStart);
    }

    if (checkcase == ttwx::CASE::either)
    {
        auto chLower = std::tolower(needle[0]);
        for (auto pos = posStart; pos < haystack.length(); ++pos)
        {
            if (std::tolower(haystack.at(pos)) == chLower)
            {
                size_t posSub = 1;
                for (; posSub < needle.length(); ++posSub)
                {
                    if (pos + posSub >= haystack.length())
                    {
                        return std::string::npos;
                    }
                    if (std::tolower(haystack.at(pos + posSub)) != std::tolower(needle.at(posSub)))
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
        wxString wxHaystack = wxString::FromUTF8(haystack.data(), haystack.length());
        wxString wxNeedle = wxString::FromUTF8(needle.data(), needle.length());
        wxHaystack.MakeLower();
        wxNeedle.MakeLower();

        auto wxPos = wxHaystack.find(wxNeedle, posStart);
        if (wxPos == wxString::npos)
        {
            return std::string::npos;
        }
        return wxPos;
    }
    return std::string::npos;
}

auto ttwx::contains(std::string_view haystack, char character, CASE checkcase) -> bool
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

auto ttwx::is_sameas(std::string_view str1, std::string_view str2, CASE checkcase) -> bool
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
        return (str1 == str2);
    }

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
            return (sub == str2.end());
        }
    }

    return (main == str1.end());
}

auto ttwx::is_sameprefix(std::string_view strMain, std::string_view strSub, CASE checkcase) -> bool
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
        wxString wxMain = wxString::FromUTF8(strMain.data(), strMain.length());
        wxString wxSub = wxString::FromUTF8(strSub.data(), strSub.length());
        wxMain.MakeLower();
        wxSub.MakeLower();

        return wxMain.StartsWith(wxSub);
    }
    FAIL_MSG("Unknown CASE value");
    return false;
}
