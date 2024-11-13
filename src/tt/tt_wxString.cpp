/////////////////////////////////////////////////////////////////////////////
// Purpose:   Enhanced version of wxString
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filename.h>

#include <cwctype>
#include <filesystem>

#include "tt.h"

#include "tt_wxString.h"

namespace fs = std::filesystem;

#ifdef __cpp_lib_char8_t
    #define CHAR8_T_CAST (char8_t const*)
#else
    #define CHAR8_T_CAST (char const*)
#endif

std::string tt_wxString::sub_cstr(size_type start_pos, size_type count) const
{
    if (start_pos == 0 && count == tt::npos)
    {
        return utf8_string();
    }

    if (start_pos < size())
    {
        return Mid(start_pos, size() - start_pos).utf8_string();
    }

    return std::string();
}

tt_wxString& tt_wxString::append_view(std::string_view str, size_t posStart, size_t len)
{
    if (posStart >= str.size())
    {
        ASSERT_MSG(posStart < str.size(), "invalid starting position for append_view");
        return *this;
    }
    if (len == tt::npos)
        len = str.size() - posStart;
    this->append(wxString::FromUTF8(str.data() + posStart, len));
    return *this;
}

tt_wxString& tt_wxString::assign_view(std::string_view str, size_t posStart, size_t len)
{
    if (str.empty())
    {
        clear();
        return *this;
    }
    if (posStart >= str.size())
    {
        ASSERT_MSG(posStart < str.size(), "invalid starting position for append_view");
        return *this;
    }
    if (len == tt::npos)
        len = str.size() - posStart;
    this->assign(wxString::FromUTF8(str.data() + posStart, len));
    return *this;
}

size_t tt_wxString::find_oneof(std::string_view set) const
{
    if (set.empty())
    {
        return npos;
    }

#if defined(_WIN32) && !(wxUSE_UNICODE_UTF8)
    auto wset = tt::utf8to16(set);
    auto found = std::wcspbrk(c_str(), wset.c_str());
#else
    std::string wset(set);
    auto found = std::strpbrk(c_str(), wset.c_str());
#endif  // _WIN32

    if (!found)
    {
        return npos;
    }
    return (static_cast<size_t>(found - c_str()));
}

bool tt_wxString::is_sameas(std::string_view str, tt::CASE checkcase) const
{
    return tt::is_sameas(utf8_string(), str, checkcase);
}

tt_wxString& tt_wxString::backslashestoforward()
{
    for (auto pos = find('\\'); pos != wxString::npos; pos = find('\\'))
    {
        replace(pos, 1, "/");
    }
    return *this;
}

tt_wxString& tt_wxString::forwardslashestoback()
{
    for (auto pos = find('/'); pos != tt::npos; pos = find('/'))
    {
        replace(pos, 1, "\\");
    }
    return *this;
}

void tt_wxString::erase_from(char ch)
{
    if (auto pos = find(ch); pos != npos)
    {
        erase(pos);
    }
}

void tt_wxString::erase_from(std::string_view sub)
{
#if defined(_WIN32) && !(wxUSE_UNICODE_UTF8)
    auto pos = find(tt::utf8to16(sub));
#else
    auto pos = find(sub.data(), 0, sub.size());
#endif  // _WIN32
    if (pos != npos)
    {
        erase(pos);
    }
}

void tt_wxString::erase_from_wx(const wxString& sub)
{
    if (auto pos = find(sub); pos != npos)
    {
        erase(pos);
    }
}

size_t tt_wxString::replace_view(std::string_view oldtext, std::string_view newtext, bool replace_all)
{
    return Replace(wxString::FromUTF8(oldtext.data(), oldtext.size()), wxString::FromUTF8(newtext.data(), newtext.size()),
                   replace_all);
}

tt_wxString& tt_wxString::replace_extension(std::string_view newExtension)
{
    if (empty())
    {
        if (newExtension.empty())
            return *this;
        if (newExtension.at(0) != '.')
            *this = '.';
        append_view(newExtension);
        return *this;
    }

    auto pos_file = find_filename();
    if (!tt::is_found(pos_file))
        pos_file = 0;

    if (auto pos = find_last_of('.'); is_found(pos) && pos > pos_file)
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
            erase(pos);
            if (newExtension.at(0) != '.')
                *this += '.';
            append_view(newExtension);
        }
    }
    else if (newExtension.size())
    {
        // Current filename doesn't have an extension, so append the new one
        if (newExtension.at(0) != '.')
            *this += '.';
        append_view(newExtension);
    }

    return *this;
}

