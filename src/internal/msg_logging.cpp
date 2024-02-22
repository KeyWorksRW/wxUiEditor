/////////////////////////////////////////////////////////////////////////////
// Purpose:   Message logging class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "msg_logging.h"

#include "mainapp.h"      // App -- Main application class
#include "mainframe.h"    // MainFrame -- Main window frame
#include "msgframe.h"     // MsgFrame -- Stores messages
#include "preferences.h"  // Set/Get wxUiEditor preferences

MsgLogging* g_pMsgLogging { nullptr };
std::vector<tt_string> g_log_msgs;

void MsgLogging::ShowLogger()
{
    if (m_bDestroyed)
    {
        m_msgFrame = new MsgFrame(&g_log_msgs, &m_bDestroyed);
        m_bDestroyed = false;
    }

    m_msgFrame->Show();
}

void MsgLogging::CloseLogger()
{
    if (!m_bDestroyed)
        m_msgFrame->Close(true);
}

void MsgLogging::AddInfoMsg(tt_string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;  // no point in adding messages if we are shutting down

    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_INFO)
    {
        auto& str = g_log_msgs.emplace_back(msg);
        str << '\n';

        if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
        {
            return;
        }

        if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW) && !m_isFirstShown)
        {
            m_isFirstShown = true;
            ShowLogger();
        }

        else if (!m_bDestroyed)
            m_msgFrame->AddInfoMsg(str);
    }

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    auto frame = wxGetMainFrame();
    if (frame && frame->IsShown())
        frame->setRightStatusField(msg);
}

void MsgLogging::Clear()
{
    if (m_msgFrame && !m_bDestroyed)
    {
        m_msgFrame->Clear();
    }
}

void MsgLogging::AddEventMsg(tt_string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;  // no point in adding messages if we are shutting down

    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_EVENT)
    {
        auto& str = g_log_msgs.emplace_back("Event: ");
        str << msg << '\n';

        if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
        {
            return;
        }

        if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW) && !m_isFirstShown)
        {
            m_isFirstShown = true;
            ShowLogger();
        }

        else if (!m_bDestroyed)
            m_msgFrame->AddEventMsg(str);
    }

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    auto frame = wxGetMainFrame();
    if (frame && frame->IsShown())
        frame->setRightStatusField(tt_string("Event: ") << msg);
}

void MsgLogging::AddWarningMsg(tt_string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;  // no point in adding messages if we are shutting down

    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING)
    {
        auto& str = g_log_msgs.emplace_back("Warning: ");
        str << msg << '\n';

        if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
        {
            return;
        }

        if (!m_isFirstShown)
        {
            m_isFirstShown = true;
            ShowLogger();
        }

        else if (!m_bDestroyed)
        {
            // Only add the message if the window was already displayed. Otherwise, it will have already added the
            // message from g_log_msgs.
            m_msgFrame->AddWarningMsg(str.view_stepover());
        }
    }

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    auto frame = wxGetMainFrame();
    if (frame && frame->IsShown())
        frame->setRightStatusField(tt_string("Warning: ") << msg);
}

void MsgLogging::AddErrorMsg(tt_string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;  // no point in adding messages if we are shutting down

    auto& str = g_log_msgs.emplace_back("Error: ");
    str << msg << '\n';

    FAIL_MSG(str);

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW) && !m_isFirstShown)
    {
        m_isFirstShown = true;
        ShowLogger();
    }

    else if (!m_bDestroyed)
        m_msgFrame->AddErrorMsg(str.view_stepover());

    auto frame = wxGetMainFrame();
    if (frame && frame->IsShown())
        frame->setRightStatusField(str);
}

void MsgLogging::OnNodeSelected()
{
    if (!m_bDestroyed && m_msgFrame)
    {
        m_msgFrame->OnNodeSelected();
    }
}

#if defined(_DEBUG)
void MsgLogging::DoLogRecord(wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    switch (level)
    {
        case wxLOG_Error:
            {
                auto& str = g_log_msgs.emplace_back("wxError: ");
                str << msg.utf8_string() << '\n';

                if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW) && !m_isFirstShown)
                {
                    m_isFirstShown = true;
                    ShowLogger();
                }

                else if (!m_bDestroyed)
                    m_msgFrame->Add_wxErrorMsg(str.view_stepover());

                auto frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                    frame->setRightStatusField(str);
            }

            // Following is for wxLogGui
            m_bErrors = true;
            m_aMessages.Add(msg);
            m_aSeverity.Add((int) level);
            m_aTimes.Add((long) (info.timestampMS / 1000));
            m_bHasMessages = true;
            break;

        case wxLOG_Warning:
            if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING)
            {
                auto& str = g_log_msgs.emplace_back("wxWarning: ");
                str << msg.utf8_string() << '\n';

                if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW) && !m_isFirstShown)
                {
                    m_isFirstShown = true;
                    ShowLogger();
                }

                else if (!m_bDestroyed)
                    m_msgFrame->Add_wxWarningMsg(str.view_stepover());

                auto frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                    frame->setRightStatusField(str);
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
            if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_INFO)
            {
                auto& str = g_log_msgs.emplace_back("wxInfo: ");
                str << msg.utf8_string() << '\n';

                if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW) && !m_isFirstShown)
                {
                    m_isFirstShown = true;
                    ShowLogger();
                }

                else if (!m_bDestroyed)
                    m_msgFrame->Add_wxInfoMsg(str.view_stepover());

                auto frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                    frame->setRightStatusField(str);
            }

            // Following is for wxLogGui
            m_aMessages.Add(msg);
            m_aSeverity.Add(wxLOG_Message);
            m_bHasMessages = true;
            break;

        case wxLOG_Status:
            {
                auto frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                    frame->setRightStatusField(tt_string() << msg.utf8_string());
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
#endif
