/////////////////////////////////////////////////////////////////////////////
// Purpose:   Assertion Dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License ( see ../LICENSE )
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <source_location>

// This should *ONLY* be called in the GUI thread!
//
// This function is available in Release builds
bool AssertionDlg(const std::source_location& location, const char* cond, std::string_view msg);

#if defined(NDEBUG) && !defined(INTERNAL_TESTING)
    #define ASSERT(cond)
    #define ASSERT_MSG(cond, msg)
    #define FAIL_MSG(msg)
#else  // not defined(NDEBUG) && !defined(INTERNAL_TESTING)
    #define ASSERT(cond)                                                  \
        if (!(cond))                                                      \
        {                                                                 \
            if (AssertionDlg(std::source_location::current(), #cond, "")) \
            {                                                             \
                wxTrap();                                                 \
            }                                                             \
        }

    #define ASSERT_MSG(cond, msg)                                            \
        if (!(cond))                                                         \
        {                                                                    \
            if (AssertionDlg(std::source_location::current(), #cond, (msg))) \
            {                                                                \
                wxTrap();                                                    \
            }                                                                \
        }

    #define FAIL_MSG(msg)                                                       \
        {                                                                       \
            if (AssertionDlg(std::source_location::current(), "failed", (msg))) \
            {                                                                   \
                wxTrap();                                                       \
            }                                                                   \
        }
#endif  // defined(NDEBUG) && !defined(INTERNAL_TESTING)
