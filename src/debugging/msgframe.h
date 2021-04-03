/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../ui/msgframe_base.h"

class MsgFrame : public MsgFrameBase
{
public:
    MsgFrame(ttlib::cstrVector* pMsgs, bool* pDestroyed, wxWindow* parent = nullptr);

    void AddInfoMsg(ttlib::cview msg) { m_textCtrl->AppendText(msg.wx_str()); };
    void AddEventMsg(ttlib::cview msg) { m_textCtrl->AppendText(msg.wx_str()); };
    void AddWarningMsg(ttlib::cview msg);
    void AddErrorMsg(ttlib::cview msg);

protected:
    // Handlers for MsgFrameBase events
    void OnClose(wxCloseEvent& WXUNUSED(event)) override;
    void OnSaveAs(wxCommandEvent& WXUNUSED(event)) override;
    void OnClear(wxCommandEvent& WXUNUSED(event)) override;
    void OnHide(wxCommandEvent& WXUNUSED(event)) override;
    void OnWarnings(wxCommandEvent& WXUNUSED(event)) override;
    void OnEvents(wxCommandEvent& WXUNUSED(event)) override;
    void OnInfo(wxCommandEvent& WXUNUSED(event)) override;
    void OnWidgetLog(wxCommandEvent& WXUNUSED(event)) override;

private:
    ttlib::cstrVector* m_pMsgs;
    bool* m_pDestroyed;
};
