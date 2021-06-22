/////////////////////////////////////////////////////////////////////////////
// Name:      ttcstr.h
// Purpose:   std::string with additional methods
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <ttcstr.h> are available only with C++17 or later."
#endif

/// @file
///
/// The ttlib::cstr class handles zero-terminated char strings. It inherits from std::string and can be used in
/// most places where std::string<char> is used. It provides additional functionality including utf8/16
/// conversions, file name handling, etc.

#include <string>
#include <string_view>

#include "ttcview.h"

namespace tt
{
    enum REPLACE : bool
    {
        once = false,
        all = true,
    };
}  // namespace tt

namespace ttlib
{
    /// @brief basic_string with additional methods.
    class cstr : public std::basic_string<char, std::char_traits<char>, std::allocator<char>>
    {
        using bs = std::basic_string<char, std::char_traits<char>, std::allocator<char>>;

    public:
        cstr(void) : bs() {}
        cstr(const char* psz) : bs(psz) {}
        cstr(std::string_view view) : bs(view) {}
        cstr(ttlib::cview view) : bs(view) {}
        cstr(const cstr& str) : bs(str) {}
        cstr(const std::string& str) : bs(str) {}

        cstr(const std::filesystem::directory_entry& dir) : bs(dir.path().string(), dir.path().string().size()) {}

#if !defined(_WX_DEFS_H_) || !defined(_WIN32)
        cstr(std::wstring_view view) : bs(std::move(utf16to8(view))) {}
#else
        cstr(const wxString& str);
#endif

        cstr& from_utf16(std::wstring_view str)
        {
            clear();
            utf16to8(str, *this);
            return *this;
        }
        std::wstring to_utf16() const;

        cstr& utf(std::wstring_view str)
        {
            clear();
            utf16to8(str, *this);
            return *this;
        }

        cstr& utf(std::string_view str)
        {
            *this = str;
            return *this;
        }

#if defined(_WIN32)
        /// Returns a copy of the string converted to UTF16 on Windows, or a normal copy on other platforms
        std::wstring wx_str() const { return to_utf16(); };
#else
        /// Returns a copy of the string converted to UTF16 on Windows, or a normal copy on other platforms
        std::string wx_str() const { return substr(); }
#endif  // _WIN32

        /// Caution: ttlib::cview will be invalid if ttlib::cstr is modified or destroyed.
        ttlib::cview subview(size_t start = 0) const
        {
            if (ttlib::is_error(start))
                start = length();
            assert(start <= length());
            return ttlib::cview(c_str() + start, length() - start);
        }

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
        size_t find_oneof(const char* pszSet) const;

        /// Find any one of the characters in a set. Returns offset if found, npos if not.
        ///
        /// This is equivalent to calling std::strpbrk but returns an offset instead of a pointer.
        size_t find_oneof(cview set, size_t start) const;

        /// Returns offset to the next whitespace character starting with pos. Returns npos if
        /// there are no more whitespaces.
        ///
        /// A whitespace character is a space, tab, eol or form feed character.
        size_t find_space(size_t start = 0) const;
        ttlib::cview view_space(size_t start = 0) const { return subview(find_space(start)); }

        /// Returns offset to the next non-whitespace character starting with pos. Returns npos
        /// if there are no more non-whitespace characters.
        ///
        /// A whitespace character is a space, tab, eol or form feed character.
        size_t find_nonspace(size_t start = 0) const;
        ttlib::cview view_nonspace(size_t start = 0) const { return subview(find_nonspace(start)); }

        /// Returns an offset to the next word -- i.e., find the first non-whitedspace character
        /// after the next whitespace character.
        ///
        /// Equivalent to find_nonspace(find_space(start)).
        size_t stepover(size_t start = 0) const;
        ttlib::cview view_stepover(size_t start = 0) const { return subview(stepover(start)); }

        /// Returns true if the sub-string is identical to the first part of the main string
        bool is_sameas(std::string_view str, tt::CASE checkcase = tt::CASE::exact) const;

        /// Returns true if the sub-string is identical to the first part of the main string
        bool is_sameprefix(std::string_view str, tt::CASE checkcase = tt::CASE::exact) const;

        int atoi() const { return ttlib::atoi(*this); }

        /// If character is found, line is truncated from the character on, and then
        /// any trailing space is removed;
        void erase_from(char ch);

        /// If string is found, line is truncated from the string on, and then
        /// any trailing space is removed;
        void erase_from(std::string_view sub);

        /// Removes whitespace: ' ', \t, \r, \\n, \f
        ///
        /// where: TRIM::right, TRIM::left, or TRIM::both
        cstr& trim(tt::TRIM where = tt::TRIM::right);

        /// Remove locale-dependent whitespace from right side
        inline void RightTrim()
        {
            erase(std::find_if(rbegin(), rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), end());
        }

        /// Remove locale-dependent whitespace from left side
        inline void LeftTrim()
        {
            erase(begin(), std::find_if(begin(), end(), [](unsigned char ch) { return !std::isspace(ch); }));
        }

        /// Remove locale-dependent whitespace from left and right side
        inline void BothTrim()
        {
            LeftTrim();
            RightTrim();
        }

