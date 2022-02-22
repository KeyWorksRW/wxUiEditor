/////////////////////////////////////////////////////////////////////////////
// Purpose:   Precompiled header file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// This header file is used to create a pre-compiled header for use in the entire project

#pragma once

#define wxUSE_GUI         1
#define wxUSE_NO_MANIFEST 1
#define wxUSE_UNICODE     1

#ifdef _MSC_VER
    #pragma warning(push)
#endif

#include <wx/defs.h>  // Declarations/definitions common to all wx source files

#if !wxCHECK_VERSION(3, 1, 5)
    #error "You must have wxWidgets 3.1.5 or later to build this project."
#endif

#if defined(__WINDOWS__)
    #include <wx/msw/wrapcctl.h>  // Wrapper for the standard <commctrl.h> header

    #if wxUSE_COMMON_DIALOGS
        #include <commdlg.h>
    #endif
#endif

// Ensure that _DEBUG is defined in non-release builds
#if !defined(NDEBUG) && !defined(_DEBUG)
    #define _DEBUG
#endif

#include <wx/debug.h>  // Misc debug functions and macros

#if !wxCHECK_VERSION(3, 1, 6)
    #if (__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
        #ifdef _MSC_VER
            // This *IS* a legitimate warning, however while wxWidgets 3.1.15 has made some progress, there are still header
            // files that do this.
            #pragma warning(disable : 5054)  // operator '|': deprecated between enumerations of different types
        #endif
    #endif
#endif

// These warnings are still generated in 3.1.16

#ifdef _MSC_VER
    #pragma warning(disable : 4267)  // conversion from 'size_t' to 'int', possible loss of data
    #pragma warning(disable : 4244)  // conversion from 'size_t' to 'int', possible loss of data
#endif

#if !defined(_WIN32) || defined(__clang__)
    // warning: unused typedef 'complete' in scopedptr.h
    #pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif

// We include these here so that C4244 and C4267 get disabled
#include <wx/choicebk.h>
#include <wx/htmllbox.h>
#include <wx/richtext/richtextbuffer.h>

#include <wx/gdicmn.h>  // Common GDI classes, types and declarations
#include <wx/msgdlg.h>  // common header and base class for wxMessageDialog
#include <wx/string.h>  // wxString class

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

#include "ttlibspace.h"  // This must be included before any other ttLib header files

#include "ttcstr.h"   // ttlib::cstr -- std::string with additional functions
#include "ttcview.h"  // ttlib::cview -- string_view functionality on a zero-terminated char string.
#include "ttstr.h"    // ttString -- wxString with ttlib::cstr equivalent functions
#include "ttsview.h"  // sview -- std::string_view with additional methods

#if !defined(int_t)

// signed integer type with width determined by platform
typedef ptrdiff_t int_t;

#endif  // not !defined(int_t)

enum class MoveDirection
{
    Up = 1,
    Down,
    Left,
    Right
};

constexpr const char* txtVersion = "wxUiEditor 0.9.1";
constexpr const char* txtCopyRight = "Copyright (c) 2019-2021 KeyWorks Software";
constexpr const char* txtAppname = "wxUiEditor";

// Use when you need to return an empty const ttlib::cstr&
extern ttlib::cstr tt_empty_cstr;

// Character used to separate the fields in a bitmap property
constexpr const char BMP_PROP_SEPARATOR = ';';

//////////////////////////////////////// macros ////////////////////////////////////////

// Use INTERNAL_ERROR(msg) if you always want to report a problem in any Debug or Release build. Use BETA_ERROR(msg) if this
// is just something you want reported on during a Debug or Release beta build (which presumably will be fixed and not occur
// before a non-beta Release). You do not need to call these if you are instead calling throw std::...()

#if defined(BETA)
    #define BETA_ERROR(msg)                                                                                              \
        {                                                                                                                \
            wxMessageBox(wxString("An internal error occured in ") << __func__ << " at line " << __LINE__ << '.' << msg, \
                         txtVersion);                                                                                    \
        }
