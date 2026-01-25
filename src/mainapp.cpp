/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <chrono>

#include <wx/arrstr.h>   // wxArrayString class
#include <wx/cmdline.h>  // wxCmdLineParser and related classes for parsing the command
#include <wx/config.h>   // wxConfig base header
#include <wx/cshelp.h>   // Context-sensitive help support classes
#include <wx/dir.h>      // wxDir is a class for enumerating the files in a directory
#include <wx/filedlg.h>  // wxFileDialog base header
#include <wx/msgout.h>   // wxMessageOutput and related classes
#include <wx/sysopt.h>   // wxSystemOptions
#include <wx/utils.h>    // Miscellaneous utilities

#include "mainapp.h"

#include "gen_common.h"                // Common component functions
#include "gen_results.h"               // Code generation file writing functions
#include "internal/msg_logging.h"      // MsgLogging -- Message logging class
#include "internal/node_search_dlg.h"  // FindNodeByClassName
#include "mainframe.h"                 // MainFrame -- Main window frame
#include "node_creator.h"              // NodeCreator class
#include "preferences.h"               // Set/Get wxUiEditor preferences
#include "project_handler.h"           // ProjectHandler class
#include "utils.h"                     // Utility functions that work with properties
#include "verify_codegen.h"  // VerifyCodeGen -- Verify that code generation did not change
#include "version.h"         // Version numbers and other constants

#include "frozen/map.h"  // frozen::map
#include "frozen/set.h"  // frozen::set

#include "ui/startup_dlg.h"  // StartupDlg -- Dialog to display if wxUE is launched with no arguments

#if defined(_WIN32) && defined(_MSC_VER)
    #pragma comment(lib, "kernel32.lib")
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "gdi32.lib")
    #pragma comment(lib, "comctl32.lib")
    #pragma comment(lib, "comdlg32.lib")
    #pragma comment(lib, "shell32.lib")

    #pragma comment(lib, "rpcrt4.lib")
    #pragma comment(lib, "advapi32.lib")

    #if wxUSE_URL_NATIVE
        #pragma comment(lib, "wininet.lib")
    #endif
#endif

// add_executable(wxUiEditor WIN32 -- this must be used if wxIMPLEMENT_APP is used.

// If wxIMPLEMENT_APP is used, then std::cout and std::cerr will not work.

// If wxIMPLEMENT_APP_CONSOLE is used, a console will be created if the app isn't being run from a
// console, however std::cout and std::cerr will work.

wxIMPLEMENT_APP(App);  // NOLINT (cppcheck-suppress)
// wxIMPLEMENT_APP_CONSOLE(App);

#if defined(_WIN32) && defined(_DEBUG)

// There are several classes that are created during initialization which are not explicitly deleted
// before the memory check, so don't be surprised to see quite a few memory "leaks"

// Uncomment the following to turn on a memory leak dump on exit.
// #define USE_CRT_MEMORY_DUMP

#endif  // _WIN32 && defined(_DEBUG)

namespace
{
    // Static storage for command-line filename (populated by ParseGenerationType or
    // GetCommandLineFilename)
    wxString s_filename;  // NOLINT (cppcheck-suppress)

    // Helper to process and normalize a filename (add extension, handle wildcards)
    auto ProcessFilename(wxString& filename) -> void
    {
        if (filename.empty())
        {
            return;
        }

        // Handle missing extension before wildcard expansion
        // Check if filename contains wildcards - if so, only add extension if no dot present
        bool has_wildcards = filename.Contains('*') || filename.Contains('?');

        if (has_wildcards)
        {
            // For wildcards, only add extension if there's no dot at all
            if (!filename.Contains('.'))
            {
                filename += ".wxui";
            }
        }
        else
        {
            // For non-wildcards, use wxFileName to properly check for extension
            wxFileName wxfn(filename);
            if (!wxfn.HasExt())
            {
                wxfn.SetExt("wxui");
                filename = wxfn.GetFullPath();
            }
        }

        // Handle wildcards after extension is added
        if (has_wildcards)
        {
            wxDir dir;
            dir.Open("./");
            if (!dir.GetFirst(&filename, filename, wxDIR_FILES))
            {
                // No match found
                filename.clear();
            }
        }
    }

