/////////////////////////////////////////////////////////////////////////////
// Purpose:   Assertion Dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License ( see ../LICENSE )
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

// Note that while it is considered more "modern" there's nothing in the current code base for
// wxUiEditor that would benefit from using std::source_location.

bool AssertionDlg(const char* filename, const char* function, int line, const char* cond, const std::string& msg);

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

    #define FAIL_MSG(msg)                                              \
        if (AssertionDlg(__FILE__, __func__, __LINE__, "failed", msg)) \
        {                                                              \
            wxTrap();                                                  \
        }
#endif  // defined(NDEBUG) && !defined(INTERNAL_TESTING)
