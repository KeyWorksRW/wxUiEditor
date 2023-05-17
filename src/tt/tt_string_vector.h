/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class for reading and writing line-oriented strings/files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <tt_string_vector.h> are available only with C++17 or later."
#endif

#include <vector>

#include "tt_string.h"

class tt_view_vector;

// Use for multi-line strings or line-oriented files
class tt_string_vector : public std::vector<tt_string>
{
public:
    tt_string_vector() {}

    // Use this constructor to break apart a single string into a vector of strings
    tt_string_vector(std::string_view str, char separator = ';', tt::TRIM trim = tt::TRIM::none)
    {
        SetString(str, separator, trim);
    }

    // Use this when a character sequence (such as "/r/n") separates the substrings
    tt_string_vector(std::string_view str, std::string_view separator, tt::TRIM trim = tt::TRIM::none)
    {
        SetString(str, separator, trim);
    }

    // Reads a line-oriented file and converts each line into a tt_string (std::string).
    bool ReadFile(std::string_view filename);

    // This will be the filename passed to ReadFile()
    tt_string& filename() { return m_filename; }

    // Call this if ReadFile() was not used and you need to store a filename.
    void set_filename(std::string_view filename) { m_filename = filename; }

    // Reads a string as if it was a file (see ReadFile). This will append to any
    // existing content.
    void ReadString(std::string_view str);

    // Iterate through a list adding each iteration as a line.
    template <class iterT>
    void Read(const iterT iter)
    {
        for (const auto& line: iter)
        {
            emplace_back(line);
        }
    };

    // Reads an array of char* strings. The last member of the array MUST be a null
    // pointer.
    void ReadArray(const char** begin);

    // Reads count items from an array of char* strings.
    void ReadArray(const char** begin, size_t count);

    // Clears the current vector of parsed strings and creates a new vector
    // If the separator is a quote, then assume each substring is contained within quotes.
    void SetString(std::string_view str, char separator = ';', tt::TRIM trim = tt::TRIM::none);

    // Clears the current vector of parsed strings and creates a new vector
    void SetString(std::string_view str, std::string_view separator, tt::TRIM trim = tt::TRIM::none);

    // Writes each line to the file adding a '\n' to the end of the line.
    bool WriteFile(const std::string& filename) const;

    // Writes to the same file that was previously read
    bool WriteFile() const { return !m_filename.empty() ? WriteFile(m_filename) : false; }

    // Searches every line to see if it contains the sub-string.
    //
    // startline is the zero-based offset to the line to start searching.
    size_t FindLineContaining(std::string_view str, size_t startline = 0, tt::CASE checkcase = tt::CASE::exact) const;

    // Finds the position of the first string identical to the specified string.
    size_t find(std::string_view str, tt::CASE checkcase = tt::CASE::exact) const { return find(0, str, checkcase); }

    // Finds the position of the first string identical to the specified string.
    size_t find(size_t start, std::string_view str, tt::CASE checkcase = tt::CASE::exact) const;

    // Finds the position of the first string with specified prefix.
    size_t findprefix(std::string_view prefix, tt::CASE checkcase = tt::CASE::exact) const
    {
        return findprefix(0, prefix, checkcase);
    }

    // Finds the position of the first string with specified prefix.
    size_t findprefix(size_t start, std::string_view prefix, tt::CASE checkcase = tt::CASE::exact) const;

    // If a line is found that contains orgStr, it will be replaced by newStr and the
    // line position is returned. If no line is found, tt::npos is returned.
    size_t ReplaceInLine(std::string_view orgStr, std::string_view newStr, size_t startline = 0,
                         tt::CASE checkcase = tt::CASE::exact);

    bool is_sameas(const tt_string_vector& other, tt::CASE checkcase = tt::CASE::exact) const;
    bool is_sameas(const tt_view_vector& other, tt::CASE checkcase = tt::CASE::exact) const;

    // Use addEmptyLine() if you need to modify the line after adding it to the end.
    //
    // Use emplace_back(str) if you need to add an existing string.
    tt_string& addEmptyLine() { return emplace_back(tt::emptystring); }

    tt_string& insertEmptyLine(size_t pos)
    {
        if (pos >= size())
            return emplace_back(tt::emptystring);
        emplace(begin() + pos, tt::emptystring);
        return at(pos);
    }

    template <typename T>
    tt_string& insertLine(size_t pos, const T& str)
    {
        if (pos >= size())
            return emplace_back(str);
        emplace(begin() + pos, str);
        return at(pos);
    }

    // Only adds the string if it doesn't already exist.
    template <typename T>
    tt_string& append(T str, tt::CASE checkcase = tt::CASE::exact)
    {
        if (auto index = find(0, str, checkcase); tt::is_found(index))
        {
            return at(index);
        }
        return emplace_back(str);
    }

    // Only adds the filename if it doesn't already exist. On Windows, the case of the
    // filename is ignored when checking to see if the filename already exists.
    tt_string& addfilename(std::string_view filename)
    {
#if defined(_WIN32)
        return append(filename, tt::CASE::either);
#else
        return append(filename, tt::CASE::exact);
#endif  // _WIN32
    }

    void RemoveLine(size_t line)
    {
        assert(line < size());
        if (line < size())
            erase(begin() + line);
    }

    void RemoveLastLine()
    {
        if (size())
            erase(begin() + (size() - 1));
    }

    template <typename T>
    void operator+=(T str)
    {
        emplace_back(str);
    }

protected:
    // Converts lines into a vector of tt_string members. Lines can end with \n, \r, or \r\n.
    void ParseLines(std::string_view str);

private:
    tt_string m_filename;
};