    // Helper to retrieve and process filename from command line
    auto GetCommandLineFilename(wxCmdLineParser& parser) -> wxString&
    {
        if (!s_filename.empty())
        {
            return s_filename;
        }

        // Try to get filename from parameter
        if (parser.GetParamCount() > 0)
        {
            s_filename = parser.GetParam(0);
            ProcessFilename(s_filename);
        }
        // Otherwise s_filename may have been set by ParseGenerationType

        return s_filename;
    }
}  // anonymous namespace

#if defined(_WIN32)
    #include <wx/msw/darkmode.h>

class DarkSettings : public wxDarkModeSettings
{
public:
    [[nodiscard]] auto GetColour(wxSystemColour index) -> wxColour
    {
        return UserPrefs.GetColour(index);
    }
};

#endif

App::App() = default;

auto App::OnInit() -> bool
{
#if defined(_WIN32) && defined(_DEBUG)
    #if !defined(USE_CRT_MEMORY_DUMP)

    _CrtSetDbgFlag(0);

    #else

    // If memory dump shows a leak, uncomment the following and use the memory dump number you want
    // to check. Works with Visual Studio Debugger, does not work with Visual Studio Code debugger.

    // _CrtSetBreakAlloc(25045);
    #endif
#endif

    wxInitAllImageHandlers();

    // The name is sort of a standard. More importantly, it is sometimes used as the mask in Windows
    // bitmaps for toolbar images.
    wxTheColourDatabase->AddColour("Grey94", wxColour(240, 240, 240));

#ifdef _MSC_VER
    #if defined(wxUSE_ON_FATAL_EXCEPTION)
    ::wxHandleFatalExceptions(true);
    #endif
#endif

#if defined(_WIN32)
    // Docs say we need this since our toolbar bitmaps >16 colors
    wxSystemOptions::SetOption(
        "msw.remap", 0);  // only remap background of toolbar bitmaps, not the actual content
#endif

    // If we're just providing text-popups for help, then this is all we need.
    wxHelpProvider::Set(new wxSimpleHelpProvider);

    SetVendorName("KeyWorks");
    UserPrefs.ReadConfig();

    if (UserPrefs.is_DarkMode())
    {
#if defined(_WIN32)
        auto* DarkModeSettings = new DarkSettings;
        MSWEnableDarkMode(0, DarkModeSettings);
#else
        // Unlike MSW, this can be set at any time and it will affect all future windows. Note,
        // however, that we have no control over the specific colors used, so we can't support our
        // High Contrast mode that we support in Windows.
        SetAppearance(wxApp::Appearance::Dark);
#endif
    }

#if defined(_DEBUG)
    m_stderr_output = std::make_unique<wxMessageOutputStderr>();
#endif

    return true;
}

auto App::OnRun() -> int
{
#if defined(_DEBUG)
    #if defined(_WIN32)
    // Attach to parent console for command-line output
    // This must be done early, before any output attempts
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        FILE* file_ptr;  // NOLINT (cppcheck-suppress)
        freopen_s(&file_ptr, "CONOUT$", "w", stdout);
        freopen_s(&file_ptr, "CONOUT$", "w", stderr);
    }
    #else
    // On Unix/Mac, stdout/stderr are automatically connected when run from terminal
    #endif
