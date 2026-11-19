/////////////////////////////////////////////////////////////////////////////
// Purpose:   Check and launch the app generated for a language target
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#include <wx/timer.h>

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
    void OnOutputTimer(wxTimerEvent& event);

private:
    void UpdateAppName();
    void AppendLog(const wxString& text);
    void UpdateKillButton();
    void TerminateApp();
    bool ReadProcessOutput();
    void DrainRemainingOutput();
    void DrainOutputBuffer(std::string& buffer, bool is_stderr);
    void LogOutputLine(bool is_stderr, const std::string& line);
    void FlushOutputBuffers();

    long m_app_pid { 0 };
    wxProcess* m_process { nullptr };
    wxButton* m_btn_kill { nullptr };

    // Every launcher is a console process (the python, ruby and julia interpreters, and the
    // kwxts.exe/kwxlua.exe wrappers), so diagnostics written to stdout/stderr would be lost when
    // the console closes. The timer drains the redirected pipes into the log.
    wxTimer m_output_timer;
    std::string m_stdout_buffer;
    std::string m_stderr_buffer;
    bool m_output_seen { false };  // stdout/stderr produced anything for the current run
};
