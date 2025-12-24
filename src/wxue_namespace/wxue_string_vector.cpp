/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxue::StringVector class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <cstddef>

#include <wx/file.h>      // wxFile - encapsulates a file handle
#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "wxue.h"

#include "wxue_string_vector.h"

namespace
{
    auto trim_left(std::string_view str, size_t start) -> size_t
    {
        auto begin = str.find_first_not_of(" \t\n\r\f\v", start);
        return (begin != std::string_view::npos) ? begin : start;
    }

    auto trim_right(std::string_view str, size_t start, size_t end) -> size_t
    {
        size_t temp_end = (end == std::string_view::npos) ? str.size() : end;
        while (temp_end > start && wxue::is_whitespace(str.at(temp_end - 1)))
        {
            --temp_end;
        }
        return temp_end;
    }

    // Find the first occurrence of any separator from the vector
    auto find_first_separator(std::string_view str, const std::vector<std::string_view>& separators,
                              size_t start_pos) -> std::pair<size_t, size_t>
    {
        size_t earliest_pos = std::string::npos;
        size_t sep_length = 0;

        for (const auto& separator: separators)
        {
            if (separator.empty())
            {
                continue;
            }

            auto pos = str.find(separator, start_pos);
            if (pos != std::string::npos &&
                (earliest_pos == std::string::npos || pos < earliest_pos))
            {
                earliest_pos = pos;
                sep_length = separator.size();
            }
        }

        return { earliest_pos, sep_length };
    }
}  // namespace

void wxue::StringVector::SetString(std::string_view str, std::string_view separator, TRIM trim)
{
    clear();
    if (trim == TRIM::both || trim == TRIM::left)
    {
        str.remove_prefix(trim_left(str, 0));
    }
    if (str.empty())
    {
        return;
    }

    // If there is a string with no separator, then add the string and return
    if (str.find_first_of(separator, 0) == std::string::npos)
    {
        // No separator found, so just add the entire string
        if (trim == TRIM::both || trim == TRIM::right)
        {
            str.remove_suffix(str.size() - trim_right(str, 0, str.size()));
        }
        if (!str.empty())
        {
            emplace_back(str);
        }
        return;
    }

    for (size_t start = 0; start < str.size();)
    {
        // Find the next separator
        auto separator_pos = str.find_first_of(separator, start);

        // Determine the end position for this segment
        size_t segment_end = (separator_pos != std::string::npos) ? separator_pos : str.size();

        // Apply left trimming if needed
        size_t segment_start = start;
        if (trim == TRIM::both || trim == TRIM::left)
        {
            segment_start = trim_left(str, start);
        }

        // Apply right trimming if needed
        size_t trimmed_end = segment_end;
        if (trim == TRIM::both || trim == TRIM::right)
        {
            trimmed_end = trim_right(str, segment_start, segment_end);
        }

        // Add the segment to the vector (empty string_view if no text)
        if (trimmed_end > segment_start)
        {
            emplace_back(str.substr(segment_start, trimmed_end - segment_start));
        }
        else
        {
            emplace_back(std::string_view(""));
        }

        // If no more separators found, we're done
        if (separator_pos == std::string::npos)
        {
            break;
        }

        // Move past the separator for next iteration
        start = separator_pos + separator.size();
    }
}

