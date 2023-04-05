/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
    #if defined(_WIN32)

        #pragma comment(lib, "kernel32.lib")
        #pragma comment(lib, "user32.lib")
        #pragma comment(lib, "gdi32.lib")
        #pragma comment(lib, "comctl32.lib")
        #pragma comment(lib, "comdlg32.lib")
        #pragma comment(lib, "shell32.lib")

        #pragma comment(lib, "rpcrt4.lib")
        #pragma comment(lib, "advapi32.lib")

        #if wxUSE_URL_NATIVE
            #pragma comment(lib, "wininet.lib")
        #endif
    #endif
#endif

#include <wx/cshelp.h>

#include "mainapp.h"  // MainApp

#include "mainframe.h"  // MainFrame -- Main window

wxIMPLEMENT_APP(MainApp);

#if defined(_WIN32) && defined(_DEBUG)

// Because of the extensive use of share_ptrs and the number of classes that are created during initialization and never
// destroyed, both the original wxFormBuilder as well as this code base will have a lot of shared_ptrs that are completely
// dereferenced when the app exits, leading to a long and time-comsuming display of "leaked" memory when run under a
// Microsoft debugger. While it would be ideal if all those shared_ptrs were freed, it's a low priority to write a bunch of
// code to walk through all the maps and vectors in order to free the pointers.

// Uncomment the following to turn on a memory leak dump on exit.
// #define USE_CRT_MEMORY_DUMP

#endif  // _WIN32 && defined(_DEBUG)

bool MainApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

#if defined(_WIN32) && defined(_DEBUG)
    #if !defined(USE_CRT_MEMORY_DUMP)

    _CrtSetDbgFlag(0);

    #else

    // If memory dump shows a leak, uncomment the following and use the memory dump number you want to check. Works with
    // Visual Studio Debugger, does not work with Visual Studio Code debugger.

    // _CrtSetBreakAlloc(25045);
    #endif
#endif

#if defined(wxUSE_ON_FATAL_EXCEPTION)
    ::wxHandleFatalExceptions(true);
#endif

    // If we're just providing text-popups for help, then this is all we need.
    wxHelpProvider::Set(new wxSimpleHelpProvider);

    SetAppDisplayName(txtAppname);
    SetVendorName("KeyWorks");

    return true;
}

int MainApp::OnRun()
{
    m_mainframe = new MainFrame(nullptr);
    m_mainframe->Show(true);

    return wxApp::OnRun();
}

int MainApp::OnExit()
{
    return wxApp::OnExit();
}
