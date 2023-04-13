/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class for reading and writing line-oriented strings/files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <tt_view_vector.h> are available only with C++17 or later."
#endif

#include <vector>

#include "tt_string_view.h"

class tt_string_vector;

class tt_view_vector : public std::vector<tt_string_view>
{
public:
    // Similar to multistr, only the vector consists of views into the original string
    tt_view_vector() {}

    tt_view_vector(std::string_view str, char separator = ';', tt::TRIM trim = tt::TRIM::none)
    {
        SetString(str, separator, trim);
    }

    // Use this when a character sequence (such as "/r/n") separates the substrings
    tt_view_vector(std::string_view str, std::string_view separator, tt::TRIM trim = tt::TRIM::none)
    {
        SetString(str, separator, trim);
    }

    // Clears the current vector of parsed strings and creates a new vector
    void SetString(std::string_view str, char separator = ';', tt::TRIM trim = tt::TRIM::none);
    void SetString(std::string_view str, std::string_view separator, tt::TRIM trim = tt::TRIM::none);

    // Reads a line-oriented file and converts each line into a std::string.
    bool ReadFile(std::string_view filename);

    // This will be the filename passed to ReadFile()
    tt_string& filename() { return m_filename; }

    // Call this if ReadFile() was not used and you need to store a filename.
    void set_filename(std::string_view filename) { m_filename = filename; }

    // Reads a string as if it was a file (see ReadFile).
    void ReadString(std::string_view str);

    // Writes each line to the file adding a '\n' to the end of the line.
    bool WriteFile(const std::string& filename) const;

    // Returns the string storing the entire file. If you change this string, all
    // the string_view vector entries will be invalid!
    tt_string& GetBuffer() { return m_buffer; }

    // Call this if you change the buffer returned by GetBuffer() to turn the buffer
    // into an array of string_views.
    void ParseBuffer();

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

    bool is_sameas(const tt_string_vector& other, tt::CASE checkcase = tt::CASE::exact) const;
    bool is_sameas(const tt_view_vector& other, tt::CASE checkcase = tt::CASE::exact) const;

protected:
    // Converts lines into a vector of std::string_view members. Lines can end with \n, \r, or \r\n.
    void ParseLines(std::string_view str);

private:
    tt_string m_buffer;
    tt_string m_filename;
};
