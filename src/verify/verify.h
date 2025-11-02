/////////////////////////////////////////////////////////////////////////////
// Purpose:   Verify functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

// Simple assertion macro for testing

#define VERIFY_EQUAL(actual, expected, message)                                                \
    [&]() -> void                                                                              \
    {                                                                                          \
        if ((actual) != (expected))                                                            \
        {                                                                                      \
            wxString verifyErrorMsg =                                                          \
                wxString::Format("VERIFICATION FAILED: %s - Expected: %s, Got: %s at line %d", \
                                 message, wxString::FromUTF8(std::to_string(expected)),        \
                                 wxString::FromUTF8(std::to_string(actual)), __LINE__);        \
            if (AssertionDlg(__FILE__, "VERIFY_EQUAL", __LINE__, "failed", verifyErrorMsg))    \
            {                                                                                  \
                wxTrap();                                                                      \
            }                                                                                  \
        }                                                                                      \
    }()

auto VerifyTTwx() -> bool;
auto VerifyStringVector() -> bool;
auto VerifyViewVector() -> bool;
