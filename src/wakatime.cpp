/////////////////////////////////////////////////////////////////////////////
// Purpose:   Updates WakaTime metrics
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <time.h>

#include <wx/filename.h>

#include "wakatime.h"  // WakaTime

#include "mainapp.h"          // App -- Main application class
#include "preferences.h"      // PREFS -- Preferences
#include "project_handler.h"  // ProjectHandler class

WakaTime::WakaTime()
{
    auto result = time(nullptr);
    if (result != -1)
    {
        m_last_heartbeat = static_cast<intmax_t>(result);
    }

    SetWakaExePath();
}

bool WakaTime::IsWakaTimeAvailable()
{
    auto result = wxFileName::GetHomeDir();
    if (result.IsEmpty())
    {
        return false;
    }

    tt_string waka_cli(result.utf8_string());
    waka_cli.append_filename(".wakatime");
    if (!waka_cli.dir_exists())
    {
        return false;
    }

#if defined(_WIN32)
    waka_cli.append_filename("wakatime-cli-windows-amd64.exe");
    if (waka_cli.file_exists())
    {
        return true;
    }
#endif  // _WIN32

    waka_cli.remove_filename();
    waka_cli.append_filename("wakatime-cli");

#if defined(_WIN32)
    waka_cli.append_filename("wakatime-cli.exe");
#else
    waka_cli.append_filename("wakatime-cli");
#endif  // _WIN32

    // REVIEW: [KeyWorks - 10-13-2021] This needs verification on Unix! Without testing, I don't really know if this is where
    // wakatime is placed.

    if (waka_cli.file_exists())
    {
        return true;
    }

    return false;
}

void WakaTime::SetWakaExePath()
{
    auto result = wxFileName::GetHomeDir();
    ASSERT_MSG(!result.IsEmpty(), "IsWakaTimeAvailable() must have returned true before calling SetWakaExePath()!");

    m_waka_cli = result.utf8_string();
    m_waka_cli.append_filename(".wakatime");
    ASSERT_MSG(m_waka_cli.dir_exists(), "IsWakaTimeAvailable() must have returned true before calling SetWakaExePath()!");
    if (!m_waka_cli.dir_exists())
    {
        m_waka_cli.clear();
        return;
    }

#if defined(_WIN32)
    tt_string win_cli(m_waka_cli);
    win_cli.append_filename("wakatime-cli-windows-amd64.exe");
    if (win_cli.file_exists())
    {
        m_waka_cli = win_cli;

        // append_filename uses forward slashes, but that might be a problem when running the executable on Windows, so
        // switch to backslash to be sure it works.
        m_waka_cli.Replace("/", "\\", true);

        return;
    }
#endif  // _WIN32

    m_waka_cli.append_filename("wakatime-cli");

#if defined(_WIN32)
    m_waka_cli.append_filename("wakatime-cli.exe");
#else
    m_waka_cli.append_filename("wakatime-cli");
#endif  // _WIN32

    // REVIEW: [KeyWorks - 10-13-2021] This needs verification on Unix! Without testing, I don't really know if this is where
    // wakatime is placed.

    if (!m_waka_cli.file_exists())
    {
        m_waka_cli.clear();
    }
}

// Number of seconds before sending WakaTime a heartbeat. WakaTime docs recommend a two minute interval (120 seconds).

constexpr const intmax_t waka_interval = 120;

void WakaTime::SendHeartbeat(bool FileSavedEvent)
{
    if (!Preferences().is_WakaTimeEnabled())
    {
        return;
    }

    if (m_waka_cli.empty())
    {
        return;
    }

    auto result = time(nullptr);
    if (result != -1)
    {
        if (FileSavedEvent || (result > m_last_heartbeat && (result - m_last_heartbeat >= waka_interval)))
        {
            m_last_heartbeat = static_cast<intmax_t>(result);
            tt_wxString cmd;
            cmd << m_waka_cli.wx_str()
                << " --plugin \"wxUiEditor/0.5.0 wxUiEditor-wakatime/0.5.0\" --category designing --project ";
            tt_wxString name = Project.ProjectFile().filename();
            name.remove_extension();
            cmd << name.wx_str();
            cmd << " --entity \"" << Project.ProjectFile() << "\"";
            if (FileSavedEvent)
            {
                cmd << " --write";
            }

            wxExecute(cmd, wxEXEC_HIDE_CONSOLE);
        }
    }
}

void WakaTime::ResetHeartbeat()
{
    if (Preferences().is_WakaTimeEnabled())
    {
        auto result = time(nullptr);

        if (result > m_last_heartbeat && (result - m_last_heartbeat >= waka_interval))
        {
            // If the user just switched away for a short period of time, we'll continue sending the heartbeats normally.
            // However, if too much time has passed, then reset the heartbeat timer so that the user doesn't get credited for
            // time spent with another app activated.

            m_last_heartbeat = static_cast<intmax_t>(result);
        }
    }
}
