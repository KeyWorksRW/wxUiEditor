/////////////////////////////////////////////////////////////////////////////
// Purpose:   Message logging class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>  // std::vector

// wx/log.h *MUST* be included before wx/generic/logg.h
#include <wx/log.h>  // Assorted wxLogXXX functions, and wxLog (sink for logs)

#include <wx/generic/logg.h>  // wxLogGui class

#include <wx/event.h>  // wxEvtHandler, wxQueueEvent, wxThreadEvent

class MsgFrame;

class MsgLogging : public wxLogGui
{
public:
    MsgLogging();

    void ShowLogger();
    void CloseLogger();

    void AddInfoMsg(std::string_view msg);
    void AddEventMsg(std::string_view msg);
    void AddWarningMsg(std::string_view msg);
    void AddAssertionMsg(std::string_view msg);
    void AddErrorMsg(std::string_view msg);

    void OnNodeSelected();

    void Clear();

    void DoLogRecord(wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info) override;

private:
    // Creates -- or re-shows -- the message window once EVT_SHOW_MSG_LOGGER has been handled.
    void OnShowLoggerEvent([[maybe_unused]] wxThreadEvent& event);

    MsgFrame* m_msgFrame { nullptr };

    // Receives EVT_SHOW_MSG_LOGGER. MsgLogging is a wxLogGui, not a wxEvtHandler, and deriving
    // from both would give it two wxObject bases, so it owns a handler instead.
    wxEvtHandler m_show_logger_handler;

    bool m_bDestroyed { true };
    bool m_isFirstShown { false };  // If false, and PREFS_MSG_WINDOW is set, then show the window

    // True while EVT_SHOW_MSG_LOGGER is queued, so that ShowLogger() cannot queue a second one
    // before the first has been handled.
    bool m_isPostPending { false };
};

extern MsgLogging* g_pMsgLogging;         // NOLINT (cppcheck-suppress)
extern std::vector<wxString> g_log_msgs;  // NOLINT (cppcheck-suppress)
