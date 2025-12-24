/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxue::string and wxue::string_view classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#if !wxUSE_UNICODE_UTF8
    #error "wxUSE_UNICODE_UTF8 must be enabled"
#endif

#include <string>
#include <string_view>

#include <wx/datetime.h>  // wxDateTime class
#include <wx/string.h>    // wxString class

#include "wxue.h"

namespace wxue
{
    // Forward declaration so string_view can reference string in deferred inline methods
    class string;

    //////////////////////////////////////////////////////////////////////////
    // string_view - std::string_view with additional methods
    //////////////////////////////////////////////////////////////////////////

    // AI Context: This file provides wxue::string and wxue::string_view as modern replacements for
    // legacy tt_string types. Both classes extend std::string/std::string_view with utility methods
    // for string manipulation (locate, contains, trim, find_space/nonspace, stepover),
    // case-insensitive comparisons, and file path operations (extension, filename,
    // replace_extension, make_relative). string_view includes moveto_* methods that modify the view
    // in-place for parsing. string adds mutation methods (erase_from, Replace, MakeLower/Upper) and
    // file system operations (file_exists, dir_exists, last_write_time). Requires
    // wxUSE_UNICODE_UTF8 for efficient wxString interop via wx() method. string_view methods are
    // delegated from string to avoid code duplication.

    class string_view : public std::basic_string_view<char, std::char_traits<char>>
    {
        using bsv = std::basic_string_view<char, std::char_traits<char>>;

    public:
        string_view(const std::string& str) : bsv(str.c_str(), str.length()) {}
        string_view(const char* str, size_t len) : bsv(str, len) {}
        string_view(const char* str) : bsv(str) {}
        string_view(std::string_view view) : bsv(view) {}

        // FromUTF8() is very efficient if wxUSE_UNICODE_UTF8 is defined as no UTF conversion is
        // done.
        [[nodiscard]] wxString wx() const { return wxString::FromUTF8(data(), size()); }

        // Returns a std::string copy of this view
        [[nodiscard]] std::string std_str() const { return std::string(data(), size()); }

        // Used when caller refuses to accept string_view as a std::string_view (e.g.
        // std::format())
        [[nodiscard]] const std::string_view& ToStdView() const { return *this; }

        // Same result as wxString::ToStdString() - returns a std::string copy
        [[nodiscard]] std::string ToStdString() const { return std::string(*this); }

        // Returns a wxue::string copy - defined after string class
        [[nodiscard]] inline string as_str() const;

        // Case-insensitive comparison.
        [[nodiscard]] int comparei(std::string_view str) const;

        // Locates the position of a substring.
        [[nodiscard]] size_t locate(std::string_view str, size_t posStart = 0,
                                    CASE check = CASE::exact) const;

        // Returns true if the sub string exists
        [[nodiscard]] bool contains(std::string_view sub, CASE checkcase = CASE::exact) const
        {
            return (locate(sub, 0, checkcase) != npos);
        }

        // Returns true if any string in the iteration list appears somewhere in the the main
        // string.
        template <class iterT>
        [[nodiscard]] bool strContains(iterT iter, CASE checkcase = CASE::exact)
        {
            for (auto& strIter: iter)
            {
                if (contains(strIter, checkcase))
                {
                    return true;
                }
            }
            return false;
        }

        // Find any one of the characters in a set. Returns offset if found, npos if not.
        [[nodiscard]] auto find_oneof(const std::string& set, size_t start = 0) const -> size_t;

        // Find any one of the characters in a set. Returns offset if found, npos if not.
        [[nodiscard]] auto find_oneof(string_view set, size_t start = 0) const -> size_t;

        // Returns offset to the next whitespace character starting with pos. Returns npos if
        // there are no more whitespaces.
        //
        // A whitespace character is a space, tab, eol or form feed character.
        [[nodiscard]] auto find_space(size_t start = 0) const -> size_t;

