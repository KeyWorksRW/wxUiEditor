/////////////////////////////////////////////////////////////////////////////
// Purpose:   ttwx::StringVector class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

// This class can be used to separate a string into multiple wxString entries based on a separator,
// or it can can be used to read a line-oriented file into memory and then create a vector of
// wxStrings, one for each line. Since it derives from std::vector, you can use standard vector
// methods to iterate through the entries, std::find_if, std::ranges::find_if, etc.

// There already exists a wxTextFile class, however it is limited to files only, and because it is
// not derived from std::vector, you cannot use standard vector methods such as interating through
// the entries, std::find_if, std::ranges::find_if, etc.

// Note that unless you need to add, remove, or modify entries, you should consider using
// ttwx::ViewVector instead, which does not duplicate the strings, but instead creates a vector of
// std::string_view entries that reference the original string. This makes ttwx::ViewVector much
// more memory efficient, and faster to create.

#if !wxUSE_UNICODE_UTF8 || !wxUSE_UTF8_LOCALE_ONLY
    #error "This code requires both wxUSE_UNICODE_UTF8 and wxUSE_UTF8_LOCALE_ONLY to be enabled."
#endif

#ifndef __has_include
    #error \
        "Compiler does not support __has_include. Please use a compiler that supports C++17 or later."
#endif

#if !__has_include(<string_view>)
    #error "<string_view> header not found. Please use a compiler that supports C++17 or later."
#endif

#include <string>
#include <string_view>
#include <vector>

#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/string.h>    // wxString class

#ifndef TTWX_H_INCLUDED
    #include "ttwx.h"  // needed for ttwx::TRIM and ttwx::get_view()
#endif

namespace ttwx
{
    class StringVector : public std::vector<std::string>
    {
    public:
        StringVector() = default;

        StringVector(std::string_view str, std::string_view separator = ";", TRIM trim = TRIM::none)
        {
            SetString(str, separator, trim);
        }
        StringVector(std::string_view str, char separator = ';', TRIM trim = TRIM::none)
        {
            SetString(str, separator, trim);
        }

        // Clears the current vector of parsed strings and creates a new vector
        // Use this when a character sequence (such as "||") separates the substrings
        void SetString(std::string_view str, std::string_view separator = ";",
                       TRIM trim = TRIM::none);

        void SetString(std::string_view str, char separator = ';', TRIM trim = TRIM::none)
        {
            std::string temp(1, separator);
            SetString(str, temp, trim);
        }

        // Call this when any of multiple character sequences can separate the substrings,
        // e.g. std::vector[] = { ";", ","}  or std::vector[] = { "\r\n", "\r", "\n" }
        void SetString(std::string_view str, const std::vector<std::string_view>& separators,
                       TRIM trim = TRIM::none);

        // wxString overloads
        void SetString(const wxString& str, char separator = ';', TRIM trim = TRIM::none)
        {
            std::string temp(1, separator);

            SetString(get_View(str), temp, trim);
        }
        void SetString(const wxString& str, std::string_view separator, TRIM trim = TRIM::none)
        {
            SetString(get_View(str), separator, trim);
        }

        // Reads a line-oriented file and creates a vector of string_views, one for each
        // line. If the file contains a BOM utf-8 header it will be skipped over and the file
        // processed normally. No other BOM types are supported.
        // File size must be less than 100 MB.
        auto ReadFile(std::string_view filename) -> bool;
        auto ReadFile(const wxString& filename) -> bool
        {
            m_filename = filename;
            return ReadFile(std::string_view(m_filename.ToStdString()));
        }
        auto ReadFile(const wxFileName& filename) -> bool
        {
            m_filename = filename.GetFullPath();
            return ReadFile(std::string_view(m_filename.ToStdString()));
        }

        // This will be the filename passed to ReadFile()
        [[nodiscard]] auto get_ReadFilename() const -> const wxString& { return m_filename; }

        // Reads a string as if it was a file (see ReadFile). This will duplicate the string,
        // so you can delete the original if needed after calling this method.
        void ReadString(std::string_view str);

        void ReadString(const wxString& str) { ReadString(get_View(str)); }

        [[nodiscard]] auto is_sameas(const ttwx::StringVector& other) const -> bool;

    private:
        // This will be the filename passed to ReadFile()
        wxString m_filename;

        std::string m_buffer;
    };
}  // namespace ttwx
