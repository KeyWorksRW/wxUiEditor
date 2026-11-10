/////////////////////////////////////////////////////////////////////////////
// Purpose:   Check and launch the app generated for a language target
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "generated/verify_sample_base.h"

enum class GenLang : unsigned int;

class wxButton;
class wxProcess;
class wxProcessEvent;

// Maintainer-only tool: run a syntax/compile check against the code generated from the current
// project, then launch the resulting GUI app and optionally kill it.
class VerifySampleDlg : public VerifySampleBase
{
public:
    explicit VerifySampleDlg(wxWindow* parent);
    ~VerifySampleDlg() override;

protected:
    // Handlers for VerifySampleBase events
    void OnCheck(wxCommandEvent& event) override;
    void OnRun(wxCommandEvent& event) override;
    void OnKill(wxCommandEvent& event) override;
    void OnClose(wxCommandEvent& event) override;
    void OnCloseWindow(wxCloseEvent& event) override;

    void OnLanguageChanged(wxCommandEvent& event);
    void OnEndProcess(wxProcessEvent& event);

private:
    static wxString GetOutputDir(GenLang lang);
    wxString GetEntryFile(GenLang lang, const wxString& output_path, const wxString& extension,
                          bool log_error = true);
    void UpdateAppName();
    void AppendLog(const wxString& text);
    void UpdateKillButton();

    long m_app_pid { 0 };
    wxProcess* m_process { nullptr };
    wxButton* m_btn_kill { nullptr };
};