tt_wxString& tt_wxString::replace_extension_wx(const wxString& newExtension)
{
    if (empty())
    {
        if (newExtension.empty())
            return *this;
        if (newExtension.at(0) != '.')
            *this = '.';
        append(newExtension);
        return *this;
    }

    auto pos_file = find_filename();
    if (!tt::is_found(pos_file))
        pos_file = 0;

    if (auto pos = find_last_of('.'); is_found(pos) && pos > pos_file)
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
            erase(pos);
            if (newExtension.at(0) != '.')
                *this += '.';
            append(newExtension);
        }
    }
    else if (newExtension.size())
    {
        // Current filename doesn't have an extension, so append the new one
        if (newExtension.at(0) != '.')
            *this += '.';
        append(newExtension);
    }

    return *this;
}

tt_wxString& tt_wxString::replace_filename(std::string_view newFilename)
{
    if (empty())
    {
        assign_view(newFilename);
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
            assign_view(newFilename);
            return *this;
        }
    }

    erase(pos + 1);
    if (newFilename.size())
        append_view(newFilename);
    return *this;
}

tt_wxString& tt_wxString::replace_filename_wx(const wxString& newFilename)
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

    erase(pos + 1);
    if (newFilename.size())
        append(newFilename);
    return *this;
}

tt_wxString& tt_wxString::append_filename(std::string_view filename)
{
    if (filename.empty())
        return *this;
    if (empty())
    {
        assign_view(filename);
        return *this;
    }

    auto last = Last();
    if (last != '/' && last != '\\')
        *this += '/';
    append_view(filename);
    return *this;
}

tt_wxString& tt_wxString::append_filename_wx(const wxString& filename)
{
    if (filename.empty())
        return *this;
    if (empty())
    {
        assign(filename);
        return *this;
    }

    auto last = Last();
    if (last != '/' && last != '\\')
        *this += '/';
    append(filename);
    return *this;
}

tt_wxString& tt_wxString::assign_path(std::filesystem::path path)
{
#ifdef _WIN32
    assign(tt::utf16to8(path.wstring()));
#else
    assign(path.string());
#endif
    return *this;
}

std::filesystem::path tt_wxString::make_path() const
{
    return fs::path(CHAR8_T_CAST utf8_string().c_str());
}

tt_wxString& tt_wxString::make_absolute()
{
    wxFileName file(*this);
    file.MakeAbsolute();

    assign(file.GetFullPath());
    return *this;
}

tt_wxString& tt_wxString::make_relative_wx(const wxString& pathBase)
{
    wxFileName file(*this);
    file.MakeRelativeTo(pathBase);

    assign(file.GetFullPath());
    return *this;
}

tt_wxString& tt_wxString::make_relative(std::string_view pathBase)
{
    wxFileName file(*this);
    file.MakeRelativeTo(tt_wxString(pathBase));

    assign(file.GetFullPath());
    return *this;
}

std::filesystem::file_time_type tt_wxString::last_write_time() const
{
    fs::path path(CHAR8_T_CAST utf8_string().c_str());
    return fs::last_write_time(path, tt::error_code);
}

std::uintmax_t tt_wxString::file_size() const
{
    fs::path path(CHAR8_T_CAST utf8_string().c_str());
    return fs::file_size(path, tt::error_code);
}

bool tt_wxString::ChangeDir(bool is_dir) const
{
    if (empty())
        return false;

    if (is_dir)
        return wxFileName::SetCwd(*this);
    else
    {
        tt_wxString tmp(*this);
        tmp.remove_filename();
        return wxFileName::SetCwd(tmp);
    }
}

tt_wxString tt_wxString::find_file(const tt_wxString& dir, const tt_wxString& filename)
{
    auto dir_iterator = std::filesystem::recursive_directory_iterator(dir.utf8_string());
    for (auto& entry: dir_iterator)
    {
        if (entry.is_regular_file())
        {
            if (entry.path().filename() == filename.utf8_string())
            {
                return entry.path().string();
            }
        }
    }

    return wxEmptyString;
}

std::string tt_wxString::sub_find_nonspace(size_t start) const
{
    return sub_cstr(find_nonspace(start));
}

std::string tt_wxString::sub_find_space(size_t start) const
{
    return sub_cstr(find_space(start));
}

std::string tt_wxString::sub_stepover(size_t start) const
{
    return sub_cstr(stepover(start));
}

tt_string_view tt_wxString::subview(size_t start) const
{
    if (static_cast<ptrdiff_t>(start) == -1)
        start = length();
    assert(start <= length());
    return std::string_view(ToStdString().c_str() + start, length() - start);
}
