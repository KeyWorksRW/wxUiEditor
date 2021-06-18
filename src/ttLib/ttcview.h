/////////////////////////////////////////////////////////////////////////////
// Name:      ttcview.h
// Purpose:   string_view functionality on a zero-terminated char string.
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/// @file
/// Provides a view of a zero-terminated char string. Includes a c_str() function and a const char*() operator
/// to make it easier to pass to functions expecting a C-style string.
///
/// Unlike std::string_view, there is no remove_suffix() function since you cannot change the length of the buffer
/// (it would no longer be zero-terminated.). There is a subview() function which returns a cview, but you can
/// only specify the starting position, not the length. You can use substr() to get a non-zero terminated
/// std::string_view.
///
/// Caution: as with std::string_view, the view is only valid as long as the string you are viewing has not
/// been modified or destroyed. This is also true of substr() and subview().

#pragma once

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <ttcview.h> are available only with C++17 or later."
#endif

#include "ttlibspace.h"  // ttlib namespace functions and declarations

// This can be used to conditionalize code where <ttcview.h> is available or not
#define _TTLIB_CVIEW_AVAILABLE_

#include <filesystem>
#include <sstream>
#include <string_view>

namespace ttlib
{
    class cview : public std::basic_string_view<char, std::char_traits<char>>
    {
        using bsv = std::basic_string_view<char, std::char_traits<char>>;

    public:
        cview(const std::string& str) : bsv(str.c_str(), str.length()) {}
        cview(const char* str, size_t len) : bsv(str, len) {}
        cview(const char* str) : bsv(str) {}

        // A string view is not guarenteed to be zero-terminated, so you can't construct from it
        cview(std::string_view str) = delete;

        /// cview is zero-terminated, so c_str() can be used wherever std::string.c_str()
        /// would be used.
        constexpr const char* c_str() const noexcept { return data(); };

        /// Can be used to pass the view to a function that expects a C-style string.
        operator const char*() const noexcept { return data(); }

#if defined(_WIN32)
        /// Returns a copy of the string converted to UTF16 on Windows, or a normal copy on other platforms
        std::wstring wx_str() const { return to_utf16(); };
#else
        /// Returns a copy of the string converted to UTF16 on Windows, or a normal copy on other platforms
        std::string wx_str() const { return std::string(*this); }
#endif  // _WIN32

        std::wstring to_utf16() const;

        /// Returns a zero-terminated view. Unlike substr(), you can only specify the starting position.
        cview subview(size_t start = 0) const
        {
            if (start > length())
                start = length();
            return cview(c_str() + start, length() - start);
        }

        /// Calling subview with a length parameter returns a standard string_view instead of
        /// a zero-terminated cview.
        std::string_view subview(size_t start, size_t len) const;

        /// Case-insensitive comparison.
        int comparei(std::string_view str) const;

        /// Locates the position of a substring.
        size_t locate(std::string_view str, size_t posStart = 0, tt::CASE check = tt::CASE::exact) const;

        /// Returns true if the sub string exists
        bool contains(std::string_view sub, tt::CASE checkcase = tt::CASE::exact) const { return (locate(sub, 0, checkcase) != npos); }

        /// Returns true if any string in the iteration list appears somewhere in the the main string.
        template<class iterT>
        bool strContains(iterT iter, tt::CASE checkcase = tt::CASE::exact)
        {
            for (auto& strIter: iter)
            {
                if (contains(strIter, checkcase))
                    return true;
            }
            return false;
        }

        /// Find any one of the characters in a set. Returns offset if found, npos if not.
        ///
        /// This is equivalent to calling std::strpbrk but returns an offset instead of a pointer.
        size_t find_oneof(const std::string& set) const;

        /// Find any one of the characters in a set. Returns offset if found, npos if not.
        ///
        /// This is equivalent to calling std::strpbrk but returns an offset instead of a pointer.
        size_t find_oneof(cview set, size_t start) const;