#else
    #define BETA_ERROR(msg) \
        {                   \
        }
#endif

#define INTERNAL_ERROR(msg)                                                                                          \
    {                                                                                                                \
        wxMessageBox(wxString("An internal error occured in ") << __func__ << " at line " << __LINE__ << '.' << msg, \
                     txtVersion);                                                                                    \
    }

#if defined(NDEBUG)

    #define MSG_INFO(msg)
    #define MSG_EVENT(msg)
    #define MSG_WARNING(msg)
    #define MSG_ERROR(msg)

#else  // not defined(NDEBUG)

// These messages can be individually enabled/disabled in the Preferences dialog (Debug tab).
// Note that none of these are displayed in a Release build.

    #include "debugging/msg_logging.h"  // MsgLogging -- Message logging class

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

#endif  // defined(NDEBUG)

#if defined(NDEBUG)

    #define ASSERT(cond)
    #define ASSERT_MSG(cond, msg)
    #define FAIL_MSG(msg)

#else  // Debug build only

    #if defined(_WIN32) && !defined(NONWIN_TEST)

bool ttAssertionMsg(const char* filename, const char* function, int line, const char* cond, const std::string& msg);

    // Unlike the wxASSERT macros, these will provide an Abort button allowing you to immediately terminate the
    // application. The assertion information is easier to read, but does not provide a call stack or the ability to
    // ignore the assertion.

        #define ASSERT(cond)                                                            \
            {                                                                           \
                if (!(cond) && ttAssertionMsg(__FILE__, __func__, __LINE__, #cond, "")) \
                {                                                                       \
                    wxTrap();                                                           \
                }                                                                       \
            }

        #define ASSERT_MSG(cond, msg)                                                  \
            if (!(cond) && ttAssertionMsg(__FILE__, __func__, __LINE__, #cond, (msg))) \
            {                                                                          \
                wxTrap();                                                              \
            }

        #define FAIL_MSG(msg)                                               \
            if (ttAssertionMsg(__FILE__, __func__, __LINE__, nullptr, msg)) \
            {                                                               \
                wxTrap();                                                   \
            }

    #else  // not defined(_WIN32)
        #define ASSERT(cond)          wxASSERT(cond)
        #define ASSERT_MSG(cond, msg) wxASSERT_MSG(cond, msg)
        #define FAIL_MSG(msg)         wxFAIL_MSG(msg)
    #endif  // _WIN32

#endif  // defined(NDEBUG)

#if defined(_WIN32) && !defined(NONWIN_TEST)
// These are essentially the same as the wxWidgets macros except that it calls ttAssertionMsg instead of
// wxFAIL_COND_MSG

    #define CHECK2_MSG(cond, op, msg)                                       \
        if (cond)                                                           \
        {                                                                   \
        }                                                                   \
        else                                                                \
        {                                                                   \
            if (ttAssertionMsg(__FILE__, __func__, __LINE__, #cond, (msg))) \
            {                                                               \
                wxTrap();                                                   \
            }                                                               \
            op;                                                             \
        }                                                                   \
        struct wxDummyCheckStruct /* just to force a semicolon */

    #define CHECK_MSG(cond, rc, msg) CHECK2_MSG(cond, return rc, msg)
    #define CHECK(cond, rc)          CHECK2_MSG(cond, return rc, (const char*) nullptr)
    #define CHECK2(cond, op)         CHECK2_MSG(cond, op, (const char*) nullptr)
    #define CHECK_RET(cond, msg)     CHECK2_MSG(cond, return, msg)

#else
    #define CHECK2_MSG(cond, op, msg) wxASSERT_MSG(cond, msg)
    #define CHECK_MSG(cond, rc, msg)  wxCHECK2_MSG(cond, return rc, msg)
    #define CHECK(cond, rc)           wxCHECK2_MSG(cond, return rc, (const char*) nullptr)
    #define CHECK2(cond, op)          wxCHECK2_MSG(cond, op, (const char*) nullptr)
    #define CHECK_RET(cond, msg)      wxCHECK2_MSG(cond, return, msg)
#endif
