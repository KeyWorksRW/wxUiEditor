///////////////////////////////////
// Purpose:   wxString with additional methods similar to tt_string
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
///////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <tt_wxString.h> are available only with C++17 or later."
#endif

#if !defined(_WX_DEFS_H_)
    #error "You must include wx/defs.h before including this file."
#endif

class tt_string;

// Version of wxString that supports std::string_view and adds most of the same methods as
// tt_string.
//
// Note: on Windows, this class assumes char* and string_view are UTF8 strings and
// automatically converts them to UTF16
class tt_wxString : public wxString
{
public:
    using wxString::wxString;  // inherit all of wxString's constructors

    tt_wxString(const wxString& str) : wxString(str) {}
    tt_wxString(void) : wxString() {}

#if defined(_WIN32)
    // When compiling for Windows, assume all char* are utf8 strings and convert them to utf16 before assigning them.

    tt_wxString(const char* str)
    {
        this->assign(wxString::FromUTF8(str));
    }
    tt_wxString(const std::string& str)
    {
        this->assign(wxString::FromUTF8(str.data(), str.size()));
    }
    tt_wxString(std::string_view str)
    {
        this->assign(wxString::FromUTF8(str.data(), str.size()));
    }
#else
    tt_wxString(const std::string& str)
    {
        this->assign(str.data(), str.size());
    }
    tt_wxString(std::string_view str)
    {
        this->assign(str.data(), str.size());
    }
#endif  // _WIN32

    // On Windows, converts to UTF8 before creating a std::string copy.
    std::string sub_cstr(size_type pos = 0, size_type count = tt::npos) const;

    tt_wxString& append_view(std::string_view str, size_t posStart = 0, size_t len = npos);

    tt_wxString& assign_view(std::string_view str, size_t posStart = 0, size_t len = npos);

    // Case-insensitive comparison.
    int comparei(std::string_view str) const;

    // Case-insensitive comparison.
    int comparei_wx(const wxString& str) const
    {
        return CmpNoCase(str);
    };

    // Locates the position of a substring.
    size_t locate(std::string_view str, size_t posStart = 0, tt::CASE checkcase = tt::CASE::exact) const;

    // Locates the position of a substring.
    size_t locate_wx(const wxString& str, size_t posStart = 0, tt::CASE checkcase = tt::CASE::exact) const;

    // Returns true if the sub string exists
    bool contains(std::string_view sub, tt::CASE checkcase = tt::CASE::exact) const
    {
        return (locate(sub, 0, checkcase) != npos);
    }

