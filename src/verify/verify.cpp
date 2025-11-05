/////////////////////////////////////////////////////////////////////////////
// Purpose:   Verify functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "verify.h"

#include "mainframe.h"  // CMainFrame

#if defined(_DEBUG)  // Starts debug section.

void MainFrame::OnVerifyTTWX(wxCommandEvent& /* event unused */)  // NOLINT (cppcheck-suppress)
{
    if (VerifyTTwx())
    {
        MSG_INFO("VerifyTTwx: All tests passed successfully!");
    }

    if (VerifyStringVector())
    {
        MSG_INFO("VerifyStringVector: All tests passed successfully!");
    }

    if (VerifyViewVector())
    {
        MSG_INFO("VerifyViewVector: All tests passed successfully!");
    }
}

#endif  // Ends debug section.
