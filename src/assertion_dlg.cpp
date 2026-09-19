/////////////////////////////////////////////////////////////////////////////
// Purpose:   Assertion Dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License ( see ../LICENSE )
/////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <mutex>

#include <wx/ffile.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include "mainapp.h"    // App -- Main application class
#include "mainframe.h"  // MainFrame -- Main window frame

static std::mutex mutex_assert;  // NOLINT (cppcheck-suppress)

// True while this thread is handling an assertion -- from just before the dialog is shown until
// the assertion has been written to the message log.
//
// Handling an assertion runs code that can raise another assertion: writing the assertion to
// the message log creates or shows the message window, and building the dialog itself can trip
// a wxWidgets assert. The nested assertion must never be handled as well -- handling it
// re-enters this file and re-locks mutex_assert on a thread that already owns it, and
// std::mutex is not recursive, so that hangs the application. LogAssertion() records it in the
// message log instead.
static thread_local bool is_handling_assert = false;

// True while this thread is writing an assertion to the message log. An assertion raised by
// that code cannot itself be logged -- doing so would recurse.
static thread_local bool is_logging_assert = false;

// RAII scope for a single assertion. IsNested() is true when this thread is already handling an
// assertion, in which case the caller must log it instead of showing a dialog.
class AssertionScope
{
public:
    AssertionScope()
    {
        if (is_handling_assert)
        {
            m_is_nested = true;
        }
        else
        {
            is_handling_assert = true;
        }
    }

    ~AssertionScope()
    {
        if (!m_is_nested)
        {
            is_handling_assert = false;
        }
    }

    AssertionScope(const AssertionScope&) = delete;
    AssertionScope& operator=(const AssertionScope&) = delete;
    AssertionScope(AssertionScope&&) = delete;
    AssertionScope& operator=(AssertionScope&&) = delete;

    bool IsNested() const { return m_is_nested; }

private:
    bool m_is_nested { false };
};

// Builds the text shown in the assertion dialog and written to the message log.
static wxString BuildAssertionText(const wxString& filename, const wxString& function, int line,
                                   const wxString& cond, const wxString& msg)
{
    wxString str;

    if (!cond.empty())
    {
        str << "Expression: " << cond << "\n\n";
    }
    if (!msg.empty())
    {
        str << "Comment: " << msg << "\n\n";
    }

    str << "File: " << filename << '\n';
    str << "Function: " << function << '\n';
    str << "Line: " << line << "\n\n";

#if defined(_DEBUG)
    str << "Run 'Attach to wxUiEditor' in VSCode, then press Yes to break into debugger.\n"
        << "No to continue, Cancel to exit program.";
#else  // INTERNAL_TESTING without _DEBUG
    str << "Press Yes to save details to log file, No to continue, Cancel to exit program.";
#endif

    return str;
}

// Adds an assertion to the message window. Used for any assertion that cannot be shown in the
// dialog: a nested one (showing it would cascade dialogs) or one the user chose to continue
// past.
//
// The status bar is deliberately left alone. AddWarningMsg() and friends update it, and that
// update asserts when the number of panes doesn't match -- which is what made pressing
// "Continue" hang the application.
static void LogAssertion(const wxString& assert_text)
{
    // An assertion raised while logging an assertion cannot be logged as well without
    // recursing, so drop it.
    if (is_logging_assert)
    {
        return;
    }

    const MainFrame* frame = wxGetApp().getMainFrame();
    if (!frame || !frame->IsShown())
    {
        return;
    }

    is_logging_assert = true;

    wxue::string log_msg = assert_text.ToStdString();
    const size_t press_yes = log_msg.find("\n\nPress Yes");
    if (wxue::is_found(press_yes))
    {
        log_msg.erase(press_yes, std::string::npos);
    }
    std::ignore = log_msg.Replace("\n\n", "\n", true);
    log_msg += '\n';
    MSG_ASSERTION(log_msg);

    is_logging_assert = false;
}

