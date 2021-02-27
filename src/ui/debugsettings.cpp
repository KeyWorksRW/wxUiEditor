/////////////////////////////////////////////////////////////////////////////
// Purpose:   Settings while running the Debug version of wxUiEditor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/config.h>  // wxConfig base header

#include "debugsettings.h"  // auto-generated: debugsettingsBase.h and debugsettingsBase.cpp

#include "mainapp.h"  // App -- App class

DebugSettings::DebugSettings(wxWindow* parent) : DebugSettingsBase(parent) {}

void DebugSettings::OnInit(wxInitDialogEvent& event)
{
    m_orgFlags = wxGetApp().GetPrefs().flags;
    m_DisplayMsgWindow = (m_orgFlags & App::PREFS_MSG_WINDOW);
    m_DisplayMsgInfo = (m_orgFlags & App::PREFS_MSG_INFO);
    m_DisplayMsgEvent = (m_orgFlags & App::PREFS_MSG_EVENT);
    m_DisplayMsgWarnng = (m_orgFlags & App::PREFS_MSG_WARNING);

    event.Skip();  // transfer all validator data to their windows and update UI
}

void DebugSettings::OnShowNow(wxCommandEvent& WXUNUSED(event))
{
    g_pMsgLogger->ShowLogger();
}

void DebugSettings::OnOK(wxCommandEvent& event)
{
    if (!Validate() || !TransferDataFromWindow())
        return;

    if (m_DisplayMsgWindow)
        m_orgFlags |= App::PREFS_MSG_WINDOW;
    else
        m_orgFlags &= ~App::PREFS_MSG_WINDOW;

    if (m_DisplayMsgInfo)
        m_orgFlags |= App::PREFS_MSG_INFO;
    else
        m_orgFlags &= ~App::PREFS_MSG_INFO;

    if (m_DisplayMsgEvent)
        m_orgFlags |= App::PREFS_MSG_EVENT;
    else
        m_orgFlags &= ~App::PREFS_MSG_EVENT;

    if (m_DisplayMsgWarnng)
        m_orgFlags |= App::PREFS_MSG_WARNING;
    else
        m_orgFlags &= ~App::PREFS_MSG_WARNING;

    if (m_orgFlags != wxGetApp().GetPrefs().flags)
    {
        wxGetApp().GetPrefs().flags = m_orgFlags;

        auto config = wxConfig::Get();
        config->SetPath("/preferences");
        config->Write("flags", m_orgFlags);
    }

    event.Skip();  // Need to call this for Persist to work
}
