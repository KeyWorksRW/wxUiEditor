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

std::string tt_wxString::sub_cstr(size_type start_pos, size_type count) const
{
    std::string str;
    if (start_pos == 0 && count == tt::npos)
    {
#if defined(_WIN32)
        tt::utf16to8(wx_str(), str);
#else
        str = *this;
#endif  // _WIN32
    }
    else if (start_pos < size())
    {
        str = Mid(start_pos, size() - start_pos).utf8_string();
    }
    return str;
}

tt_wxString& tt_wxString::append_view(std::string_view str, size_t posStart, size_t len)
{
    if (posStart >= str.size())
    {
        assertm(posStart < str.size(), "invalid starting position for append_view");
        return *this;
    }
    if (len == npos)
        len = (str.size() - posStart);
#if defined(_WIN32)
    this->append(wxString::FromUTF8(str.data() + posStart, len));
#else
    this->append(str.data() + posStart, len);
#endif  // _WIN32
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
        assertm(posStart < str.size(), "invalid starting position for append_view");
        return *this;
    }
    if (len == npos)
        len = (str.size() - posStart);
#if defined(_WIN32)
    this->assign(wxString::FromUTF8(str.data() + posStart, len));
#else
    this->assign(str.data() + posStart, len);
#endif  // _WIN32
    return *this;
}

int tt_wxString::comparei(std::string_view str) const
{
    tt_wxString tmp(str);
    return CmpNoCase(tmp);
}

size_t tt_wxString::locate(std::string_view vstr, size_t posStart, tt::CASE checkcase) const
{
    if (vstr.empty() || posStart >= size())
        return npos;

    tt_wxString str(vstr);

    if (checkcase == tt::CASE::exact)
        return find(str, posStart);

#if defined(_WIN32)
    // Note that we don't support tt::CASE::utf8 under windows

    auto chLower = std::towlower(str[0]);
    for (auto pos = posStart; pos < length(); ++pos)
    {
        if (std::towlower(at(pos)) == chLower)
        {
            size_t posSub;
            for (posSub = 1; posSub < str.length(); ++posSub)
            {
                if (pos + posSub >= length())
                    return npos;
                if (std::towlower(at(pos + posSub)) != std::towlower(str.at(posSub)))
                    break;
            }
            if (posSub >= str.length())
                return pos;
        }
    }

#else
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

#endif  // _WIN32

    return npos;
}

size_t tt_wxString::locate_wx(const wxString& str, size_t posStart, tt::CASE checkcase) const
{
    if (str.empty() || posStart >= size())
        return npos;

    if (checkcase == tt::CASE::exact)
        return find(str, posStart);

#if defined(_WIN32)
    auto chLower = std::towlower(str[0]);
    for (auto pos = posStart; pos < length(); ++pos)
    {
        if (std::towlower(at(pos)) == chLower)
        {
            size_t posSub;
            for (posSub = 1; posSub < str.length(); ++posSub)
            {
                if (pos + posSub >= length())
                    return npos;
                if (std::towlower(at(pos + posSub)) != std::towlower(str.at(posSub)))
                    break;
            }
            if (posSub >= str.length())
                return pos;
        }
    }
#else
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
#endif  // _WIN32

    return npos;
}

size_t tt_wxString::find_oneof(std::string_view set) const
{
    if (set.empty())
        return npos;

#if defined(_WIN32)
    auto wset = ttlib::utf8to16(set);
    auto found = std::wcspbrk(c_str(), wset.c_str());
#else
    std::string wset(set);
    auto found = std::strpbrk(c_str(), wset.c_str());
#endif  // _WIN32

    if (!found)
        return npos;
    return (static_cast<size_t>(found - c_str()));
}

#if defined(_WIN32)

size_t tt_wxString::find_oneof_wx(const wxString& set) const
{
    if (set.empty())
        return npos;

    auto found = std::wcspbrk(c_str(), set.c_str());
    if (!found)
        return npos;
    return (static_cast<size_t>(found - c_str()));
}

#endif

size_t tt_wxString::find_space(size_t start) const
{
    if (start >= length())
        return npos;

#if defined(_WIN32)
    auto found = std::wcspbrk(c_str() + start, L" \t\r\n\f");
#else
    auto found = std::strpbrk(c_str() + start, " \t\r\n\f");
#endif
    if (!found)
        return npos;
    return (static_cast<size_t>(found - c_str()));
}

size_t tt_wxString::find_nonspace(size_t start) const
{
    for (; start < size(); ++start)
    {
#if defined(_WIN32)
        if (!std::wcschr(L" \t\r\n\f", at(start)))
            break;
#else
        if (!std::strchr(" \t\r\n\f", at(start)))
            break;
#endif
    }
    return start;
}

