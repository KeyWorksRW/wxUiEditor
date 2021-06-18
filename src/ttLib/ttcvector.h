/////////////////////////////////////////////////////////////////////////////
// Name:      ttcvector.h
// Purpose:   Vector of ttlib::cstr strings
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <ttcvector.h> are available only with C++17 or later."
#endif

/// @file
///
/// The ttlib::cstrVector class stores ttlib::cstr (zero-terminated char containter class) strings. It inherits
/// from std::vector, providing all of the functionality of std::vector along with some functionality specific to
/// string handling. It can be used in most places where std::string<char> is used.

#include <vector>

#include "ttcstr.h"

namespace ttlib
{
    /// Contains a vector of cstr classes
    class cstrVector : public std::vector<ttlib::cstr>
    {
    public:
        /// Same as find(pos, ch) but with a boolean result
        bool bfind(size_type pos, char ch) const { return (at(pos).find(ch) != tt::npos); }

        /// Same as find(pos, str) but with a boolean result
        bool bfind(size_type pos, std::string_view str) const { return (at(pos).find(str) != tt::npos); }

        template<typename T>
        /// Only adds the string if it doesn't already exist.
        ttlib::cstr& append(T str, tt::CASE checkcase = tt::CASE::exact)
        {
            if (auto index = find(0, str, checkcase); !ttlib::is_error(index))
            {
                return at(index);
            }
            return emplace_back(str);
        }

        /// Only adds the filename if it doesn't already exist. On Windows, the case of the
        /// filename is ignored when checking to see if the filename already exists.
        ttlib::cstr& addfilename(std::string_view filename)
        {
#if defined(_WIN32)
                return append(filename, tt::CASE::either);
#else
                return append(filename, tt::CASE::exact);
#endif  // _WIN32
        }

        bool has_filename(std::string_view filename) const
        {
#if defined(_WIN32)
            return (find(0, filename, tt::CASE::either) != tt::npos);
#else
            return (find(0, filename, tt::CASE::exact) != tt::npos);
#endif  // _WIN32
        }

        /// Finds the position of the first string identical to the specified string.
        size_t find(std::string_view str, tt::CASE checkcase = tt::CASE::exact) const
        {
            return find(0, str, checkcase);
        }

        /// Finds the position of the first string identical to the specified string.
        size_t find(size_t start, std::string_view str, tt::CASE checkcase = tt::CASE::exact) const;

        /// Finds the position of the first string with specified prefix.
        size_t findprefix(std::string_view prefix, tt::CASE checkcase = tt::CASE::exact) const
        {
            return findprefix(0, prefix, checkcase);
        }

        /// Finds the position of the first string with specified prefix.
        size_t findprefix(size_t start, std::string_view prefix, tt::CASE checkcase = tt::CASE::exact) const;

        /// Finds the position of the first string containing the specified sub-string.
        size_t contains(std::string_view substring, tt::CASE checkcase = tt::CASE::exact) const
        {
            return contains(0, substring, checkcase);
        }

        /// Finds the position of the first string containing the specified sub-string.
        size_t contains(size_t start, std::string_view substring, tt::CASE checkcase = tt::CASE::exact) const;

        template<typename T>
        /// Unlike append(), this will add the string even if it already exists.
        void operator+=(T str) { emplace_back(str); }
    };

}  // namespace ttlib
