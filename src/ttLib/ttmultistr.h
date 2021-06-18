/////////////////////////////////////////////////////////////////////////////
// Name:      ttmultistr.h
// Purpose:   Breaks a single string into multiple strings
// Author:    Ralph Walden
// Copyright: Copyright (c) 2018-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <ttmultistr.h> are available only with C++17 or later."
#endif

#include "ttcstr.h"

/// @file
/// These two classes break a string containing substrings into a vector of substrings. Use ttlib::multistr if you want
/// a copy of the substring that can be modified. Use ttlib::multiview if you just need to iterate through the
/// substrings.
///
/// An example usage is getting the PATH$ envionment variable which contains multiple paths separated by a semicolon.
/// Handing the PATH$ string to either of these classes would give you a vector of each individual path.

namespace ttlib
{
    class multistr : public std::vector<ttlib::cstr>
    {
    public:
        multistr() {}

        multistr(std::string_view str, char separator = ';') { SetString(str, separator); }

        // Use this when a character sequence (such as "/r/n") separates the substrings
        multistr(std::string_view str, std::string_view separator) { SetString(str, separator); }

        // Clears the current vector of parsed strings and creates a new vector
        void SetString(std::string_view str, char separator = ';');
        void SetString(std::string_view str, std::string_view separator);
    };

    class multiview : public std::vector<std::string_view>
    {
    public:
        // Similar to multistr, only the vector consists of views into the original string
        multiview() {}

        multiview(std::string_view str, char separator = ';') { SetString(str, separator); }

        // Use this when a character sequence (such as "/r/n") separates the substrings
        multiview(std::string_view str, std::string_view separator) { SetString(str, separator); }

        // Clears the current vector of parsed strings and creates a new vector
        void SetString(std::string_view str, char separator = ';');
        void SetString(std::string_view str, std::string_view separator);
    };
}  // namespace ttlib
