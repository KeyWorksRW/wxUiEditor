/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
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

    void ShowMsgWindow();
    bool AutoMsgWindow() const;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    void DbgCurrentTest(wxCommandEvent& event);
#endif

    void setMainFrameClosing() { m_isMainFrameClosing = true; }
    bool isMainFrameClosing() { return m_isMainFrameClosing; }

    auto getProjectVersion() { return m_ProjectVersion; }

    bool AskedAboutMissingDir(const wxString path) { return (m_missing_dirs.find(path) != m_missing_dirs.end()); }
    void AddMissingDir(const wxString path) { m_missing_dirs.insert(path); }

    bool isDarkMode() const noexcept { return m_isDarkMode; }
    bool isDarkHighContrast() const noexcept { return m_isDarkHighContrast; }

    // Determines whether the testing menu is enabled
    bool isTestingMenuEnabled() const noexcept { return m_TestingMenuEnabled; }

    // Determines whether the testing switch is enabled
    bool isTestingSwitch() const noexcept { return m_is_testing_switch; }
    void setTestingSwitch(bool value) noexcept { m_is_testing_switch = value; }

    bool isGenerating() const noexcept { return m_is_generating; }

protected:
    bool OnInit() override;

#if defined(_MSC_VER) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)
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
    bool m_is_testing_switch { false };
    bool m_is_generating { false };  // true if generating code from the command line

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

// Do *NOT* use this before wxGetApp() has been initialized.
// This test is available in release builds with the testing menu enabled.
#define TEST_CONDITION(cond, msg)                                              \
    if (wxGetApp().isTestingMenuEnabled())                                     \
    {                                                                          \
        if (!(cond) && AssertionDlg(__FILE__, __func__, __LINE__, #cond, msg)) \
        {                                                                      \
            wxTrap();                                                          \
        }                                                                      \
    }
