/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "mainframe_base.h"

class MainFrame : public MainFrameBase
{
public:
    MainFrame(wxWindow* parent);

protected:
    void OnTreebook(wxCommandEvent& event) override;
    void OnListbook(wxCommandEvent& event) override;
    void OnNotebook(wxCommandEvent& event) override;
    void OnQuit(wxCommandEvent& event) override;
    void OnWizard(wxCommandEvent& event) override;
    void OnRibbonDialog(wxCommandEvent& event) override;
    void OnOtherCtrls(wxCommandEvent& event) override;
    void OnCommonDialog(wxCommandEvent& event) override;
};