#endif

    NodeCreation.Initialize();

    wxCmdLineParser parser(argc, argv);
    OnInitCmdLine(parser);
    parser.AddParam("Project filename", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

    // Because this is a GUI app and may not have been run from a console, std::cout and
    // std::cerr will not work. Instead, messages are written to a log file. The log file is
    // the project filename with the extension changed to ".log".

    // Used with gen_*, test_*, or verify_* to limit generation to a single form
    parser.AddLongOption("form", "limit generation to specified form class name",
                         wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);

    parser.AddLongOption("gen_cpp", "generate C++ files and exit");
    parser.AddLongOption("gen_perl", "generate Perl files and exit");
    parser.AddLongOption("gen_python", "generate python files and exit");
    parser.AddLongOption("gen_ruby", "generate ruby files and exit");
    parser.AddLongOption("gen_xrc", "generate XRC files and exit");

    parser.AddLongOption("gen_all", "generate all language files and exit");

    // verify_* options generate code internally and compare against existing files on disk.
    // If differences are detected, the diff output is written to a .log file (same name as the
    // project file but with .log extension) and a non-zero exit code is returned.
    //
    // With --form "ClassName", verification is limited to a single form's generated files.
    // This is useful for debugging code generation issues with a specific form.
    //
    // Exit codes:
    //   0 = Success (no differences found)
    //   1 = Failure (differences detected - check log file)
    //   2 = File not found
    //   3 = Invalid (e.g., form name not found in project)
    //
    // Agent usage: To properly capture exit codes in PowerShell, use Start-Process:
    //
    //   $proc = Start-Process -FilePath "wxUiEditord.exe" `
    //       -ArgumentList "--verify_cpp","project.wxui","--form","FormClassName" `
    //       -Wait -NoNewWindow -PassThru
    //   $exitCode = $proc.ExitCode
    //
    // If exit code is non-zero, open the log file to examine differences:
    //   key_open with filePath: "<project_dir>/<project_name>.log"

    parser.AddLongSwitch("verify_cpp", "verify generating C++ files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_perl", "verify generating Perl files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_python", "verify generating Python files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_ruby", "verify generating Ruby files did not change",
                         wxCMD_LINE_HIDDEN);

    parser.AddLongSwitch("verify_all", "verify generating all language files did not change",
                         wxCMD_LINE_HIDDEN);

    // Just a quick way to generate perl, python, and ruby
    parser.AddLongOption("gen_quick", "generate all script files and exit", wxCMD_LINE_VAL_STRING,
                         wxCMD_LINE_HIDDEN);

    // Primarily used for codegen_test/ but could be used for other coverage testing as well.
    parser.AddLongOption("gen_coverage", "generate all language types for test coverage",
                         wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);

    // TODO: [Randalphwa - 11-29-2025] Add an "-auto_load" hidden switch that will find all the
    // *.wxui files in the current directory. If there's only one, set that as the project file. If
    // there's more than one, load a dialog to let the user select which project file to load.

    // The "test" options will not write any files, it simply runs the code generation skipping
    // the part where files get written, and generates the log file.

    // TODO: [Randalphwa - 11-29-2025] These could potentially be used similar to verify, with the
    // possibility of using wxMessageOutputDebug() to write the file differences to Debug Console.

    parser.AddLongSwitch("verbose", "verbose log file", wxCMD_LINE_HIDDEN);

    // test_* options run code generation logic without writing any files to disk.
    // Unlike verify_*, test_* does NOT compare against existing files - it only exercises the
    // code generation paths and writes timing/diagnostic info to a log file.
    //
    // Use test_* when you want to:
    //   - Verify that code generation completes without errors/crashes
    //   - Measure code generation performance (timing info in log)
    //   - Debug code generation paths without modifying any files
    //
    // Use verify_* when you want to:
    //   - Detect if generated code would differ from files on disk
    //   - Validate that refactoring didn't change code generation output
    //   - CI/CD pipelines that need to fail if code generation is out of sync
    //
    // With --form "ClassName", test generation is limited to a single form.
    //
    // Exit codes: Always returns 0 unless the project file cannot be loaded.
    // The log file (project_name.log) contains generation timing and any warnings.

    parser.AddLongSwitch("test_cpp", "generate C++ code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_perl", "generate Perl code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_python", "generate Python code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_ruby", "generate Ruby code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_xrc", "generate XRC code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_all", "generate all code and exit", wxCMD_LINE_HIDDEN);

    parser.AddLongSwitch("test_menu", "create test menu to the right of the Help menu",
                         wxCMD_LINE_HIDDEN | wxCMD_LINE_SWITCH_NEGATABLE);
    parser.AddLongSwitch("load_last", "Load last opened project",
                         wxCMD_LINE_HIDDEN | wxCMD_LINE_SWITCH_NEGATABLE);

    parser.AddLongSwitch("data-version", "return current data_version", wxCMD_LINE_HIDDEN);

    parser.Parse();

    // Return current data_version for AI tools and exit immediately
    if (parser.Found("data-version"))
    {
        wxMessageOutput::Get()->Printf("%d", curSupportedVer);
        return 0;
    }

#if defined(INTERNAL_TESTING)
    m_TestingMenuEnabled = true;
#endif
    if (auto result = parser.FoundSwitch("test_menu"); result != wxCMD_SWITCH_NOT_FOUND)
    {
        m_TestingMenuEnabled = (result == wxCMD_SWITCH_ON);
    }
#if defined(_DEBUG)
    m_TestingMenuEnabled = true;
    m_is_testing_switch = true;
#endif  // _DEBUG

    if (wxGetApp().isTestingMenuEnabled() && !g_pMsgLogging)
    {
        g_pMsgLogging = new MsgLogging();
        wxLog::SetActiveTarget(g_pMsgLogging);

        // Use our own assertion handler
        wxSetAssertHandler(ttAssertionHandler);
    }

    /*
        Command-line options are categorized into three types for non-interactive operation:

        gen_* options - Generate code files for the specified language(s) and write them to disk,
                        then exit. Used for build automation and CI/CD pipelines.

        verify_* options - Generate code internally and compare against existing files to verify
                           that code generation produces identical output. Returns non-zero exit
                           code if differences are detected. Does not modify any files.

        test_* options - Execute code generation logic without writing any files. Primarily used
                         for testing code generation paths and generating log files for debugging.
    */

    bool is_verify_mode = false;
    constexpr frozen::set<std::string_view, 5> verify_options = { "verify_cpp", "verify_perl",
                                                                  "verify_python", "verify_ruby",
                                                                  "verify_all" };

    for (const auto& opt: verify_options)
    {
        if (parser.Found(wxString(opt)))
        {
            is_verify_mode = true;
            break;
        }
    }

    bool is_project_loaded = false;

    if (is_verify_mode)
    {
        return VerifyCodeGen(parser, is_project_loaded);
    }

    // A positive return value means code generation was for command-line only
    auto result = Generate(parser, is_project_loaded);
    if (result == cmd_gen_project_not_loaded)
    {
        return 1;
    }
    if (result == cmd_gen_success)
    {
        return 0;
    }

    /*
        Normal GUI mode: Create the main window and handle project loading.

        If a project file was specified on the command line, load it directly. Otherwise, if
        --load_last was specified or enabled in preferences, attempt to load the most recent
        project from history. If no project is loaded by this point, display the startup dialog
        to let the user choose or create a project. The main window is shown only after a project
        is successfully loaded.
    */

    m_frame = new MainFrame();
    ASSERT(m_frame);

    if (result == cmd_project_file_only)
    {
        const auto& filename = GetCommandLineFilename(parser);
        if (!Project.LoadProject(filename, true))
        {
            wxMessageBox(wxString("Unable to load project file: ") << filename,
                         "Project Load Error", wxOK | wxICON_ERROR);
            is_project_loaded = false;
        }
        else
        {
            is_project_loaded = true;
        }
    }
    else if (result == cmd_no_params || result == cmd_gen_project_not_found)
    {
        if (auto switch_result = parser.FoundSwitch("load_last");
            switch_result != wxCMD_SWITCH_NOT_FOUND || UserPrefs.is_LoadLastProject())
        {
            auto& file_history = m_frame->getFileHistory();
            if (file_history.GetCount() == 0)
            {
                is_project_loaded = false;
            }
            else
            {
                wxue::string file = file_history.GetHistoryFile(0).utf8_string();
                if (!file.file_exists())
                {
                    file_history.RemoveFileFromHistory(0);
                    wxMessageBox(wxue::string("Last project file does not exist: ") << file,
                                 "Missing Project File", wxOK | wxICON_ERROR);
                }
                else
                {
                    is_project_loaded = Project.LoadProject(file);
                }
            }
        }
    }

    if (!is_project_loaded)
    {
        is_project_loaded = DsisplayStartupDlg(nullptr);
    }

    if (is_project_loaded)
    {
        m_frame->Show();
        SetTopWindow(m_frame);

#if defined(_DEBUG)
        // if (AutoMsgWindow())
        // ShowMsgWindow();
#endif  // _DEBUG

        return wxApp::OnRun();
    }

    if (!m_frame)
    {
        m_frame->Close();
        m_frame = nullptr;
    }
    return 1;
}

