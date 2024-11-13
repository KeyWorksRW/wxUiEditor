///////////////////////////////////
// Purpose:   wxString with additional methods similar to tt_string
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
///////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <tt_wxString.h> are available only with C++17 or later."
#endif

#if !defined(_WX_DEFS_H_)
    #error "You must include wx/defs.h before including this file."
#endif

// wxString must be set to use UTF8 strings, and set so that narrow strings are UTF8.
// These two settings ensure that wxString uses std::string for it's implementation.
#if !wxUSE_UNICODE_UTF8
    #error "wxUSE_UNICODE_UTF8 must be set to 1 in setup.h."
#elif !wxUSE_UTF8_LOCALE_ONLY
    #error "wxUSE_UTF8_LOCALE_ONLY must be set to 1 in setup.h."
#endif

#include <filesystem>

#include "tt_string.h"  // std::string with additional methods

// wxWidgets 3.3 requires a C++11 compiler, which allows using std::string as the underlying storage
// system for wxString, provided that both wxUSE_UNICODE_UTF8 and wxUSE_UTF8_LOCALE_ONLY are set to
// 1 in setup.h. The tt_wxString class inherits from wxString and extends it with additional methods
// and an operator that makes it possible to pass a tt_wxString object as a parameter to any
// function that expects a std::string_view parameter.

// Note that the methods that end with _wx are only needed when passing in wxString objects. If you
// pass in a tt_wxString object to a regular method, it will be automatically converted to a
// std::string_view thereby using the normal methods that take std::string_view ot tt_string_view
// parameters.

// Version of wxString that supports std::string_view and adds most of the same methods as
// tt_string. It can also be passed to any function that expects a std::string_view
// parameter.
class tt_wxString : public wxString
{
public:
    using wxString::wxString;  // inherit all of wxString's constructors

    tt_wxString(const wxString& str) : wxString(str) {}
    tt_wxString(void) : wxString() {}

    // Currently, even with wxUSE_UNICODE_UTF8 enabled, a UTF8 code page enabled via /utf8 compiler
    // switch, and setting the app to UTF8 in the manifest, calling assign() still converts the
    // string to UTF16 then back down to UTF8 (on Windows). So, we use FromUTF8Unchecked() instead
    // which is highly efficient if wxUSE_UNICODE_UTF8 and wxUSE_UTF8_LOCALE_ONLY are true.

    // Note that FromUTF8Unchecked() is very efficient in Release builds, but in Debug builds it
    // first calls wxASSERT(wxStringOperations::IsValidUtf8String(str));

    // REVIEW: [Randalphwa - 11-04-2024] Need to verify with latest code which calls assign
    // directly. If it does still do the double conversion, it should be entered as a bug report in
    // wxWidgets.

    tt_wxString(const char* str) { this->assign(wxString::FromUTF8Unchecked(str)); }
    tt_wxString(const std::string& str) { this->assign(wxString::FromUTF8Unchecked(str.data(), str.size())); }
    tt_wxString(std::string_view str) { this->assign(wxString::FromUTF8Unchecked(str.data(), str.size())); }

    tt_wxString(const std::filesystem::directory_entry& dir) { this->assign(dir.path().generic_wstring()); }

    // Call utf8_string() to get the entire string. Otherwise, this will call Mid().utf8_string()
    std::string sub_cstr(size_type pos = 0, size_type count = tt::npos) const;

    std::filesystem::path make_path() const;

    // Caution: tt_string_view will be invalid if tt_wxString is modified or destroyed.
    //
    // Simply pass tt_wxString to any function that expects a std::string_view parameter
    // rather than this function unless you specifically need a sub string.
    tt_string_view subview(size_t start = 0) const;

    // Used when caller refuses to accept tt_string_view via subview as a std::string_view
    // (e.g. std::format())
    const std::string_view ToStdView(size_t start = 0) const { return subview(start); }

    // Effectively, this is equivalent to a cast which then gives you full access to
    // tt_string methods.
    const tt_string& ToTTString() const { return static_cast<const tt_string&>(ToStdString()); }

    // Case-insensitive comparison.
    //
    // Note that tt_wxString has an operator to return a std::string_view so passing in a
    // tt_wxString works as well.
    int comparei(std::string_view str) const
    {
        tt_wxString str2(str);
        return CmpNoCase(str2);
    }

    tt_wxString& append_view(std::string_view str, size_t posStart = 0, size_t len = npos);

    tt_wxString& assign_view(std::string_view str, size_t posStart = 0, size_t len = npos);

    // Locates the position of a substring.
    size_t locate(std::string_view str, size_t posStart = 0, tt::CASE checkcase = tt::CASE::exact) const
    {
        return ToTTString().locate(str, posStart, checkcase);
    }

    // Locates the position of a substring.
    size_t locate_wx(const wxString& str, size_t posStart = 0, tt::CASE checkcase = tt::CASE::exact) const
    {
        return locate(static_cast<tt_wxString>(str), posStart, checkcase);
    }

