// This header file is used to create a pre-compiled header for use in the entire project

#pragma once

// Ensure that _DEBUG is defined in non-release builds
#if !defined(NDEBUG) && !defined(_DEBUG)
    #define _DEBUG
#endif

#if defined(_DEBUG) && !defined(INTERNAL_TESTING)
    #define INTERNAL_TESTING
#endif

#define wxUSE_UNICODE     1
#define wxUSE_GUI         1
#define wxUSE_NO_MANIFEST 1

#ifdef _MSC_VER
    #pragma warning(push)
#endif

#include "wx/defs.h"  // compiler detection; includes setup.h

#if !wxCHECK_VERSION(3, 1, 0)
    #error "You must have wxWidgets 3.1.0 or later to build this project."
#endif

#if defined(__WINDOWS__)
    #include "wx/msw/wrapcctl.h"

    #if wxUSE_COMMON_DIALOGS
        #include <commdlg.h>
    #endif
#endif

#include <wx/gdicmn.h>  // Common GDI classes, types and declarations
#include <wx/msgdlg.h>  // common header and base class for wxMessageDialog
#include <wx/string.h>  // wxString class

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// Without this, a huge number of #included wxWidgets header files will generate the warning
#pragma warning(disable : 4251)  // needs to have dll-interface to be used by clients of class

inline constexpr const auto txtVersion = "QuickTest 1.0.0";
inline constexpr const auto txtCopyRight = "Copyright (c) 2023 KeyWorks Software (Ralph Walden)";
inline constexpr const auto txtAppname = "QuickTest";
