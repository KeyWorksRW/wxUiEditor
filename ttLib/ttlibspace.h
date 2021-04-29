/////////////////////////////////////////////////////////////////////////////
// Purpose:   ttlib namespace functions and declarations
// Author:    Ralph Walden
// Copyright: Copyright (c) 1998-2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <ttlibspace.h> are available only with C++17 or later."
#endif

// With #pragma once, a header guard shouldn't be necessary and causes unwanted indentation by clang-format. The
// following #if/#endif check verifies that the file wasn't read twice.

#if defined(_TTLIB_NAMESPACE_H_GUARD_)
    #error "#pragma once failed -- header is being read a second time!"
#else
    #define _TTLIB_NAMESPACE_H_GUARD_  // sanity check to confirm that #pragma once is working as expected
#endif

#include <cctype>
#include <filesystem>  // directory_entry
#include <stdlib.h>    // for std::abs(long)
#include <string>
#include <string_view>

class wxString;  // for use with functions in ttLibwx.lib or UNIX builds that require wxWidgets

#if defined(NDEBUG)
/// Use this macro to comment out parameters that are not used in Release builds
    #define DBG_PARAM(param) /* param */
#else
/// Use this macro to comment out parameters that are only used in Debug builds
    #define DBG_PARAM(param) param
#endif

#if defined(_WIN32)
// Use this macro to comment out parameters that are not used when building for Windows
    #define UNUSED_IN_WIN32(param) /* param */
#else
// Use this macro to comment out parameters that are not used when building for Windows
    #define UNUSED_IN_WIN32(param) param
#endif

// Using TT_ASSERT macros provides different assertion handling depending on the platform
#if !defined(TT_ASSERT)
    #if defined(_WIN32)
        #define TT_ASSERT(exp)          ttASSERT(exp)
        #define TT_ASSERT_MSG(exp, msg) ttASSERT_MSG(exp, msg)
    #elif !defined(_WIN32) && defined(wxASSERT)
        #define TT_ASSERT(exp)          wxASSERT(exp)
        #define TT_ASSERT_MSG(exp, msg) wxASSERT_MSG(exp, msg)
    #else
        #define TT_ASSERT(exp)          assert(exp)
        #define TT_ASSERT_MSG(exp, msg) assert(((void) msg, exp))
    #endif
#endif

#ifndef assertm
/// assert with a message
    #define assertm(exp, msg) assert(((void) msg, exp))
#endif

#if defined(_WIN32)
/// This is just a shortcut to make code easier to read
    #define _ls(id) ttlib::LoadStringEx(static_cast<WORD>(id))
#endif  // _WIN32

// The tt namespace is for enums and constexpr values that only require a header. The ttlib namespace is for
// functions and classes that require linking to ttLib.lib in order to use.

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
        both
    };

}  // namespace tt

namespace ttlib
{
    class cstr;  // forward definition
    class cview;

    extern const std::string emptystring;

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

    /// Is ch the start of a utf8 sequence?
    constexpr inline bool is_utf8(char ch) noexcept { return ((ch & 0xC0) != 0x80); }

    template<typename T>
    /// Compares result against -1 -- use with returns from find, contains, locate, etc.
    constexpr bool is_error(T result)
    {
        return (static_cast<ptrdiff_t>(result)) == -1;
    }

    template<typename T>
    /// Compares result against -1 -- use with returns from find, contains, locate, etc.
    constexpr bool is_found(T result)
    {
        return (static_cast<ptrdiff_t>(result)) != -1;
    }

    /// Returns true if strings are identical
    bool is_sameas(std::string_view str1, std::string_view str2, tt::CASE checkcase = tt::CASE::exact);

    /// Returns true if the sub-string is identical to the first part of the main string
    bool is_sameprefix(std::string_view strMain, std::string_view strSub, tt::CASE checkcase = tt::CASE::exact);

