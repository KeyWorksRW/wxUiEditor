/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/app.h>  // wxAppBase class and macros used for declaration of wxApp

#if wxCHECK_VERSION(3, 3, 0)
    #if defined(_WIN32)
        #include <wx/msw/darkmode.h>
    #endif
#endif

#include "node_classes.h"  // Forward defintions of Node classes

class Project;

namespace pugi
{
    class xml_document;
}

class ImportXML;
class MainFrame;
class ProjectSettings;

struct EmbeddedImage;
struct ImageBundle;

constexpr const auto ImportProjectVersion = 13;

#if wxCHECK_VERSION(3, 3, 0)
    #if defined(_WIN32)

class DarkSettings : public wxDarkModeSettings
{
public:
    wxColour GetColour(wxSystemColour index);
};

    #endif  // _WIN32
#endif      // wxCHECK_VERSION(3, 3, 0)

class App : public wxApp
{
public:
    App();

    MainFrame* GetMainFrame() { return m_frame; }

    bool isFireCreationMsgs() const;

    bool IsPjtMemberPrefix() const;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)

    void ShowMsgWindow();
    bool AutoMsgWindow() const;

    void DbgCurrentTest(wxCommandEvent& event);
#endif

    void SetMainFrameClosing() { m_isMainFrameClosing = true; }
    bool isMainFrameClosing() { return m_isMainFrameClosing; }

    void ShowPreferences(wxWindow* parent);

    auto GetProjectVersion() { return m_ProjectVersion; }

    bool AskedAboutMissingDir(const wxString path) { return (m_missing_dirs.find(path) != m_missing_dirs.end()); }
    void AddMissingDir(const wxString path) { m_missing_dirs.insert(path); }

    bool isDarkMode() const noexcept { return m_isDarkMode; }
    bool isDarkHighContrast() const noexcept { return m_isDarkHighContrast; }

protected:
    bool OnInit() override;

#if wxUSE_ON_FATAL_EXCEPTION && wxUSE_STACKWALKER
    void OnFatalException() override;
#endif

    int OnRun() override;
    int OnExit() override;

private:
    // Every time we try to write to a directory that doesn't exist, we ask the user if they
    // want to create it. If they choose No then we store the path here and never ask again
    // for the current session.
    std::set<wxString> m_missing_dirs;

    // ProjectSettings* m_pjtSettings { nullptr };

    MainFrame* m_frame { nullptr };

    int m_ProjectVersion;
    bool m_isMainFrameClosing { false };
    bool m_isProject_updated { false };

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    bool m_isDarkMode { false };
    bool m_isDarkHighContrast { false };
#else
    bool m_isDarkMode { false };
    bool m_isDarkHighContrast { false };
#endif
};

DECLARE_APP(App)
