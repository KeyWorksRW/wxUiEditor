/////////////////////////////////////////////////////////////////////////////
// Purpose:   AssertionDlg() stand-in for the wxui_tests binary
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// wxui_core is compiled with the same precompiled header as wxUiEditor, so the ASSERT_MSG and
// FAIL_MSG macros inside the wxue_namespace sources expand to calls to AssertionDlg() in Debug
// and in release+testing builds. The real implementation (src/assertion_dlg.cpp) opens a modal
// wxDialog and reaches into the main frame via wxGetApp(), so the unit-test binary links this
// instead of the GUI application.
//
// Assertions are reported to stderr rather than breaking the run: a debugger trap would abort
// the whole binary and hide which of the remaining test cases also fail. ctest runs with
// --output-on-failure, so anything written here shows up in the CI log.

#include <cstdio>
#include <string>

#include <wx/string.h>

#include "assertion_dlg.h"

bool AssertionDlg(const char* filename, const char* function, int line, const char* cond,
                  const wxString& msg)
{
    const std::string message = msg.utf8_string();

    std::fprintf(stderr, "Assertion failed: %s\n", cond != nullptr ? cond : "(no condition)");
    std::fprintf(stderr, "  file: %s\n", filename != nullptr ? filename : "(unknown)");
    std::fprintf(stderr, "  func: %s\n", function != nullptr ? function : "(unknown)");
    std::fprintf(stderr, "  line: %d\n", line);
    std::fprintf(stderr, "  msg:  %s\n", message.c_str());

    return false;
}
