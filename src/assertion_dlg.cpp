/////////////////////////////////////////////////////////////////////////////
// Purpose:   Assertion Dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License ( see ../LICENSE )
/////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <mutex>

#include <wx/msgdlg.h>

#include "mainapp.h"    // App -- Main application class
#include "mainframe.h"  // MainFrame -- Main window frame

static std::mutex g_mutexAssert;

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
    else
    {
        if (auto frame = wxGetApp().getMainFrame(); frame && frame->IsShown())
        {
            if (wxGetApp().isTestingMenuEnabled())
            {
                tt_string log_msg = str.ToStdString();
                if (auto pos = log_msg.find("\n\nPress Yes"); tt::is_found(pos))
                {
                    log_msg.erase(pos, std::string::npos);
                }
                log_msg.Replace("\n\n", "\n", true);
                log_msg += "\n";
                MSG_WARNING(log_msg);
            }
        }
    }

    return false;
}

// wxSetAssertHandler(ttAssertionHandler) will change wxASSERT dialogs to this one.
void ttAssertionHandler(const wxString& filename, int line, const wxString& function, const wxString& cond,
                        const wxString& msg)
{
    // This is in case additional message processing results in an assert while this one is already being displayed.
    std::unique_lock<std::mutex> classLock(g_mutexAssert);

    wxString str;

    if (cond.size())
        str << "Expression: " << cond << "\n\n";
    if (msg.size())
        str << "Comment: " << msg << "\n\n";

    str << "File: " << filename << "\n";
    str << "Function: " << function << "\n";
    str << "Line: " << line << "\n\n";
    str << "Press Yes to call wxTrap, No to continue, Cancel to exit program.";

    wxMessageDialog dlg(nullptr, str, "Assertion!", wxCENTRE | wxYES_NO | wxCANCEL);
    dlg.SetYesNoCancelLabels("wxTrap", "Continue", "Exit program");

    auto answer = dlg.ShowModal();

    if (answer == wxID_YES)
    {
        wxTrap();
    }
    else if (answer == wxID_CANCEL)
    {
        std::quick_exit(2);
    }
    else
    {
        if (auto frame = wxGetApp().getMainFrame(); frame && frame->IsShown())
        {
            if (wxGetApp().isTestingMenuEnabled())
            {
                tt_string log_msg = str.ToStdString();
                if (auto pos = log_msg.find("\n\nPress Yes"); tt::is_found(pos))
                {
                    log_msg.erase(pos, std::string::npos);
                }
                log_msg.Replace("\n\n", "\n", true);
                log_msg += "\n";
                MSG_WARNING(log_msg);
            }
        }
    }
}