size_t tt_wxString::stepover(size_t start) const
{
    auto pos = find_space(start);
    if (pos != npos)
    {
        pos = find_nonspace(pos);
    }
    return pos;
}

bool tt_wxString::is_sameas(std::string_view str, tt::CASE checkcase) const
{
    tt_wxString tmp(str);
    return IsSameAs(tmp, checkcase == tt::CASE::exact);
}

bool tt_wxString::is_sameprefix(std::string_view vstr, tt::CASE checkcase) const
{
    if (vstr.empty())
        return empty();

    if (empty() || size() < vstr.size())
        return false;

#if defined(_WIN32)
    auto str = ttlib::utf8to16(vstr);
#else
    std::string str(vstr);
#endif  // _WIN32

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
    else
    {
        auto iterMain = begin();
        for (auto iterSub: str)
        {
#if defined(_WIN32)
            if (std::towlower(*iterMain++) != std::towlower(iterSub))
                return false;
#else
            if (std::tolower(*iterMain++) != std::tolower(iterSub))
                return false;
#endif
        }
        return true;
    }

    return false;
}

bool tt_wxString::is_sameprefix_wx(const wxString& str, tt::CASE checkcase) const
{
    if (str.empty())
        return empty();

    if (empty() || size() < str.size())
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
    else
    {
        auto iterMain = begin();
        for (auto iterSub: str)
        {
#if defined(_WIN32)
            if (std::towlower(*iterMain++) != std::towlower(iterSub))
                return false;
#else
            if (std::tolower(*iterMain++) != std::tolower(iterSub))
                return false;
#endif  // _WIN32
        }
        return true;
    }
}

tt_wxString& tt_wxString::backslashestoforward()
{
    for (auto pos = find('\\'); pos != wxString::npos; pos = find('\\'))
    {
        replace(pos, 1, wxT("/"));
    }
    return *this;
}

tt_wxString& tt_wxString::forwardslashestoback()
{
    for (auto pos = find('/'); pos != tt::npos; pos = find('/'))
    {
        replace(pos, 1, L"\\");
    }
    return *this;
}

tt_wxString tt_wxString::extension() const
{
    if (empty())
        return wxEmptyString;

    auto pos = find_last_of('.');
    if (!is_found(pos))
        return wxEmptyString;

    // . by itself is a folder
    else if (pos + 1 >= length())
        return wxEmptyString;

    // .. is not a valid extension (it's usually part of a folder as in "../dir/")
    else if (c_str()[pos + 1] == '.')
        return wxEmptyString;

    tt_wxString dest(*this, pos, size() - pos);
    return dest;
}

tt_wxString tt_wxString::filename() const
{
    if (empty())
        return wxEmptyString;

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
            tt_wxString dest(*this);
            return dest;
        }
    }

    tt_wxString dest(*this, pos + 1, size() - (pos + 1));
    return dest;
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
#if defined(_WIN32)
    auto pos = find(ttlib::utf8to16(sub));
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
    tt_wxString old_text(oldtext);
    tt_wxString new_text(newtext);

    return Replace(old_text, new_text, replace_all);
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
    if (!ttlib::is_found(pos_file))
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
    if (!ttlib::is_found(pos_file))
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

size_t tt_wxString::find_filename() const noexcept
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
#if defined(_WIN32)
    file.MakeRelativeTo(ttlib::utf8to16(pathBase));
#else
    file.MakeRelativeTo(std::string(pathBase));
#endif  // _WIN32

    assign(file.GetFullPath());
    return *this;
}

bool tt_wxString::ChangeDir(bool is_dir) const
{
    if (empty())
        return false;
    try
    {
        if (is_dir)
        {
            auto dir = std::filesystem::directory_entry(std::filesystem::path(wx_str()));
            if (dir.exists())
            {
                std::filesystem::current_path(dir);
                return true;
            }
        }
        else
        {
            tt_wxString tmp(*this);
            tmp.remove_filename();
            if (tmp.empty())
                return false;
            auto dir = std::filesystem::directory_entry(std::filesystem::path(tmp.wx_str()));
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

tt_wxString tt_wxString::find_file(const tt_wxString& dir, const tt_wxString& filename)
{
    auto dir_iterator = std::filesystem::recursive_directory_iterator(dir.wx_str());
    for (auto& entry: dir_iterator)
    {
        if (entry.is_regular_file())
        {
            if (entry.path().filename() == filename.wx_str())
            {
                return entry.path().string();
            }
        }
    }

    return wxEmptyString;
}

std::string tt_wxString::sub_find_nonspace(size_t start) const { return sub_cstr(find_nonspace(start)); }

std::string tt_wxString::sub_find_space(size_t start) const { return sub_cstr(find_space(start)); }

std::string tt_wxString::sub_stepover(size_t start) const { return sub_cstr(stepover(start)); }
