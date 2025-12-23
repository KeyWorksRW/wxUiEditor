/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/app.h>  // wxAppBase class and macros used for declaration of wxApp

#include "tt_string_vector.h"  // tt_string_vector -- Read/Write line-oriented strings/files

#if defined(_DEBUG)
    #include <memory>  // std::unique_ptr

class wxMessageOutputStderr;
#endif

class Project;
class GenResults;

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

    auto static isFireCreationMsgs() -> bool;

    auto static isPjtMemberPrefix() -> bool;

    // Returns true if command line option --gen_coverage is specified. The assumption is
    // that after code generation, syntax checks will be performed on all languages, and some
    // warning messages need not be generated (such as language not supporting a widget
    // type).
    auto isCoverageTesting() const noexcept -> bool { return m_is_coverage_testing; }

    void static ShowMsgWindow();
    auto static AutoMsgWindow() -> bool;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    // Don't make this static or Bind() will not work
    void DbgCurrentTest(wxCommandEvent& event);
#endif

#if defined(_DEBUG)
    // Writes to stderr even when running as a GUI application
    void DebugOutput(const wxString& str);
#endif

    void setMainFrameClosing() { m_isMainFrameClosing = true; }
    auto isMainFrameClosing() const -> bool { return m_isMainFrameClosing; }

    auto get_ProjectVersion() const { return m_ProjectVersion; }

    auto AskedAboutMissingDir(const wxString& path) -> bool
    {
        return (m_missing_dirs.contains(path));
    }
    void AddMissingDir(const wxString& path) { m_missing_dirs.insert(path); }

    auto isDarkMode() const noexcept -> bool { return m_isDarkMode; }
    auto isDarkHighContrast() const noexcept -> bool { return m_isDarkHighContrast; }

    // Determines whether the testing menu is enabled
    auto isTestingMenuEnabled() const noexcept -> bool { return m_TestingMenuEnabled; }
    void set_TestingMenuEnabled(bool value) noexcept { m_TestingMenuEnabled = value; }

    // Determines whether the testing switch is enabled
    bool isTestingSwitch() const noexcept { return m_is_testing_switch; }
    void setTestingSwitch(bool value) noexcept { m_is_testing_switch = value; }

    // TODO: [Randalphwa - 12-09-2025] Verify() sets this, but no code generation functions check
    // it. This might be a good candidate for using wxMessageOutputDebug(), or just expanded
    // messages for the log file if we are creating oen.

    // Returns true if --verbose is specified on the command line.
    auto isVerboseCodeGen() const noexcept -> bool { return m_is_verbose_codegen; }
    void set_VerboseCodeGen(bool value) noexcept { m_is_verbose_codegen = value; }

    // Returns true if code is being generated from the command line.
    auto is_Generating() const noexcept -> bool { return m_is_generating; }
    void set_Generating(bool value) noexcept { m_is_generating = value; }

    // Add warning or error messages to this if is_Generating() is true (which means code is
    // being generated from the command line).
    auto get_CmdLineLog() -> tt_string_vector& { return m_cmdline_log; }

protected:
    bool OnInit() override;

#if defined(_MSC_VER) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)
    void OnFatalException() override;
#endif

    auto OnRun() -> int override;
    auto OnExit() -> int override;

private:
    enum : int
    {
        cmd_no_params = -2,
        cmd_gen_project_not_found = -1,
        cmd_project_file_only = 0,
        cmd_gen_project_not_loaded = 1,
        cmd_gen_success = 2,

    };

    // Returns a positive value if command-line only code generation was requested and handled.
    auto Generate(wxCmdLineParser& parser, bool& is_project_loaded) -> int;

    // Helper methods for Generate()
    static auto ParseGenerationType(wxCmdLineParser& parser) -> std::pair<size_t, bool>;

    [[nodiscard]] static auto FindProjectFile(wxString& filename) -> bool;

    static auto LoadProjectFile(const tt_string& tt_filename, size_t generate_type,
                                bool& is_project_loaded) -> bool;

    static void LogGenerationResults(GenResults& results, std::vector<std::string>& class_list,
                                     bool test_only, std::string_view language_type);

    static void GenerateAllLanguages(size_t generate_type, bool test_only, GenResults& results,
                                     std::vector<std::string>& class_list);
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

    int m_ProjectVersion { 15 };
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

#if defined(_DEBUG)
    std::unique_ptr<wxMessageOutputStderr> m_stderr_output;
#endif
};

DECLARE_APP(App)  // NOLINT (cppcheck-suppress)

// Temporarily disables the testing menu for the scope of this object.
// Restores the previous state in the destructor.
class DisableTestingMenuScope
{
public:
    DisableTestingMenuScope() : m_was_enabled(wxGetApp().isTestingMenuEnabled())
    {
        wxGetApp().set_TestingMenuEnabled(false);
    }

    ~DisableTestingMenuScope() { wxGetApp().set_TestingMenuEnabled(m_was_enabled); }

    // Delete copy and move constructors
    DisableTestingMenuScope(const DisableTestingMenuScope&) = delete;
    auto operator=(const DisableTestingMenuScope&) -> DisableTestingMenuScope& = delete;
    DisableTestingMenuScope(DisableTestingMenuScope&&) = delete;
    auto operator=(DisableTestingMenuScope&&) -> DisableTestingMenuScope& = delete;

private:
    bool m_was_enabled;
};

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
