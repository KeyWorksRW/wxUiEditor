/////////////////////////////////////////////////////////////////////////////
// Purpose:   Message logging class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-19-2026]

#include "msg_logging.h"

#include "mainapp.h"      // App -- Main application class
#include "mainframe.h"    // MainFrame -- Main window frame
#include "msgframe.h"     // MsgFrame -- Stores messages
#include "preferences.h"  // Set/Get wxUiEditor preferences

// Posted -- not sent -- when the message window has to be created. Creating the window runs
// wxWidgets code that can raise an assertion, and ShowLogger() is reached from the assertion
// handler, so the window has to be built from a clean stack rather than from inside an assertion.
wxDEFINE_EVENT(EVT_SHOW_MSG_LOGGER, wxThreadEvent);

// Message history -- MsgFrame replays the entire vector when the window is created.
//
// Never hold a reference to an element of g_log_msgs across a call that can re-enter the logging
// functions below. MsgFrame methods, setRightStatusField() and ShowLogger() all run wxWidgets
// code that can log again (a nested assertion or wxLog call), and an emplace_back() from such a
// re-entrant call can reallocate the vector, leaving that reference dangling. Build the entry in
// a local wxString and push a copy instead -- the copy is deliberate.
//
// Every MSG_*() function and MsgLogging::DoLogRecord() runs on the main (GUI) thread only, and
// MsgFrame iterates this vector from that same thread, so g_log_msgs needs no lock.
MsgLogging* g_pMsgLogging { nullptr };  // NOLINT (cppcheck-suppress)
std::vector<wxString> g_log_msgs;       // NOLINT (cppcheck-suppress)

void MSG_INFO(const std::string& msg)
{
    if (g_pMsgLogging)
    {
        g_pMsgLogging->AddInfoMsg(msg);
    }
}

void MSG_WARNING([[maybe_unused]] const std::string& msg)
{
    if (g_pMsgLogging)
    {
        g_pMsgLogging->AddWarningMsg(msg);
    }
}

void MSG_ASSERTION([[maybe_unused]] const std::string& msg)
{
    if (g_pMsgLogging)
    {
        g_pMsgLogging->AddAssertionMsg(msg);
    }
}

void MSG_ERROR([[maybe_unused]] const std::string& msg)
{
    if (g_pMsgLogging)
    {
        g_pMsgLogging->AddErrorMsg(msg);
    }
}

MsgLogging::MsgLogging()
{
    m_show_logger_handler.Bind(EVT_SHOW_MSG_LOGGER, &MsgLogging::OnShowLoggerEvent, this);
}

void MsgLogging::ShowLogger()
{
    if (m_bDestroyed)
    {
        // Defer creating the window to the event loop. ShowLogger() is called by the assertion
        // handler, and building the window runs wxWidgets code that can raise another assertion --
        // which would then have to be handled while the current assertion is still in progress.
        if (!m_isPostPending && !wxGetApp().isMainFrameClosing())
        {
            m_isPostPending = true;
            wxQueueEvent(&m_show_logger_handler, new wxThreadEvent(EVT_SHOW_MSG_LOGGER));
        }
        return;
    }

    if (m_msgFrame)
    {
        m_msgFrame->Show();
    }
}

void MsgLogging::OnShowLoggerEvent([[maybe_unused]] wxThreadEvent& event)
{
    m_isPostPending = false;

    if (wxGetApp().isMainFrameClosing())
    {
        return;  // don't create a window while the application is shutting down
    }

    if (!m_bDestroyed && m_msgFrame)
    {
        // A ShowLogger() call made after this event was queued already created the window.
        m_msgFrame->Show();
        return;
    }

    // Everything added while this event was queued is already in g_log_msgs, and MsgFrame
    // displays the entire vector, so no message added before now is lost.
    //
    // The frame *must* be parented to the main frame. MsgFrame is a top-level window, and
    // wxWidgets deletes a top-level window only as part of its parent's destruction (see
    // wxWindowMSW::~wxWindowMSW, which calls DestroyChildren()). An parentless frame outlives the
    // main frame, and because it is a top-level window it also keeps the app's main loop alive
    // (wxTopLevelWindowBase::IsLastBeforeExit()), leaving a stray message window on screen with no
    // main window behind it. CloseLogger() alone cannot cover this: it is called from
    // MainFrame::OnClose() only, so it is skipped on any shutdown path that does not deliver a
    // close event to the main frame.
    MainFrame* const main_frame = wxGetMainFrame();
    if (!main_frame)
    {
        return;  // no window to own the frame, and an parentless one could never be cleaned up
    }

    // Lifetime contract: MsgFrame stores the pointers passed here -- g_log_msgs and m_bDestroyed
    // (a MsgLogging member). Both must outlive the frame, so CloseLogger() must close the frame
    // before this MsgLogging object and before g_log_msgs are destroyed. See MsgFrame's
    // constructor for its side of the contract.
    m_msgFrame = new MsgFrame(&g_log_msgs, &m_bDestroyed, main_frame);
    m_bDestroyed = false;
    m_msgFrame->Show();
}

