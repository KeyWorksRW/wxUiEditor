/////////////////// Non-generated Copyright/License Info ////////////////////
// Purpose:   Settings while running the Debug version of wxUiEditor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if defined(INTERNAL_TESTING)

    #include <wx/config.h>  // wxConfig base header

    #include "mainapp.h"  // App -- App class

    #include "debugsettings.h"  // DebugSettings -- Settings while running the Debug version of wxUiEditor
    #include "internal/msg_logging.h"  // MsgLogging -- Message logging class
    #include "preferences.h"           // Preferences -- Preferences class

// If this constructor is used, the caller must call Create(parent)
DebugSettings::DebugSettings() {}

DebugSettings::DebugSettings(wxWindow* parent)
{
    Create(parent);
}

void DebugSettings::OnInit(wxInitDialogEvent& event)
{
    m_orgFlags = UserPrefs.GetDebugFlags();
    m_DisplayMsgWindow = (m_orgFlags & Prefs::PREFS_MSG_WINDOW);
    m_DisplayMsgInfo = (m_orgFlags & Prefs::PREFS_MSG_INFO);
    m_DisplayMsgEvent = (m_orgFlags & Prefs::PREFS_MSG_EVENT);
    m_DisplayMsgWarning = (m_orgFlags & Prefs::PREFS_MSG_WARNING);
    m_FireCreationMsgs = (m_orgFlags & Prefs::PREFS_CREATION_MSG);

    event.Skip();  // transfer all validator data to their windows and update UI
}

void DebugSettings::OnShowNow(wxCommandEvent& /* event unused */)
{
    g_pMsgLogging->ShowLogger();
}

void DebugSettings::OnOK(wxCommandEvent& event)
{
    if (!Validate() || !TransferDataFromWindow())
        return;

    if (m_DisplayMsgWindow)
        m_orgFlags |= Prefs::PREFS_MSG_WINDOW;
    else
        m_orgFlags &= ~Prefs::PREFS_MSG_WINDOW;

    if (m_DisplayMsgInfo)
        m_orgFlags |= Prefs::PREFS_MSG_INFO;
    else
        m_orgFlags &= ~Prefs::PREFS_MSG_INFO;

    if (m_DisplayMsgEvent)
        m_orgFlags |= Prefs::PREFS_MSG_EVENT;
    else
        m_orgFlags &= ~Prefs::PREFS_MSG_EVENT;

    if (m_DisplayMsgWarning)
        m_orgFlags |= Prefs::PREFS_MSG_WARNING;
    else
        m_orgFlags &= ~Prefs::PREFS_MSG_WARNING;

    if (m_FireCreationMsgs)
        m_orgFlags |= Prefs::PREFS_CREATION_MSG;
    else
        m_orgFlags &= ~Prefs::PREFS_CREATION_MSG;

    if (m_orgFlags != UserPrefs.GetDebugFlags())
    {
        UserPrefs.SetDebugFlags(m_orgFlags);
        UserPrefs.WriteConfig();
    }

    event.Skip();  // Need to call this for Persist to work
}

#endif  // defined(INTERNAL_TESTING)