// Saves assertion/crash details to a user-chosen log file via wxFileDialog.
void SaveAssertionInfo(const wxString& content)
{
    wxFileDialog file_dlg(nullptr, "Save Assertion Log", "", "assertion_log.txt",
                          "Text files (*.txt)|*.txt|Log files (*.log)|*.log|All files (*.*)|*.*",
                          wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (file_dlg.ShowModal() == wxID_CANCEL)
    {
        return;
    }

    const wxString filepath = file_dlg.GetPath();
    wxFFile file(filepath, "w");
    if (file.IsOpened())
    {
        file.Write(content);
    }
}

// Note that this returns bool allowing the ASSERT macro to call wxTrap in the caller's code rather
// than trapping in this function and then having to step out of this function to get to the
// function that threw the assert.

bool AssertionDlg(const char* filename, const char* function, int line, const char* cond,
                  const wxString& msg)
{
    const wxString str =
        BuildAssertionText(filename, function, line, cond ? wxString(cond) : wxString(), msg);

    // Must stay in scope for the entire function: the message logging at the end runs code that can
    // raise another assertion.
    const AssertionScope assert_scope;
    if (assert_scope.IsNested())
    {
        // A second dialog would cascade, and the caller cannot be trapped from here, but the
        // assertion must not be silently discarded.
        LogAssertion(str);
        return false;
    }

    wxMessageDialog message_dlg(nullptr, str, "Assertion!", wxCENTRE | wxYES_NO | wxCANCEL);
#if defined(_DEBUG)
    message_dlg.SetYesNoCancelLabels("DebugBreak", "Continue", "Exit program");
#else  // INTERNAL_TESTING without _DEBUG
    message_dlg.SetYesNoCancelLabels("Save to log", "Continue", "Exit program");
#endif

    int answer = wxID_NO;
    {
        // Only one assertion dialog at a time. The lock is released before the assertion is
        // logged below, so that creating or showing the message window never runs while it is
        // held.
        const std::scoped_lock<std::mutex> lock(mutex_assert);
        answer = message_dlg.ShowModal();
    }

#if defined(_DEBUG)
    if (answer == wxID_YES)
    {
        return true;
    }
#else  // INTERNAL_TESTING without _DEBUG
    if (answer == wxID_YES)
    {
        SaveAssertionInfo(str);
        return false;
    }
#endif

    if (answer == wxID_CANCEL)
    {
        std::quick_exit(2);
    }

    LogAssertion(str);

    return false;
}

// wxSetAssertHandler(ttAssertionHandler) will change wxASSERT dialogs to this one.
void ttAssertionHandler(const wxString& filename, int line, const wxString& function,
                        const wxString& cond, const wxString& msg)
{
    const wxString str = BuildAssertionText(filename, function, line, cond, msg);

    // Same nested-assertion rule as AssertionDlg(), and for the same reason.
    const AssertionScope assert_scope;
    if (assert_scope.IsNested())
    {
        // A second dialog would cascade, but the assertion must not be silently discarded.
        LogAssertion(str);
        return;
    }

    wxMessageDialog message_dlg(nullptr, str, "Assertion!", wxCENTRE | wxYES_NO | wxCANCEL);
#if defined(_DEBUG)
    message_dlg.SetYesNoCancelLabels("DebugBreak", "Continue", "Exit program");
#else  // INTERNAL_TESTING without _DEBUG
    message_dlg.SetYesNoCancelLabels("Save to log", "Continue", "Exit program");
#endif

    int answer = wxID_NO;
    {
        const std::scoped_lock<std::mutex> lock(mutex_assert);
        answer = message_dlg.ShowModal();
    }

    if (answer == wxID_YES)
    {
#if defined(_DEBUG)
        wxTrap();
#else  // INTERNAL_TESTING without _DEBUG
        SaveAssertionInfo(str);
#endif
    }
    else if (answer == wxID_CANCEL)
    {
        std::quick_exit(2);
    }
    else
    {
        LogAssertion(str);
    }
}
