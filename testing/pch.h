// This header file is used to create a pre-compiled header for use in the entire project

#pragma once

#define wxUSE_UNICODE     1
#define wxUSE_GUI         1
#define wxUSE_NO_MANIFEST 1  // This is required for compiling using CLANG 9 and earlier

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

constexpr const auto txtVersion = "wxUiTesting 1.0.0";
constexpr const auto txtCopyRight = "Copyright (c) 2020 KeyWorks Software (Ralph Walden)";
constexpr const auto txtAppname = "wxUiTesting";
