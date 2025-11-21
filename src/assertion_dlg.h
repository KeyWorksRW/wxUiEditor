/////////////////////////////////////////////////////////////////////////////
// Purpose:   Assertion Dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License ( see ../LICENSE )
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file provides project-specific assertion macros (ASSERT, ASSERT_MSG, FAIL_MSG)
// that improve debugging over wxASSERT by calling wxTrap() in the caller's context rather than in
// the assertion handler. These macros display a custom dialog allowing developers to: continue
// execution, trigger a debugger breakpoint via wxTrap(), or exit the program. The assertions are
// compiled out in release builds unless INTERNAL_TESTING is defined. Use these macros instead of
// raw assert() or throw for runtime validation throughout the wxUiEditor codebase.

#pragma once  // NOLINT(#pragma once in main file)

// Note that while it is considered more "modern" there's nothing in the current code base for
// wxUiEditor that would benefit from using std::source_location.

auto AssertionDlg(const char* filename, const char* function, int line, const char* cond,
                  const wxString& msg) -> bool;
void ttAssertionHandler(const wxString& filename, int line, const wxString& function,
                        const wxString& cond, const wxString& msg);

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
#else  // not defined(NDEBUG) && !defined(INTERNAL_TESTING)
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
// NOLINTEND(cppcoreguidelines-macro-usage)
#endif  // defined(NDEBUG) && !defined(INTERNAL_TESTING)