        // Returns offset to the next non-whitespace character starting with pos. Returns npos
        // if there are no more non-whitespace characters.
        //
        // A whitespace character is a space, tab, eol or form feed character.
        [[nodiscard]] auto find_nonspace(size_t start = 0) const -> size_t;

        // Returns an offset to the next word -- i.e., find the first non-whitespace character
        // after the next whitespace character.
        //
        // Equivalent to find_nonspace(find_space(start)).
        [[nodiscard]] auto stepover(size_t start = 0) const -> size_t;

        // Returns true if the sub-string is identical to the first part of the main string
        [[nodiscard]] auto is_sameas(std::string_view str, CASE checkcase = CASE::exact) const
            -> bool;

        // Returns true if the sub-string is identical to the first part of the main string
        [[nodiscard]] auto is_sameprefix(std::string_view str, CASE checkcase = CASE::exact) const
            -> bool;

        [[nodiscard]] int atoi(size_t start = 0) const { return wxue::atoi(data() + start); }

        // Returns true if current filename contains the specified case-insensitive extension.
        [[nodiscard]] auto has_extension(std::string_view ext) const -> bool
        {
            string_view cur_extension(data(), size());
            return cur_extension.has_extension(ext);
        }

        // Returns true if current filename contains the specified case-insensitive file name.
        [[nodiscard]] auto has_filename(std::string_view name) const -> bool
        {
            string_view cur_filename(data(), size());
            return cur_filename.has_filename(name);
        }

        // Returns a string_view to the current extension. View is empty if there is no
        // extension.
        //
        // Caution: string_view is only valid until the string it points to is modified or
        // destroyed.
        [[nodiscard]] auto extension() const noexcept -> string_view;

        // Returns a string_view to the current filename. View is empty if there is no filename.
        //
        // Caution: string_view is only valid until the string it points to is modified or
        // destroyed.
        [[nodiscard]] auto filename() const noexcept -> string_view;

        // Returns true if the current string refers to an existing file.
        [[nodiscard]] auto file_exists() const -> bool;

        // Returns true if the current string refers to an existing directory.
        [[nodiscard]] auto dir_exists() const -> bool;

        // If string is found, view is truncated from the string on, and then
        // any trailing space is removed.
        auto erase_from(std::string_view sub, CASE check = CASE::exact) -> string_view&;

        // Removes whitespace: ' ', \t, \r, \\n, \f
        //
        // where: TRIM::right, TRIM::left, or TRIM::both
        auto trim(TRIM where = TRIM::right) -> string_view&;

        // Unlike substr(), this will not throw an exception if start is out of range.
        [[nodiscard]] auto subview(size_t start = 0) const -> string_view
        {
            if (start > length())
            {
                return wxue::emptystring;
            }

            return { data() + start, length() - start };
        }

        [[nodiscard]] string_view subview(size_t start, size_t len) const;

        // Returns a string_view of the characters between chBegin and chEnd. This is typically
        // used to view the contents of a quoted string.
        //
        // Unless chBegin is a whitespace character, all whitespace characters starting with
        // offset will be ignored.
        [[nodiscard]] auto view_substr(size_t offset, char chBegin = '"', char chEnd = '"') const
            -> string_view;

        // All of the following view_() functions will return an empty string_view if the
        // specified character cannot be found, or the start position is out of range (including
        // start == npos).

        [[nodiscard]] auto view_space(size_t start = 0) const -> string_view
        {
            return subview(find_space(start));
        }
        [[nodiscard]] auto view_nonspace(size_t start = 0) const -> string_view
        {
            return subview(find_nonspace(start));
        }
        [[nodiscard]] auto view_stepover(size_t start = 0) const -> string_view
        {
            return subview(stepover(start));
        }
        [[nodiscard]] auto view_digit(size_t start = 0) const -> string_view;
        [[nodiscard]] auto view_nondigit(size_t start = 0) const -> string_view;

