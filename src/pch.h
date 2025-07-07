/////////////////////////////////////////////////////////////////////////////
// Purpose:   Precompiled header file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// This header file is used to create a pre-compiled header for use in the entire project

#pragma once  // NOLINT(#pragma once in main file)

#if defined(_MSVC_LANG)
    #if (_MSVC_LANG < 202002L)
        #error "This project requires C++20 or later"
    #endif
#elif (__cplusplus < 202002L)
    #error "This project requires C++20 or later"
#endif

// Ensure that _DEBUG is defined in non-release builds
#if !defined(NDEBUG) && !defined(_DEBUG)
    #define _DEBUG
#endif

#if defined(_DEBUG) && !defined(INTERNAL_TESTING)
    #define INTERNAL_TESTING
#endif

#define wxUSE_GUI         1
#define wxUSE_NO_MANIFEST 1

#ifdef _MSC_VER
    #pragma warning(push)
#endif

#if defined(__clang__)
    // suppress warnings caused by wxCHECKED_DELETE
    #pragma clang diagnostic ignored "-Wunused-local-typedef"
    #pragma clang diagnostic ignored "-Wignored-attributes"
#endif

#include <wx/defs.h>  // Declarations/definitions common to all wx source files

#if defined(__WINDOWS__)
    #include <wx/msw/wrapcctl.h>  // Wrapper for the standard <commctrl.h> header

    #if wxUSE_COMMON_DIALOGS
        #include <commdlg.h>
    #endif
#endif

#if defined(_DEBUG)
    #include <wx/debug.h>  // Misc debug functions and macros
#endif

// included here so that C5054 gets disabled
// #include <wx/propgrid/propgridpagestate.h>

#include <wx/gdicmn.h>  // Common GDI classes, types and declarations
#include <wx/msgdlg.h>  // common header and base class for wxMessageDialog
#include <wx/string.h>  // wxString class

// Clang-cl needs this because propgrid.h doesn't include it
#include <wx/textctrl.h>  // wxTextAttr and wxTextCtrlBase class - the interface of wxTextCtrl

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// Without this, a huge number of #included wxWidgets header files will generate the warning
#pragma warning(disable : 4251)  // needs to have dll-interface to be used by clients of class

// REVIEW: [Randalphwa - 08-11-2023] See gen_infobar.cpp for an example. This may be fixed
// in later versions of wxWidgets, but for now, we need to disable this warning.
#pragma warning(disable : 5054)  // deprecated between enumerations of different types

#if defined(__clang__)
    // Same as #pragma warning(disable : 5054)
    #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"

    // warning: definition of implicit copy constructor for '...' is deprecated
    #pragma clang diagnostic ignored "-Wdeprecated-copy"

    //  warning: attribute declaration must precede definition
    #pragma clang diagnostic ignored "-Wignored-attributes"

    // warning: unused typedef 'complete'
    #pragma clang diagnostic ignored "-Wunused-local-typedef"

#endif

#include <map>
#include <string>

#include "tt_string.h"  // tt_string -- std::string with additional methods

#ifndef wxBITMAP_TYPE_SVG
    #define wxBITMAP_TYPE_SVG static_cast<wxBitmapType>(wxBITMAP_TYPE_ANY - 1)
#endif

// This is used around languages that are not supported by wxUiEditor. They surround code that might
// be useful if any of the languages are enabled (in which case, the specific code should be moved
// out of the conditional block.
#define GENERATE_NEW_LANG_CODE 0

// To prevent accidentally overlapping event ids, all starting values for enumerated id values
// should use one of these defines.

#define START_RIBBON_IDS      wxID_HIGHEST + 1
#define START_MAINFRAME_IDS   wxID_HIGHEST + 1000
#define START_NAVTOOL_IDS     wxID_HIGHEST + 2000
#define START_DOCVIEW_IDS     wxID_HIGHEST + 3000
#define START_TESTING_IDS     wxID_HIGHEST + 4000
#define START_IMPORT_FILE_IDS wxID_HIGHEST + 5000

enum class MoveDirection
{
    Up = 1,
    Down,
    Left,
    Right
};

// This is used to determine the type of file that is being generated. Note that the Code class only
// supports a single language at a time, and passing in multiple languages will cause it to fail to
// generate any language. As bit flags, this can be used by generators to indicate which languages
// the generator supports.
enum GenLang
{
    GEN_LANG_NONE = 0,
    GEN_LANG_CPLUSPLUS = 1,
    GEN_LANG_PERL = 1 << 2,
    GEN_LANG_PYTHON = 1 << 3,
    GEN_LANG_RUBY = 1 << 4,
    GEN_LANG_RUST = 1 << 5,

    // REVIEW: [Randalphwa - 01-10-2025] These languages are currently not supported. If you enable
    // any of them then you *MUST* change the values of GEN_LANG_XRC and GEN_LANG_XML below.

    // GEN_LANG_FORTRAN = 1 << 6,
    // GEN_LANG_HASKELL = 1 << 7,
    // GEN_LANG_LUA = 1 << 8,

