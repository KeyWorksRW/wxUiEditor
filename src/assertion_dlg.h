/////////////////////////////////////////////////////////////////////////////
// Purpose:   Assertion Dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License ( see ../LICENSE )
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

// Note that while it is considered more "modern" there's nothing in the current code base for
// wxUiEditor that would benefit from using std::source_location.

auto AssertionDlg(const char* filename, const char* function, int line, const char* cond,
                  const std::string& msg) -> bool;

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
    #define ASSERT(cond)                                                      \
        if (!(cond) && AssertionDlg(__FILE__, __func__, __LINE__, #cond, "")) \
        {                                                                     \
            wxTrap();                                                         \
        }

    #define ASSERT_MSG(cond, msg)                                                \
        if (!(cond) && AssertionDlg(__FILE__, __func__, __LINE__, #cond, (msg))) \
        {                                                                        \
            wxTrap();                                                            \
        }

// Leave this as a MACRO so that it doesn't require a semicolon at the end of the line when it is
// used.
    #define FAIL_MSG(msg)                                              \
        if (AssertionDlg(__FILE__, __func__, __LINE__, "failed", msg)) \
        {                                                              \
            wxTrap();                                                  \
        }
#endif  // defined(NDEBUG) && !defined(INTERNAL_TESTING)
