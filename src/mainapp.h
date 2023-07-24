/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/app.h>  // wxAppBase class and macros used for declaration of wxApp

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

class App : public wxApp
{
public:
    App();

    MainFrame* getMainFrame() { return m_frame; }

    bool isFireCreationMsgs() const;

    bool isPjtMemberPrefix() const;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)

    void ShowMsgWindow();
    bool AutoMsgWindow() const;

    void DbgCurrentTest(wxCommandEvent& event);
#endif

    void setMainFrameClosing() { m_isMainFrameClosing = true; }
    bool isMainFrameClosing() { return m_isMainFrameClosing; }

    auto getProjectVersion() { return m_ProjectVersion; }

    bool AskedAboutMissingDir(const wxString path) { return (m_missing_dirs.find(path) != m_missing_dirs.end()); }
    void AddMissingDir(const wxString path) { m_missing_dirs.insert(path); }

    bool isDarkMode() const noexcept { return m_isDarkMode; }
    bool isDarkHighContrast() const noexcept { return m_isDarkHighContrast; }

    bool isTestingMenuEnabled() const noexcept { return m_TestingMenuEnabled; }

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
    // bool m_isProject_updated { false };
    bool m_TestingMenuEnabled { false };

#if (DARK_MODE)
    bool m_isDarkMode { true };
#else
    bool m_isDarkMode { false };
#endif

#if (DARK_MODE && DARK_HIGH_CONTRAST)
    bool m_isDarkHighContrast { true };
#else
    bool m_isDarkHighContrast { false };
#endif
};

DECLARE_APP(App)
