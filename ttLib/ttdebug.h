/////////////////////////////////////////////////////////////////////////////
// Name:      ttdebug.h
// Purpose:   ttASSERT macros
// Author:    Ralph Walden
// Copyright: Copyright (c) 2000-2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// Under a _DEBUG build, these ASSERTS will display a message box giving you the
// option to ignore the assert, break into a debugger, or exit the program.

// Note that all functions listed here are available in release builds. Only the macros are removed in release
// builds

#pragma once

#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
    #error "The contents of <ttdebug.h> are available only with C++17 or later."
#endif

// With #pragma once, a header guard shouldn't be necessary and causes unwanted indentation by clang-format. The
// following #if/#endif check verifies that the file wasn't read twice.

#if defined(_TT_LIB_DEBUG_H_GUARD_)
    #error "#pragma once failed -- header is being read a second time!"
#else
    #define _TT_LIB_DEBUG_H_GUARD_  // sanity check to confirm that #pragma once is working as expected
#endif

#if defined(_WIN32)

    #include <sstream>

    #include "ttcview.h"

namespace ttlib
{
    /// The following messages are passed to ttlib::wintrace() to talk to the ttTrace.exe app and include a text string.

    /// The message names are modeled after the messages sent by KeyHelp. However, you can use them however you want.
    /// The ttTrace Window has a Preferences dialog that controls which messages are displayed. By default, all messages
    /// are displayed, but you can shut off the ones you don't currently want to see.

    /// This message is saved so that if it the exact same message is sent more than once,
    /// then only the first one will be displayed until a different message is sent.
    constexpr const unsigned int WMP_TRACE_GENERAL = (WM_USER + 0x1f3);

    constexpr const unsigned int WMP_TRACE_EVENT = (WM_USER + 0x1f4);
    constexpr const unsigned int WMP_TRACE_WARNING = (WM_USER + 0x1f5);
    constexpr const unsigned int WMP_TRACE_PROPERTY = (WM_USER + 0x1f6);
    constexpr const unsigned int WMP_TRACE_SCRIPT = (WM_USER + 0x1f7);
    constexpr const unsigned int WMP_TRACE_ERROR = (WM_USER + 0x1f8);

    constexpr const unsigned int WMP_SET_TITLE = (WM_USER + 0x1fa);  // set ttTrace window title

    /// Displays message in ttTrace.exe window if it is running. By default, this will not
    /// show the same msg string more than once until a different string is sent.
    void wintrace(const std::string& msg, unsigned int type = WMP_TRACE_GENERAL);

    /// The following messages are sent to ttlib::wintrace without a text string.

    constexpr const unsigned int WMP_CLEAR_TRACE = (WM_USER + 0x1f9);  // clears the ttTrace window

    constexpr const unsigned int WMP_HIDE_GENERAL = (WM_USER + 0x1fb);
    constexpr const unsigned int WMP_HIDE_EVENT = (WM_USER + 0x1fc);
    constexpr const unsigned int WMP_HIDE_WARNING = (WM_USER + 0x1fd);
    constexpr const unsigned int WMP_HIDE_PROPERTY = (WM_USER + 0x1fe);
    constexpr const unsigned int WMP_HIDE_SCRIPT = (WM_USER + 0x1ff);
    constexpr const unsigned int WMP_HIDE_ERROR = (WM_USER + 0x200);

    constexpr const unsigned int WMP_SHOW_GENERAL = (WM_USER + 0x201);
    constexpr const unsigned int WMP_SHOW_EVENT = (WM_USER + 0x202);
    constexpr const unsigned int WMP_SHOW_WARNING = (WM_USER + 0x203);
    constexpr const unsigned int WMP_SHOW_PROPERTY = (WM_USER + 0x204);
    constexpr const unsigned int WMP_SHOW_SCRIPT = (WM_USER + 0x205);
    constexpr const unsigned int WMP_SHOW_ERROR = (WM_USER + 0x206);

    /// Used by the ttTRACE_LAUNCH() macro to launch ttTrace.exe if it isn't already running
    constexpr const unsigned int WMP_LAUNCH_TRACE = (WM_USER + 0x250);

    /// Use this to send CLEAR, HIDE, or SHOW messages that don't include any text
    void wintrace(unsigned int type = WMP_CLEAR_TRACE);

    /// handle to the ttTrace main window
    extern HWND hwndTrace;

    /// class name of window to send trace messages to
    extern const char* txtTraceClass;

    /// name of shared memory to write to
    extern const char* txtTraceShareName;

    // Called by ttDISABLE_ASSERTS and ttENABLE_ASSERTS macros in Debug builds
    void allow_asserts(bool allowasserts = false);

    /// By default, duplicate assert messages are displayed. Set to false to prevent any previously seen assert.
    ///
    /// If you call this with true, all previously seen asserts will be cleared.
    void duplicate_asserts(bool allowDuplicates = true);

    int CheckItemID(HWND hwnd, int id, const char* pszID, const char* pszFile, const char* pszFunc, int line);
}  // namespace ttlib

bool ttAssertionMsg(const char* filename, const char* function, int line, const char* cond, const std::string& msg);
bool ttdoReportLastError(const char* filename, const char* function, int line);

inline bool ttAssertionMsg(const char* filename, const char* function, int line, const char* cond, const std::stringstream& msg)
{
    return ttAssertionMsg(filename, function, line, cond, msg.str().c_str());
}

#endif  // end of Windows-only section.

// Following section is so that tt macros will be removed in BOTH Release and all non-Windows builds