    // These should always be the last languages in the list.
    GEN_LANG_XRC = 1 << 6,
    GEN_LANG_XML = 1 << 7,
};

// Used to index fields in a bitmap property
enum PropIndex
{
    IndexType = 0,
    IndexImage,
    IndexArtID = IndexImage,
    IndexSize,
};

namespace xrc
{
    enum
    {
        all_unsupported = 0,
        min_size_supported = 1 << 0,
        max_size_supported = 1 << 1,
        hidden_supported = 1 << 2,
    };

    enum
    {
        no_flags = 0,
        add_comments = 1 << 0,  // add comments prop_var_comment comments
        use_xrc_dir = 1 << 1,   // if prop_xrc_dir is set, use that instead of prop_art_directory
        previewing = 1 << 2,    // overrides add_comments and use_xrc_dir

        format_no_indent_nodes =
            1 << 3,  // do not indent nodes in the XRC file (pugi::format_indent off)
        format_indent_attributes =
            1 << 4,  // indent attributes in the XRC file (pugi::format_indent_attributes on)
        format_indent_with_spaces = 1 << 5,  // indent with spaces instead of tabs
    };
}  // namespace xrc

// When chaniging txtVersion, you also need to change the version in wxUiEditor.rc and
// wxUiEditor.exe.manifest and ../CMakeLists.txt

constexpr const char* txtVersion = "wxUiEditor 1.2.9.0";
constexpr const char* txtCopyRight = "Copyright (c) 2019-2025 KeyWorks Software";
constexpr const char* txtAppname = "wxUiEditor";

// This is the highest project number supported by this build of wxUiEditor. It should be
// updated after every release, if there are any changes to the project format that might
// require a newer version.
constexpr const int curSupportedVer = 21;

// This is the default minimum required version for all generators. It is the version used by
// the 1.0.0 release.
constexpr const int minRequiredVer = 15;

// 1.0.0 == version 15
// 1.1.0 == version 16
// 1.1.1 == version 17 (beta 1 of 1.1.2)
// 1.1.2 == version 18
// 1.2.0 == version 19
// 1.2.1 == version 20
// 1.2.9 == version 21 (beta of 1.3.0)
// 1.3.0 == version 21 (1.3.0)

// Use when you need to return an empty const tt_string&
extern tt_string tt_empty_cstr;

// Character used to separate the fields in a bitmap property
constexpr const char BMP_PROP_SEPARATOR = ';';

void MSG_INFO(const std::string& msg);
void MSG_WARNING(const std::string& msg);
void MSG_ERROR(const std::string& msg);

//////////////////////////////////////// macros ////////////////////////////////////////

#include "assertion_dlg.h"  // Assertion Dialog
#include "to_casts.h"       // to_int -- Smart Numeric Casts

#if defined(NDEBUG) && !defined(INTERNAL_TESTING)
    #define CHECK2_MSG(cond, op, msg) wxASSERT_MSG(cond, msg)
    #define CHECK_MSG(cond, rc, msg)  wxCHECK2_MSG(cond, return rc, msg)
    #define CHECK(cond, rc)           wxCHECK2_MSG(cond, return rc, (const char*) nullptr)
    #define CHECK2(cond, op)          wxCHECK2_MSG(cond, op, (const char*) nullptr)
    #define CHECK_RET(cond, msg)      wxCHECK2_MSG(cond, return, msg)
#else

// These are essentially the same as the wxWidgets macros except that it calls AssertionDlg instead
// of wxFAIL_COND_MSG

    #define CHECK2_MSG(cond, op, msg)                                     \
        if (cond)                                                         \
        {                                                                 \
        }                                                                 \
        else                                                              \
        {                                                                 \
            if (AssertionDlg(__FILE__, __func__, __LINE__, #cond, (msg))) \
            {                                                             \
                wxTrap();                                                 \
            }                                                             \
            op;                                                           \
        }                                                                 \
        struct wxDummyCheckStruct /* just to force a semicolon */

    #define CHECK_MSG(cond, rc, msg) CHECK2_MSG(cond, return rc, msg)
    #define CHECK(cond, rc)          CHECK2_MSG(cond, return rc, (const char*) nullptr)
    #define CHECK2(cond, op)         CHECK2_MSG(cond, op, (const char*) nullptr)
    #define CHECK_RET(cond, msg)     CHECK2_MSG(cond, return, msg)

#endif  // defined(NDEBUG) && !defined(INTERNAL_TESTING)

// While this file does change frequently, when it does almost every file in the project needs to be
// recompiled. Since the files have to be recompiled anyway, we might as well pre-compile this file.
#include "gen_enums.h"  // Enumerations used by the generators

// This file changes rarely, but like gen_enums.h, when it does change, a large number of files in
// the project need to be recompiled (currently 214 out of 274). The Node class is an integral part
// of all the generators, as well as most of the Panels so it makes sense to pre-compile this file.
#include "node.h"  // Node class
