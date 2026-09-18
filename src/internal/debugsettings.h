/////////////////////////////////////////////////////////////////////////////
// Purpose:
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "generated/debugsettings_base.h"

class DebugSettings : public DebugSettingsBase
{
public:
    DebugSettings();  // If you use this constructor, you must call Create(parent)
    DebugSettings(wxWindow* parent);

protected:
    // Handlers for DebugSettingsBase events

    void OnInit(wxInitDialogEvent& event) override;
    void OnOK(wxCommandEvent& event) override;
    void OnShowNow(wxCommandEvent& event) override;

private:
    long m_orgFlags;
};