#if !defined(NDEBUG) && defined(_WIN32)
    #define ttASSERT(cond)                                                          \
        {                                                                           \
            if (!(cond) && ttAssertionMsg(__FILE__, __func__, __LINE__, #cond, "")) \
            {                                                                       \
                __debugbreak();                                                     \
            }                                                                       \
        }

    #define ttASSERT_MSG(cond, msg)                                                  \
        {                                                                            \
            if (!(cond) && ttAssertionMsg(__FILE__, __func__, __LINE__, #cond, msg)) \
            {                                                                        \
                __debugbreak();                                                      \
            }                                                                        \
        }

    #define ttFAIL(msg)                                                 \
        if (ttAssertionMsg(__FILE__, __func__, __LINE__, nullptr, msg)) \
        {                                                               \
            __debugbreak();                                             \
        }

    #define ttFAIL_MSG(msg)                                                 \
        {                                                                   \
            if (ttAssertionMsg(__FILE__, __func__, __LINE__, nullptr, msg)) \
            {                                                               \
                __debugbreak();                                             \
            }                                                               \
        }

    #define ttASSERT_HRESULT(hr, msg)                                                     \
        {                                                                                 \
            if (FAILED(hr) && ttAssertionMsg(__FILE__, __func__, __LINE__, nullptr, msg)) \
            {                                                                             \
                __debugbreak();                                                           \
            }                                                                             \
        }

    #define ttASSERT_NONEMPTY(psz)                                                                               \
        {                                                                                                        \
            if ((!psz || !*psz) && ttAssertionMsg(__FILE__, __func__, __LINE__, #psz, "Null or empty pointer!")) \
            {                                                                                                    \
                __debugbreak();                                                                                  \
            }                                                                                                    \
        }

    #define ttASSERT_STRING(str)                                                                    \
        {                                                                                           \
            if (str.empty() && ttAssertionMsg(__FILE__, __func__, __LINE__, #str, "Empty string!")) \
            {                                                                                       \
                __debugbreak();                                                                     \
            }                                                                                       \
        }

    /// In _DEBUG builds this will display an assertion dialog first then it will throw
    /// an excpetion. In Release builds, only the exception is thrown.
    #define ttTHROW(msg)                                                    \
        {                                                                   \
            if (ttAssertionMsg(__FILE__, __func__, __LINE__, nullptr, msg)) \
            {                                                               \
                __debugbreak();                                             \
            }                                                               \
            throw msg;                                                      \
        }

    /// Display the last system error from GetLastError()
    #define ttLAST_ERROR()                                         \
        {                                                          \
            if (ttdoReportLastError(__FILE__, __func__, __LINE__)) \
            {                                                      \
                __debugbreak();                                    \
            }                                                      \
        }

    // This still executes the expression in non-DEBUG builds, it just doesn't check the result.
    #define ttVERIFY(exp) (void) ((!!(exp)) || ttAssertionMsg(__FILE__, __func__, __LINE__, #exp, ""))

    /// Causes all calls to ttAssertionMsg to immediately return.
    #define ttDISABLE_ASSERTS() ttlib::allow_asserts(false)

    /// Causes ttAssertionMsg to run normally
    #define ttENABLE_ASSERTS() ttlib::allow_asserts(true)

    /// All ttTRACE macros are automatically removed in Release builds. Call ttlib::wintrace()
    /// directly if you need tracing in a release build.
    #define ttTRACE(msg)         ttlib::wintrace(msg, ttlib::WMP_TRACE_GENERAL)
    #define ttTRACE_ERROR(msg)   ttlib::wintrace(msg, ttlib::WMP_TRACE_ERROR)
    #define ttTRACE_WARNING(msg) ttlib::wintrace(msg, ttlib::WMP_TRACE_WARNING)

    #define ttTRACE_EVENT(msg)    ttlib::wintrace(msg, ttlib::WMP_TRACE_EVENT)
    #define ttTRACE_PROPERTY(msg) ttlib::wintrace(msg, ttlib::WMP_TRACE_PROPERTY)
    #define ttTRACE_SCRIPT(msg)   ttlib::wintrace(msg, ttlib::WMP_TRACE_SCRIPT)

    #define ttTRACE_CLEAR()    ttlib::wintrace(ttlib::WMP_CLEAR_TRACE)
    #define ttTRACE_TITLE(msg) ttlib::wintrace(msg, ttlib::WMP_SET_TITLE)

    /// Use this to send any of the WMP_SHOW_... or WMP_HIDE... messages.
    #define ttTRACE_FILTER(type) ttlib::wintrace(type)

    /// This will try to locate the window for ttTrace.exe, and attempt to launch it if
    /// the window is not found.
    #define ttTRACE_LAUNCH() ttlib::wintrace(ttlib::WMP_LAUNCH_TRACE)

#else  // Release build or non-Windows build

    #define ttASSERT(cond)
    #define ttASSERT_MSG(cond, msg)
    #define ttFAIL(msg)
    #define ttFAIL_MSG(msg)

    #define ttVERIFY(exp) ((void) (exp))

    #define ttTRACE(msg)
    #define ttTRACE_ERROR(msg)
    #define ttTRACE_WARNING(msg)

    #define ttTRACE_EVENT(msg)
    #define ttTRACE_PROPERTY(msg)
    #define ttTRACE_SCRIPT(msg)

    #define ttTRACE_CLEAR()
    #define ttTRACE_TITLE(msg)

    #define ttTRACE_FILTER(type)

    #define ttTRACE_LAUNCH()

    #define ttASSERT_NONEMPTY(ptr)
    #define ttASSERT_STRING(str)

    #define ttDISABLE_ASSERTS()
    #define ttENABLE_ASSERTS()

    #define ttASSERT_HRESULT(hr, pszMsg)
    #define ttLAST_ERROR()

    #define ttTHROW(msg) \
        {                \
            throw msg;   \
        }

#endif  // End Release/_WIN32 conditional
