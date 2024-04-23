/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class for reading and writing line-oriented strings/files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include "tt_string_vector.h"

#include "tt_view_vector.h"  // tt_view_vector -- Class for reading and writing line-oriented strings/files

void tt_string_vector::SetString(std::string_view str, char separator, tt::TRIM trim)
{
    clear();
    if (str.empty())
        return;

    // If the separator is a quote, then assume each substring is contained within quotes.
    if (separator == '"')
    {
        auto start = str.find_first_of(separator);
        if (start == tt::npos)
        {
            return;
        }

        str.remove_prefix(start);
        emplace_back();
        auto& first_item = back();
        auto end = first_item.ExtractSubString(str);
        if (first_item.empty())
        {
            // Ignore empty items
            pop_back();
        }
        else
        {
            if (trim != tt::TRIM::none)
            {
                first_item.trim(trim);
            }
        }

        while (end != tt::npos && end + 1 < str.length())
        {
            start = str.find_first_of(separator, end + 1);
            if (start == tt::npos)
            {
                return;
            }
            str.remove_prefix(start);
            emplace_back();
            auto& last_item = back();
            end = last_item.ExtractSubString(str);
            if (last_item.empty())
            {
                // Ignore empty items
                pop_back();
            }
            else
            {
                if (trim != tt::TRIM::none)
                {
                    last_item.trim(trim);
                }
            }
        }

        return;
    }

    size_t start = 0;
    size_t end = str.find_first_of(separator);

    // The last string will not have a separator, so end == tt::npos, but we still need to add that final string
    for (;;)
    {
        emplace_back();
        if (trim == tt::TRIM::both || trim == tt::TRIM::left)
        {
            auto begin = str.find_first_not_of(" \t\n\r\f\v", start);
            if (begin != tt::npos)
            {
                start = begin;
            }
            else
            {
                if (end != tt::npos)
                {
                    start = end;
                }
                else
                {
                    // We're at the end, there's nothing here but whitespace, so we're done
                    break;
                }
            }
        }

        if (trim == tt::TRIM::both || trim == tt::TRIM::right)
        {
            auto temp_end = end;
            if (end == tt::npos)
                temp_end = str.length();
            while (temp_end > start && tt::is_whitespace(str.at(temp_end - 1)))
            {
                --temp_end;
            }
            back().assign(str.substr(start, temp_end - start));
        }

        else if (end == tt::npos)
        {
            back().assign(str.substr(start, str.length() - start));
        }
        else
        {
            back().assign(str.substr(start, end - start));
        }

        // The last string will not have a separator after it so end will already be set to tt::npos
        if (end == tt::npos)
            break;

        start = end + sizeof(char);
        if (start >= str.length())
            break;

        end = str.find_first_of(separator, start);
    }
}

void tt_string_vector::SetString(std::string_view str, std::string_view separator, tt::TRIM trim)
{
    clear();
    size_t start = 0;
    size_t end = str.find_first_of(separator);

    // The last string will not have a separator, so end == tt::npos, but we still need to add that final string
    for (;;)
    {
        emplace_back();
        if (trim == tt::TRIM::both || trim == tt::TRIM::left)
        {
            auto begin = str.find_first_not_of(" \t\n\r\f\v", start);
            if (begin != tt::npos)
            {
                start = begin;
            }
            else
            {
                if (end != tt::npos)
                {
                    start = end;
                }
                else
                {
                    // We're at the end, there's nothing here but whitespace, so we're done
                    break;
                }
            }
        }

        if (trim == tt::TRIM::both || trim == tt::TRIM::right)
        {
            auto temp_end = end;
            if (end == tt::npos)
                temp_end = str.length();
            while (temp_end > start && tt::is_whitespace(str.at(temp_end - 1)))
            {
                --temp_end;
            }
            back().assign(str.substr(start, temp_end - start));
        }

        else if (end == tt::npos)
        {
            back().assign(str.substr(start, str.length() - start));
        }
        else
        {
            back().assign(str.substr(start, end - start));
        }

        // The last string will not have a separator after it so end will already be set to tt::npos
        if (end == tt::npos)
            break;

        start = end + separator.length();
        if (start >= str.length())
            break;

        end = str.find_first_of(separator, start);
    }
}

size_t tt_string_vector::find(size_t start, std::string_view str, tt::CASE checkcase) const
{
    for (; start < size(); ++start)
    {
        if (tt::is_sameas(at(start), str, checkcase))
            return start;
    }
    return tt::npos;
}

