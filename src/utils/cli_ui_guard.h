/////////////////////////////////////////////////////////////////////////////
// Purpose:   Prevents modal dialogs from blocking command-line operations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/modalhook.h>  // wxModalDialogHook

// Command-line operations such as --gen_cpp and --verify_cpp must run unattended, but
// constructing any wxDialog and calling ShowModal() blocks until a button is pressed. An
// unattended process -- a test script or CI workflow -- can never press that button, so the
// process hangs forever.
//
// Every wxMessageBox()/wxMessageDialog::ShowModal() call routes through wxModalDialogHook
// (see WX_HOOK_MODAL_DIALOG in wx/modalhook.h), so installing a single hook covers all ~190
// call sites at once instead of auditing each one.
//
// The hook only intercepts when Project.is_UiAllowed() is false. Interactive builds keep
// m_allow_ui == true, so Enter() returns wxID_NONE and the dialog displays normally, which
// leaves GUI behaviour byte-for-byte unchanged.
namespace cli_ui
{
    // Registers the hook that suppresses modal dialogs while Project.is_UiAllowed() is false.
    // Safe to call more than once; only the first call registers. Keeps the hook alive for the
    // life of the process.
    void InstallModalGuard();

    // Always writes the message to stderr, and also records it in the command-line log when
    // code is being generated. Used by the modal guard and available to any command-line path
    // that needs to report a problem without a dialog.
    void ReportMessage(const wxString& msg);

}  // namespace cli_ui
