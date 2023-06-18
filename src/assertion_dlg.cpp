/////////////////////////////////////////////////////////////////////////////
// Purpose:   Assertion Dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License ( see ../LICENSE )
/////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <mutex>

#include <wx/msgdlg.h>

static std::mutex g_mutexAssert;

#if defined(__cpp_consteval)

bool AssertionDlg(const std::source_location& location, const char* cond, std::string_view msg)
{
    // This is in case additional message processing results in an assert while this one is already being displayed.
    std::unique_lock<std::mutex> classLock(g_mutexAssert);

    tt_string str;

    if (cond)
        str << "Expression: " << cond << "\n\n";
    if (!msg.empty())
        str << "Comment: " << msg << "\n\n";

    str << "File: " << location.file_name() << "\n";
    str << "Function: " << location.function_name() << "\n";
    str << "Line: " << (size_t) location.line() << "\n";

    wxMessageDialog dlg(nullptr, str.make_wxString(), "Assertion!", wxCENTRE | wxYES_NO | wxCANCEL);
    dlg.SetYesNoCancelLabels("wxTrap", "Continue", "Exit program");

    auto answer = dlg.ShowModal();

    if (answer == wxID_YES)
    {
        return true;
    }
    else if (answer == wxID_CANCEL)
    {
        std::quick_exit(2);
    }

    return false;
}

#else  // not defined(__cpp_consteval)

bool AssertionDlg(const char* filename, const char* function, int line, const char* cond, const std::string& msg)
{
    // This is in case additional message processing results in an assert while this one is already being displayed.
    std::unique_lock<std::mutex> classLock(g_mutexAssert);

    tt_string str;

    if (cond)
        str << "Expression: " << cond << "\n\n";
    if (!msg.empty())
        str << "Comment: " << msg << "\n\n";

    str << "File: " << filename << "\n";
    str << "Function: " << function << "\n";
    str << "Line: " << line << "\n\n";
    str << "Press Yes to call wxTrap, No to continue, Cancel to exit program.";

    wxMessageDialog dlg(nullptr, str.make_wxString(), "Assertion!", wxCENTRE | wxYES_NO | wxCANCEL);
    dlg.SetYesNoCancelLabels("wxTrap", "Continue", "Exit program");

    auto answer = dlg.ShowModal();

    if (answer == wxID_YES)
    {
        return true;
    }
    else if (answer == wxID_CANCEL)
    {
        std::quick_exit(2);
    }

    return false;
}

#endif  // defined(__cpp_consteval)
