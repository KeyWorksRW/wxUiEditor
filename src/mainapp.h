/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/app.h>  // wxAppBase class and macros used for declaration of wxApp

#include "tt_string_vector.h"  // tt_string_vector -- Read/Write line-oriented strings/files

class Project;

namespace pugi
{
    class xml_document;
}

class ImportXML;
class MainFrame;
class ProjectSettings;
class EmbeddedImage;

struct ImageBundle;

[[maybe_unused]] constexpr const auto ImportProjectVersion = 13;

class App : public wxApp
{
public:
    App();

    MainFrame* getMainFrame() { return m_frame; }

    bool isFireCreationMsgs() const;

    bool isPjtMemberPrefix() const;

    // Returns true if command line option --gen_coverage is specified. The assumption is
    // that after code generation, syntax checks will be performed on all languages, and some
    // warning messages need not be generated (such as language not supporting a widget
    // type).
    bool isCoverageTesting() const noexcept { return m_is_coverage_testing; }

    void ShowMsgWindow();
    bool AutoMsgWindow() const;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    void DbgCurrentTest(wxCommandEvent& event);
#endif

    void setMainFrameClosing() { m_isMainFrameClosing = true; }
    bool isMainFrameClosing() { return m_isMainFrameClosing; }

    auto get_ProjectVersion() { return m_ProjectVersion; }

    bool AskedAboutMissingDir(const wxString path)
    {
        return (m_missing_dirs.find(path) != m_missing_dirs.end());
    }
    void AddMissingDir(const wxString path) { m_missing_dirs.insert(path); }

    bool isDarkMode() const noexcept { return m_isDarkMode; }
    bool isDarkHighContrast() const noexcept { return m_isDarkHighContrast; }

    // Determines whether the testing menu is enabled
    bool isTestingMenuEnabled() const noexcept { return m_TestingMenuEnabled; }

    // Determines whether the testing switch is enabled
    bool isTestingSwitch() const noexcept { return m_is_testing_switch; }
    void setTestingSwitch(bool value) noexcept { m_is_testing_switch = value; }

    // Returns true if --verbose is specified on the command line.
    bool isVerboseCodeGen() const noexcept { return m_is_verbose_codegen; }

    // Returns true if code is being generated from the command line.
    bool is_Generating() const noexcept { return m_is_generating; }

    // Add warning or error messages to this if is_Generating() is true (which means code is
    // being generated from the command line).
    tt_string_vector& get_CmdLineLog() { return m_cmdline_log; }

protected:
    bool OnInit() override;

#if defined(_MSC_VER) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)
    void OnFatalException() override;
#endif

    int OnRun() override;
    int OnExit() override;

    // Returns a positive value if command-line only code generation was requested and handled.
    int Generate(wxCmdLineParser& parser, bool& is_project_loaded);

private:
    // Every time we try to write to a directory that doesn't exist, we ask the user if they
    // want to create it. If they choose No then we store the path here and never ask again
    // for the current session.
    std::set<wxString> m_missing_dirs;

    // If code is being generated from the command line, then error/warning messages should
    // be added to this vector -- they will be written to a log file when code generation is
    // complete.
    tt_string_vector m_cmdline_log;

    // ProjectSettings* m_pjtSettings { nullptr };

    MainFrame* m_frame { nullptr };

    int m_ProjectVersion;
    bool m_isMainFrameClosing { false };
    // bool m_isProject_updated { false };
    bool m_TestingMenuEnabled { false };
    bool m_is_testing_switch { false };
    bool m_is_generating { false };       // true if generating code from the command line
    bool m_is_verbose_codegen { false };  // true if verbose code generation is enabled (--verbose)
    bool m_is_coverage_testing {
        false
    };  // true if generating code for test coverage (--gen_coverage)

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
