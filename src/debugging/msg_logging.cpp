/////////////////////////////////////////////////////////////////////////////
// Purpose:   Message logging class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "msg_logging.h"

#include "mainapp.h"    // App -- Main application class
#include "mainframe.h"  // MainFrame -- Main window frame
#include "msgframe.h"   // MsgFrame -- Stores messages

MsgLogging* g_pMsgLogging { nullptr };

void MsgLogging::ShowLogger()
{
    if (m_bDestroyed)
    {
        m_msgFrame = new MsgFrame(&m_Msgs, &m_bDestroyed);
        m_bDestroyed = false;
    }

    m_msgFrame->Show();
}

void MsgLogging::CloseLogger()
{
    if (!m_bDestroyed)
        m_msgFrame->Close(true);
}

void MsgLogging::AddInfoMsg(ttlib::cview msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_INFO)
    {
        auto& str = m_Msgs.emplace_back(msg);
        str << '\n';

        if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
        {
            m_isFirstShown = true;
            ShowLogger();
        }

        else if (!m_bDestroyed)
            m_msgFrame->AddInfoMsg(str);
    }

    auto frame = wxGetApp().GetMainFrame();
    if (frame && frame->IsShown())
        frame->SetRightStatusField(msg);
}

void MsgLogging::AddEventMsg(ttlib::cview msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_EVENT)
    {
        auto& str = m_Msgs.emplace_back("Event: ");
        str << msg << '\n';

        if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
        {
            m_isFirstShown = true;
            ShowLogger();
        }

        else if (!m_bDestroyed)
            m_msgFrame->AddEventMsg(str);
    }

    auto frame = wxGetApp().GetMainFrame();
    if (frame && frame->IsShown())
        frame->SetRightStatusField(ttlib::cstr("Event: ") << msg);
}

void MsgLogging::AddWarningMsg(ttlib::cview msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_WARNING)
    {
        auto& str = m_Msgs.emplace_back("Warning: ");
        str << msg << '\n';

        if (!m_isFirstShown)
        {
            m_isFirstShown = true;
            ShowLogger();
        }

        else if (!m_bDestroyed)
        {
            // Only add the message if the window was already displayed. Otherwise, it will have already added the message
            // from m_Msgs.
            m_msgFrame->AddWarningMsg(str.view_stepover());
        }
    }

    auto frame = wxGetApp().GetMainFrame();
    if (frame && frame->IsShown())
        frame->SetRightStatusField(ttlib::cstr("Warning: ") << msg);
}

void MsgLogging::AddErrorMsg(ttlib::cview msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    auto& str = m_Msgs.emplace_back("Error: ");
    str << msg << '\n';

    if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
    {
        m_isFirstShown = true;
        ShowLogger();
    }

    else if (!m_bDestroyed)
        m_msgFrame->AddErrorMsg(str.view_stepover());

    auto frame = wxGetApp().GetMainFrame();
    if (frame && frame->IsShown())
        frame->SetRightStatusField(str);
}