        // Returns view to the next whitespace character. View is empty if there are no more
        // whitespaces.
        [[nodiscard]] static auto find_space(std::string_view str) noexcept -> string_view;

        // Returns view to the next non-whitespace character. View is empty if there are no
        // non-whitespace characters.
        [[nodiscard]] static auto find_nonspace(std::string_view str) noexcept -> string_view;

        // Equivalent to find_nonspace(find_space(str)).
        [[nodiscard]] static auto stepover(std::string_view str) noexcept -> string_view;

        /////////////////////////////////////////////////////////////////////////////////
        // Note: all moveto_() functions start from the beginning of the string_view. On success
        // they change the string_view and return true. On failure, the string_view remains
        // unchanged.
        /////////////////////////////////////////////////////////////////////////////////

        // Move start position to the next whitespace character
        auto moveto_space() noexcept -> bool;

        // Move start position to the next non-whitespace character
        auto moveto_nonspace() noexcept -> bool;

        // Move start position to the next word (views the next whitespace, then the next
        // non-whitespace after that)
        auto moveto_nextword() noexcept -> bool;

        // Move start position to the next numerical character
        auto moveto_digit() noexcept -> bool;

        // Move start position to the next non-numerical character
        auto moveto_nondigit() noexcept -> bool;

        // Move start position to the extension in the current path
        auto moveto_extension() noexcept -> bool;

        // Move start position to the filename in the current path.
        //
        // A filename is any string after the last '/' (or '\' on Windows) in the current
        // string_view.
        auto moveto_filename() noexcept -> bool;

        // Move start position to the substr in the current string, returning true if
        // found.
        //
        // If StepOverIfFound is true, start position is set to the first non-whitespace
        // character found after substr.
        auto moveto_substr(std::string_view substr, bool StepOverIfFound = false) noexcept -> bool;

        auto operator==(string_view str) const -> bool { return this->is_sameas(str); }
    };

    //////////////////////////////////////////////////////////////////////////
    // string - std::string with additional methods
    //////////////////////////////////////////////////////////////////////////

    // std::string with additional methods.
    //
    // On Windows, std::wstring/wstring_view is assumed to be UTF16 and is automatically
    // converted to UTF8 in constructors and assignments.
    class string : public std::basic_string<char, std::char_traits<char>, std::allocator<char>>
    {
        using std_base = std::basic_string<char, std::char_traits<char>, std::allocator<char>>;

    public:
        string() { assign(wxue::emptystring); }
        string(const char* psz) { assign(psz); }
        string(const char* psz, size_t len) : std_base(psz, len) {}
        string(std::string_view view) : std_base(view) {}
        string(const string& str) : std_base(str) {}
        string(const std::string& str) : std_base(str) {}
        string(string&& str) noexcept : std_base(str) {}

        string(const wxString& str) { *this = str.ToStdString(); }

        ~string() = default;

        auto operator=(const string& str) -> string&
        {
            std_base::operator=(str);
            return *this;
        }

        string& operator=(string&& str) noexcept
        {
            std_base::operator=(str);
            return *this;
        }

        // FromUTF8() is very efficient if wxUSE_UNICODE_UTF8 is defined as no UTF conversion is
        // done.
        [[nodiscard]] wxString wx() const { return wxString::FromUTF8(data(), size()); }

        // Caution: string_view will be invalid if string is modified or destroyed.
        [[nodiscard]] string_view subview(size_t start = 0) const
        {
            if (start > length())
            {
                return wxue::emptystring;
            }
            return string_view(data() + start, length() - start);
        }

        // Caution: view is only valid until string is modified or destroyed!
        [[nodiscard]] auto subview(size_t start, size_t len) const -> string_view
        {
            if (start >= size())
            {
                return wxue::emptystring;
            }
            return { c_str() + start, std::min(len, size() - start) };
        }

        // Used when caller refuses to accept string as a std::string (e.g., std::format()).
        // Name is identical to wxString::ToStdString()
        [[nodiscard]] auto ToStdString() const -> const std::string& { return *this; }