auto App::OnExit() -> int
{
    return wxApp::OnExit();
}

#if defined(_DEBUG)
auto App::DebugOutput(const wxString& str) -> void
{
    if (m_stderr_output)
    {
        m_stderr_output->Output(str);
        fflush(stderr);
    }
}
#endif

auto App::isFireCreationMsgs() -> bool
{
    return (UserPrefs.GetDebugFlags() & Prefs::PREFS_CREATION_MSG);
}

auto App::isPjtMemberPrefix() -> bool
{
    return (UserPrefs.GetProjectFlags() & Prefs::PREFS_PJT_MEMBER_PREFIX);
}

auto App::AutoMsgWindow() -> bool
{
    return (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW);
}

#if defined(_WIN32) && defined(_DEBUG) && defined(wxUSE_ON_FATAL_EXCEPTION) && \
    defined(wxUSE_STACKWALKER)

    #include <wx/stackwalk.h>

class StackLogger : public wxStackWalker
{
public:
    [[nodiscard]] auto GetCalls() -> auto& { return m_calls; }

protected:
    auto OnStackFrame(const wxStackFrame& frame) -> void override
    {
        if (frame.HasSourceLocation())
        {
            wxue::string source;
            source << frame.GetFileName().utf8_string() << ':' << (to_int) frame.GetLine();

            wxString params;
            if (auto paramCount = frame.GetParamCount(); paramCount > 0)
            {
                params << "(";

                for (size_t i = 0; i < paramCount; ++i)
                {
                    wxString type;
                    wxString name;
                    wxString value;
                    if (frame.GetParam(i, &type, &name, &value))
                    {
                        params << type << " " << name << " = " << value << ", ";
                    }
                }

                params << ")";
            }

            if (params.size() > 100)
            {
                params = "(...)";
            }

            m_calls.emplace_back()
                << (to_int) frame.GetLevel() << ' ' << frame.GetName().utf8_string()
                << params.utf8_string() << ' ' << source;
        }
        else
        {
            m_calls.emplace_back()
                << (to_int) frame.GetLevel() << ' ' << frame.GetName().utf8_string();
        }
    }

private:
    std::vector<wxue::string> m_calls;
};

