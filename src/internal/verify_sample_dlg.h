/////////////////////////////////////////////////////////////////////////////
// Purpose:   Check and launch the app generated for a language target
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/dialog.h>

enum class GenLang : unsigned int;

class wxChoice;
class wxCloseEvent;
class wxProcess;
class wxProcessEvent;
class wxTextCtrl;

// Maintainer-only tool: run a syntax/compile check against the code generated from the current
// project, then launch the resulting GUI app and optionally kill it.
class VerifySampleDlg : public wxDialog
{
public:
    explicit VerifySampleDlg(wxWindow* parent);
    ~VerifySampleDlg() override;

private:
    wxString GetOutputDir(GenLang lang) const;
    wxString GetEntryFile(GenLang lang, const wxString& output_path, const wxString& extension);
    void AppendLog(const wxString& text);

    void OnCheck(wxCommandEvent& event);
    void OnRun(wxCommandEvent& event);
    void OnKill(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnEndProcess(wxProcessEvent& event);

    wxChoice* m_choice_lang { nullptr };
    wxTextCtrl* m_log { nullptr };
    long m_app_pid { 0 };
    wxProcess* m_process { nullptr };
};
