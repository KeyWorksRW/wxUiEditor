/////////////////// Non-generated Copyright/License Info ////////////////////
// Purpose:   Settings while running the Debug version of wxUiEditor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-19-2026]

#include <wx/config.h>  // wxConfig base header

#include <array>  // for std::array

#include "mainapp.h"  // App -- App class

#include "debugsettings.h"  // DebugSettings -- Settings while running the Debug version of wxUiEditor
#include "internal/msg_logging.h"  // MsgLogging -- Message logging class
#include "preferences.h"           // Preferences -- Preferences class

// If this constructor is used, the caller must call Create(parent)
DebugSettings::DebugSettings() {}

DebugSettings::DebugSettings(wxWindow* parent)
{
    if (!Create(parent))
    {
        wxFAIL_MSG("DebugSettings::Create failed");
    }
}

std::array<DebugSettings::MsgFlagEntry, 5> DebugSettings::GetMsgFlagEntries()
{
    return { {
        { Prefs::PREFS_MSG_WINDOW, &m_DisplayMsgWindow },
        { Prefs::PREFS_MSG_INFO, &m_DisplayMsgInfo },
        { Prefs::PREFS_MSG_EVENT, &m_DisplayMsgEvent },
        { Prefs::PREFS_MSG_WARNING, &m_DisplayMsgWarning },
        { Prefs::PREFS_CREATION_MSG, &m_FireCreationMsgs },
    } };
}

void DebugSettings::OnInit(wxInitDialogEvent& event)
{
    m_orgFlags = UserPrefs.GetDebugFlags();
    for (const MsgFlagEntry& entry: GetMsgFlagEntries())
    {
        *entry.target = ((m_orgFlags & entry.mask) != 0);
    }

    event.Skip();  // transfer all validator data to their windows and update UI
}

void DebugSettings::OnShowNow(wxCommandEvent& /* event unused */)
{
    // g_pMsgLogging is only created by the Debug build of the app; guard against the menu item
    // firing before it exists.
    if (g_pMsgLogging != nullptr)
    {
        g_pMsgLogging->ShowLogger();
    }
}

void DebugSettings::OnOK(wxCommandEvent& event)
{
    if (!Validate() || !TransferDataFromWindow())
    {
        return;
    }

    for (const MsgFlagEntry& entry: GetMsgFlagEntries())
    {
        if (*entry.target)
        {
            m_orgFlags |= entry.mask;
        }
        else
        {
            m_orgFlags &= ~entry.mask;
        }
    }

    if (m_orgFlags != UserPrefs.GetDebugFlags())
    {
        UserPrefs.SetDebugFlags(m_orgFlags);
        UserPrefs.WriteConfig();
    }

    event.Skip();  // Need to call this for Persist to work
}
