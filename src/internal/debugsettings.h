/////////////////////////////////////////////////////////////////////////////
// Purpose:
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-19-2026]

#pragma once

#include <array>  // for std::array

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
    long m_orgFlags { 0L };

    // Pairs a Prefs::PREFS_MSG_* mask with the validator member it controls so OnInit and OnOK
    // stay in sync when a new debug flag is added.
    struct MsgFlagEntry
    {
        long mask { 0L };
        bool* target { nullptr };
    };

    /// Returns each debug-flag mask paired with a pointer to its validator member.
    std::array<MsgFlagEntry, 5> GetMsgFlagEntries();
};
