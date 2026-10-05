/////////////////////////////////////////////////////////////////////////////
// Purpose:   Prevents modal dialogs from blocking command-line operations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "cli_ui_guard.h"

#include <wx/log.h>     // wxLogStderr
#include <wx/msgdlg.h>  // wxMessageDialogBase -- GetMessage()
#include <wx/string.h>

#include "mainapp.h"          // App -- is_Generating(), get_CmdLineLog()
#include "project_handler.h"  // Project -- is_UiAllowed()

// True while this thread is inside cli_ui::ReportMessage() for an intercepted dialog.
// Reporting a message runs code that can itself raise an assert or show a dialog; that nested
// dialog must not be reported again, or the reporting would recurse.
static thread_local bool is_reporting = false;

// RAII scope for reporting an intercepted dialog's message. The destructor always clears
// is_reporting, so a message raised while reporting cannot leave the flag set and suppress
// later reports on this thread.
class ReportingScope
{
public:
    ReportingScope() { is_reporting = true; }

    ~ReportingScope() { is_reporting = false; }

    ReportingScope(const ReportingScope&) = delete;
    ReportingScope& operator=(const ReportingScope&) = delete;
    ReportingScope(ReportingScope&&) = delete;
    ReportingScope& operator=(ReportingScope&&) = delete;
};

// Suppresses modal dialogs while Project.is_UiAllowed() is false. This is the single choke
// point that covers every wxMessageBox()/wxMessageDialog call site, because wxWidgets routes
// all of them through wxModalDialogHook before the dialog is shown.
//
// File-scope rather than in an anonymous namespace (project policy prefers `static` for
// file-local declarations, and an anonymous namespace triggers
// misc-avoid-anonymous-namespace). The name is unique to this translation unit.
class CliUiGuard : public wxModalDialogHook
{
protected:
    int Enter(wxDialog* dialog) override
    {
        if (Project.is_UiAllowed())
        {
            // Interactive build: no interception, the dialog is shown normally.
            return wxID_NONE;
        }

        // Headless command-line run: report the message instead of blocking on a dialog that
        // no one can dismiss. wxMessageDialogBase::GetMessage() covers both the native and the
        // generic message dialog. Other dialog types (file pickers) have no message to log.
        //
        // The message is skipped while a report is already in progress: a message raised by the
        // reporting code itself must not be reported again.
        if (!is_reporting)
        {
            wxString msg;
            if (auto* message_dialog = dynamic_cast<wxMessageDialogBase*>(dialog);
                message_dialog != nullptr)
            {
                msg = message_dialog->GetMessage();
            }

            if (!msg.empty())
            {
                const ReportingScope reporting_scope;
                cli_ui::ReportMessage(msg);
            }
        }

        // A non-wxID_NONE result tells wxWidgets to skip showing the dialog and return this
        // value from ShowModal() instead. wxID_NO is the safe default: prompts read it as "no"
        // and error notices discard it.
        //
        // It must not be wxID_CANCEL. The assertion handler (src/assertion_dlg.cpp) reads
        // wxID_CANCEL as the user choosing "Exit program" and calls std::quick_exit(2), which
        // would abort a headless run on the first assertion -- before the log the assertion was
        // written to is ever saved -- instead of recording the assertion and continuing.
        return wxID_NO;
    }
};

// Kept for the life of the process -- Unregister() is called from the destructor, so the
// guard must outlive every dialog call. Allocated once by InstallModalGuard() and
// deliberately never freed.
static CliUiGuard* g_cli_ui_guard { nullptr };

namespace cli_ui
{
    void InstallModalGuard()
    {
        if (g_cli_ui_guard)
        {
            return;
        }

        g_cli_ui_guard = new CliUiGuard();
        g_cli_ui_guard->Register();
    }

    void ReportMessage(const wxString& msg)
    {
        // Always emit to stderr so an unattended run -- a CI workflow or test script that only
        // captures process output -- sees the message. This matters most while generating: the
        // .log the message is also recorded in is only saved when the run completes, so a run
        // that aborts first would otherwise lose the message entirely.
        //
        // wxLogStderr writes to FILE* directly and therefore works whether or not a console is
        // attached.
        wxLogStderr(nullptr).LogText(msg);

        if (wxGetApp().is_Generating())
        {
            // A .log file is being produced (--gen_* / --test_* / --verify_*), so record the
            // message there as well, using the same prefixed format the generator uses.
            wxue::string& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
            log_msg << "Error: " << msg.utf8_string();
        }
    }
}  // namespace cli_ui