    // Returns true if the sub string exists
    bool contains(std::string_view sub, tt::CASE checkcase = tt::CASE::exact) const
    {
        return (locate(sub, 0, checkcase) != npos);
    }

    // Returns true if the sub string exists
    bool contains_wx(const wxString& sub, tt::CASE checkcase = tt::CASE::exact) const
    {
        return contains(static_cast<tt_wxString>(sub), checkcase);
    }

    // Returns true if any char* string in the iteration list appears somewhere in the the
    // main string.
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

    // Returns true if any wxString in the iteration list appears somewhere in the the main
    // string.
    template <class iterT>
    bool strContains_wx(iterT iter, tt::CASE checkcase = tt::CASE::exact)
    {
        for (auto& strIter: iter)
        {
            if (contains_wx(strIter, checkcase))
                return true;
        }
        return false;
    }

    // Find any one of the characters in a set. Returns offset if found, npos if not.
    //
    // This is equivalent to calling std::strpbrk but returns an offset instead of a pointer.
    size_t find_oneof(std::string_view set) const;

    // Returns offset to the next whitespace character starting with pos. Returns npos if
    // there are no more whitespaces.
    //
    // A whitespace character is a space, tab, eol or form feed character.
    size_t find_space(size_t start = 0) const { return ToTTString().find_space(start); }

    // Returns a UTF8 copy of the string starting with the next whitespace character after
    // pos. Returns an empty string if there are no more whitespaces.
    //
    // A whitespace character is a space, tab, eol or form feed character.
    std::string sub_find_space(size_t start = 0) const;

    // Returns offset to the next non-whitespace character starting with pos. Returns npos
    // if there are no more non-whitespace characters.
    //
    // A whitespace character is a space, tab, eol or form feed character.
    size_t find_nonspace(size_t start = 0) const { return ToTTString().find_nonspace(start); }

    // Returns offset to the next whitespace character starting with pos. Returns npos if
    // there are no more whitespaces.
    //
    // A whitespace character is a space, tab, eol or form feed character.
    tt_string_view view_space(size_t start = 0) const { return subview(find_space(start)); }

    // Returns offset to the next non-whitespace character starting with pos. Returns npos
    // if there are no more non-whitespace characters.
    //
    // A whitespace character is a space, tab, eol or form feed character.
    tt_string_view view_nonspace(size_t start = 0) const { return subview(find_nonspace(start)); }

    // Returns an offset to the next word -- i.e., find the first non-whitedspace character
    // after the next whitespace character.
    //
    // Equivalent to find_nonspace(find_space(start)).
    tt_string_view view_stepover(size_t start = 0) const { return subview(stepover(start)); }

    // Returns a UTF8 copy of the string starting with the next non-whitespace character
    // after pos. Returns an empty string if there are no more non-whitespaces.
    //
    // A whitespace character is a space, tab, eol or form feed character.
    std::string sub_find_nonspace(size_t start = 0) const;
    // tt_string sub_find_nonspace(size_t start = 0) const { return sub_cstr(find_nonspace(start)); }

    // Returns an offset to the next word -- i.e., find the first non-whitespace character
    // after the next whitespace character.
    //
    // Equivalent to find_nonspace(find_space(start)).
    size_t stepover(size_t start = 0) const { return ToTTString().stepover(start); }

    // Returns a UTF8 copy of the string starting with the next word. Returns an empty
    // string if there is no next word.
    //
    // Equivalent to sub_cstr(find_nonspace(find_space(start))).
    std::string sub_stepover(size_t start = 0) const;

    // Returns true if the strings are identical.
    //
    // On Windows, the string will first be converted to UTF16 before comparing.
    bool is_sameas(std::string_view str, tt::CASE checkcase = tt::CASE::exact) const;

    bool is_sameas_wx(const wxString& str, tt::CASE checkcase = tt::CASE::exact) const
    {
        return (checkcase == tt::CASE::exact) ? Cmp(str) == 0 : CmpNoCase(str) == 0;
    }

    // Returns true if the sub-string is identical to the first part of the main string
    bool is_sameprefix(std::string_view str, tt::CASE checkcase = tt::CASE::exact) const
    {
        return ToTTString().is_sameprefix(str, checkcase);
    }

    // Returns true if the sub-string is identical to the first part of the main string
    bool is_sameprefix_wx(const wxString& str, tt::CASE checkcase = tt::CASE::exact) const
    {
        return is_sameprefix(static_cast<tt_wxString>(str), checkcase);
    }

    int atoi(size_t start = 0) const { return tt::atoi(subview().data() + start); }

    // If character is found, line is truncated from the character on
    void erase_from(char ch);

    // If string is found, line is truncated from the string on
    void erase_from(std::string_view sub);

    // If string is found, line is truncated from the string on
    void erase_from_wx(const wxString& sub);

    // Replace first (or all) occurrences of substring with another one
    size_t replace_view(std::string_view oldtext, std::string_view newtext, bool replace_all = tt::REPLACE::once);

    bool is_found(size_t pos) const { return (pos != npos); }

