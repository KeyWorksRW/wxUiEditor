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

class MsgFrame;

class MsgLogging : public wxLogGui
{
public:
    void ShowLogger();
    void CloseLogger();

    void AddInfoMsg(std::string_view msg);
    void AddEventMsg(std::string_view msg);
    void AddWarningMsg(std::string_view msg);
    void AddErrorMsg(std::string_view msg);

    void OnNodeSelected();

    void Clear();

    void DoLogRecord(wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info) override;

private:
    MsgFrame* m_msgFrame { nullptr };

    bool m_bDestroyed { true };
    bool m_isFirstShown { false };  // If false, and PREFS_MSG_WINDOW is set, then show the window
};

extern MsgLogging* g_pMsgLogging;          // NOLINT (cppcheck-suppress)
extern std::vector<tt_string> g_log_msgs;  // NOLINT (cppcheck-suppress)
