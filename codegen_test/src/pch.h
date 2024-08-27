/////////////////////////////////////////////////////////////////////////////
// Purpose:   Precompiled header file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// This header file is used to create a pre-compiled header for use in the entire project

#pragma once  // NOLINT(#pragma once in main file)

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
#include <wx/string.h>  // wxString class

// Clang-cl needs this because propgrid.h doesn't include it
#include <wx/textctrl.h>  // wxTextAttr and wxTextCtrlBase class - the interface of wxTextCtrl

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// Without this, a huge number of #included wxWidgets header files will generate the warning
#pragma warning(disable : 4251)  // needs to have dll-interface to be used by clients of class
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