    /// Return a view to the portion of the string beginning with the sub string.
    ///
    /// Return view is empty if substring is not found.
    std::string_view find_str(std::string_view main, std::string_view sub, tt::CASE checkcase = tt::CASE::exact);

    /// Returns the position of sub within main, or npos if not found.
    size_t findstr_pos(std::string_view main, std::string_view sub, tt::CASE checkcase = tt::CASE::exact);

    /// Returns true if the sub string exists withing the main string.
    ///
    /// Same as find_str but with a boolean return instead of a string_view.
    bool contains(std::string_view main, std::string_view sub, tt::CASE checkcase = tt::CASE::exact);

    template<class iterT>
    /// Returns true if any string in the iteration list appears somewhere in the the main string.
    bool strContains(std::string_view str, iterT iter, tt::CASE checkcase = tt::CASE::exact)
    {
        for (auto& strIter: iter)
        {
            if (ttlib::contains(str, strIter, checkcase))
                return true;
        }
        return false;
    }

    /// Returns a pointer to the next character in a UTF8 string.
    const char* next_utf8_char(const char* psz) noexcept;

    /// Returns view to the next whitespace character. View is empty if there are no more
    /// whitespaces.
    std::string_view find_space(std::string_view str) noexcept;

    /// Returns position of next whitespace character or npos if not found.
    size_t find_space_pos(std::string_view str);

    /// Returns view to the next non-whitespace character. View is empty if there are no
    /// non-whitespace characters.
    std::string_view find_nonspace(std::string_view str) noexcept;

    /// Returns position of next non-whitespace character or npos if not found.
    size_t find_nonspace_pos(std::string_view str) noexcept;

    /// Equivalent to find_nonspace(find_space(str)).
    std::string_view stepover(std::string_view str) noexcept;

    /// Equivalent to find_nonspace(find_space(str)) returning the position or npos.
    size_t stepover_pos(std::string_view str) noexcept;

    /// Returns a zero-terminated view of the first whitespace character. View is empty if
    /// there is no whitespace character.
    cview view_space(const std::string& str, size_t startpos = 0) noexcept;

    /// Returns a zero-terminated view of the first non-whitespace character. View is empty
    /// if there is no non-whitespace character.
    cview view_nonspace(const std::string& str, size_t startpos = 0) noexcept;

    /// Locates the next whitespace character, and returns a zero-terminated view to the
    /// first non-whitespace character after that whitespace character.
    cview view_stepover(const std::string& str, size_t startpos = 0) noexcept;

    /// Generates hash of string using djb2 hash algorithm
    size_t get_hash(std::string_view str) noexcept;

    /// Converts a string into an integer.
    ///
    /// If string begins with '0x' it is assumed to be hexadecimal and is converted.
    /// String may begin with a '-' or '+' to indicate the sign of the integer.
    int atoi(std::string_view str) noexcept;

    /// Converts a signed integer into a string.
    ///
    /// If format is true, the number will be formatted with ',' or '.' depending on the
    /// current locale.
    ttlib::cstr itoa(int val, bool format = false);

    /// Converts a size_t into a string.
    ///
    /// If format is true, the number will be formatted with ',' or '.' depending on the
    /// current locale.
    ttlib::cstr itoa(size_t val, bool format = false);

    /// Return a view to a filename's extension. View is empty if there is no extension.
    std::string_view find_extension(std::string_view str);

    /// Determines whether the character at pos is part of a filename. This will
    /// differentiate between '.' being used as part of a path (. for current directory, or ..
    /// for relative directory) versus being the leading character in a file.
    bool is_valid_filechar(std::string_view str, size_t pos);

    /// Converts all backslashes in a filename to forward slashes.
    ///
    /// Note: Windows handles paths that use forward slashes, so backslashes are normally
    /// unnecessary.
    void backslashestoforward(std::string& str);

    /// Performs a check to see if a directory entry is a filename and contains the
    /// specified extension.
    bool has_extension(std::filesystem::directory_entry name, std::string_view extension, tt::CASE checkcase = tt::CASE::exact);