void MsgLogging::CloseLogger()
{
    if (!m_bDestroyed && m_msgFrame)
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
        // Deliberate copy: a reference into g_log_msgs can dangle if logging re-enters.
        wxString log_entry = wxString::FromUTF8(msg.data(), msg.size());
        log_entry << '\n';
        g_log_msgs.emplace_back(log_entry);

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
            m_msgFrame->AddInfoMsg(log_entry.ToStdString());
        }
    }

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    MainFrame* frame = wxGetMainFrame();
    if (frame && frame->IsShown())
    {
        frame->setRightStatusField(wxString::FromUTF8(msg.data(), msg.size()));
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
        // Deliberate copy: a reference into g_log_msgs can dangle if logging re-enters.
        wxString log_entry("Event: ");
        log_entry << wxString::FromUTF8(msg.data(), msg.size()) << '\n';
        g_log_msgs.emplace_back(log_entry);

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
            m_msgFrame->AddEventMsg(log_entry.ToStdString());
        }
    }

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    MainFrame* frame = wxGetMainFrame();
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
        // Deliberate copy: a reference into g_log_msgs can dangle if logging re-enters.
        wxString log_entry("Warning: ");
        log_entry << wxString::FromUTF8(msg.data(), msg.size()) << '\n';
        g_log_msgs.emplace_back(log_entry);

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
            m_msgFrame->AddWarningMsg(wxue::stepover(log_entry));
        }
    }

    if (!g_pMsgLogging)  // g_pMsgLogging doesn't get created until the main window is created
    {
        return;
    }

    MainFrame* frame = wxGetMainFrame();
    if (frame && frame->IsShown())
    {
        frame->setRightStatusField(wxString("Warning: ")
                                   << wxString::FromUTF8(msg.data(), msg.size()));
    }
}

void MsgLogging::AddAssertionMsg(std::string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
    {
        return;  // no point in adding messages if we are shutting down
    }

    if (!(UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING))
    {
        return;
    }

    wxString assert_body = wxString::FromUTF8(msg.data(), msg.size());
    assert_body << '\n';

    wxString log_entry("Assertion: ");
    log_entry << assert_body;
    g_log_msgs.emplace_back(log_entry);

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
        //
        // MsgFrame::AddAssertionMsg() prepends the colored "Assertion: " prefix itself, so only
        // the message body is passed to it.
        m_msgFrame->AddAssertionMsg(assert_body.utf8_string());
    }

    // The status bar is deliberately left untouched. This runs while an assertion is being
    // handled, and ueStatusBar::DoUpdateStatusText() asserts when the number of panes doesn't
    // match -- updating it here is what made pressing "Continue" hang the application.
}

void MsgLogging::AddErrorMsg(std::string_view msg)
{
    if (wxGetApp().isMainFrameClosing())
    {
        return;  // no point in adding messages if we are shutting down
    }

    // Deliberate copy: a reference into g_log_msgs can dangle if logging re-enters.
    wxString log_entry("Error: ");
    log_entry << wxString::FromUTF8(msg.data(), msg.size()) << '\n';
    g_log_msgs.emplace_back(log_entry);

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
        m_msgFrame->AddErrorMsg(wxue::stepover(log_entry));
    }

    MainFrame* frame = wxGetMainFrame();
    if (frame && frame->IsShown())
    {
        // log_entry ends in '\n' for the message window; the status field must not.
        frame->setRightStatusField(log_entry.Left(log_entry.length() - 1));
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
                // Deliberate copy: a reference into g_log_msgs can dangle if logging re-enters.
                wxString log_entry("wxError: ");
                log_entry << msg.utf8_string() << '\n';
                g_log_msgs.emplace_back(log_entry);

                if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW) && !m_isFirstShown)
                {
                    m_isFirstShown = true;
                    ShowLogger();
                }

                else if (!m_bDestroyed)
                {
                    m_msgFrame->Add_wxErrorMsg(wxue::stepover(log_entry));
                }

                MainFrame* frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                {
                    frame->setRightStatusField(log_entry.Left(log_entry.length() - 1));
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
                // Deliberate copy: a reference into g_log_msgs can dangle if logging re-enters.
                wxString log_entry("wxWarning: ");
                log_entry << msg.utf8_string() << '\n';
                g_log_msgs.emplace_back(log_entry);

                if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW) && !m_isFirstShown)
                {
                    m_isFirstShown = true;
                    ShowLogger();
                }

                else if (!m_bDestroyed)
                {
                    m_msgFrame->Add_wxWarningMsg(wxue::stepover(log_entry));
                }

                MainFrame* frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                {
                    frame->setRightStatusField(log_entry.Left(log_entry.length() - 1));
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
                // Deliberate copy: a reference into g_log_msgs can dangle if logging re-enters.
                wxString log_entry("wxInfo: ");
                log_entry << msg.utf8_string() << '\n';
                g_log_msgs.emplace_back(log_entry);

                if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW) && !m_isFirstShown)
                {
                    m_isFirstShown = true;
                    ShowLogger();
                }

                else if (!m_bDestroyed)
                {
                    m_msgFrame->Add_wxInfoMsg(wxue::stepover(log_entry));
                }

                MainFrame* frame = wxGetMainFrame();
                if (frame && frame->IsShown())
                {
                    frame->setRightStatusField(log_entry.Left(log_entry.length() - 1));
                }
            }

            // Following is for wxLogGui
            m_aMessages.Add(msg);
            m_aSeverity.Add((int) level);
            m_aTimes.Add((long) (info.timestampMS / 1000));
            m_bHasMessages = true;
            break;

        case wxLOG_Status:
            {
                MainFrame* frame = wxGetMainFrame();
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
            wxLog::DoLogRecord(level, msg, info);
            break;

        case wxLOG_Progress:
        case wxLOG_User:
            // just ignore those: passing them to the base class would result in asserts from
            // DoLogText() because DoLogTextAtLevel() would call it as it doesn't know how to handle
            // these levels otherwise
            break;

        default:
            wxLog::DoLogRecord(level, msg, info);
            break;
    }
}
