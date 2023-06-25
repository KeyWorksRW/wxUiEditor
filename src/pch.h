/////////////////////////////////////////////////////////////////////////////
// Purpose:   Precompiled header file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// This header file is used to create a pre-compiled header for use in the entire project

#pragma once  // NOLINT(#pragma once in main file)

// Ensure that _DEBUG is defined in non-release builds
#if !defined(NDEBUG) && !defined(_DEBUG)
    #define _DEBUG
#endif

#if defined(_DEBUG) && !defined(INTERNAL_TESTING)
    #define INTERNAL_TESTING
#endif

#define wxUSE_GUI         1
#define wxUSE_NO_MANIFEST 1

// Allows tt additions to pugixml (e.g. as_sview(), as_cstr(), etc.)
#define TTLIB_ADDITIONS 1

#ifdef _MSC_VER
    #pragma warning(push)
#endif

#if defined(__clang__)
    // suppress warnings caused by wxCHECKED_DELETE
    #pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif

#include <wx/defs.h>  // Declarations/definitions common to all wx source files

#if !wxCHECK_VERSION(3, 2, 0)
    #error "You must have wxWidgets 3.2.0 or later to build this project."
#endif

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

#include <map>
#include <unordered_map>

#include <set>
#include <unordered_set>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "tt/tt.h"  // tt namespace functions and declarations

#include "tt/tt_string.h"         // tt_string -- std::string with additional methods
#include "tt/tt_string_vector.h"  // tt_string_vector -- Class for reading and writing line-oriented strings/files
#include "tt/tt_string_view.h"    // tt_string_view -- std::string_view with additional methods
#include "tt/tt_view_vector.h"    // tt_view_vector -- Class for reading and writing line-oriented strings/files
#include "tt/tt_wxString.h"       // tt_wxString -- wxString with additional methods similar to tt_string

enum class MoveDirection
{
    Up = 1,
    Down,
    Left,
    Right
};

// This is used to determine the type of file that is being generated
enum
{
    GEN_LANG_NONE,
    GEN_LANG_CPLUSPLUS,
    GEN_LANG_PYTHON,
    GEN_LANG_XRC,
};

namespace xrc
{
    enum : size_t
    {
        all_unsupported = 0,
        min_size_supported = 1 << 0,
        max_size_supported = 1 << 1,
        hidden_supported = 1 << 2,
    };

    enum : size_t
    {
        no_flags = 0,
        add_comments = 1 << 0,  // add comments about unsupported properties
        use_xrc_dir = 1 << 1,   // if prop_xrc_dir is set, use that instead of prop_art_directory
        previewing = 1 << 2,    // overrides add_comments and use_xrc_dir
    };
}  // namespace xrc

// When chaniging txtVersion, you also need to change the version in wxUiEditor.rc and
// wxUiEditor.exe.manifest and ../CMakeLists.txt

constexpr const char* txtVersion = "wxUiEditor 1.1.2";
constexpr const char* txtCopyRight = "Copyright (c) 2019-2023 KeyWorks Software";
constexpr const char* txtAppname = "wxUiEditor";

// This is the highest project number supported by this build of wxUiEditor. It should be
// updated after every release, if there are any changes to the project format that might
// require a newer version.
constexpr const int curSupportedVer = 18;

// This is the default minimum required version for all generators. It is the version used by
// the 1.0.0 release.
constexpr const int minRequiredVer = 15;

// 1.0.0 == version 15
// 1.1.0 == version 16
// 1.1.1 == version 17

// Use when you need to return an empty const tt_string&
extern tt_string tt_empty_cstr;

// Character used to separate the fields in a bitmap property
constexpr const char BMP_PROP_SEPARATOR = ';';

//////////////////////////////////////// macros ////////////////////////////////////////

#if defined(NDEBUG) && !defined(INTERNAL_TESTING)

    #define MSG_INFO(msg)
    #define MSG_EVENT(msg)
    #define MSG_WARNING(msg)
    #define MSG_ERROR(msg)

    // Use this macro to comment out parameters that are not used in Release builds
    #define TESTING_PARAM(param) /* param */

#else

// These messages can be individually enabled/disabled in the Preferences dialog (Debug tab).
// Note that none of these are displayed in a Release build.

    #include "internal/msg_logging.h"  // MsgLogging -- Message logging class

    #define MSG_INFO(msg)                   \
        {                                   \
            g_pMsgLogging->AddInfoMsg(msg); \
        }
    #define MSG_EVENT(msg)                   \
        {                                    \
            g_pMsgLogging->AddEventMsg(msg); \
        }
    #define MSG_WARNING(msg)                   \
        {                                      \
            g_pMsgLogging->AddWarningMsg(msg); \
        }
    #define MSG_ERROR(msg)                   \
        {                                    \
            g_pMsgLogging->AddErrorMsg(msg); \
        }

    // Use this macro to comment out parameters that are not used in Release builds
    #define TESTING_PARAM(param) param

#endif  // defined(NDEBUG) && !defined(INTERNAL_TESTING)

#include "assertion_dlg.h"  // Assertion Dialog
#include "to_casts.h"       // to_int -- Smart Numeric Casts

#if defined(NDEBUG) && !defined(INTERNAL_TESTING)
    #define CHECK2_MSG(cond, op, msg) wxASSERT_MSG(cond, msg)
    #define CHECK_MSG(cond, rc, msg)  wxCHECK2_MSG(cond, return rc, msg)
    #define CHECK(cond, rc)           wxCHECK2_MSG(cond, return rc, (const char*) nullptr)
    #define CHECK2(cond, op)          wxCHECK2_MSG(cond, op, (const char*) nullptr)
    #define CHECK_RET(cond, msg)      wxCHECK2_MSG(cond, return, msg)
#else

// These are essentially the same as the wxWidgets macros except that it calls AssertionDlg instead of
// wxFAIL_COND_MSG

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
