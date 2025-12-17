/////////////////////////////////////////////////////////////////////////////
// Purpose:   Message logging class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "msg_logging.h"

#include "mainapp.h"      // App -- Main application class
#include "mainframe.h"    // MainFrame -- Main window frame
#include "msgframe.h"     // MsgFrame -- Stores messages
#include "preferences.h"  // Set/Get wxUiEditor preferences

MsgLogging* g_pMsgLogging { nullptr };  // NOLINT (cppcheck-suppress)
std::vector<wxString> g_log_msgs;       // NOLINT (cppcheck-suppress)

void MSG_INFO(const std::string& msg)
{
    if (wxGetApp().isTestingMenuEnabled() && g_pMsgLogging)
    {
        g_pMsgLogging->AddInfoMsg(msg);
    }
}

void MSG_WARNING(const std::string& msg)
{
    if (wxGetApp().isTestingMenuEnabled() && g_pMsgLogging)
    {
        g_pMsgLogging->AddWarningMsg(msg);
    }
}

void MSG_ERROR(const std::string& msg)
{
    if (wxGetApp().isTestingMenuEnabled() && g_pMsgLogging)
    {
        g_pMsgLogging->AddErrorMsg(msg);
    }
}

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
    {
        m_msgFrame->Close(true);
    }
}

void MsgLogging::AddInfoMsg(std::string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
    {
        return;  // no point in adding messages if we are shutting down
    }

    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_INFO)
    {
        auto& str = g_log_msgs.emplace_back(wxString::FromUTF8(msg.data(), msg.size()));
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
        {
            m_msgFrame->AddInfoMsg(str.ToStdString());
        }
    }

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    auto* frame = wxGetMainFrame();
    if (frame && frame->IsShown())
    {
        frame->setRightStatusField(msg);
    }
}

void MsgLogging::Clear()
{
    if (m_msgFrame && !m_bDestroyed)
    {
        m_msgFrame->Clear();
    }
}

void MsgLogging::AddEventMsg(std::string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
    {
        return;  // no point in adding messages if we are shutting down
    }

    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_EVENT)
    {
        auto& str = g_log_msgs.emplace_back("Event: ");
        str << wxString::FromUTF8(msg.data(), msg.size()) << '\n';

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
        {
            m_msgFrame->AddEventMsg(str.ToStdString());
        }
    }

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    auto* frame = wxGetMainFrame();
    if (frame && frame->IsShown())
    {
        frame->setRightStatusField(wxString("Event: ")
                                   << wxString::FromUTF8(msg.data(), msg.size()));
    }
}

void MsgLogging::AddWarningMsg(std::string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
    {
        return;  // no point in adding messages if we are shutting down
    }

    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING)
    {
        auto& str = g_log_msgs.emplace_back("Warning: ");
        str << wxString::FromUTF8(msg.data(), msg.size()) << '\n';

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
            // Only add the message if the window was already displayed. Otherwise, it will have
            // already added the message from g_log_msgs.
            m_msgFrame->AddWarningMsg(ttwx::stepover(str));
        }
    }

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    auto* frame = wxGetMainFrame();
    if (frame && frame->IsShown())
    {
        frame->setRightStatusField(wxString("Warning: ")
                                   << wxString::FromUTF8(msg.data(), msg.size()));
    }
}

void MsgLogging::AddErrorMsg(std::string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
    {
        return;  // no point in adding messages if we are shutting down
    }

    auto& str = g_log_msgs.emplace_back("Error: ");
    str << wxString::FromUTF8(msg.data(), msg.size()) << '\n';

    // [Randalphwa - 03-04-2024]
    // If AddErrorMsg is called during an event handler then FAIL_MSG can be called multiple
    // times. While std::unique_lock prevents re-entrance, it can still result in a crash. If
    // you really need to stop in this call, set a breakpoint and stop in the debugger. Do
    // *not* call FAIL_MSG(str);
    // FAIL_MSG(str);

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
    {
        m_msgFrame->AddErrorMsg(ttwx::stepover(str));
    }

    auto* frame = wxGetMainFrame();
    if (frame && frame->IsShown())
    {
        frame->setRightStatusField(str);
    }
}

void MsgLogging::OnNodeSelected()
{
    if (!m_bDestroyed && m_msgFrame)
    {
        m_msgFrame->OnNodeSelected();
    }
}

void MsgLogging::DoLogRecord(wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info)
{
    if (wxGetApp().isMainFrameClosing())
    {
        return;
    }

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
                {
                    m_msgFrame->Add_wxErrorMsg(ttwx::stepover(str));
                }

                auto* frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                {
                    frame->setRightStatusField(str);
                }
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
                {
                    m_msgFrame->Add_wxWarningMsg(ttwx::stepover(str));
                }

                auto* frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                {
                    frame->setRightStatusField(str);
                }
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
                {
                    m_msgFrame->Add_wxInfoMsg(ttwx::stepover(str));
                }

                auto* frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                {
                    frame->setRightStatusField(str);
                }
            }

            // Following is for wxLogGui
            m_aMessages.Add(msg);
            m_aSeverity.Add(wxLOG_Message);
            m_bHasMessages = true;
            break;

        case wxLOG_Status:
            {
                auto* frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                {
                    frame->setRightStatusField(msg);
                }
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
            // just ignore those: passing them to the base class would result in asserts from
            // DoLogText() because DoLogTextAtLevel() would call it as it doesn't know how to handle
            // these levels otherwise
            break;
    }
}
