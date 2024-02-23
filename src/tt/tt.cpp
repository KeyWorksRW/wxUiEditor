/////////////////////////////////////////////////////////////////////////////
// Purpose:   tt namespace functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <filesystem>  // directory_entry
#include <string>

#include "tt.h"

#include "tt_string.h"  // tt_string -- std::string with additional methods

using namespace tt;

const std::string tt::emptystring;
std::error_code tt::error_code;

const char* tt::next_utf8_char(const char* psz) noexcept
{
    if (!psz)
        return nullptr;

    if (!*psz)
        return psz;

    size_t i = 0;
    (void) (is_utf8(psz[++i]) || is_utf8(psz[++i]) || is_utf8(psz[++i]));

    return psz + i;
}

std::string_view tt::find_space(std::string_view str) noexcept
{
    if (str.size())
    {
        for (size_t pos = 0; pos < str.size(); ++pos)
        {
            if (is_whitespace(str.at(pos)))
            {
                return str.substr(pos);
            }
        }
    }

    return {};
}

size_t tt::find_space_pos(std::string_view str)
{
    if (auto view = find_space(str); view.size())
        return (str.size() - view.size());
    return tt::npos;
}

std::string_view tt::find_nonspace(std::string_view str) noexcept
{
    if (str.size())
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

size_t tt::find_nonspace_pos(std::string_view str) noexcept
{
    if (auto view = find_nonspace(str); view.size())
    {
        return (str.size() - view.size());
    }

    return tt::npos;
}

std::string_view tt::stepover(std::string_view str) noexcept
{
    if (str.empty())
        return {};

    size_t pos;
    for (pos = 0; pos < str.size(); ++pos)
    {
        if (is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.size())
        return {};

    for (; pos < str.size(); ++pos)
    {
        if (!is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.size())
        return {};
    else
        return str.substr(pos);
}

size_t tt::stepover_pos(std::string_view str) noexcept
{
    if (auto view = stepover(str); view.size())
    {
        return (str.size() - view.size());
    }

    return tt::npos;
}

bool tt::is_sameprefix(std::string_view strMain, std::string_view strSub, CASE checkcase)
{
    if (strSub.empty())
        return strMain.empty();

    if (strMain.empty() || strMain.size() < strSub.size())
        return false;

    if (checkcase == CASE::exact)
    {
        auto iterMain = strMain.begin();
        for (auto iterSub: strSub)
        {
            if (*iterMain++ != iterSub)
                return false;
        }
        return true;
    }
    else if (checkcase == CASE::either)
    {
        auto iterMain = strMain.begin();
        for (auto iterSub: strSub)
        {
            if (std::tolower(*iterMain++) != std::tolower(iterSub))
                return false;
        }
        return true;
    }
    else if (checkcase == CASE::utf8)
    {
        auto utf8locale = std::locale("en_US.utf8");
        auto iterMain = strMain.begin();
        for (auto iterSub: strSub)
        {
            if (std::tolower(*iterMain++, utf8locale) != std::tolower(iterSub, utf8locale))
                return false;
        }
        return true;
    }
    assert(!"Unknown CASE value");
    return false;
}

std::string_view tt::find_str(std::string_view main, std::string_view sub, CASE checkcase)
{
    if (sub.empty())
        return {};

    if (checkcase == CASE::exact)
    {
        if (auto pos = main.find(sub); pos < main.length())
        {
            return main.substr(pos);
        }

        return {};
    }

    auto chLower = std::tolower(sub[0]);

    for (auto pos = 0U; pos < main.length(); ++pos)
    {
        if (std::tolower(main.at(pos)) == chLower)
        {
            size_t posSub;
            for (posSub = 1; posSub < sub.length(); ++posSub)
            {
                if (pos + posSub >= main.length())
                    return {};
                if (std::tolower(main.at(pos + posSub)) != std::tolower(sub.at(posSub)))
                    break;
            }
            if (posSub >= sub.length())
                return main.substr(pos);
        }
    }
    return {};
}

size_t tt::findstr_pos(std::string_view main, std::string_view sub, CASE checkcase)
{
    if (auto view = find_str(main, sub, checkcase); view.size())
    {
        return (main.size() - view.size());
    }

    return tt::npos;
}

bool tt::contains(std::string_view main, std::string_view sub, CASE checkcase)
{
    return tt::find_str(main, sub, checkcase).size();
}

bool tt::contains(std::string_view main, char ch, tt::CASE checkcase)
{
    if (checkcase == CASE::exact)
    {
        if (auto pos = main.find(ch); pos < main.length())
        {
            return true;
        }

        return false;
    }

    auto chLower = std::tolower(ch);

    for (auto& iter: main)
    {
        if (std::tolower(iter) == chLower)
        {
            return true;
        }
    }
    return false;
}

bool tt::is_sameas(std::string_view str1, std::string_view str2, CASE checkcase)
{
    if (str1.size() != str2.size())
        return false;

    if (str1.empty())
        return str2.empty();

    if (checkcase == CASE::exact)
        return (str1.compare(str2) == 0);

    auto main = str1.begin();
    auto sub = str2.begin();
    while (sub != str2.end())
    {
        auto diff = std::tolower(main[0]) - std::tolower(sub[0]);
        if (diff != 0)
            return false;
        ++main;
        ++sub;
        if (main == str1.end())
            return (sub != str2.end() ? false : true);
    }

    return (main != str1.end() ? false : true);
}

int tt::atoi(std::string_view str) noexcept
{
    if (str.empty())
        return 0;

    str = find_nonspace(str);

    int total = 0;
    size_t pos = 0;

    if (str[0] == '0' && str.length() > 1 && (str[1] == 'x' || str[1] == 'X'))
    {
        pos += 2;  // skip over 0x prefix in hexadecimal strings

        for (; pos < str.size(); ++pos)
        {
            auto c = str[pos];
            if (c >= '0' && c <= '9')
                total = 16 * total + (c - '0');
            else if (c >= 'a' && c <= 'f')
                total = total * 16 + c - 'a' + 10;
            else if (c >= 'A' && c <= 'F')
                total = total * 16 + c - 'A' + 10;
            else
                break;
        }
        return total;
    }

    bool negative = false;
    if (str[pos] == '-')
    {
        negative = true;
        ++pos;
    }
    else if (str[pos] == '+')
    {
        negative = false;
        ++pos;
    }

    for (; pos < str.size() && str[pos] >= '0' && str[pos] <= '9'; ++pos)
    {
        total = 10 * total + (str[pos] - '0');
    }

    return (negative ? -total : total);
}

tt_string tt::itoa(int value, bool format)
{
    if (!format)
    {
        return std::to_string(value);
    }
#ifdef __cpp_lib_format
    return std::format(std::locale(""), "{:L}", value);
#else
    return std::to_string(value);
#endif
}

tt_string tt::itoa(size_t value, bool format)
{
    if (!format)
    {
        return std::to_string(value);
    }
#ifdef __cpp_lib_format
    return std::format(std::locale(""), "{:L}", value);
#else
    return std::to_string(value);
#endif
}

std::string_view tt::find_extension(std::string_view str)
{
    auto pos = str.rfind('.');
    if (pos == std::string_view::npos)
        return {};
    else if (pos + 1 >= str.size())  // . by itself is a folder
        return {};
    else if (pos < 2 && (str.at(pos + 1) == '.'))
        return {};

    return str.substr(pos);
}

bool tt::is_valid_filechar(std::string_view str, size_t pos)
{
    if (str.empty() || pos > str.size())
        return false;

    switch (str.at(pos))
    {
        case '.':
            if (pos + 1 >= str.size())
            {
                return false;  // . by itself is a folder
            }
            if (pos < 2 && (str.at(pos + 1) == '.'))
            {
                return false;  // ".." is a folder
            }
            return true;

        case '<':
        case '>':
        case ':':
        case '/':
        case '\\':
        case '|':
        case '?':
        case '*':
        case 0:
            return false;
    }
    return true;
}

void tt::backslashestoforward(std::string& str)
{
    for (auto pos = str.find('\\'); pos != std::string::npos; pos = str.find('\\'))
    {
        str.replace(pos, 1, "/");
    }
}

void tt::backslashestoforward(wxString& str)
{
    for (auto pos = str.find('\\'); pos != std::string::npos; pos = str.find('\\'))
    {
        str.replace(pos, 1, "/");
    }
}

bool tt::has_extension(std::filesystem::directory_entry name, std::string_view extension, CASE checkcase)
{
    if (!name.is_directory())
    {
        auto ext = name.path().extension();
        if (ext.empty())
            return false;
        return is_sameas(ext.string(), extension, checkcase);
    }
    return false;
}

bool tt::ChangeDir(std::string_view newdir)
{
    if (newdir.empty())
        return false;
    try
    {
#if defined(_WIN32)
        std::wstring str16;
        tt::utf8to16(newdir, str16);
        auto dir = std::filesystem::directory_entry(std::filesystem::path(str16));
#else
        auto dir = std::filesystem::directory_entry(std::filesystem::path(newdir));
#endif
        if (dir.exists())
        {
            std::filesystem::current_path(dir);
            return true;
        }
    }
    catch (const std::exception& /* e */)
    {
    }
    return false;
}

bool tt::dir_exists(std::string_view dir)
{
    if (dir.empty())
        return false;
    try
    {
#if defined(_WIN32)
        std::wstring str16;
        tt::utf8to16(dir, str16);
        auto path = std::filesystem::directory_entry(std::filesystem::path(str16));
#else
        auto path = std::filesystem::directory_entry(std::filesystem::path(dir));
#endif
        return (path.exists() && path.is_directory());
    }
    catch (const std::exception& /* e */)
    {
    }
    return false;
}

bool tt::file_exists(std::string_view filename)
{
    if (filename.empty())
        return false;
    try
    {
#if defined(_WIN32)
        std::wstring str16;
        utf8to16(filename, str16);
        auto path = std::filesystem::directory_entry(std::filesystem::path(str16));
#else
        auto path = std::filesystem::directory_entry(std::filesystem::path(filename));
#endif
        return (path.exists() && !path.is_directory());
    }
    catch (const std::exception& /* e */)
    {
    }
    return false;
}

#define UINT8(ch)  static_cast<uint8_t>(ch)
#define CHAR8(ch)  static_cast<char>(ch)
#define CHAR16(ch) static_cast<char16_t>(ch)

std::string tt::utf16to8(std::wstring_view str)
{
    std::string str8;
    tt::utf16to8(str, str8);
    return str8;
}

void tt::utf16to8(std::wstring_view str, std::string& dest)
{
    for (size_t pos = 0; pos < str.size(); ++pos)
    {
        uint32_t val = (str[pos] & 0xFFFF);
        if (val >= 0xD800 && val <= 0xDBFF)
        {
            val = (val << 10) + (str[++pos] & 0xFFFF) + 0xFCA02400;
        }

        if (val < 0x80)
        {
            dest.push_back(CHAR8(val));
        }
        else if (val < 0x800)
        {
            dest.push_back(CHAR8((val >> 6) | 0xC0));
            dest.push_back(CHAR8((val & 0x3f) | 0x80));
        }
        else if (val < 0x10000)
        {
            dest.push_back(CHAR8((val >> 12) | 0xE0));
            dest.push_back(CHAR8(((val >> 6) & 0x3F) | 0x80));
            dest.push_back(CHAR8((val & 0x3f) | 0x80));
        }
        else
        {
            dest.push_back(CHAR8((val >> 18) | 0xF0));
            dest.push_back(CHAR8(((val >> 12) & 0x3F) | 0x80));
            dest.push_back(CHAR8(((val >> 6) & 0x3F) | 0x80));
            dest.push_back(CHAR8((val & 0x3f) | 0x80));
        }
    }
}

std::wstring tt::utf8to16(std::string_view str)
{
    std::wstring str16;
    tt::utf8to16(str, str16);
    return str16;
}

void tt::utf8to16(std::string_view str, std::wstring& dest)
{
    for (size_t pos = 0; pos < str.length(); ++pos)
    {
        if (UINT8(str[pos]) < 0x80)
            dest.push_back(CHAR16(str[pos]));
        else
        {
            uint32_t val = (str[pos] & 0xFF);
            if ((UINT8(str[pos]) >> 5) == 6)
            {
                ASSERT_MSG(pos + 1 < str.size(), "Invalid UTF8 string");
                if (pos + 1 < str.size())
                {
                    val = ((val << 6) & 0x7FF) + (str[++pos] & 0x3F);
                }
            }

            else if ((UINT8(str[pos]) >> 4) == 14)
            {
                ASSERT_MSG(pos + 2 < str.size(), "Invalid UTF8 string");
                if (pos + 2 < str.size())
                {
                    val = ((val << 12) & 0xFFFF) + ((str[++pos] << 6) & 0xFFF);
                    val += (str[++pos] & 0x3F);
                }
            }

            else if ((UINT8(str[pos]) >> 3) == 30)
            {
                ASSERT_MSG(pos + 3 < str.size(), "Invalid UTF8 string");
                if (pos + 3 < str.size())
                {
                    val = ((val << 18) & 0x1FFFFF) + ((str[++pos] << 12) & 0x3FFFF);
                    val += (str[++pos] << 6) & 0xFFF;
                    val += (str[++pos] & 0x3F);
                }
            }
            else
            {
                ASSERT_MSG(str[pos], "Invalid UTF8 string");
            }

            if (val > 0xFFFF)
            {
                dest.push_back(CHAR16((val >> 10) + 0xD7C0));
                dest.push_back(CHAR16((val & 0x3FF) + 0xDC00));
            }
            else
            {
                dest.push_back(CHAR16(val));
            }
        }
    }
}

#if (defined(_WIN32))
    #include <shellapi.h>
#else
    #include <wx/utils.h>
#endif

#if (defined(_WIN32))
HINSTANCE tt::ShellRun_wx(const wxString& filename, const wxString& args, const wxString& dir, INT nShow, HWND hwndParent)
{
    #if !(wxUSE_UNICODE_UTF8)
    return ShellExecuteW(hwndParent, NULL, filename.c_str(), args.c_str(), dir.c_str(), nShow);
    #else
    return ShellExecuteW(hwndParent, NULL, tt::utf8to16(filename.utf8_string()).c_str(),
                         tt::utf8to16(args.utf8_string()).c_str(), tt::utf8to16(dir.utf8_string()).c_str(), nShow);
    #endif
}
#else
int tt::ShellRun_wx(const wxString& filename, const wxString& args, const wxString& dir, int /* nShow */,
                    void* /* hwndParent */)
{
    // Run the command using the wxWidgets method for executing a file
    wxString cmd = filename + " " + args;
    wxExecuteEnv env;
    env.cwd = dir;
    return wxExecute(cmd, wxEXEC_ASYNC, nullptr, &env);
}
#endif
