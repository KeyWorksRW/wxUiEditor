/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class for reading and writing line-oriented strings/files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include "tt_view_vector.h"

#include "tt_string_vector.h"  // tt_string_vector -- Class for reading and writing line-oriented strings/files

void tt_view_vector::SetString(std::string_view str, char separator, tt::TRIM trim)
{
    clear();
    if (str.empty())
        return;

    size_t start = 0;
    size_t end = str.find_first_of(separator);

    // The last string will not have a separator, so end == tt::npos, but we still need to add that final string
    for (;;)
    {
        emplace_back(tt::emptystring);
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
            back() = str.substr(start, temp_end - start);
        }

        else if (end == tt::npos)
        {
            back() = str.substr(start, str.length() - start);
        }
        else
        {
            back() = str.substr(start, end - start);
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

void tt_view_vector::SetString(std::string_view str, std::string_view separator, tt::TRIM trim)
{
    clear();
    size_t start = 0;
    size_t end = str.find_first_of(separator);

    // The last string will not have a separator, so end == tt::npos, but we still need to add that final string
    for (;;)
    {
        emplace_back(tt::emptystring);
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
            back() = str.substr(start, temp_end - start);
        }

        else if (end == tt::npos)
        {
            back() = str.substr(start, str.length() - start);
        }
        else
        {
            back() = str.substr(start, end - start);
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

bool tt_view_vector::ReadFile(std::string_view filename)
{
    m_filename.assign(filename);

    clear();
    std::ifstream file(m_filename, std::ios::binary);
    if (!file.is_open())
        return false;
    m_buffer.assign(std::istreambuf_iterator<char>(file), {});
    if (m_buffer.size() > 2)
    {
        // Check for BOM LE or BOM UTF-8 -- other types are not supported.
        if (m_buffer[0] == static_cast<char>(0xFF) && m_buffer[1] == static_cast<char>(0xFE))
        {
            // BOM LE format, so convert to utf-8 before parsing

            // While the commented line is valid, it generates a security warning in CodeQL.
            // The static cast to a const void* is an attempt to avoid the security warning.

            // auto utf8_buf = tt::utf16to8(reinterpret_cast<const wchar_t*>(m_buffer.c_str() + 2));
            auto utf8_buf = tt::utf16to8(reinterpret_cast<const wchar_t*>(static_cast<const void*>((m_buffer.c_str() + 2))));
            m_buffer.clear();
            m_buffer = std::move(utf8_buf);
            ParseLines(m_buffer);
        }
        else if (m_buffer[0] == static_cast<char>(0xEF) && m_buffer[1] == static_cast<char>(0xBB) &&
                 m_buffer[1] == static_cast<char>(0xBF))
        {
            // BOM utf-8 string, so skip over the BOM and process normally
            ParseLines(m_buffer.c_str() + 3);
        }
        else
        {
            ParseLines(m_buffer);
        }
    }
    else
    {
        // A file with only 2 bytes or less is probably worthless, but parse it anyway.
        ParseLines(m_buffer);
    }

    return true;
}

void tt_view_vector::ReadString(std::string_view str)
{
    if (!str.empty())
    {
        m_buffer.assign(str);
        ParseLines(m_buffer);
    }
}

bool tt_view_vector::WriteFile(const std::string& filename) const
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

void tt_view_vector::ParseLines(std::string_view str)
{
    size_t posBeginLine = 0;
    for (size_t pos = 0; pos < str.size(); ++pos)
    {
        if (str[pos] == '\r')
        {
            if (pos > posBeginLine)
            {
                emplace_back(str.data() + posBeginLine, pos - posBeginLine);
            }
            else
            {
                emplace_back(nullptr, 0);
            }

            // Some Apple format files only use \r. Windows files tend to use \r\n.
            if (pos + 1 < str.size() && str[pos + 1] == '\n')
                ++pos;
            posBeginLine = pos + 1;
        }
        else if (str[pos] == '\n')
        {
            if (pos > posBeginLine)
            {
                emplace_back(str.data() + posBeginLine, pos - posBeginLine);
            }
            else
            {
                emplace_back(nullptr, 0);
            }
            posBeginLine = pos + 1;
        }
    }
}

void tt_view_vector::ParseBuffer()
{
    clear();

    size_t posBeginLine = 0;
    for (size_t pos = 0; pos < m_buffer.size(); ++pos)
    {
        if (m_buffer[pos] == '\r')
        {
            if (pos > posBeginLine)
            {
                emplace_back(m_buffer.data() + posBeginLine, pos - posBeginLine);
            }
            else
            {
                emplace_back(nullptr, 0);
            }

            // Some Apple format files only use \r. Windows files tend to use \r\n.
            if (pos + 1 < m_buffer.size() && m_buffer[pos + 1] == '\n')
                ++pos;
            posBeginLine = pos + 1;
        }
        else if (m_buffer[pos] == '\n')
        {
            if (pos > posBeginLine)
            {
                emplace_back(m_buffer.data() + posBeginLine, pos - posBeginLine);
            }
            else
            {
                emplace_back(nullptr, 0);
            }
            posBeginLine = pos + 1;
        }
    }
}

size_t tt_view_vector::FindLineContaining(std::string_view str, size_t start, tt::CASE checkcase) const
{
    for (; start < size(); ++start)
    {
        if (tt::contains(at(start), str, checkcase))
            return start;
    }
    return tt::npos;
}

bool tt_view_vector::is_sameas(const tt_view_vector& other, tt::CASE checkcase) const
{
    if (size() != other.size())
        return false;

    size_t pos = 0;
    for (; pos < other.size(); ++pos)
    {
        if (!tt::is_sameas(at(pos), other[pos], checkcase))
            break;
    }
    return (pos == size());
}

bool tt_view_vector::is_sameas(const tt_string_vector& other, tt::CASE checkcase) const
{
    if (size() != other.size())
        return false;

    size_t pos = 0;
    for (; pos < other.size(); ++pos)
    {
        if (!tt::is_sameas(at(pos), other[pos], checkcase))
            break;
    }
    return (pos == size());
}