    /// Confirms newdir exists and is a directory and then changes to that directory.
    ///
    /// Returns false only if newdir is not an existing directory. Throws filesystem_error
    /// if the directory is valid but could not be changed to.
    bool ChangeDir(std::string_view newdir);

    bool dir_exists(std::string_view dir);
    bool file_exists(std::string_view filename);

    void utf8to16(std::string_view str, std::wstring& dest);
    void utf16to8(std::wstring_view str, std::string& dest);

    std::wstring utf8to16(std::string_view str);
    ttlib::cstr utf16to8(std::wstring_view str);

    /// Remove locale-dependent whitespace from right side of string
    inline void RightTrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    }

    /// Remove locale-dependent whitespace from left side of string
    inline void LeftTrim(std::string& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    }

    /// Remove locale-dependent whitespace from left and right side of string
    inline void BothTrim(std::string& s)
    {
        ttlib::LeftTrim(s);
        ttlib::RightTrim(s);
    }
}  // namespace ttlib

// clang-format off

#if defined(_WIN32)

#include <windows.h>

// clang-format on

//////////////////////////////// Windows-only section ////////////////////////
//                                                                          //
// The following functions can only be used when compiling for Windows.     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

namespace ttlib
{
    template<typename T_MSG, typename T_WPARAM, typename T_LPARAM>
    /// Calls SendMessageW() without having to cast the parameter types to UINT, WPARAM
    /// and LPARAM
    ///
    /// This calls SendMessageW even if UNICODE was not defined
    LRESULT SendMsg(HWND hwnd, T_MSG msg, T_WPARAM wParam, T_LPARAM lParam)
    {
        // C-style case used to let compiler determine which cast is needed for specific parameter type
        return ::SendMessageW(hwnd, (UINT) msg, (WPARAM) wParam, (LPARAM) lParam);
    }

    template<typename T_MSG, typename T_WPARAM>
    /// This calls SendMessageW even if UNICODE was not defined
    LRESULT SendMsg(HWND hwnd, T_MSG msg, T_WPARAM wParam)
    {
        // C-style case used to let compiler determine which cast is needed for specific parameter type
        return ::SendMessageW(hwnd, (UINT) msg, (WPARAM) wParam, NULL);
    }

    template<typename T_MSG>
    /// This calls SendMessageW even if UNICODE was not defined
    LRESULT SendMsg(HWND hwnd, T_MSG msg)
    {
        // C-style case used to let compiler determine which cast is needed for specific parameter type
        return ::SendMessageW(hwnd, (UINT) msg, NULL, NULL);
    }

    /// Sets title to use in all calls to ttlib::MsgBox.
    ///
    /// Title will be automatically converted to UTF16.
    void SetMsgBoxTitle(std::string_view utf8Title);

    /// Converts the message to UTF16 and displays it in a Windows message box (MessageBox(...))
    ///
    /// Title (caption) is whatever was set by last call to ttlib::SetMsgBoxTitle().
    int MsgBox(std::string_view utf8str, UINT uType = MB_OK | MB_ICONWARNING);

    /// Converts the message and caption to UTF16 and displays them in a Windows message box
    /// (MessageBox(...))
    int MsgBox(std::string_view utf8str, std::string_view utf8Caption, UINT uType = MB_OK | MB_ICONWARNING);

    /// Loads the string resource and displays it in a MessageBox. Uses language and module
    /// current set in ttlib::lang_info.
    ///
    /// Title (caption) is whatever was set by last call to ttlib::SetMsgBoxTitle().
    int MsgBox(WORD idStrResource, UINT uType = MB_OK | MB_ICONWARNING);

    /// Converts window text to UTF8 and returns it in a ttlib::cstr container
    ttlib::cstr GetWndText(HWND hwnd);

    /// Converts window text to UTF8 and assigns it to str
    bool GetWndText(HWND hwnd, std::string& str);

