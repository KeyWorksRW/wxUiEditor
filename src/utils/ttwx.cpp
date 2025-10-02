/////////////////////////////////////////////////////////////////////////////
// Purpose:   ttwx namespace functions and declarations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filename.h>
#include <wx/string.h>

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
        if (std::strchr(group.c_str(), src.at(src_start)) != nullptr)
        {
            break;
        }
    }
    return (src_start < src.size() ? src_start : std::string::npos);
}

auto ttwx::find_nonspace(std::string_view str) noexcept -> std::string_view
{
    if (!str.empty())
    {
        for (size_t pos = 0; pos < str.size(); ++pos)
        {
            if (!is_whitespace(str.at(pos)))
            {
                return str.substr(pos);
            }
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
            // wxWidgets prefers to using the native path separator, however Windows handles a
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
    if (chBegin == '`' || chBegin == '\'')
    {
        chEnd = '\'';
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
