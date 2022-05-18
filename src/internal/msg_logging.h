/////////////////////////////////////////////////////////////////////////////
// Purpose:   Message logging class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

// In a Debug build, we use our custom logging class to retrieve wxWidgets messages. In a Release build with INTERNAL_TESTING
// set, we still have the custom class and window, but there is no log window to derive from, or messages from wxWidgets to
// intercept.

// clang-format off
#if defined(_DEBUG)
    // wx/log.h *MUST* be included before wx/generic/logg.h
    #include <wx/log.h>           // Assorted wxLogXXX functions, and wxLog (sink for logs)

    #include <wx/generic/logg.h>  // wxLogGui class
#endif
// clang-format on

class MsgFrame;

#if defined(_DEBUG)
class MsgLogging : public wxLogGui
#else
class MsgLogging
#endif
{
public:
    void ShowLogger();
    void CloseLogger();

    void AddInfoMsg(ttlib::sview msg);
    void AddEventMsg(ttlib::sview msg);
    void AddWarningMsg(ttlib::sview msg);
    void AddErrorMsg(ttlib::sview msg);

    void OnNodeSelected();

#if defined(_DEBUG)
    void DoLogRecord(wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info) override;
#endif

private:
    MsgFrame* m_msgFrame { nullptr };
    std::vector<ttlib::cstr> m_Msgs;

    bool m_bDestroyed { true };
    bool m_isFirstShown { false };  // If false, and PREFS_MSG_WINDOW is set, then show the window
};

extern MsgLogging* g_pMsgLogging;
