/////////////////////////////////////////////////////////////////////////////
// Purpose:   Assertion Dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License ( see ../LICENSE )
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

// Note that while it is considered more "modern" there's nothing in the current code base for
// wxUiEditor that would benefit from using std::source_location.

auto AssertionDlg(const char* filename, const char* function, int line, const char* cond,
                  const wxString& msg) -> bool;
void ttAssertionHandler(const wxString& filename, int line, const wxString& function,
                        const wxString& cond, const wxString& msg);

// Saves assertion/crash details to a user-chosen log file via wxFileDialog.
// Used by OnAssertFailure() in release+testing builds and by the assertion dialog.
void SaveAssertionInfo(const wxString& content);

// The advantage of using ASSERT over wxASSERT is that ASSERT allows the macro to execute wxTrap in
// the caller's code, so that you don't have to step out of the assertion function to get back to
// the code that threw the assert.

// Also note that wxASSERT will still be compiled in release builds unless setup.h specifically
// disables it. By default, wxASSERT is enabled in release builds for wxWidgets, which would mean it
// would compile in wxUiEditor code as well. Using ASSERT means it is only compiled into Release
// builds if INTERNAL_TESTING is set.

#if defined(NDEBUG) && !defined(INTERNAL_TESTING)
    #define ASSERT(cond)
    #define ASSERT_MSG(cond, msg)
    #define FAIL_MSG(msg)

    // In release builds, shows a simple wxMessageBox with the message only — no file/line
    // details, no debugger break. Use for conditions that would cause a crash if ignored
    // (e.g., nullptr guards).
    #define ASSERT_OR_WARN(cond, msg)                              \
        if (!(cond))                                               \
        {                                                          \
            wxMessageBox((msg), "Warning", wxOK | wxICON_WARNING); \
        }
#elif defined(_DEBUG)
    #define ASSERT(cond)                                                                 \
        if (!(cond) && AssertionDlg(__FILE__, __func__, __LINE__, #cond, wxEmptyString)) \
        {                                                                                \
            wxTrap();                                                                    \
        }

    #define ASSERT_MSG(cond, msg)                                                \
        if (!(cond) && AssertionDlg(__FILE__, __func__, __LINE__, #cond, (msg))) \
        {                                                                        \
            wxTrap();                                                            \
        }

    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
    #define FAIL_MSG(msg)                                                \
        if (AssertionDlg(__FILE__, __func__, __LINE__, "failed", (msg))) \
        {                                                                \
            wxTrap();                                                    \
        }
    // In debug builds, identical to ASSERT_MSG — shows file/line/condition with option
    // to break into the debugger.
    #define ASSERT_OR_WARN(cond, msg) ASSERT_MSG(cond, msg)
// NOLINTEND(cppcoreguidelines-macro-usage)

#else  // INTERNAL_TESTING without _DEBUG
    // In release+testing builds, show the assertion dialog but never break into the
    // debugger. The dialog offers a "Save to log" option instead.
    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
    #define ASSERT(cond)                                                                    \
        if (!(cond))                                                                        \
        {                                                                                   \
            std::ignore = AssertionDlg(__FILE__, __func__, __LINE__, #cond, wxEmptyString); \
        }

    #define ASSERT_MSG(cond, msg)                                                   \
        if (!(cond))                                                                \
        {                                                                           \
            std::ignore = AssertionDlg(__FILE__, __func__, __LINE__, #cond, (msg)); \
        }

    #define FAIL_MSG(msg)                                                              \
        {                                                                              \
            std::ignore = AssertionDlg(__FILE__, __func__, __LINE__, "failed", (msg)); \
        }

    // In testing builds, identical to ASSERT_MSG — shows file/line/condition with
    // option to save to log.
    #define ASSERT_OR_WARN(cond, msg) ASSERT_MSG(cond, msg)
// NOLINTEND(cppcoreguidelines-macro-usage)
#endif  // defined(NDEBUG) && !defined(INTERNAL_TESTING)
