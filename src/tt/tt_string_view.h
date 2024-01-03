/////////////////////////////////////////////////////////////////////////////
// Purpose:   std::string_view with additional methods
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <tt_string_view.h> are available only with C++17 or later."
#endif

#include <string_view>

#include "tt.h"  // tt namespace functions and declarations

class tt_string_view : public std::basic_string_view<char, std::char_traits<char>>
{
    using bsv = std::basic_string_view<char, std::char_traits<char>>;

public:
    tt_string_view(const std::string& str) : bsv(str.c_str(), str.length()) {}
    tt_string_view(const char* str, size_t len) : bsv(str, len) {}
    tt_string_view(const char* str) : bsv(str) {}
    tt_string_view(std::string_view view) : bsv(view) {}

    // FromUTF8() is very efficient if wxUSE_UNICODE_UTF8 is defined as no UTF conversion is
    // done.
    wxString make_wxString() const { return wxString::FromUTF8(data(), size()); }

#if defined(_WIN32) && !(wxUSE_UNICODE_UTF8)
    /// Returns a copy of the string converted to UTF16 on Windows, or a normal copy on other platforms
    std::wstring wx_str() const { return to_utf16(); };
#else
    /// Returns a copy of the string converted to UTF16 on Windows, or a normal copy on other platforms
    std::string wx_str() const { return std::string(*this); }
#endif  // _WIN32

    // Used when caller refuses to accept tt_string_view as a std::string_view (e.g.
    // std::format())
    const std::string_view& ToStdView() const { return *this; }

    // Same result as wxString::ToStdString()
    const std::string ToStdString() const { return std::string(*this); }

    std::string as_str() const { return std::string(*this); }
    std::wstring as_utf16() const { return to_utf16(); };

    std::wstring to_utf16() const;

    /// Case-insensitive comparison.
    int comparei(std::string_view str) const;

    /// Locates the position of a substring.
    size_t locate(std::string_view str, size_t posStart = 0, tt::CASE check = tt::CASE::exact) const;

    /// Returns true if the sub string exists
    bool contains(std::string_view sub, tt::CASE checkcase = tt::CASE::exact) const
    {
        return (locate(sub, 0, checkcase) != npos);
    }

    /// Returns true if any string in the iteration list appears somewhere in the the main string.
    template <class iterT>
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
    size_t find_oneof(const std::string& set, size_t start = 0) const;

    /// Find any one of the characters in a set. Returns offset if found, npos if not.
    size_t find_oneof(tt_string_view set, size_t start = 0) const;

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

    int atoi(size_t start = 0) const { return tt::atoi(data() + start); }

    /// Returns true if current filename contains the specified case-insensitive extension.
    bool has_extension(std::string_view ext) const { return tt::is_sameas(extension(), ext, tt::CASE::either); }

    /// Returns true if current filename contains the specified case-insensitive file name.
    bool has_filename(std::string_view name) const { return tt::is_sameas(filename(), name, tt::CASE::either); }

    /// Returns a tt_string_view to the current extension. View is empty if there is no extension.
    ///
    /// Caution: tt_string_view is only valid until the string it points to is modified or
    /// destroyed.
    tt_string_view extension() const noexcept;

    /// Returns a tt_string_view to the current filename. View is empty if there is no filename.
    ///
    /// Caution: tt_string_view is only valid until the string it points to is modified or
    /// destroyed.
    tt_string_view filename() const noexcept;

    /// Returns true if the current string refers to an existing file.
    bool file_exists() const;

    /// Returns true if the current string refers to an existing directory.
    bool dir_exists() const;

    /// If string is found, line is truncated from the string on, and then
    /// any trailing space is removed.
    tt_string_view& erase_from(std::string_view sub, tt::CASE check = tt::CASE::exact);

    /// Removes whitespace: ' ', \t, \r, \\n, \f
    ///
    /// where: TRIM::right, TRIM::left, or TRIM::both
    tt_string_view& trim(tt::TRIM where = tt::TRIM::right);

    /// Unlike substr(), this will not throw an exception if start is out of range.
    tt_string_view subview(size_t start = 0) const
    {
        if (start > length())
            return tt::emptystring;
        else
            return tt_string_view(data() + start, length() - start);
    }

    tt_string_view subview(size_t start, size_t len) const;

    /// Returns a tt_string_view of the characters between chBegin and chEnd. This is typically used
    /// to tt_string_view the contents of a quoted string.
    ///
    /// Unless chBegin is a whitespace character, all whitespace characters starting with
    /// offset will be ignored.
    tt_string_view view_substr(size_t offset, char chBegin = '"', char chEnd = '"') const;

    // All of the following view_() functions will return an empty tt_string_view if the specified character cannot be
    // found, or the start position is out of range (including start == npos).

    tt_string_view view_space(size_t start = 0) const { return subview(find_space(start)); }
    tt_string_view view_nonspace(size_t start = 0) const { return subview(find_nonspace(start)); }
    tt_string_view view_stepover(size_t start = 0) const { return subview(stepover(start)); }
    tt_string_view view_digit(size_t start = 0) const;
    tt_string_view view_nondigit(size_t start = 0) const;

    /// Returns view to the next whitespace character. View is empty if there are no more
    /// whitespaces.
    static tt_string_view find_space(std::string_view str) noexcept;

    /// Returns view to the next non-whitespace character. View is empty if there are no
    /// non-whitespace characters.
    static tt_string_view find_nonspace(std::string_view str) noexcept;

    /// Equivalent to find_nonspace(find_space(str)).
    static tt_string_view stepover(std::string_view str) noexcept;

    /// Generates hash of current string using djb2 hash algorithm
    size_t get_hash() const noexcept;

    /////////////////////////////////////////////////////////////////////////////////
    // Note: all moveto_() functions start from the beginning of the tt_string_view. On success
    // they change the tt_string_view and return true. On failure, the tt_string_view remains unchanged.
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
    /// tt_string_view.
    bool moveto_filename() noexcept;

    /// Move start position to the substr in the current string, returning true if
    /// found.
    ///
    /// If StepOverIfFound is true, start position is set to the first non-whitespace
    /// character found after substr.
    bool moveto_substr(std::string_view substr, bool StepOverIfFound = false) noexcept;

    bool operator==(tt_string_view str) { return this->is_sameas(str); }
};
