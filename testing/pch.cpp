// Thie file is used to create a pre-compiled header for use in the entire project (required by MSVC)

#if defined(_WIN32)
    #define wxMSVC_VERSION_AUTO
#endif

#include "pch.h"

#ifdef _MSC_VER
    #define wxMSVC_VERSION_ABI_COMPAT
    #include <msvc/wx/setup.h>  // This will add #pragmas for the wxWidgets libraries

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
