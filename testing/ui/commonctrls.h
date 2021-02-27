/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common controls dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "commonctrls_base.h"

class CommonCtrls : public CommonCtrlsBase
{
public:
    CommonCtrls(wxWindow* parent = nullptr);

protected:
    void OnListBox(wxCommandEvent& event) override;
    void OnSlider(wxCommandEvent& event) override;
    void OnRadioBox(wxCommandEvent& event) override;
    void OnListChecked(wxCommandEvent& event) override;
    void OnComboClose(wxCommandEvent& event) override;
    void OnCombo(wxCommandEvent& event) override;
    void OnChoice(wxCommandEvent& event) override;
    void OnRadio(wxCommandEvent& event) override;
    void OnFirstBtn(wxCommandEvent& event) override;
    void OnCheckBox(wxCommandEvent& event) override;
    void OnProcessEnter(wxCommandEvent& event) override;
};