        // Used when caller refuses to accept via subview as a std::string_view
        // (e.g. std::format())
        [[nodiscard]] auto ToStdView(size_t start = 0) const -> std::string_view
        {
            return subview(start);
        }

        // Case-insensitive comparison. Delegates to string_view.
        [[nodiscard]] auto comparei(std::string_view str) const -> int
        {
            return string_view(*this).comparei(str);
        }

        // Locates the position of a substring. Delegates to string_view.
        [[nodiscard]] size_t locate(std::string_view str, size_t posStart = 0,
                                    CASE check = CASE::exact) const
        {
            return string_view(*this).locate(str, posStart, check);
        }

        // Returns true if the sub string exists. Delegates to string_view.
        [[nodiscard]] auto contains(std::string_view sub, CASE checkcase = CASE::exact) const
            -> bool
        {
            return string_view(*this).contains(sub, checkcase);
        }

        // Returns true if any string in the iteration list appears somewhere in the the main
        // string.
        template <class iterT>
        [[nodiscard]] auto strContains(iterT iter, CASE checkcase = CASE::exact) -> bool
        {
            return string_view(*this).strContains(iter, checkcase);
        }

        // Find any one of the characters in a set. Returns offset if found, npos if not.
        // Delegates to string_view.
        [[nodiscard]] auto find_oneof(const char* pszSet) const -> size_t
        {
            return string_view(*this).find_oneof(string_view(pszSet));
        }

        // Find any one of the characters in a set. Returns offset if found, npos if not.
        // Delegates to string_view.
        [[nodiscard]] auto find_oneof(const char* set, size_t start) const -> size_t
        {
            return string_view(*this).find_oneof(string_view(set), start);
        }

        // Returns offset to the next whitespace character starting with pos. Returns npos if
        // there are no more whitespaces. Delegates to string_view.
        [[nodiscard]] auto find_space(size_t start = 0) const -> size_t
        {
            return string_view(*this).find_space(start);
        }

        // Returns offset to the next non-whitespace character starting with pos. Returns npos
        // if there are no more non-whitespace characters. Delegates to string_view.
        [[nodiscard]] auto find_nonspace(size_t start = 0) const -> size_t
        {
            return string_view(*this).find_nonspace(start);
        }

        // Returns an offset to the next word -- i.e., find the first non-whitespace character
        // after the next whitespace character. Delegates to string_view.
        [[nodiscard]] auto stepover(size_t start = 0) const -> size_t
        {
            return string_view(*this).stepover(start);
        }

        [[nodiscard]] auto view_space(size_t start = 0) const -> string_view
        {
            return subview(find_space(start));
        }
        [[nodiscard]] auto view_nonspace(size_t start = 0) const -> string_view
        {
            return subview(find_nonspace(start));
        }
        [[nodiscard]] auto view_stepover(size_t start = 0) const -> string_view
        {
            return subview(stepover(start));
        }

        // Returns true if the sub-string is identical to the first part of the main string.
        // Delegates to string_view.
        [[nodiscard]] auto is_sameas(std::string_view str, CASE checkcase = CASE::exact) const
            -> bool
        {
            return string_view(*this).is_sameas(str, checkcase);
        }

        // Returns true if the sub-string is identical to the first part of the main string.
        // Delegates to string_view.
        [[nodiscard]] auto is_sameprefix(std::string_view str, CASE checkcase = CASE::exact) const
            -> bool
        {
            return string_view(*this).is_sameprefix(str, checkcase);
        }

        [[nodiscard]] auto atoi(size_t start = 0) const -> int
        {
            return wxue::atoi(c_str() + start);
        }

        // If character is found, line is truncated from the character on, and then
        // any trailing space is removed.
        void erase_from(char chr);

        // If string is found, line is truncated from the string on, and then
        // any trailing space is removed.
        void erase_from(std::string_view sub);

