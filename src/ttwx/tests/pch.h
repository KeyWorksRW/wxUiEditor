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

#include <wx/gdicmn.h>  // Common GDI classes, types and declarations
#include <wx/msgdlg.h>  // common header and base class for wxMessageDialog
#include <wx/string.h>  // wxString class

// Clang-cl needs this because propgrid.h doesn't include it
#include <wx/textctrl.h>  // wxTextAttr and wxTextCtrlBase class - the interface of wxTextCtrl

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#if !(wxUSE_UNICODE_UTF8 && wxUSE_UTF8_LOCALE_ONLY)
    #error "wxUSE_UNICODE_UTF8 and wxUSE_UTF8_LOCALE_ONLY must be enabled for this project."
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

    // include\wx/propgrid/propgrid.h(123,9): warning: unknown pragma ignored [-Wunknown-pragmas]
    #pragma clang diagnostic ignored "-Wunknown-pragmas"

#endif

// Mapping isn't used in this header file, but is used in so many source files that it makes sense
// to read it into the pre-compiled header. The ""IWYU pragma: keep"" comment shuts off the clangd
// warning about an unused include.
#include <map>  // IWYU pragma: keep

#include <string>       // IWYU pragma: keep
#include <string_view>  // IWYU pragma: keep
#include <vector>       // IWYU pragma: keep

// ttwx namespace functions, classes, and declarations
#include "ttwx.h"  // IWYU pragma: keep

#define ASSERT(cond) wxASSERT_MSG(cond, "Assertion failed")
#define ASSERT_MSG(cond, msg) wxASSERT_MSG(cond, msg)
#define FAIL_MSG(msg) wxFAIL_MSG(msg)