#endif  // defined(_DEBUG) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)

#if defined(_MSC_VER) && defined(wxUSE_ON_FATAL_EXCEPTION)

auto App::OnFatalException() -> void
{
    #if defined(_DEBUG) && defined(wxUSE_STACKWALKER)

    StackLogger logger;
    logger.WalkFromException();
    for (auto& iter: logger.GetCalls())
    {
        // We're only interested in our own source code, so ignore the rest.
        if (!iter.contains("wxUiEditor"))
        {
            continue;
        }

        wxLogDebug(iter.c_str());
    }

    // We now have the relevant call stack displayed in the debugger, so break into it.
    wxTrap();

    #endif  // _DEBUG

    // Let the user know something terrible happened.
    wxMessageBox("A fatal exception has occured!", txtVersion);
}

#endif  // defined(_MSC_VER) && defined(wxUSE_ON_FATAL_EXCEPTION)

auto App::ShowMsgWindow() -> void
{
    g_pMsgLogging->ShowLogger();
}

#if defined(_DEBUG) || defined(INTERNAL_TESTING)

    #include "newdialogs/new_mdi.h"  // NewMdiForm -- Dialog for creating a new MDI application

// Don't make this static or Bind() will not work
auto App::DbgCurrentTest(wxCommandEvent& /* event unused */) -> void  // NOLINT (cppcheck-suppress)
{
    wxGetMainFrame()->SelectNode(Project.get_ProjectNode(), evt_flags::force_selection);

    NewMdiForm dlg(wxGetFrame().getWindow());
    if (dlg.ShowModal() == wxID_OK)
    {
        dlg.CreateNode();
    }
}

#endif

