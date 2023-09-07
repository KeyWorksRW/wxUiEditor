/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
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

#include <wx/config.h>

#include "mainapp.h"     // MainApp

#include "main_frame.h"  // CMainFrame

wxIMPLEMENT_APP(MainApp);

bool MainApp::OnInit()
{
#if defined(wxUSE_ON_FATAL_EXCEPTION)
    ::wxHandleFatalExceptions(true);
#endif

    SetAppDisplayName(txtAppname);
    SetVendorName("KeyWorks");

    m_mainframe = new MainFrame(nullptr);
    m_mainframe->Show(true);
    return true;
}

int MainApp::OnExit()
{
    return wxApp::OnExit();
}
