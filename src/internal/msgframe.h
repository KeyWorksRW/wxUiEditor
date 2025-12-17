/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string_view>

#include <wx/log.h>

#include "msgframe_base.h"

#include "ttwx/ttwx.h"  // ttwx functions

class MsgFrame : public MsgFrameBase
{
public:
    MsgFrame(std::vector<wxString>* pMsgs, bool* pDestroyed, wxWindow* parent = nullptr);

    void OnNodeSelected();

    void AddErrorMsg(std::string_view msg);
    void Add_wxErrorMsg(std::string_view msg);

    void AddWarningMsg(std::string_view msg);
    void Add_wxWarningMsg(std::string_view msg);

    void AddInfoMsg(std::string_view msg)
    {
        m_textCtrl->AppendText(wxString::FromUTF8(msg.data(), msg.size()));
    };
    void AddEventMsg(std::string_view msg)
    {
        m_textCtrl->AppendText(wxString::FromUTF8(msg.data(), msg.size()));
    };

    void Add_wxInfoMsg(std::string_view msg);

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
    std::vector<wxString>* m_pMsgs;
    bool* m_pDestroyed;

    bool m_isNodeInfoPage { false };
    bool m_isXrcPage { false };
};