// Helper: Parse command-line options to determine generation type
auto App::ParseGenerationType(wxCmdLineParser& parser) -> std::pair<size_t, bool>
{
    // Map option names to their corresponding generation type values
    constexpr frozen::map<std::string_view, size_t, 8> gen_options = {
        { "gen_cpp", GEN_LANG_CPLUSPLUS },
        { "gen_perl", GEN_LANG_PERL },
        { "gen_python", GEN_LANG_PYTHON },
        { "gen_ruby", GEN_LANG_RUBY },
        { "gen_xrc", GEN_LANG_XRC },
        { "gen_all", (GEN_LANG_CPLUSPLUS | GEN_LANG_PERL | GEN_LANG_PYTHON | GEN_LANG_RUBY) },
        { "gen_quick", (GEN_LANG_PERL | GEN_LANG_PYTHON | GEN_LANG_RUBY) },
        { "gen_coverage", (GEN_LANG_CPLUSPLUS | GEN_LANG_PERL | GEN_LANG_PYTHON | GEN_LANG_RUBY) },
    };

    size_t generate_type = GEN_LANG_NONE;
    bool test_only = false;

    // Check gen_* options (mutually exclusive)
    for (const auto& [option_name, option_type]: gen_options)
    {
        wxString option_filename;
        if (parser.Found(wxString(option_name), &option_filename))
        {
            generate_type = option_type;

            // Store the filename from the option value
            if (!option_filename.empty())
            {
                s_filename = option_filename;
                ProcessFilename(s_filename);
            }

            if (option_name == "gen_coverage")
            {
                wxGetApp().m_is_coverage_testing = true;
            }
            break;  // Only first match counts for gen_* options
        }
    }

    // Check test_* options (can be combined)
    constexpr frozen::map<std::string_view, GenLang, 5> test_options = {
        { "test_cpp", GEN_LANG_CPLUSPLUS }, { "test_perl", GEN_LANG_PERL },
        { "test_python", GEN_LANG_PYTHON }, { "test_ruby", GEN_LANG_RUBY },
        { "test_xrc", GEN_LANG_XRC },
    };

    for (const auto& [option_name, option_type]: test_options)
    {
        wxString option_filename;
        if (parser.Found(wxString(option_name), &option_filename))
        {
            generate_type = (generate_type | option_type);
            test_only = true;

            // Store the filename from the option value (first one wins)
            if (!option_filename.empty() && s_filename.empty())
            {
                s_filename = option_filename;
                ProcessFilename(s_filename);
            }
        }
    }

    return std::make_pair(generate_type, test_only);
}

// Helper: Find project file if filename is empty
[[nodiscard]] auto App::FindProjectFile(wxString& filename) -> bool
{
    wxDir dir;
    dir.Open("./");
    if (!dir.GetFirst(&filename, "*" + std::string(PROJECT_FILE_EXTENSION), wxDIR_FILES))
    {
        wxMessageBox("No project file found in current directory. Filenane is required if "
                     "switch is used.",
                     "Command-line Switch Error", wxOK | wxICON_ERROR);
        return false;
    }
    return true;
}

// Helper: Load or import the project file
auto App::LoadProjectFile(const wxue::string& filename, size_t generate_type,
                          bool& is_project_loaded) -> bool
{
    if (!filename.extension().is_sameas(PROJECT_FILE_EXTENSION, wxue::CASE::either) &&
        !filename.extension().is_sameas(PROJECT_LEGACY_FILE_EXTENSION, wxue::CASE::either))
    {
        is_project_loaded = Project.ImportProject(filename, generate_type == GEN_LANG_NONE);
    }
    else
    {
        is_project_loaded = Project.LoadProject(filename, generate_type == GEN_LANG_NONE);
    }
    return is_project_loaded;
}

// Helper: Log results for each language generation
auto App::LogGenerationResults(GenResults& results, std::vector<std::string>& class_list,
                               bool test_only, std::string_view language_type) -> void
{
    if (results.GetUpdatedFiles().size() || results.GetCreatedFiles().size() || class_list.size())
    {
        if (test_only)
        {
            for (auto& iter: class_list)
            {
                auto& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
                log_msg << "Needs updating: " << iter;
            }
        }
        else
        {
            for (auto& iter: results.GetUpdatedFiles())
            {
                auto& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
                log_msg << "Updated: " << iter;
            }
            for (auto& iter: results.GetCreatedFiles())
            {
                auto& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
                log_msg << "Created: " << iter;
            }
        }
    }
    else
    {
        auto& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
        log_msg << "All " << results.GetFileCount() << " generated " << language_type
                << " files are current";
    }

    for (auto& iter: results.GetMsgs())
    {
        if (iter.contains("Elapsed time"))
        {
            continue;
        }
        auto& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
        log_msg << iter;
    }
}