    // Returns true if the sub string exists
    bool contains_wx(const wxString& sub, tt::CASE checkcase = tt::CASE::exact) const
    {
        return (locate_wx(sub, 0, checkcase) != npos);
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

    // Find any one of the characters in a set. Returns offset if found, npos if not.
    //
    // This is equivalent to calling std::wcspbrk but returns an offset instead of a pointer.
    size_t find_oneof_wx(const wxString& set) const;

    // Returns offset to the next whitespace character starting with pos. Returns npos if
    // there are no more whitespaces.
    //
    // A whitespace character is a space, tab, eol or form feed character.
    size_t find_space(size_t start = 0) const;

    // Returns a UTF8 copy of the string starting with the next whitespace character after
    // pos. Returns an empty string if there are no more whitespaces.
    //
    // A whitespace character is a space, tab, eol or form feed character.
    std::string sub_find_space(size_t start = 0) const;

    // Returns offset to the next non-whitespace character starting with pos. Returns npos
    // if there are no more non-whitespace characters.
    //
    // A whitespace character is a space, tab, eol or form feed character.
    size_t find_nonspace(size_t start = 0) const;

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
    size_t stepover(size_t start = 0) const;

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
    bool is_sameprefix(std::string_view str, tt::CASE checkcase = tt::CASE::exact) const;

    // Returns true if the sub-string is identical to the first part of the main string
    bool is_sameprefix_wx(const wxString& str, tt::CASE checkcase = tt::CASE::exact) const;

    int atoi() const
    {
        long val;
        ToLong(&val);
        return static_cast<int>(val);
    }

    // If character is found, line is truncated from the character on
    void erase_from(char ch);

    // If string is found, line is truncated from the string on
    void erase_from(std::string_view sub);

    // If string is found, line is truncated from the string on
    void erase_from_wx(const wxString& sub);

    // Replace first (or all) occurrences of substring with another one
    size_t replace_view(std::string_view oldtext, std::string_view newtext, bool replace_all = tt::REPLACE::once);

    bool is_found(size_t pos) const
    {
        return (pos != npos);
    }

    ////////// filename functions ////////////
    //
    // The following functions assume the current string is a file name.
    //
    ///////////////////////////////

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
    bool has_extension(std::string_view ext, tt::CASE checkcase = tt::CASE::either)
    {
        return extension().is_sameas(ext, checkcase);
    }

    // ext param should begin with a period (e.g., ".cpp")
    bool has_extension_wx(const wxString& ext, tt::CASE checkcase = tt::CASE::either)
    {
        return extension().is_sameas_wx(ext, checkcase);
    }

    // Returns true if current filename contains the specified case-insensitive file name.
    bool has_filename(std::string_view name, tt::CASE checkcase = tt::CASE::either) const
    {
        return filename().is_sameas(name, checkcase);
    }

    // Returns true if current filename contains the specified case-insensitive file name.
    bool has_filename_wx(const wxString& name, tt::CASE checkcase = tt::CASE::either) const
    {
        return filename().is_sameas_wx(name, checkcase);
    }

    // Returns a copy of the current extension or wxEmptyStr if there is no extension.
    tt_wxString extension() const;

    // Returns a copy of the current filename or wxEmptyStr if there is no filename.
    tt_wxString filename() const;

    // Returns offset to the current filename or tt::npos if there is no filename.
    size_t find_filename() const noexcept;

    // Replaces any existing extension with a new extension, or appends the extension if the
    // current file name doesn't have an extension.
    tt_wxString& replace_extension(std::string_view newExtension);

    // Replaces any existing extension with a new extension, or appends the extension if the
    // current file name doesn't have an extension.
    tt_wxString& replace_extension_wx(const wxString& newExtension);

    // Removes the extension portion of the file name.
    tt_wxString& remove_extension()
    {
        return replace_extension(std::string_view());
    };

    tt_wxString& replace_filename(std::string_view newFilename = std::string_view());
    tt_wxString& replace_filename_wx(const wxString& newFilename);

    tt_wxString& remove_filename()
    {
        return replace_filename(std::string_view());
    };

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

    // Changes any current path to an absolute path.
    tt_wxString& make_absolute();

    // Returns the file name which can be used to access this file if the current directory is pathBase
    tt_wxString& make_relative_wx(const wxString& pathBase);

    // Returns the file name which can be used to access this file if the current directory is pathBase
    tt_wxString& make_relative(std::string_view pathBase);

    // Returns true if the current string refers to an existing file.
    bool file_exists() const
    {
        return wxFileExists(*this);
    };

    // Returns true if the current string refers to an existing directory.
    bool dir_exists() const
    {
        return wxDirExists(*this);
    };

    // Confirms current string is an existing directory and then changes to that directory.
    //
    // If is_dir is false, current sting is assumed to contain a filename in the path to
    // change to.
    bool ChangeDir(bool is_dir = true) const;

    // This will return a full path to the file if found, or an empty string if not found.
    //
    // All subdirectories of the specified directory will be searched.
    static tt_wxString find_file(const tt_wxString& dir, const tt_wxString& filename);

    ////////// std::string functions ////////////
    //
    // The following functions assume the current string is a file name.
    //
    ///////////////////////////////

    // wxString contains several functions that work the same as std::string/wstring functions.
    // The following functions are additional std::string/wstring functions that are not
    // part of wxString.

    // Like std::string.back(), return is undefined if string is empty
    auto back()
    {
        return GetChar(size() - 1);
    }

    // Like std::string.front(), return is undefined if string is empty
    auto front()
    {
        return GetChar(0);
    }

    void pop_back()
    {
        RemoveLast();
    }
};

// Retrieves the current working directory. Construct with (true) to restore the
// directory in the dtor.
class tt_cwd : public tt_wxString
{
public:
    enum : bool
    {
        no_restore = false,
        restore = true
    };

    // Specify true to restore the directory in the destructor
    tt_cwd(bool restore = no_restore)
    {
        assignCwd();
        if (restore)
            m_restore.assign(*this);
    }

    ~tt_cwd()
    {
        if (m_restore.size())
            m_restore.ChangeDir();
    }

private:
    tt_wxString m_restore;
};
