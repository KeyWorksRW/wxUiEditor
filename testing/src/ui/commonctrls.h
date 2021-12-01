/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common controls dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "commonctrls_base.h"

class PopupWin;

class CommonCtrls : public CommonCtrlsBase
{
public:
    CommonCtrls(wxWindow* parent = nullptr);
    ~CommonCtrls();

protected:
    void OnPopupBtn(wxCommandEvent& event) override;
    void OnListBox(wxCommandEvent& event) override;
    void OnSlider(wxCommandEvent& event) override;
    void OnRadioBox(wxCommandEvent& event) override;
    void OnListChecked(wxCommandEvent& event) override;

private:
    PopupWin* m_popup_win { nullptr };
};
