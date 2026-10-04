/////////////////////////////////////////////////////////////////////////////
// Purpose:   Prevents modal dialogs from blocking command-line operations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "cli_ui_guard.h"

#include "cli_ui_guard.h"

#include <wx/log.h>     // wxLogStderr
#include <wx/msgdlg.h>  // wxMessageDialogBase -- GetMessage()
#include <wx/string.h>

#include "mainapp.h"          // App -- is_Generating(), get_CmdLineLog()
#include "project_handler.h"  // Project -- is_UiAllowed()

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
        wxString msg;
        if (auto* message_dialog = dynamic_cast<wxMessageDialogBase*>(dialog);
            message_dialog != nullptr)
        {
            msg = message_dialog->GetMessage();
        }

        if (!msg.empty())
        {
            cli_ui::ReportMessage(msg);
        }

        // A non-wxID_NONE result tells wxWidgets to skip showing the dialog and return this
        // value from ShowModal() instead. wxID_CANCEL is the safe default: callers treat it
        // as "no" for prompts and discard it for error notices.
        return wxID_CANCEL;
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
        if (wxGetApp().is_Generating())
        {
            // A .log file is being produced (--gen_* / --test_* / --verify_*), so record the
            // message there using the same prefixed format the generator uses.
            wxue::string& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
            log_msg << "Error: " << msg.utf8_string();
            return;
        }

        // No log file is being produced, so fall back to stderr. wxLogStderr writes to FILE*
        // directly and therefore works whether or not a console is attached.
        wxLogStderr(nullptr).LogText(msg);
    }
}  // namespace cli_ui