        /// Returns a view of the characters between chBegin and chEnd. This is typically used
        /// to view the contents of a quoted string. Returns the position of the ending
        ///  character in src.
        ///
        /// Unless chBegin is a whitespace character, all whitespace characters starting with
        /// offset will be ignored.
        std::string_view view_substr(size_t offset, char chBegin = '"', char chEnd = '"');

        /// Assigns the string between chBegin and chEnd. This is typically used to copy the
        /// contents of a quoted string. Returns the position of the ending character in src.
        ///
        /// Unless chBegin is a whitespace character, all whitespace characters starting with
        /// offset will be ignored.
        size_t AssignSubString(std::string_view src, char chBegin = '"', char chEnd = '"');

        /// Extracts a string from another string using start and end characters deduced from
        /// the first non-whitespace character after offset. Supports double and single quotes,
        /// angle and square brackets, and parenthesis.
        ///
        /// The return position is to the character in src that ended the string, or **npos** if no
        /// ending character was found.
        size_t ExtractSubString(std::string_view src, size_t offset = 0);

        /// Replace first (or all) occurrences of substring with another one
        size_t Replace(std::string_view oldtext, std::string_view newtext, bool replace_all = tt::REPLACE::once,
                       tt::CASE checkcase = tt::CASE::exact);

        /// Replace everything from pos to the end of the current string with str
        cstr& replace_all(size_t pos, std::string_view str)
        {
            replace(pos, length() - pos, str);
            return *this;
        }

        /// Generates hash of current string using djb2 hash algorithm
        size_t get_hash() const noexcept;

        /// Convert the entire string to lower case. Assumes the string is UTF8.
        cstr& MakeLower();

        /// Convert the entire string to upper case. Assumes the string is UTF8.
        cstr& MakeUpper();

        /// Assign the specified environment variable, returning true if found.
        ///
        /// Current string is replaced if found, cleared if not.
        bool assignEnvVar(ttlib::cview env_var);

        /// Similer to sprintf, but without floating point support.
        ///
        /// %v expects a std::string_view argument.
        ///
        /// %k flag will place a string argument in quotes, and format a numerical argument
        /// with commas or periods (depending on the current locale).
        ///
        /// %z is considered unsigned unless the value is -1.
        cstr& cdecl Format(std::string_view format, ...);

        /// Caution: view is only valid until cstr is modified or destroyed!
        std::string_view subview(size_t start, size_t len) const;

        /// Converts all backslashes in the string to forward slashes.
        ///
        /// Note: Windows works just fine using forward slashes instead of backslashes.
        cstr& backslashestoforward();

        /// Add a trailing forward slash (default is only if there isn't one already)
        void addtrailingslash(bool always = false)
        {
            if (always || back() != '/')
                push_back('/');
        }

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

        /// Returns offset to the current filename or tt::npos if there is no filename.
        size_t find_filename() const noexcept;

        /// Replaces any existing extension with a new extension, or appends the extension if the
        /// name doesn't currently have an extension.
        ///
        /// Returns view to the entire string.
        cstr& replace_extension(std::string_view newExtension);

        /// Replaces the extension portion of the string. Returns a view to the entire string.
        cstr& remove_extension() { return replace_extension(std::string_view {}); };

        /// Replaces the filename portion of the string. Returns a view to the entire string.
        cstr& replace_filename(std::string_view newFilename);
        cstr& replace_filename(std::wstring_view newFilename) { return replace_filename(utf16to8(newFilename)); }

        /// Removes the filename portion of the string. Returns a view to the entire string.
        cstr& remove_filename() { return replace_filename(""); }

        /// Appends the filename -- assumes current string is a path. This will add a trailing
        /// slash (if needed) before adding the filename.
        cstr& append_filename(std::string_view filename);
        cstr& append_filename(std::wstring_view filename) { return append_filename(utf16to8(filename)); }

        /// Makes the current path relative to the supplied path. Use "." to be relative to the current
        /// directory. Supplied path should not contain a filename.
        ///
        /// Unlike fs::relative(), this will not resolve symbolic links, allowing it to work
        /// even when you are using a directory with a symbolic link to a different drive.
        cstr& make_relative(ttlib::cview relative_to);

        /// Changes any current path to an absolute path.
        cstr& make_absolute();

        /// Replaces current string with the full path to the current working directory.
        cstr& assignCwd();

        /// Returns true if the current string refers to an existing file.
        bool file_exists() const;

        /// Returns true if the current string refers to an existing directory.
        bool dir_exists() const;

        cstr& operator<<(std::string_view str)
        {
            *this += str;
            return *this;
        }

        cstr& operator<<(std::wstring_view str)
        {
            utf16to8(str, *this);
            return *this;
        }

        cstr& operator<<(char ch)
        {
            *this += ch;
            return *this;
        }

        cstr& operator<<(int i)
        {
            *this += itoa(i);
            return *this;
        }

        cstr& operator<<(size_t i)
        {
            *this += itoa(i);
            return *this;
        }

        cstr& assign_wx(const wxString& str);
        cstr& append_wx(const wxString& str);
    };  // end cstr class

}  // namespace ttlib
