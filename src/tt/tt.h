/////////////////////////////////////////////////////////////////////////////
// Purpose:   tt namespace functions and declarations
// Author:    Ralph Walden
// Copyright: Copyright (c) 1998-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <tt.h> are available only with C++17 or later."
#endif

#include <filesystem>
#include <string_view>

class tt_string;

namespace tt
{
    /// Use to compare a size_t against -1
    constexpr size_t npos = static_cast<size_t>(-1);

    enum class CASE : size_t
    {
        exact,
        either,
        utf8  // comparisons are done by converting characters to lowercase UTF8
    };

    enum class TRIM : size_t
    {
        right,
        left,
        both,
        none
    };

    enum REPLACE : bool
    {
        once = false,
        all = true,
    };

    extern const std::string emptystring;

    // This is primarily used by tt_string when it calls std::systemfile functions.
    extern std::error_code error_code;

    // clang-format off

    // These functions are provided for convenience since they cast a char to unsigned char before calling the std::
    // library function.

    inline bool is_alnum(char ch) { return std::isalnum(static_cast<unsigned char>(ch)); }
    inline bool is_alpha(char ch) { return std::isalpha(static_cast<unsigned char>(ch)); }
    inline bool is_blank(char ch) { return std::isblank(static_cast<unsigned char>(ch)); }
    inline bool is_cntrl(char ch) { return std::iscntrl(static_cast<unsigned char>(ch)); }
    inline bool is_digit(char ch) { return std::isdigit(static_cast<unsigned char>(ch)); }
    inline bool is_graph(char ch) { return std::isgraph(static_cast<unsigned char>(ch)); }
    inline bool is_lower(char ch) { return std::islower(static_cast<unsigned char>(ch)); }
    inline bool is_print(char ch) { return std::isprint(static_cast<unsigned char>(ch)); }
    inline bool is_punctuation(char ch) { return std::ispunct(static_cast<unsigned char>(ch)); }
    inline bool is_upper(char ch) { return std::isupper(static_cast<unsigned char>(ch)); }
    inline bool is_whitespace(char ch) { return std::isspace(static_cast<unsigned char>(ch)); }

    // Is ch the start of a utf8 sequence?
    constexpr inline bool is_utf8(char ch) noexcept { return ((ch & 0xC0) != 0x80); }

    template <typename T>
    // Compares result against -1 -- use with returns from find, contains, locate, etc.
    constexpr bool is_found(T result) { return (static_cast<ptrdiff_t>(result)) != -1; }

    // Returns true if strings are identical
    bool is_sameas(std::string_view str1, std::string_view str2, tt::CASE checkcase = tt::CASE::exact);

    // Returns true if the sub-string is identical to the first part of the main string
    bool is_sameprefix(std::string_view strMain, std::string_view strSub, tt::CASE checkcase = tt::CASE::exact);

    // Return a view to the portion of the string beginning with the sub string.
    //
    // Return view is empty if substring is not found.
    std::string_view find_str(std::string_view main, std::string_view sub, tt::CASE checkcase = tt::CASE::exact);

    // Returns the position of sub within main, or npos if not found.
    size_t findstr_pos(std::string_view main, std::string_view sub, tt::CASE checkcase = tt::CASE::exact);

    // Returns true if the sub string exists withing the main string.
    //
    // Same as find_str but with a boolean return instead of a string_view.
    bool contains(std::string_view main, std::string_view sub, tt::CASE checkcase = tt::CASE::exact);

    // Returns true if the character string exists withing the main string.
    //
    // Same as find() but with a boolean return instead of a size_t, and the ability to check
    // non-exact case.
    bool contains(std::string_view main, char ch, tt::CASE checkcase = tt::CASE::exact);

    // clang-format on

    // Returns a pointer to the next character in a UTF8 string.
    const char* next_utf8_char(const char* psz) noexcept;

    // Returns view to the next whitespace character. View is empty if there are no more
    // whitespaces.
    std::string_view find_space(std::string_view str) noexcept;

    // Returns position of next whitespace character or npos if not found.
    size_t find_space_pos(std::string_view str);

    // Returns view to the next non-whitespace character. View is empty if there are no
    // non-whitespace characters.
    std::string_view find_nonspace(std::string_view str) noexcept;

    // Returns position of next non-whitespace character or npos if not found.
    size_t find_nonspace_pos(std::string_view str) noexcept;

    // Equivalent to find_nonspace(find_space(str)).
    std::string_view stepover(std::string_view str) noexcept;

    // Equivalent to find_nonspace(find_space(str)) returning the position or npos.
    size_t stepover_pos(std::string_view str) noexcept;

    // Converts a string into an integer.
    //
    // If string begins with '0x' it is assumed to be hexadecimal and is converted.
    // String may begin with a '-' or '+' to indicate the sign of the integer.
    int atoi(std::string_view str) noexcept;

    template <typename T>
    // Converts a numeric vaslue into a string.
    std::string itoa(T value)
    {
        return std::to_string(value);
    }

    // Converts a signed integer into a string.
    //
    // If format is true, the number will be formatted with ',' or '.' depending on the
    // current locale.
    tt_string itoa(int val, bool format);

    // Converts a size_t into a string.
    //
    // If format is true, the number will be formatted with ',' or '.' depending on the
    // current locale.
    tt_string itoa(size_t val, bool format);

    // Return a view to a filename's extension. View is empty if there is no extension.
    std::string_view find_extension(std::string_view str);

    // Determines whether the character at pos is part of a filename. This will
    // differentiate between '.' being used as part of a path (. for current directory, or ..
    // for relative directory) versus being the leading character in a file.
    bool is_valid_filechar(std::string_view str, size_t pos);

    // Converts all backslashes in a filename to forward slashes.
    //
    // Note: Windows handles paths that use forward slashes, so backslashes are normally
    // unnecessary.
    void backslashestoforward(std::string& str);
    void backslashestoforward(wxString& str);

    // Performs a check to see if a directory entry is a filename and contains the
    // specified extension.
    bool has_extension(std::filesystem::directory_entry name, std::string_view extension,
                       tt::CASE checkcase = tt::CASE::exact);

    // Confirms newdir exists and is a directory and then changes to that directory.
    //
    // Returns false only if newdir is not an existing directory. Throws filesystem_error
    // if the directory is valid but could not be changed to.
    bool ChangeDir(std::string_view newdir);

    bool dir_exists(std::string_view dir);
    bool file_exists(std::string_view filename);

    void utf8to16(std::string_view str, std::wstring& dest);
    void utf16to8(std::wstring_view str, std::string& dest);

    std::wstring utf8to16(std::string_view str);
    std::string utf16to8(std::wstring_view str);

    // Remove locale-dependent whitespace from right side of string
    inline void RightTrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                             [](unsigned char ch)
                             {
                                 return !std::isspace(ch);
                             })
                    .base(),
                s.end());
    }

    // Remove locale-dependent whitespace from left side of string
    inline void LeftTrim(std::string& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                        [](unsigned char ch)
                                        {
                                            return !std::isspace(ch);
                                        }));
    }

    // Remove locale-dependent whitespace from left and right side of string
    inline void BothTrim(std::string& s)
    {
        tt::LeftTrim(s);
        tt::RightTrim(s);
    }
#if (defined(_WIN32))
    HINSTANCE ShellRun_wx(const wxString& filename, const wxString& args, const wxString& directory,
                          INT nShow = SW_SHOWNORMAL, HWND hwndParent = NULL);
#else
    int ShellRun_wx(const wxString& filename, const wxString& args, const wxString& directory, int nShow = 0,
                    void* hwndParent = nullptr);
#endif
}  // namespace tt