        /// Returns offset to the next whitespace character starting with pos. Returns npos if
        /// there are no more whitespaces.
        ///
        /// A whitespace character is a space, tab, eol or form feed character.
        size_t find_space(size_t start = 0) const;

        /// Returns offset to the next non-whitespace character starting with pos. Returns npos
        /// if there are no more non-whitespace characters.
        ///
        /// A whitespace character is a space, tab, eol or form feed character.
        size_t find_nonspace(size_t start = 0) const;

        /// Returns an offset to the next word -- i.e., find the first non-whitedspace character
        /// after the next whitespace character.
        ///
        /// Equivalent to find_nonspace(find_space(start)).
        size_t stepover(size_t start = 0) const;

        /// Returns true if the sub-string is identical to the first part of the main string
        bool is_sameas(std::string_view str, tt::CASE checkcase = tt::CASE::exact) const;

        /// Returns true if the sub-string is identical to the first part of the main string
        bool is_sameprefix(std::string_view str, tt::CASE checkcase = tt::CASE::exact) const;

        int atoi() const { return ttlib::atoi(*this); }

        // You can't remove a suffix and still have the view zero-terminated
        constexpr void remove_suffix(size_type n) = delete;

        /// Returns true if current filename contains the specified case-insensitive extension.
        bool has_extension(std::string_view ext) const { return ttlib::is_sameas(extension(), ext, tt::CASE::either); }

        /// Returns true if current filename contains the specified case-insensitive file name.
        bool has_filename(std::string_view name) const { return ttlib::is_sameas(filename(), name, tt::CASE::either); }

        /// Returns a view to the current extension. View is empty if there is no extension.
        ///
        /// Caution: view is only valid until cstr is modified or destroyed.
        ttlib::cview extension() const noexcept;

        /// Returns a view to the current filename. View is empty if there is no filename.
        ///
        /// Caution: view is only valid until cstr is modified or destroyed.
        ttlib::cview filename() const noexcept;

        /// Returns true if the current string refers to an existing file.
        bool file_exists() const;

        /// Returns true if the current string refers to an existing directory.
        bool dir_exists() const;

        /// Returns a view of the characters between chBegin and chEnd. This is typically used
        /// to view the contents of a quoted string.
        ///
        /// Unless chBegin is a whitespace character, all whitespace characters starting with
        /// offset will be ignored.
        std::string_view view_substr(size_t offset, char chBegin = '"', char chEnd = '"');

        // All of the following view_() functions will return an empty view if the specified character cannot be
        // found, or the start position is out of range (including start == npos).

        cview view_space(size_t start = 0) const { return subview(find_space(start)); }
        cview view_nonspace(size_t start = 0) const { return subview(find_nonspace(start)); }
        cview view_stepover(size_t start = 0) const { return subview(stepover(start)); }
        cview view_digit(size_t start = 0) const;
        cview view_nondigit(size_t start = 0) const;

        /// Generates hash of current string using djb2 hash algorithm
        size_t get_hash() const noexcept;

        /////////////////////////////////////////////////////////////////////////////////
        // Note: all moveto_() functions start from the beginning of the view. On success
        // they change the view and return true. On failure, the view remains unchanged.
        /////////////////////////////////////////////////////////////////////////////////

        /// Move start position to the next whitespace character
        bool moveto_space() noexcept;

        /// Move start position to the next non-whitespace character
        bool moveto_nonspace() noexcept;

        /// Move start position to the next word (views the next whitespace, then the next
        /// non-whitespace after that)
        bool moveto_nextword() noexcept;

        /// Move start position to the next numerical character
        bool moveto_digit() noexcept;

        /// Move start position to the next non-numerical character
        bool moveto_nondigit() noexcept;

        /// Move start position to the extension in the current path
        bool moveto_extension() noexcept;

        /// Move start position to the filename in the current path.
        ///
        /// A filename is any string after the last '/' (or '\' on Windows) in the current
        /// view.
        bool moveto_filename() noexcept;

        bool operator==(ttlib::cview str)
        {
            return this->is_sameas(str);
        }

    };
}  // namespace ttlib
