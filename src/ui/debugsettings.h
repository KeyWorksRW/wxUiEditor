/////////////////////////////////////////////////////////////////////////////
// Purpose:   Settings while running the Debug version of wxUiEditor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "debugsettingsBase.h"

class DebugSettings : public DebugSettingsBase
{
public:
	DebugSettings(wxWindow* parent = nullptr);

protected:

	// Handlers for DebugSettingsBase events.

	void OnInit(wxInitDialogEvent& event) override;
	void OnShowNow(wxCommandEvent& WXUNUSED(event)) override;
	void OnOK(wxCommandEvent& WXUNUSED(event)) override;

private:
	long m_orgFlags;
};