// Helper: Generate code for all requested languages
auto App::GenerateAllLanguages(size_t generate_type, bool test_only, GenResults& results,
                               std::vector<std::string>& class_list) -> void
{
    // If a form filter is specified, find that form
    Node* form_node = nullptr;
    const auto& form_filter = wxGetApp().get_FormFilter();
    if (!form_filter.empty())
    {
        form_node = FindNodeByClassName(Project.get_ProjectNode(), form_filter);
        if (!form_node)
        {
            auto& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
            log_msg << "Error: Form '" << form_filter << "' not found in project";
            return;
        }
    }

    auto GenCode = [&](GenLang language)
    {
        if (generate_type & language)
        {
            results.Clear();
            class_list.clear();

            // Use filtered form if specified, otherwise entire project
            if (form_node)
            {
                results.SetNodes(form_node);
            }
            else
            {
                results.SetNodes(Project.get_ProjectNode());
            }
            results.SetLanguages(language);

            if (test_only)
            {
                // Use compare_only mode to check what needs updating
                results.SetMode(GenResults::Mode::compare_only);
                std::ignore = results.Generate();

                // Extract filenames from diffs for logging
                for (const auto& diff: results.GetFileDiffs())
                {
                    class_list.emplace_back(diff.filename);
                }
            }
            else
            {
                results.SetMode(GenResults::Mode::generate_and_write);
                std::ignore = results.Generate();
            }

            LogGenerationResults(results, class_list, test_only, GenLangToString(language));
        }
    };

    GenCode(GEN_LANG_CPLUSPLUS);
    GenCode(GEN_LANG_PERL);
    GenCode(GEN_LANG_PYTHON);
    GenCode(GEN_LANG_RUBY);
    GenCode(GEN_LANG_XRC);
}

auto App::Generate(wxCmdLineParser& parser, bool& is_project_loaded) -> int
{
    if (!parser.GetParamCount() && !parser.GetArguments().size())
    {
        return cmd_no_params;
    }

    if (parser.FoundSwitch("verbose") == wxCMD_SWITCH_ON)
    {
        m_is_verbose_codegen = true;
    }

    // Check for form filter option
    wxString form_filter;
    if (parser.Found("form", &form_filter))
    {
        m_form_filter = form_filter.ToStdString();
    }

    auto [generate_type, test_only] = ParseGenerationType(parser);

    const auto& filename_str = GetCommandLineFilename(parser);

    if (generate_type == GEN_LANG_NONE)
    {
        if (filename_str.empty())
        {
            return cmd_no_params;
        }

        return cmd_project_file_only;
    }

    // If we get here then we were asked to generate at least one language type

    wxue::string filename = filename_str;
    filename.make_absolute();
    wxue::string log_file = filename;
    log_file.replace_extension(".log");

    if (!filename.file_exists())
    {
        m_cmdline_log.clear();
        m_cmdline_log.emplace_back(wxue::string("Unable to find project file: ") += filename);
        m_cmdline_log.WriteFile(log_file);
        return 1;
    }

    m_is_generating = true;
    GenResults results;
    if (wxGetApp().isTestingMenuEnabled())
    {
        results.StartClock();
    }

    LoadProjectFile(filename, generate_type, is_project_loaded);
    if (!is_project_loaded)
    {
        m_cmdline_log.clear();
        m_cmdline_log.emplace_back(wxue::string("Unable to load project file: ") += filename);
        m_cmdline_log.WriteFile(log_file);
        return cmd_gen_project_not_loaded;
    }

    m_cmdline_log.clear();
    std::vector<std::string> class_list;
    auto start_time = std::chrono::steady_clock::now();

    GenerateAllLanguages(generate_type, test_only, results, class_list);

    auto& log_msg = m_cmdline_log.emplace_back();
    auto end_time = std::chrono::steady_clock::now();
    size_t total_elapsed_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    log_msg << "Total elapsed time: " << total_elapsed_time << " milliseconds";
    m_cmdline_log.WriteFile(log_file);

    return cmd_gen_success;
}
