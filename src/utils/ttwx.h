/////////////////////////////////////////////////////////////////////////////
// Purpose:   ttwx namespace functions and declarations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

// This is not a header guard, it simply indicates that ttwx.h has been included.
#define TTWX_H_INCLUDED

// filefn.h assumes wx/string.h has been included elsewhere. That's probably true, but
// we'll include it out of order here to be sure. It's needed throughout this file in
// addition to filefn.h.
#include <wx/string.h>  // wxString class

#include <wx/filefn.h>  // File- and directory-related functions

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

namespace ttwx
{
    // clang-format off

    // These functions are provided for convenience since they cast a char to unsigned char before
    // calling the std:: library function.

    inline auto is_alnum(char character) -> bool { return std::isalnum(static_cast<unsigned char>(character)) != 0; }
    inline auto is_alpha(char character) -> bool { return std::isalpha(static_cast<unsigned char>(character)) != 0; }
    inline auto is_blank(char character) -> bool { return std::isblank(static_cast<unsigned char>(character)) != 0; }
    inline auto is_cntrl(char character) -> bool { return std::iscntrl(static_cast<unsigned char>(character)) != 0; }
    inline auto is_digit(char character) -> bool { return std::isdigit(static_cast<unsigned char>(character)) != 0; }
    inline auto is_graph(char character) -> bool { return std::isgraph(static_cast<unsigned char>(character)) != 0; }
    inline auto is_lower(char character) -> bool { return std::islower(static_cast<unsigned char>(character)) != 0; }
    inline auto is_print(char character) -> bool { return std::isprint(static_cast<unsigned char>(character)) != 0; }
    inline auto is_punctuation(char character) -> bool { return std::ispunct(static_cast<unsigned char>(character)) != 0; }
    inline auto is_upper(char character) -> bool { return std::isupper(static_cast<unsigned char>(character)) != 0; }
    inline auto is_whitespace(char character) -> bool { return std::isspace(static_cast<unsigned char>(character)) != 0; }

    // clang-format on

    template <typename T>
    // Compares result against -1 -- use with returns from find, contains, locate, etc.
    constexpr auto is_found(T result) -> bool
    {
        return (static_cast<ptrdiff_t>(result)) != -1;
    }

    // Find any one of the characters in a group. Returns offset from the beginning of the
    // src string if found, npos if not.
    auto find_oneof(const wxString& src, const std::string& group, size_t src_start = 0) -> size_t;

    // Returns view to the next whitespace character. View is empty if there are no more
    // whitespaces.
    auto find_space(std::string_view str) noexcept -> std::string_view;
    inline auto find_space(const wxString& str) noexcept -> std::string_view
    {
        return find_space(std::string_view(str.ToStdString()));
    }

    auto find_nonspace(std::string_view str) noexcept -> std::string_view;
    inline auto find_nonspace(const wxString& str) noexcept -> std::string_view
    {
        return find_nonspace(std::string_view(str.ToStdString()));
    }

    // Equivalent to find_nonspace(find_space(str)).
    auto stepover(std::string_view str) noexcept -> std::string_view;
    inline auto stepover(const wxString& str) noexcept -> std::string_view
    {
        return stepover(std::string_view(str.ToStdString()));
    }

    template <typename T>
    // Converts a numeric value into a string.
    auto itoa(T value) -> std::string
    {
        return std::to_string(value);
    }

    inline void back_slashesto_forward(wxString& str)
    {
        str.Replace("\\", "/", true);
    }

    inline void forward_slashesto_back(wxString& str)
    {
        str.Replace("/", "\\", true);
    }

    // Add a trailing forward slash (default is only if there isn't either a forward or
    // backslash already)
    inline void add_trailing_slash(wxString& str, bool always = false)
    {
        if (always || (str.ToStdString().back() != '/' && str.ToStdString().back() != '\\'))
        {
            str += '/';
        }
    }

    inline auto get_View(const wxString& str) -> std::string_view
    {
        return std::string_view(str.ToStdString());
    }

    // Extracts a string from another string using start and end characters deduced from
    // the first non-whitespace character after offset. Supports double and single quotes,
    // angle and square brackets, and parenthesis.
    //
    // The return position is to the character in src that ended the string, or
    // **std::string::npos** if no ending character was found.
    auto extract_substring(std::string_view src, wxString& dest, size_t start) -> size_t;

    // Identical to extract_substring only it returns a wxString instead of a size_t
    inline auto create_substring(std::string_view src, size_t offset = 0) -> wxString
    {
        wxString dest;
        extract_substring(src, dest, offset);
        return dest;
    }

    // **************** File/path related functions ****************

    // wxWidgets normally uses wxFileName for path manipulations. These functions allow you to use a
    // wxString for a file/path name while still allowing some common path manipulations.

    auto find_extension(std::string_view str) -> std::string_view;
    inline auto find_extension(const wxString& str) -> std::string_view
    {
        return find_extension(std::string_view(str.ToStdString()));
    }

    // Replaces any existing extension with a new extension, or appends the extension if the
    // name doesn't currently have an extension.
    void replace_extension(wxString& str, std::string_view new_extension);

    auto find_filename(std::string_view str) noexcept -> std::string_view;
    inline auto find_filename(const wxString& str) noexcept -> std::string_view
    {
        return find_filename(std::string_view(str.ToStdString()));
    }

    // Appends the filename -- assumes current string is a path. This will add a trailing slash (if
    // needed) before adding the filename. Returns the modified path so that you can chain calls.
    //
    // Note that if the string ends with a filename, and you call this function, the original
    // filename will be converted into a folder name followed by the new filename.
    auto append_filename(wxString& path, const wxString& filename) -> wxString&;

    auto append_folder_name(wxString& path, const wxString& folder_name) -> wxString&;

    // Saves the current working directory and optionally restores it when the object goes
    // out of scope.
    class SaveCwd
    {
    public:
        enum class RestoreOption : std::uint8_t
        {
            no_restore = 0,
            restore = 1
        };

        explicit SaveCwd(RestoreOption option = RestoreOption::restore) : m_restore_option(option)
        {
            m_saved_cwd = wxGetCwd();
        }

        SaveCwd(const SaveCwd&) = default;
        SaveCwd(SaveCwd&&) = delete;
        SaveCwd& operator=(const SaveCwd&) = default;
        SaveCwd& operator=(SaveCwd&&) = delete;

        [[nodiscard]] auto get_SavedCwd() const -> const wxString& { return m_saved_cwd; }

        ~SaveCwd()
        {
            if (m_restore_option == RestoreOption::restore)
            {
                // Deliberately ignoring the return value because there's nothing we can do about it
                // here.
                (void) wxSetWorkingDirectory(m_saved_cwd);
            }
        }

    private:
        wxString m_saved_cwd;
        RestoreOption m_restore_option;
    };
}  // namespace ttwx
