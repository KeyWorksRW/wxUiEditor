/////////////////////////////////////////////////////////////////////////////
// Purpose:   Window for displaying MSG_ messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License (see Apache License)
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/event.h>
#include <wx/frame.h>
#include <wx/log.h>
#include <wx/textctrl.h>

#include <ttcvector.h>  // cstrVector -- Vector of ttlib::cstr strings

class CMsgFrame : public wxFrame
{
public:
    CMsgFrame(ttlib::cstrVector* pMsgs, bool* pDestroyed);

    void OnClose(wxCloseEvent& event);

    void OnSave(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);

    void AddInfoMsg(ttlib::cview msg) { m_pTextCtrl->AppendText(msg.wx_str()); };
    void AddEventMsg(ttlib::cview msg) { m_pTextCtrl->AppendText(msg.wx_str()); };
    void AddWarningMsg(ttlib::cview msg);
    void AddErrorMsg(ttlib::cview msg);

private:
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CMsgFrame);

    wxTextCtrl* m_pTextCtrl { nullptr };

    ttlib::cstrVector* m_pMsgs;
    bool* m_pDestroyed;
};

class MsgLogger
{
public:
    void ShowLogger();
    void CloseLogger();

    void AddInfoMsg(ttlib::cview msg);
    void AddEventMsg(ttlib::cview msg);
    void AddWarningMsg(ttlib::cview msg);
    void AddErrorMsg(ttlib::cview msg);

private:
    CMsgFrame* m_msgFrame { nullptr };
    ttlib::cstrVector m_Msgs;

    bool m_bDestroyed { true };
    bool m_isFirstShown { false };  // If false, and PREFS_MSG_WINDOW is set, then show the window
};

extern MsgLogger* g_pMsgLogger;
