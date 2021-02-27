/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog containing special Debugging commands
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "optionsdlg_base.h"

class OptionsDlg : public OptionsDlgBase
{
public:
    OptionsDlg(wxWindow* parent) : OptionsDlgBase(parent) {}

protected:
	void OnAffirmative(wxCommandEvent& WXUNUSED(event)) override;
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
};
