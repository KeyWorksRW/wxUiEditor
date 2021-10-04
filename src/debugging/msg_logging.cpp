/////////////////////////////////////////////////////////////////////////////
// Purpose:   Message logging class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
            // Only add the message if the window was already displayed. Otherwise, it will have already added the
            // message from m_Msgs.
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

void MsgLogging::DoLogRecord(wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    switch (level)
    {
        case wxLOG_Error:
            {
                auto& str = m_Msgs.emplace_back("wxError: ");
                str << msg.wx_str() << '\n';

                if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
                {
                    m_isFirstShown = true;
                    ShowLogger();
                }

                else if (!m_bDestroyed)
                    m_msgFrame->Add_wxErrorMsg(str.view_stepover());

                auto frame = wxGetApp().GetMainFrame();
                if (frame && frame->IsShown())
                    frame->SetRightStatusField(str);
            }

            // Following is for wxLogGui
            m_bErrors = true;
            m_aMessages.Add(msg);
            m_aSeverity.Add((int) level);
            m_aTimes.Add((long) (info.timestampMS / 1000));
            m_bHasMessages = true;
            break;

        case wxLOG_Warning:
            if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_WARNING)
            {
                auto& str = m_Msgs.emplace_back("wxWarning: ");
                str << msg.wx_str() << '\n';

                if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
                {
                    m_isFirstShown = true;
                    ShowLogger();
                }

                else if (!m_bDestroyed)
                    m_msgFrame->Add_wxWarningMsg(str.view_stepover());

                auto frame = wxGetApp().GetMainFrame();
                if (frame && frame->IsShown())
                    frame->SetRightStatusField(str);
            }

            // Following is for wxLogGui
            m_bWarnings = true;
            m_aMessages.Add(msg);
            m_aSeverity.Add((int) level);
            m_aTimes.Add((long) (info.timestampMS / 1000));
            m_bHasMessages = true;
            break;

        case wxLOG_Info:
        case wxLOG_Message:
            if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_INFO)
            {
                auto& str = m_Msgs.emplace_back("wxInfo: ");
                str << msg.wx_str() << '\n';

                if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
                {
                    m_isFirstShown = true;
                    ShowLogger();
                }

                else if (!m_bDestroyed)
                    m_msgFrame->Add_wxInfoMsg(str.view_stepover());

                auto frame = wxGetApp().GetMainFrame();
                if (frame && frame->IsShown())
                    frame->SetRightStatusField(str);
            }

            // Following is for wxLogGui
            m_aMessages.Add(msg);
            m_aSeverity.Add(wxLOG_Message);
            m_bHasMessages = true;
            break;

        case wxLOG_Status:
            {
                auto frame = wxGetApp().GetMainFrame();
                if (frame && frame->IsShown())
                    frame->SetRightStatusField(ttlib::cstr() << msg.wx_str());
            }
            break;

        case wxLOG_Debug:
        case wxLOG_Trace:
            wxLog::DoLogRecord(level, msg, info);
            break;

        case wxLOG_FatalError:
        case wxLOG_Max:
            // This should never occur...
            break;

        case wxLOG_Progress:
        case wxLOG_User:
            // just ignore those: passing them to the base class would result in asserts from DoLogText() because
            // DoLogTextAtLevel() would call it as it doesn't know how to handle these levels otherwise
            break;
    }
}
