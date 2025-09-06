/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/log.h>

#include "msgframe_base.h"

class MsgFrame : public MsgFrameBase
{
public:
    MsgFrame(std::vector<tt_string>* pMsgs, bool* pDestroyed, wxWindow* parent = nullptr);

    void OnNodeSelected();

    void AddErrorMsg(tt_string_view msg);
    void Add_wxErrorMsg(tt_string_view msg);

    void AddWarningMsg(tt_string_view msg);
    void Add_wxWarningMsg(tt_string_view msg);

    void AddInfoMsg(tt_string_view msg) { m_textCtrl->AppendText(msg.make_wxString()); };
    void AddEventMsg(tt_string_view msg) { m_textCtrl->AppendText(msg.make_wxString()); };

    void Add_wxInfoMsg(tt_string_view msg);

    void Clear();

protected:
    void UpdateNodeInfo();

    // Handlers for MsgFrameBase events
    void OnClear(wxCommandEvent& /* event unused */) override;
    void OnClose(wxCloseEvent& /* event unused */) override;
    void OnEvents(wxCommandEvent& /* event unused */) override;
    void OnHide(wxCommandEvent& /* event unused */) override;
    void OnInfo(wxCommandEvent& /* event unused */) override;
    void OnPageChanged(wxAuiNotebookEvent& /* event unused */) override;
    void OnParent(wxCommandEvent& event) override;
    void OnSaveAs(wxCommandEvent& /* event unused */) override;
    void OnWarnings(wxCommandEvent& /* event unused */) override;

private:
    std::vector<tt_string>* m_pMsgs;
    bool* m_pDestroyed;

    bool m_isNodeInfoPage { false };
    bool m_isXrcPage { false };
};
