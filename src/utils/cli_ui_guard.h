/////////////////////////////////////////////////////////////////////////////
// Purpose:   Prevents modal dialogs from blocking command-line operations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(INTERNAL_TESTING)
    #include <functional>
#endif

#include <wx/modalhook.h>  // wxModalDialogHook

#if defined(INTERNAL_TESTING)
    #include <wx/string.h>
#endif

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

#if defined(INTERNAL_TESTING)
    // Debug-only. While a scripted answer is set, every modal dialog intercepted by the guard
    // returns it instead of being shown; wxID_NONE disables. The observer, if set, is notified
    // with the dialog's message and the answer actually returned.
    void SetScriptedDialogAnswer(int answer);
    void SetDialogObserver(std::function<void(const wxString& message, int answer)> observer);
#endif

}  // namespace cli_ui
