// This header file is used to create a pre-compiled header for use in the entire project

#pragma once

#define wxUSE_UNICODE     1
#define wxUSE_GUI         1
#define wxUSE_NO_MANIFEST 1  // This is required for compiling using CLANG 9 and earlier

// This *IS* a legitimate warning, however while wxWidgets 3.1.6 has made some progress, there are still header files that
// do this, and of course we can't assume the user is compiling with a version of wxWidgets where it has been fixed.

#ifdef _MSC_VER
    #pragma warning(disable : 5054)  // operator '|': deprecated between enumerations of different types
#endif

#ifdef _MSC_VER
    #pragma warning(push)
#endif

#include <wx/defs.h>

#if defined(__WINDOWS__)
    #include <wx/msw/wrapcctl.h>

    #if wxUSE_COMMON_DIALOGS
        #include <commdlg.h>
    #endif
#endif

// Ensure that _DEBUG is defined in non-release builds
#if !defined(NDEBUG) && !defined(_DEBUG)
    #define _DEBUG
#endif

// The following are common enough that we include them here rather than in every (or most) source files

#include <wx/debug.h>
#include <wx/msgdlg.h>
#include <wx/string.h>

#ifdef _MSC_VER
// These warnings are still generated in 3.1.16

    #pragma warning(disable : 4267)  // conversion from 'size_t' to 'int', possible loss of data
    #pragma warning(disable : 4244)  // conversion from 'size_t' to 'int', possible loss of data
#endif

// We include these here so that C4244 and C4267 get disabled
#include <wx/choicebk.h>                 // conversion from 'int' to 'wxTextAttrDimensionFlags', possible loss of data
#include <wx/richtext/richtextbuffer.h>  // conversion from 'int' to 'wxTextAttrDimensionFlags', possible loss of data
#include <wx/richtext/richtextctrl.h>    // conversion from 'int' to 'wxTextAttrDimensionFlags', possible loss of data

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// Without this, a huge number of #included wxWidgets header files will generate the warning
#pragma warning(disable : 4251)  // needs to have dll-interface to be used by clients of class

// sub-number if the version of wxWidgets this is built on
constexpr const auto txtVersion = "cpptest 1.1.320";
constexpr const auto txtCopyRight = "Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)";
constexpr const auto txtAppname = "cpptest";