size_t tt_string_vector::findprefix(size_t start, std::string_view str, tt::CASE checkcase) const
{
    if (checkcase == tt::CASE::exact)
    {
        for (; start < size(); ++start)
        {
            if (tt::is_sameprefix(at(start), str, checkcase))
                return start;
        }
    }
    else
    {
        for (; start < size(); ++start)
        {
            if (tt::is_sameprefix(at(start), str, checkcase))
                return start;
        }
    }
    return tt::npos;
}
bool tt_string_vector::ReadFile(std::string_view filename)
{
    m_filename.assign(filename);
    clear();
#if defined(_WIN32)
    auto path = std::filesystem::path(m_filename.c_str());
    std::ifstream file(path, std::ios::binary);
#else
    std::ifstream file(m_filename, std::ios::binary);
#endif  // _WIN32
    if (!file.is_open())
        return false;
    std::string buf(std::istreambuf_iterator<char>(file), {});
    if (buf.size() > 2)
    {
        // Check for BOM LE or BOM UTF-8 -- other types are not supported.
        if (buf[0] == static_cast<char>(0xFF) && buf[1] == static_cast<char>(0xFE))
        {
            // BOM LE format, so convert to utf-8 before parsing
            auto utf8_buf = tt::utf16to8(reinterpret_cast<const wchar_t*>(buf.c_str() + 2));
            ParseLines(utf8_buf);
        }
        else if (buf[0] == static_cast<char>(0xEF) && buf[1] == static_cast<char>(0xBB) && buf[1] == static_cast<char>(0xBF))
        {
            // BOM utf-8 string, so skip over the BOM and process normally
            ParseLines(buf.c_str() + 3);
        }
        else
        {
            ParseLines(buf);
        }
    }
    else
    {
        // A file with only 2 bytes or less is probably worthless, but parse it anyway.
        ParseLines(buf);
    }
    return true;
}

bool tt_string_vector::WriteFile(const std::string& filename) const
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
        return false;
    for (auto iter: *this)
    {
        file << iter << '\n';
    }

    return true;
}

void tt_string_vector::ReadString(std::string_view str)
{
    if (!str.empty())
        ParseLines(str);
}

void tt_string_vector::ReadArray(const char** begin)
{
    assert(begin);
    if (!begin)
        return;

    while (*begin)
    {
        push_back(*begin);
        ++begin;
    }
}

void tt_string_vector::ReadArray(const char** begin, size_t count)
{
    assert(begin && count != tt::npos);
    if (!begin || count == tt::npos)
        return;

    while (count > 0)
    {
        push_back(*begin++);
        --count;
    }
}

void tt_string_vector::ParseLines(std::string_view str)
{
    size_t posBeginLine = 0;
    for (size_t pos = 0; pos < str.size(); ++pos)
    {
        if (str[pos] == '\r')
        {
            emplace_back();
            if (pos > posBeginLine)
            {
                back().assign(str.substr(posBeginLine, pos - posBeginLine));
            }
            else
            {
                back().assign(tt::emptystring);
            }

            // Some Apple format files only use \r. Windows files tend to use \r\n.
            if (pos + 1 < str.size() && str[pos + 1] == '\n')
                ++pos;
            posBeginLine = pos + 1;
        }
        else if (str[pos] == '\n')
        {
            emplace_back();
            if (pos > posBeginLine)
            {
                back().assign(str.substr(posBeginLine, pos - posBeginLine));
            }
            else
            {
                back().assign(tt::emptystring);
            }
            posBeginLine = pos + 1;
        }
    }
}

size_t tt_string_vector::FindLineContaining(std::string_view str, size_t start, tt::CASE checkcase) const
{
    for (; start < size(); ++start)
    {
        if (at(start).contains(str, checkcase))
            return start;
    }
    return tt::npos;
}

size_t tt_string_vector::ReplaceInLine(std::string_view orgStr, std::string_view newStr, size_t posLine, tt::CASE checkcase)
{
    for (; posLine < size(); ++posLine)
    {
        if (at(posLine).contains(orgStr, checkcase))
        {
            at(posLine).Replace(orgStr, newStr, false, checkcase);
            return posLine;
        }
    }
    return tt::npos;
}

bool tt_string_vector::is_sameas(const tt_view_vector& other, tt::CASE checkcase) const
{
    if (size() != other.size())
        return false;

    size_t pos = 0;
    for (; pos < other.size(); ++pos)
    {
        if (!at(pos).is_sameas(other[pos], checkcase))
            break;
    }
    return (pos == size());
}

bool tt_string_vector::is_sameas(const tt_string_vector& other, tt::CASE checkcase) const
{
    if (size() != other.size())
        return false;

    size_t pos = 0;
    for (; pos < other.size(); ++pos)
    {
        if (!at(pos).is_sameas(other[pos], checkcase))
            break;
    }
    return (pos == size());
}