void wxue::StringVector::SetString(std::string_view str,
                                   const std::vector<std::string_view>& separators, TRIM trim)
{
    clear();
    if (trim == TRIM::both || trim == TRIM::left)
    {
        str.remove_prefix(trim_left(str, 0));
    }
    if (str.empty())
    {
        return;
    }

    // If there is a string with no separator, then add the string and return
    if (auto [separator_pos, separator_length] = find_first_separator(str, separators, 0);
        separator_pos == std::string::npos)
    {
        // No separator found, so just add the entire string
        if (trim == TRIM::both || trim == TRIM::right)
        {
            str.remove_suffix(str.size() - trim_right(str, 0, str.size()));
        }
        if (!str.empty())
        {
            emplace_back(str);
        }
        return;
    }

    for (size_t start = 0; start < str.size();)
    {
        // Find the next separator
        auto [separator_pos, separator_length] = find_first_separator(str, separators, start);

        // Determine the end position for this segment
        size_t segment_end = (separator_pos != std::string::npos) ? separator_pos : str.size();

        // Apply left trimming if needed
        size_t segment_start = start;
        if (trim == TRIM::both || trim == TRIM::left)
        {
            segment_start = trim_left(str, start);
        }

        // Apply right trimming if needed
        size_t trimmed_end = segment_end;
        if (trim == TRIM::both || trim == TRIM::right)
        {
            trimmed_end = trim_right(str, segment_start, segment_end);
        }

        // Add the segment to the vector (empty string_view if no text)
        if (trimmed_end > segment_start)
        {
            emplace_back(str.substr(segment_start, trimmed_end - segment_start));
        }
        else
        {
            emplace_back(std::string_view(""));
        }

        // If no more separators found, we're done
        if (separator_pos == std::string::npos)
        {
            break;
        }

        // Move past the separator for next iteration
        start = separator_pos + separator_length;
    }
}

namespace
{
    constexpr char BOM_UTF8_0 = static_cast<char>(0xEF);
    constexpr char BOM_UTF8_1 = static_cast<char>(0xBB);
    constexpr char BOM_UTF8_2 = static_cast<char>(0xBF);

    constexpr uintmax_t MAX_FILE_SIZE = static_cast<const uintmax_t>(100 * 1024 * 1024);

}  // namespace

auto wxue::StringVector::ReadFile(std::string_view filename) -> bool
{
    m_filename = wxString::FromUTF8(filename.data(), filename.size());

    clear();  // clear any existing entries in the vector

    wxFile file(m_filename);
    if (!file.IsOpened())
    {
        return false;
    }

    auto file_size = wxFileName::GetSize(m_filename);
    ASSERT_MSG(file_size < MAX_FILE_SIZE, "File size is unreasonably large");
    if (file_size >= MAX_FILE_SIZE)
    {
        return false;
    }
    m_buffer.resize(file_size.GetValue());
    file.Read(m_buffer.data(), m_buffer.size());

    std::vector<std::string_view> lineSeparators = { "\r\n", "\r", "\n" };
    std::string_view string_buffer(m_buffer);

    if (m_buffer.size() > 2 && m_buffer[0] == BOM_UTF8_0 && m_buffer[1] == BOM_UTF8_1 &&
        m_buffer[2] == BOM_UTF8_2)
    {
        // BOM utf-8 string, so skip over the BOM and process normally
        string_buffer.remove_prefix(3);
    }
    SetString(string_buffer, lineSeparators);
    m_buffer.clear();
    m_buffer.shrink_to_fit();

    return true;
}

void wxue::StringVector::ReadString(std::string_view str)
{
    if (!str.empty())
    {
        m_buffer.assign(str);
        std::vector<std::string_view> lineSeparators = { "\r\n", "\r", "\n" };
        SetString(m_buffer, lineSeparators);

        // WARNING! str probably points to m_buffer, so it will be invalid after the clear().
        m_buffer.clear();
        m_buffer.shrink_to_fit();
    }
}

auto wxue::StringVector::is_sameas(const wxue::StringVector& other) const -> bool
{
    if (size() != other.size())
    {
        return false;
    }

    for (size_t pos = 0; pos < other.size(); ++pos)
    {
        if (at(pos) != other[pos])
        {
            return false;
        }
    }
    return true;
}

auto wxue::StringVector::WriteFile(const wxString& filename) const -> bool
{
    wxFile file(filename, wxFile::write);
    if (!file.IsOpened())
    {
        return false;
    }

    for (const auto& iter: *this)
    {
        wxString line = wxString::FromUTF8(iter) + "\n";
        if (!file.Write(line))
        {
            return false;
        }
    }

    return true;
}