        // Removes whitespace: ' ', \t, \r, \\n, \f
        //
        // where: TRIM::right, TRIM::left, or TRIM::both
        auto trim(TRIM where = TRIM::right) -> string&;

        // Remove locale-dependent whitespace from right side
        void RightTrim()
        {
            erase(std::ranges::find_if(*this | std::views::reverse,
                                       [](unsigned char character)
                                       {
                                           return !std::isspace(character);
                                       })
                      .base(),
                  end());
        }

        // Remove locale-dependent whitespace from left side
        void LeftTrim()
        {
            erase(begin(), std::ranges::find_if(*this,
                                                [](unsigned char character) -> bool
                                                {
                                                    return !std::isspace(character);
                                                }));
        }

        // Remove locale-dependent whitespace from left and right side
        void BothTrim()
        {
            LeftTrim();
            RightTrim();
        }

        // Returns a view of the characters between chBegin and chEnd. This is typically used
        // to view the contents of a quoted string. Returns the position of the ending
        //  character in src.
        //
        // Unless chBegin is a whitespace character, all whitespace characters starting with
        // offset will be ignored.
        [[nodiscard]] auto view_substr(size_t offset, char chBegin = '"', char chEnd = '"') const
            -> string_view
        {
            return string_view(*this).view_substr(offset, chBegin, chEnd);
        }

        // Assigns the string between chBegin and chEnd. This is typically used to copy the
        // contents of a quoted string. Returns the position of the ending character in src.
        //
        // Unless chBegin is a whitespace character, all whitespace characters starting with
        // offset will be ignored.
        [[nodiscard]] auto AssignSubString(std::string_view src, char chBegin = '"',
                                           char chEnd = '"') -> size_t;

        // Extracts a string from another string using start and end characters deduced from
        // the first non-whitespace character after offset. Supports double and single quotes,
        // angle and square brackets, and parenthesis.
        //
        // The return position is to the character in src that ended the string, or **npos** if no
        // ending character was found.
        [[nodiscard]] auto ExtractSubString(std::string_view src, size_t offset = 0) -> size_t;

        // Identical to ExtractSubString only it returns string& instead of a size_t
        auto CreateSubString(std::string_view src, size_t offset = 0) -> string&
        {
            (void) ExtractSubString(src, offset);
            return *this;
        }

        // Replace first (or all) occurrences of substring with another one
        [[nodiscard]] size_t Replace(std::string_view oldtext, std::string_view newtext,
                                     bool replace_all = REPLACE::once,
                                     CASE checkcase = CASE::exact);

        // Replace everything from pos to the end of the current string with str
        auto replace_all(size_t pos, std::string_view str) -> string&
        {
            replace(pos, length() - pos, str);
            return *this;
        }

        // Convert the entire string to lower case. Assumes the string is UTF8.
        auto MakeLower() -> string&;

        // Convert the entire string to upper case. Assumes the string is UTF8.
        auto MakeUpper() -> string&;

        // Assign the specified environment variable, returning true if found.
        //
        // Current string is replaced if found, cleared if not.
        [[nodiscard]] auto assignEnvVar(const char* env_var) -> bool;

        // Converts all backslashes in the string to forward slashes.
        //
        // Note: Windows works just fine using forward slashes instead of backslashes.
        auto backslashestoforward() -> string&;

        // Converts all forward slashes in the string to backward slashes.
        //
        // Note: Windows API functions normally work fine with forward slashes instead of
        // backslashes.
        auto forwardslashestoback() -> string&;

        // Add a trailing forward slash (default is only if there isn't one already)
        void addtrailingslash(bool always = false)
        {
            if (always || back() != '/')
            {
                push_back('/');
            }
        }

        // Returns true if current filename contains the specified case-insensitive extension.
        // Delegates to string_view.
        [[nodiscard]] auto has_extension(std::string_view ext) const -> bool
        {
            return string_view(*this).has_extension(ext);
        }