    ////////// filename functions ////////////
    //
    // The following functions assume the current string is a file name.
    //
    /////////////////////////////////////////

    // Add a trailing forward slash (default is only if there isn't one already). Use this
    // function to ensure a directory name will not be interpreted as a file name.
    void addtrailingslash(bool always = false)
    {
        if (always || Last() != '/')
            *this += '/';
    }

    // Converts all backslashes in the string to forward slashes.
    //
    // Note: Windows API functions work fine with forward slashes instead of backslashes.
    tt_wxString& backslashestoforward();

    // Converts all forward slashes in the string to backward slashes.
    //
    // Note: Windows API functions normally work fine with forward slashes instead of
    // backslashes.
    tt_wxString& forwardslashestoback();

    // ext param should begin with a period (e.g., ".cpp")
    bool has_extension(std::string_view ext)
    {
        return subview().has_extension(ext);
    }

    // ext param should begin with a period (e.g., ".cpp")
    bool has_extension_wx(const wxString& ext)
    {
        return subview().has_extension(static_cast<tt_wxString>(ext));
    }

    // Returns true if current filename contains the specified case-insensitive file name.
    bool has_filename(std::string_view name) const
    {
        return subview().has_filename(name);
    }

    // Returns true if current filename contains the specified case-insensitive file name.
    bool has_filename_wx(const wxString& name) const
    {
        return subview().has_filename(static_cast<tt_wxString>(name));
    }

    // Returns a view to the current extension. View is empty if there is no extension.
    //
    // Caution: view is only valid until tt_string is modified or destroyed.
    tt_string_view extension() const
    {
        return ToTTString().extension();
    }

    // Returns a view to the current filename. View is empty if there is no filename.
    //
    // Caution: view is only valid until tt_string is modified or destroyed.
    tt_string_view filename() const
    {
        return ToTTString().filename();
    }

    // Returns offset to the current filename or tt::npos if there is no filename.
    size_t find_filename() const noexcept { return ToTTString().find_filename(); }

    // Replaces any existing extension with a new extension, or appends the extension if the
    // current file name doesn't have an extension.
    tt_wxString& replace_extension(std::string_view newExtension);

    // Replaces any existing extension with a new extension, or appends the extension if the
    // current file name doesn't have an extension.
    tt_wxString& replace_extension_wx(const wxString& newExtension);

    // Removes the extension portion of the file name.
    tt_wxString& remove_extension() { return replace_extension(std::string_view()); };

    tt_wxString& replace_filename(std::string_view newFilename = std::string_view());
    tt_wxString& replace_filename_wx(const wxString& newFilename);

    tt_wxString& remove_filename() { return replace_filename(std::string_view()); };

    // Appends the file name -- assumes current string is a directory. This will add a
    // trailing slash (if needed) before adding the file name.
    tt_wxString& append_filename(std::string_view filename);

    // Appends the file name -- assumes current string is a directory. This will add a
    // trailing slash (if needed) before adding the filename.
    tt_wxString& append_filename_wx(const wxString& filename);

    // Replaces current string with the full path to the current working directory.
    tt_wxString& assignCwd()
    {
        assign(wxGetCwd());
        return *this;
    };

    tt_wxString& assign_path(std::filesystem::path path);

    // Changes any current path to an absolute path.
    tt_wxString& make_absolute();

    // Returns the file name which can be used to access this file if the current directory is pathBase
    tt_wxString& make_relative(std::string_view pathBase);

    // Returns the file name which can be used to access this file if the current directory is pathBase
    tt_wxString& make_relative_wx(const wxString& pathBase);

    // Returns true if the current string refers to an existing file.
    bool file_exists() const { return wxFileExists(*this); };

    // Returns true if the current string refers to an existing directory.
    bool dir_exists() const { return wxDirExists(*this); };

    // Retrieves the last write time of the current file.
    //
    // Throws std::filesystem::filesystem_error on underlying OS API errors.
    std::filesystem::file_time_type last_write_time() const;

    std::uintmax_t file_size() const;

    // Confirms current string is an existing directory and then changes to that directory.
    //
    // If is_dir is false, current sting is assumed to contain a filename in the path to
    // change to.
    bool ChangeDir(bool is_dir = true) const;

    // This will return a full path to the file if found, or an empty string if not found.
    //
    // All subdirectories of the specified directory will be searched.
    static tt_wxString find_file(const tt_wxString& dir, const tt_wxString& filename);

    ////////// std::string functions ///////////////////////////////////
    //
    // The following functions assume the current string is a file name.
    //
    ////////////////////////////////////////////////////////////////////

    // wxString contains several functions that work the same as std::string/wstring functions.
    // The following functions are additional std::string/wstring functions that are not
    // part of wxString.

    // Like std::string.back(), return is undefined if string is empty
    auto back() { return ToStdString().back(); }

    // Like std::string.front(), return is undefined if string is empty
    auto front() { return ToStdString().front(); }

    void pop_back() { RemoveLast(); }

    operator std::string_view() const { return std::string_view(ToStdString().c_str(), length()); }
};
