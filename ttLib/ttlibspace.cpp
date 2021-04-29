/////////////////////////////////////////////////////////////////////////////
// Name:      ttlibspace.cpp
// Purpose:   ttlib namespace functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <cassert>
#include <cctype>
#include <locale>

#include "ttcstr.h"
#include "ttcview.h"
#include "ttlibspace.h"

using namespace ttlib;
using namespace tt;

// Global empty string.
const std::string ttlib::emptystring { std::string() };

const char* ttlib::next_utf8_char(const char* psz) noexcept
{
    if (!psz)
        return nullptr;
    if (!*psz)
        return psz;
    size_t i = 0;
    (void) (ttlib::is_utf8(psz[++i]) || ttlib::is_utf8(psz[++i]) || ttlib::is_utf8(psz[++i]));

    return psz + i;
}

size_t ttlib::get_hash(std::string_view str) noexcept
{
    if (str.empty())
        return 0;

    // djb2 hash algorithm

    size_t hash = 5381;

    for (auto iter: str)
        hash = ((hash << 5) + hash) ^ iter;

    return hash;
}

std::string_view ttlib::find_space(std::string_view str) noexcept
{
    if (str.empty())
        return {};
    size_t pos;
    for (pos = 0; pos < str.length(); ++pos)
    {
        if (ttlib::is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.length())
        return {};
    else
        return str.substr(pos);
}

size_t ttlib::find_space_pos(std::string_view str)
{
    auto view = ttlib::find_space(str);
    if (view.empty())
        return tt::npos;
    else
        return (str.size() - view.size());
}

std::string_view ttlib::find_nonspace(std::string_view str) noexcept
{
    if (str.empty())
        return {};
    size_t pos;
    for (pos = 0; pos < str.length(); ++pos)
    {
        if (!ttlib::is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.length())
        return {};
    else
        return str.substr(pos);
}

size_t ttlib::find_nonspace_pos(std::string_view str) noexcept
{
    auto view = ttlib::find_nonspace(str);
    if (view.empty())
        return tt::npos;
    else
        return (str.size() - view.size());
}

std::string_view ttlib::stepover(std::string_view str) noexcept
{
    if (str.empty())
        return {};

    size_t pos;
    for (pos = 0; pos < str.length(); ++pos)
    {
        if (ttlib::is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.length())
        return {};

    for (; pos < str.length(); ++pos)
    {
        if (!ttlib::is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.length())
        return {};
    else
        return str.substr(pos);
}

size_t ttlib::stepover_pos(std::string_view str) noexcept
{
    auto view = ttlib::stepover(str);
    if (view.empty())
        return tt::npos;
    else
        return (str.size() - view.size());
}

ttlib::cview ttlib::view_space(const std::string& str, size_t startpos) noexcept
{
    if (str.empty() || startpos > str.length())
        return ttlib::cview(ttlib::emptystring.c_str(), 0);
    size_t pos;
    for (pos = startpos; pos < str.length(); ++pos)
    {
        if (ttlib::is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.length())
        return ttlib::cview(ttlib::emptystring.c_str(), 0);
    else
        return ttlib::cview(str.c_str() + pos, str.length() - pos);
}

ttlib::cview ttlib::view_nonspace(const std::string& str, size_t startpos) noexcept
{
    if (str.empty())
        return ttlib::cview(ttlib::emptystring.c_str(), 0);
    size_t pos;
    for (pos = startpos; pos < str.length(); ++pos)
    {
        if (!ttlib::is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.length())
        return ttlib::cview(ttlib::emptystring.c_str(), 0);
    else
        return ttlib::cview(str.c_str() + pos, str.length() - pos);
}

ttlib::cview ttlib::view_stepover(const std::string& str, size_t startpos) noexcept
{
    if (str.empty())
        return ttlib::cview(ttlib::emptystring.c_str(), 0);

    size_t pos;
    for (pos = startpos; pos < str.length(); ++pos)
    {
        if (ttlib::is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.length())
        return ttlib::cview(ttlib::emptystring.c_str(), 0);

    for (; pos < str.length(); ++pos)
    {
        if (!ttlib::is_whitespace(str.at(pos)))
            break;
    }
    if (pos >= str.length())
        return ttlib::cview(ttlib::emptystring.c_str(), 0);
    else
        return ttlib::cview(str.c_str() + pos, str.length() - pos);
}

bool ttlib::is_sameprefix(std::string_view strMain, std::string_view strSub, CASE checkcase)
{
    if (strSub.empty())
        return strMain.empty();

    if (strMain.empty() || strMain.length() < strSub.length())
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

std::string_view ttlib::find_str(std::string_view main, std::string_view sub, CASE checkcase)
{
    if (sub.empty())
        return {};

    if (checkcase == CASE::exact)
    {
        auto pos = main.find(sub);
        if (pos < main.length())
            return main.substr(pos);

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

size_t ttlib::findstr_pos(std::string_view main, std::string_view sub, CASE checkcase)
{
    auto view = ttlib::find_str(main, sub, checkcase);
    if (view.empty())
        return tt::npos;
    else
        return (main.size() - view.size());
}

bool ttlib::contains(std::string_view main, std::string_view sub, CASE checkcase)
{
    return !ttlib::find_str(main, sub, checkcase).empty();
}

bool ttlib::is_sameas(std::string_view str1, std::string_view str2, CASE checkcase)
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

int ttlib::atoi(std::string_view str) noexcept
{
    if (str.empty())
        return 0;

    str = ttlib::find_nonspace(str);

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

std::string_view ttlib::find_extension(std::string_view str)
{
    auto pos = str.rfind('.');
    if (pos == std::string_view::npos)
        return {};
    else if (pos + 1 >= str.length())  // . by itself is a folder
        return {};
    else if (pos < 2 && (str.at(pos + 1) == '.'))
        return {};

    return str.substr(pos);
}

bool ttlib::is_valid_filechar(std::string_view str, size_t pos)
{
    if (str.empty() || pos > str.length())
        return false;

    switch (str.at(pos))
    {
        case '.':
            if (pos + 1 >= str.length())  // . by itself is a folder
                return false;
            if (pos < 2 && (str.at(pos + 1) == '.'))
                return false;  // ".." is a folder
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

void ttlib::backslashestoforward(std::string& str)
{
    for (auto pos = str.find('\\'); pos != std::string::npos; pos = str.find('\\'))
    {
        str.replace(pos, 1, "/");
    }
}

bool ttlib::has_extension(std::filesystem::directory_entry name, std::string_view extension, CASE checkcase)
{
    if (!name.is_directory())
    {
        auto ext = name.path().extension();
        if (ext.empty())
            return false;
        return ttlib::is_sameas(ext.string(), extension, checkcase);
    }
    return false;
}

bool ttlib::ChangeDir(std::string_view newdir)
{
    if (newdir.empty())
        return false;
    try
    {
#if defined(_WIN32)
        std::wstring str16;
        ttlib::utf8to16(newdir, str16);
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

bool ttlib::dir_exists(std::string_view dir)
{
    if (dir.empty())
        return false;
    try
    {
#if defined(_WIN32)
        std::wstring str16;
        ttlib::utf8to16(dir, str16);
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

bool ttlib::file_exists(std::string_view filename)
{
    if (filename.empty())
        return false;
    try
    {
#if defined(_WIN32)
        std::wstring str16;
        ttlib::utf8to16(filename, str16);
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

ttlib::cstr ttlib::itoa(int val, bool format)
{
    ttlib::cstr str;
    if (format)
        str.Format("%kd", val);
    else
        str.Format("%d", val);
    return str;
}

ttlib::cstr ttlib::itoa(size_t val, bool format)
{
    ttlib::cstr str;
    if (format)
        str.Format("%kzu", val);
    else
        str.Format("%zu", val);
    return str;
}

#define UINT8(ch)  static_cast<uint8_t>(ch)
#define CHAR8(ch)  static_cast<char>(ch)
#define CHAR16(ch) static_cast<char16_t>(ch)

ttlib::cstr ttlib::utf16to8(std::wstring_view str)
{
    ttlib::cstr str8;
    ttlib::utf16to8(str, str8);
    return str8;
}

void ttlib::utf16to8(std::wstring_view str, std::string& dest)
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

std::wstring ttlib::utf8to16(std::string_view str)
{
    std::wstring str16;
    ttlib::utf8to16(str, str16);
    return str16;
}

void ttlib::utf8to16(std::string_view str, std::wstring& dest)
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
                assert(pos + 1 < str.size());
                val = ((val << 6) & 0x7FF) + (str[++pos] & 0x3F);
            }

            else if ((UINT8(str[pos]) >> 4) == 14)
            {
                assert(pos + 2 < str.size());
                val = ((val << 12) & 0xFFFF) + ((str[++pos] << 6) & 0xFFF);
                val += (str[++pos] & 0x3F);
            }

            else if ((UINT8(str[pos]) >> 3) == 30)
            {
                assert(pos + 3 < str.size());
                val = ((val << 18) & 0x1FFFFF) + ((str[++pos] << 12) & 0x3FFFF);
                val += (str[++pos] << 6) & 0xFFF;
                val += (str[++pos] & 0x3F);
            }
            else
            {
                assertm(str[pos], "Invalid UTF8 string");
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