        // Returns true if current filename contains the specified case-insensitive file name.
        // Delegates to string_view.
        [[nodiscard]] bool has_filename(std::string_view name) const
        {
            return string_view(*this).has_filename(name);
        }

        // Returns a view to the current extension. View is empty if there is no extension.
        //
        // Caution: view is only valid until string is modified or destroyed.
        [[nodiscard]] string_view extension() const noexcept
        {
            return string_view(*this).extension();
        }

        // Returns a view to the current filename. View is empty if there is no filename.
        //
        // Caution: view is only valid until string is modified or destroyed.
        [[nodiscard]] string_view filename() const noexcept
        {
            return string_view(*this).filename();
        }

        // Returns offset to the current filename or npos if there is no filename.
        [[nodiscard]] size_t find_filename() const noexcept;

        // Replaces any existing extension with a new extension, or appends the extension if the
        // name doesn't currently have an extension.
        //
        // Returns view to the entire string.
        auto replace_extension(std::string_view newExtension) -> string&;

        // Replaces the extension portion of the string. Returns a view to the entire string.
        auto remove_extension() -> string& { return replace_extension(std::string_view {}); };

        // Replaces the filename portion of the string. Returns a view to the entire string.
        auto replace_filename(std::string_view newFilename) -> string&;

        // Removes the filename portion of the string. Returns a view to the entire string.
        auto remove_filename() -> string& { return replace_filename(""); }

        // Appends the filename -- assumes current string is a path. This will add a trailing
        // slash (if needed) before adding the filename.
        auto append_filename(std::string_view filename) -> string&;

        // Makes the current path relative to the supplied path. Use an empty string to be
        // relative to the current directory. Supplied path should not contain a filename.
        auto make_relative(string_view relative_to) -> string&;

        // Changes any current path to an absolute path.
        auto make_absolute() -> string&;

        // Replaces current string with the full path to the current working directory.
        auto assignCwd() -> string&;

        // Returns true if the current string refers to an existing file.
        // Delegates to string_view.
        [[nodiscard]] auto file_exists() const -> bool { return string_view(*this).file_exists(); }

        // Returns true if the current string refers to an existing directory.
        // Delegates to string_view.
        [[nodiscard]] auto dir_exists() const -> bool { return string_view(*this).dir_exists(); }

        // Retrieves the last write time of the current file.
        // Returns an invalid wxDateTime on error.
        [[nodiscard]] auto last_write_time() const -> wxDateTime;

        [[nodiscard]] auto file_size() const -> wxULongLong;

        // Confirms current string is an existing directory and then changes to that directory.
        //
        // If is_dir is false, current string is assumed to contain a filename in the path to
        // change to.
        [[nodiscard]] auto ChangeDir(bool is_dir = true) const -> bool;

        auto operator<<(std::string_view str) -> string&
        {
            *this += str;
            return *this;
        }

        auto operator<<(char chr) -> string&
        {
            *this += chr;
            return *this;
        }

        string& operator<<(int value)
        {
            *this += std::to_string(value);
            return *this;
        }

        string& operator<<(size_t value)
        {
            *this += std::to_string(value);
            return *this;
        }

        auto assign_wx(const wxString& str) -> string&
        {
            *this = str.ToStdString();
            return *this;
        }

        auto append_wx(const wxString& str) -> string&
        {
            *this += str.ToStdString();
            return *this;
        }

        // Forward slashes are fine. Recursive will create all parent directories as needed.
        [[nodiscard]] static auto MkDir(const string& path, bool recursive = false) -> bool;

        // Returns the current working directory as a string.
        [[nodiscard]] static auto GetCwd() -> string { return wxGetCwd().ToStdString(); }

    };  // end string class

    //////////////////////////////////////////////////////////////////////////
    // Deferred inline implementations for string_view methods needing string
    //////////////////////////////////////////////////////////////////////////

    inline auto string_view::as_str() const -> string
    {
        return { data(), size() };
    }
}  // namespace wxue