    /// Sends LB_GETTEXT, converts the result to UTF8 and returns it in std::string.
    ///
    /// returned string will be empty() if index is invalid.
    ttlib::cstr GetListboxText(HWND hwndLB, WPARAM index);

    /// Sends LB_GETTEXT, converts the result to UTF8 and returns it in str.
    ///
    /// Returns false if index is invalid (and sets str to ttEmptyString)
    bool GetListboxText(HWND hwndLB, WPARAM index, std::string& str);

    /// Sends CB_GETLBTEXT, converts the result to UTF8 and returns it in std::string.
    ///
    /// ttlib::cstr will be empty() if index is invalid.
    ttlib::cstr GetComboLBText(HWND hwndLB, WPARAM index);

    /// Sends CB_GETLBTEXT, converts the result to UTF8 and returns it in str.
    ///
    /// Returns false if index is invalid (and sets str to ttEmptyString)
    bool GetComboLBText(HWND hwndLB, WPARAM index, std::string& str);

    /// Converts the text to UTF16 before calling SetWindowTextW(...)
    void SetWndText(HWND hwnd, std::string_view utf8str);

    /// Loads the specified UTF8 text file from a resource (calls Windows LoadResource API).
    ///
    /// Use LoadStringEx() to load a string resource from a STRINGTABLE.
    ttlib::cstr LoadTextResource(DWORD idResource, HMODULE hmodResource = NULL);

    /// Converts all text to UTF16 before calling ShellExecuteW(...)
    HINSTANCE ShellRun(std::string_view filename, std::string_view args, std::string_view directory, INT nShow = SW_SHOWNORMAL,
                       HWND hwndParent = NULL);

    /// Only available in ttLibwx.lib (wxWidgets + Windows)
    HINSTANCE ShellRun_wx(const wxString& filename, const wxString& args, const wxString& directory, INT nShow = SW_SHOWNORMAL,
                          HWND hwndParent = NULL);

    HFONT CreateLogFont(std::string_view TypeFace, size_t point, bool Bold = false, bool Italics = false);

    struct LANGINFO
    {
        HMODULE hinstResource { NULL };
        USHORT PrimaryLanguage { LANG_NEUTRAL };
        USHORT SubLanguage { SUBLANG_NEUTRAL };
    };
    extern LANGINFO lang_info;

    /// Loads the string from the resource file using language and resource module specified
    /// in ttlib::lang_info. String is converted to UTF8 before storing in Result.
    bool LoadStringEx(std::string& Result, WORD id);

    /// Loads the string from the resource file using language and resource module specified
    /// in ttlib::lang_info. Only available in ttLibwx.lib (wxWidgets + Windows).
    bool LoadStringEx(wxString& Result, WORD id);

    /// Loads the string from the resource file using language and resource module specified
    /// in ttlib::lang_info. String is converted to UTF8 before storing in Result.
    ttlib::cstr LoadStringEx(WORD id);

    /// Loads the string from the resource file using language and resource module specified
    /// in ttlib::lang_info. Only available in ttLibwx.lib (wxWidgets + Windows).
    wxString LoadStringEx_wx(WORD id);

    /// Set the resource handle and language to use for loading resources. If hinstResource
    /// is NULL, the current executable is used. Otherwise it must be the handle returned
    /// by LoadLibrary().
    void SetLangInfo(HMODULE hinstResource, USHORT PrimaryLanguage, USHORT SubLanguage);

    inline int rcHeight(const RECT* prc) { return prc->bottom - prc->top; };

    inline int rcHeight(const RECT rc) { return rc.bottom - rc.top; };

    inline int rcWidth(const RECT* prc) { return prc->right - prc->left; };

    inline int rcWidth(const RECT rc) { return rc.right - rc.left; };

    inline bool isPosInRect(const RECT* prc, int xPos, int yPos)
    {
        return (xPos >= prc->left && xPos <= prc->right && yPos >= prc->top && yPos <= prc->bottom);
    }

}  // namespace ttlib

#endif  // end _WIN32 section
