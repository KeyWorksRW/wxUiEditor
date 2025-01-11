/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <wx/arrstr.h>   // wxArrayString class
#include <wx/cmdline.h>  // wxCmdLineParser and related classes for parsing the command
#include <wx/config.h>   // wxConfig base header
#include <wx/cshelp.h>   // Context-sensitive help support classes
#include <wx/dir.h>      // wxDir is a class for enumerating the files in a directory
#include <wx/filedlg.h>  // wxFileDialog base header
#include <wx/sysopt.h>   // wxSystemOptions
#include <wx/utils.h>    // Miscellaneous utilities

#include "mainapp.h"

#include "bitmaps.h"               // Contains various images handling functions
#include "gen_common.h"            // Common component functions
#include "gen_results.h"           // Code generation file writing functions
#include "internal/msg_logging.h"  // MsgLogging -- Message logging class
#include "mainframe.h"             // MainFrame -- Main window frame
#include "node.h"                  // Node -- Node class
#include "node_creator.h"          // NodeCreator class
#include "preferences.h"           // Set/Get wxUiEditor preferences
#include "project_handler.h"       // ProjectHandler class
#include "utils.h"                 // Utility functions that work with properties

#include "ui/startup_dlg.h"  // StartupDlg -- Dialog to display if wxUE is launched with no arguments

#include "pugixml.hpp"

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

// If wxIMPLEMENT_APP_CONSOLE is used, a console will be created if the app isn't being run from a console,
// however std::cout and std::cerr will work.

wxIMPLEMENT_APP(App);
// wxIMPLEMENT_APP_CONSOLE(App);

#if defined(_WIN32) && defined(_DEBUG)

// There are several classes that are created during initialization which are not explicitly deleted before the memory check,
// so don't be surprised to see quite a few memory "leaks"

// Uncomment the following to turn on a memory leak dump on exit.
// #define USE_CRT_MEMORY_DUMP

#endif  // _WIN32 && defined(_DEBUG)

void ttAssertionHandler(const wxString& filename, int line, const wxString& function, const wxString& cond,
                        const wxString& msg);

tt_string tt_empty_cstr;

#if defined(_WIN32)
    #include <wx/msw/darkmode.h>

class DarkSettings : public wxDarkModeSettings
{
public:
    wxColour GetColour(wxSystemColour index) { return UserPrefs.GetColour(index); }
};

#endif

App::App() {}

bool App::OnInit()
{
#if defined(_WIN32) && defined(_DEBUG)
    #if !defined(USE_CRT_MEMORY_DUMP)

    _CrtSetDbgFlag(0);

    #else

    // If memory dump shows a leak, uncomment the following and use the memory dump number you want to check. Works with
    // Visual Studio Debugger, does not work with Visual Studio Code debugger.

    // _CrtSetBreakAlloc(25045);
    #endif
#endif

    wxInitAllImageHandlers();

    // The name is sort of a standard. More importantly, it is sometimes used as the mask in Windows bitmaps for toolbar
    // images.
    wxTheColourDatabase->AddColour("Grey94", wxColour(240, 240, 240));

#ifdef _MSC_VER
    #if defined(wxUSE_ON_FATAL_EXCEPTION)
    ::wxHandleFatalExceptions(true);
    #endif
#endif

#if defined(_WIN32)
    // Docs say we need this since our toolbar bitmaps >16 colors
    wxSystemOptions::SetOption("msw.remap", 0);  // only remap background of toolbar bitmaps, not the actual content
#endif

    // If we're just providing text-popups for help, then this is all we need.
    wxHelpProvider::Set(new wxSimpleHelpProvider);

    SetVendorName("KeyWorks");
    UserPrefs.ReadConfig();

#if defined(_WIN32)

    // [Randalphwa - 03-29-2023] Currently, this isn't really usable because we hard-code
    // colors in our property sheet and scintilla code displays.
    if (UserPrefs.is_DarkMode())
    {
        auto* DarkModeSettings = new DarkSettings;
        MSWEnableDarkMode(0, DarkModeSettings);
    }
#endif

    return true;
}

int App::OnRun()
{
    NodeCreation.Initialize();
    bool is_project_loaded = false;

    wxCmdLineParser parser(argc, argv);
    OnInitCmdLine(parser);
    parser.AddParam("Project filename", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

    // Because this is a GUI app and may not have been run from a console, std::cout and
    // std::cerr will not work. Instead, messages are written to a log file. The log file is
    // the project filename with the extension changed to ".log".

    parser.AddLongOption("gen_cpp", "generate C++ files and exit");
    parser.AddLongOption("gen_python", "generate python files and exit");
    parser.AddLongOption("gen_ruby", "generate ruby files and exit");
    parser.AddLongOption("gen_haskell", "generate Haskell files and exit");
    parser.AddLongOption("gen_lua", "generate Lua files and exit");
    parser.AddLongOption("gen_perl", "generate Perl files and exit");
    parser.AddLongOption("gen_rust", "generate Rust files and exit");

    parser.AddLongOption("gen_all", "generate all language files and exit");

    // [Randalphwa - 02-08-2023] This probably works, but will remain hidden until it is
    // tested. That said, I'm doubtful that it has any actual value other than for testing -- I
    // just don't see a reason for a user to want to use the command line to generate XRC
    // files.
    parser.AddLongSwitch("gen_xrc", "generate XRC files and exit", wxCMD_LINE_HIDDEN);

    // The "test" options will not write any files, it simply runs the code generation skipping
    // the part where files get written, and generates the log file.

    parser.AddLongSwitch("test_cpp", "generate C++ code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_python", "generate Python code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_ruby", "generate Ruby code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_haskell", "generate Haskell code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_lua", "generate Lua code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_perl", "generate Perl code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_rust", "generate Rust code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_xrc", "generate XRC code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_all", "generate all code and exit", wxCMD_LINE_HIDDEN);

    parser.AddLongSwitch("test_menu", "create test menu to the right of the Help menu",
                         wxCMD_LINE_HIDDEN | wxCMD_LINE_SWITCH_NEGATABLE);
    parser.AddLongSwitch("load_last", "Load last opened project", wxCMD_LINE_HIDDEN | wxCMD_LINE_SWITCH_NEGATABLE);

    parser.Parse();
#if defined(INTERNAL_TESTING)
    m_TestingMenuEnabled = true;
#endif
    if (auto result = parser.FoundSwitch("test_menu"); result != wxCMD_SWITCH_NOT_FOUND)
    {
        m_TestingMenuEnabled = (result == wxCMD_SWITCH_ON ? true : false);
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

    if (parser.GetParamCount() || parser.GetArguments().size())
    {
        wxString filename;
        if (parser.GetParamCount())
        {
            filename = parser.GetParam(0);
        }

        tt_string log_file;
        size_t generate_type = GEN_LANG_NONE;
        bool test_only = false;
        if (parser.Found("gen_cpp", &filename))
        {
            generate_type = GEN_LANG_CPLUSPLUS;
        }
        else if (parser.Found("gen_perl", &filename))
        {
            generate_type = GEN_LANG_PERL;
        }
        else if (parser.Found("gen_python", &filename))
        {
            generate_type = GEN_LANG_PYTHON;
        }
        else if (parser.Found("gen_ruby", &filename))
        {
            generate_type = GEN_LANG_RUBY;
        }
        else if (parser.Found("gen_rust", &filename))
        {
            generate_type = GEN_LANG_RUST;
        }
        else if (parser.Found("gen_xrc", &filename))
        {
            generate_type = GEN_LANG_XRC;
        }
#if GENERATE_NEW_LANG_CODE
        else if (parser.Found("gen_haskell", &filename))
        {
            generate_type = GEN_LANG_HASKELL;
        }
        else if (parser.Found("gen_lua", &filename))
        {
            generate_type = GEN_LANG_LUA;
        }
#endif  // GENERATE_NEW_LANG_CODE

        else if (parser.Found("gen_all", &filename))
        {
            generate_type = (GEN_LANG_CPLUSPLUS | GEN_LANG_PERL | GEN_LANG_PYTHON | GEN_LANG_RUBY | GEN_LANG_RUST);
#if GENERATE_NEW_LANG_CODE
            generate_type |= (GEN_LANG_FORTRAN | GEN_LANG_HASKELL | GEN_LANG_LUA);
#endif  // GENERATE_NEW_LANG_CODE
        }

        if (parser.Found("test_cpp", &filename))
        {
            generate_type = (generate_type | GEN_LANG_CPLUSPLUS);
            test_only = true;
        }
        if (parser.Found("test_perl", &filename))
        {
            generate_type = (generate_type | GEN_LANG_PERL);
            test_only = true;
        }
        if (parser.Found("test_python", &filename))
        {
            generate_type = (generate_type | GEN_LANG_PYTHON);
            test_only = true;
        }
        if (parser.Found("test_ruby", &filename))
        {
            generate_type = (generate_type | GEN_LANG_RUBY);
            test_only = true;
        }
        if (parser.Found("test_rust", &filename))
        {
            generate_type = (generate_type | GEN_LANG_RUST);
            test_only = true;
        }
        if (parser.Found("test_xrc", &filename))
        {
            generate_type = (generate_type | GEN_LANG_XRC);
            test_only = true;
        }
#if GENERATE_NEW_LANG_CODE
        if (parser.Found("test_haskell", &filename))
        {
            generate_type = (generate_type | GEN_LANG_HASKELL);
            test_only = true;
        }
        if (parser.Found("test_lua", &filename))
        {
            generate_type = (generate_type | GEN_LANG_LUA);
            test_only = true;
        }
#endif  // GENERATE_NEW_LANG_CODE

        if (generate_type != GEN_LANG_NONE && filename.empty())
        {
            wxDir dir;
            dir.Open("./");
            if (!dir.GetFirst(&filename, "*.wxui", wxDIR_FILES))
            {
                wxMessageBox("No project file found in current directory. Filenane is required if switch is used.",
                             "Command-line Switch Error", wxOK | wxICON_ERROR);
                return 1;
            }
        }

        if (generate_type == GEN_LANG_NONE)
        {
            // If we're not generating code, then we need to create the main frame so that
            // LoadProject() and ImportProject() can fire events.
            m_frame = new MainFrame();
        }

        tt_string tt_filename = filename;

        tt_filename.make_absolute();
        log_file = filename;
        log_file.replace_extension(".log");
        GenResults results;
        if (tt_filename.file_exists())
        {
            if (generate_type != GEN_LANG_NONE)
            {
                m_is_generating = true;
                if (wxGetApp().isTestingMenuEnabled())
                    results.StartClock();
            }
            if (!tt_filename.extension().is_sameas(".wxui", tt::CASE::either) &&
                !tt_filename.extension().is_sameas(".wxue", tt::CASE::either))
            {
                is_project_loaded = Project.ImportProject(tt_filename, generate_type == GEN_LANG_NONE);
            }
            else
            {
                is_project_loaded = Project.LoadProject(tt_filename, generate_type == GEN_LANG_NONE);
            }
        }
        else
        {
            if (generate_type != GEN_LANG_NONE)
            {
                tt_string_vector log;
                log.emplace_back(tt_string("Unable to find project file: ") << filename.utf8_string());
                log.WriteFile(log_file);
                return 1;
            }
        }

        if (generate_type != GEN_LANG_NONE)
        {
            if (!is_project_loaded)
            {
                tt_string_vector log;
                log.emplace_back(tt_string("Unable to load project file: ") << filename.utf8_string());
                log.WriteFile(log_file);
                return 1;
            }

            tt_string_vector log;
            std::vector<tt_string> class_list;

            auto log_results = [&](std::string_view language_type = {})
            {
                results.msgs.emplace_back(language_type);
                if (results.updated_files.size() || class_list.size())
                {
                    if (test_only)
                    {
                        for (auto& iter: class_list)
                        {
                            auto& log_msg = log.emplace_back();
                            log_msg << "Needs updating: " << iter;
                        }
                    }
                    else
                    {
                        for (auto& iter: results.updated_files)
                        {
                            auto& log_msg = log.emplace_back();
                            log_msg << "Updated: " << iter;
                        }
                    }
                }
                else
                {
                    auto& log_msg = log.emplace_back();
                    log_msg << "All " << results.file_count << " generated files are current";
                }

                for (auto& iter: results.msgs)
                {
                    auto& log_msg = log.emplace_back();
                    log_msg << iter;
                }
            };

            // Passing a class_list reference will cause the code generator to process all the
            // top-level forms, but only populate class_list with the names of the forms that
            // would be changed if the file was written. If test_only is set, then we use this
            // mechanism and write any special messages that code generation caused (warnings,
            // errors, timing, etc.) to a log file.

            if (generate_type & GEN_LANG_CPLUSPLUS)
            {
                results.clear();
                if (wxGetApp().isTestingMenuEnabled())
                    results.StartClock();
                GenerateCppFiles(results, test_only ? &class_list : nullptr);
                log_results("Generating C++ files");
            }
            if (generate_type & GEN_LANG_PERL)
            {
                results.clear();
                if (wxGetApp().isTestingMenuEnabled())
                    results.StartClock();
                GenerateLanguageFiles(results, test_only ? &class_list : nullptr, GEN_LANG_PERL);
                log_results("Generating Perl files");
            }
            if (generate_type & GEN_LANG_PYTHON)
            {
                results.clear();
                if (wxGetApp().isTestingMenuEnabled())
                    results.StartClock();
                GenerateLanguageFiles(results, test_only ? &class_list : nullptr, GEN_LANG_PYTHON);
                log_results("Generating Python files");
            }
            if (generate_type & GEN_LANG_RUBY)
            {
                results.clear();
                if (wxGetApp().isTestingMenuEnabled())
                    results.StartClock();
                GenerateLanguageFiles(results, test_only ? &class_list : nullptr, GEN_LANG_RUBY);
                log_results("Generating Ruby files");
            }
            if (generate_type & GEN_LANG_XRC)
            {
                results.clear();
                if (wxGetApp().isTestingMenuEnabled())
                    results.StartClock();
                GenerateXrcFiles(results, {}, test_only ? &class_list : nullptr);
                log_results("Generating XRC files");
            }
            if (generate_type & GEN_LANG_RUST)
            {
                results.clear();
                if (wxGetApp().isTestingMenuEnabled())
                    results.StartClock();
                GenerateLanguageFiles(results, test_only ? &class_list : nullptr, GEN_LANG_RUST);
                log_results("Generating Rust files");
            }
#if GENERATE_NEW_LANG_CODE
            if (generate_type & GEN_LANG_HASKELL)
            {
                results.clear();
                if (wxGetApp().isTestingMenuEnabled())
                    results.StartClock();
                GenerateLanguageFiles(results, test_only ? &class_list : nullptr, GEN_LANG_HASKELL);
                log_results("Generating Haskell files");
            }
            if (generate_type & GEN_LANG_LUA)
            {
                results.clear();
                if (wxGetApp().isTestingMenuEnabled())
                    results.StartClock();
                GenerateLanguageFiles(results, test_only ? &class_list : nullptr, GEN_LANG_LUA);
                log_results("Generating Lua files");
            }
#endif

            if (log.size())
            {
                log.WriteFile(log_file);
            }

            return 0;
        }
    }

    if (!m_frame)  // nothing passed on the command line, so frame not created yet
    {
        m_frame = new MainFrame();
    }

    if (!is_project_loaded)
    {
        if (auto result = parser.FoundSwitch("load_last");
            result != wxCMD_SWITCH_NOT_FOUND || UserPrefs.is_LoadLastProject())
        {
            auto& file_history = m_frame->getFileHistory();
            tt_string file = file_history.GetHistoryFile(0).utf8_string();
            is_project_loaded = Project.LoadProject(file);
        }
    }

    if (!is_project_loaded)
    {
        StartupDlg start_dlg(nullptr);
        if (auto result = start_dlg.ShowModal(); result == wxID_OK)
        {
            switch (start_dlg.GetCommandType())
            {
                case StartupDlg::START_MRU:
                    if (!start_dlg.GetProjectFile().extension().is_sameas(".wxui", tt::CASE::either) &&
                        !start_dlg.GetProjectFile().extension().is_sameas(".wxue", tt::CASE::either))
                    {
                        is_project_loaded = Project.ImportProject(start_dlg.GetProjectFile());
                    }
                    else
                    {
                        is_project_loaded = Project.LoadProject(start_dlg.GetProjectFile());
                    }
                    break;

                case StartupDlg::START_EMPTY:
                    is_project_loaded = Project.NewProject(true);
                    break;

                case StartupDlg::START_CONVERT:
                    is_project_loaded = Project.NewProject(false);
                    break;

                case StartupDlg::START_OPEN:
                    {
                        // TODO: [KeyWorks - 02-21-2021] A CodeBlocks file will contain all of the wxSmith resources -- so it
                        // would actually make sense to process it since we can combine all of those resources into our
                        // single project file.

                        wxFileDialog dialog(nullptr, "Open or Import Project", wxEmptyString, wxEmptyString,
                                            "wxUiEditor Project File (*.wxui)|*.wxui"
                                            "|wxCrafter Project File (*.wxcp)|*.wxcp"
                                            "|DialogBlocks Project File (*.fjd)|*.fjd"
                                            "|wxFormBuilder Project File (*.fbp)|*.fbp"
                                            "|wxGlade File (*.wxg)|*.wxg"
                                            "|wxSmith File (*.wxs)|*.wxs"
                                            "|XRC File (*.xrc)|*.xrc"
                                            "|Windows Resource File (*.rc)|*.rc||",
                                            wxFD_OPEN);

                        if (dialog.ShowModal() == wxID_OK)
                        {
                            tt_string filename = dialog.GetPath().utf8_string();
                            if (!filename.extension().is_sameas(".wxui", tt::CASE::either) &&
                                !filename.extension().is_sameas(".wxue", tt::CASE::either))
                            {
                                is_project_loaded = Project.ImportProject(filename);
                            }
                            else
                            {
                                is_project_loaded = Project.LoadProject(dialog.GetPath());
                            }
                        }
                    }
                    break;
            }
        }
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
    else
    {
        m_frame->Close();
        return 1;
    }
}

int App::OnExit()
{
    return wxApp::OnExit();
}

bool App::isFireCreationMsgs() const
{
    return (UserPrefs.GetDebugFlags() & Prefs::PREFS_CREATION_MSG);
}

bool App::isPjtMemberPrefix() const
{
    return (UserPrefs.GetProjectFlags() & Prefs::PREFS_PJT_MEMBER_PREFIX);
}

bool App::AutoMsgWindow() const
{
    return (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW);
}

#if defined(_WIN32) && defined(_DEBUG) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)

    #include <wx/stackwalk.h>

class StackLogger : public wxStackWalker
{
public:
    auto& GetCalls() { return m_calls; }

protected:
    void OnStackFrame(const wxStackFrame& frame) override
    {
        if (frame.HasSourceLocation())
        {
            tt_string source;
            source << frame.GetFileName().utf8_string() << ':' << (to_int) frame.GetLine();

            wxString params;
            if (auto paramCount = frame.GetParamCount(); paramCount > 0)
            {
                params << "(";

                for (size_t i = 0; i < paramCount; ++i)
                {
                    wxString type, name, value;
                    if (frame.GetParam(i, &type, &name, &value))
                    {
                        params << type << " " << name << " = " << value << ", ";
                    }
                }

                params << ")";
            }

            if (params.size() > 100)
                params = "(...)";

            m_calls.emplace_back() << (to_int) frame.GetLevel() << ' ' << frame.GetName().utf8_string()
                                   << params.utf8_string() << ' ' << source;
        }
        else
        {
            m_calls.emplace_back() << (to_int) frame.GetLevel() << ' ' << frame.GetName().utf8_string();
        }
    }

    std::vector<tt_string> m_calls;
};

#endif  // defined(_DEBUG) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)

#if defined(_MSC_VER) && defined(wxUSE_ON_FATAL_EXCEPTION)

void App::OnFatalException()
{
    #if defined(_DEBUG) && defined(wxUSE_STACKWALKER)

    StackLogger logger;
    logger.WalkFromException();
    for (auto& iter: logger.GetCalls())
    {
        // We're only interested in our own source code, so ignore the rest.
        if (!iter.contains("wxUiEditor"))
            continue;

        wxLogDebug(iter.c_str());
    }

    // We now have the relevant call stack displayed in the debugger, so break into it.
    wxTrap();

    #endif  // _DEBUG

    // Let the user know something terrible happened.
    wxMessageBox("A fatal exception has occured!", txtVersion);
}

#endif  // defined(_MSC_VER) && defined(wxUSE_ON_FATAL_EXCEPTION)

void App::ShowMsgWindow()
{
    g_pMsgLogging->ShowLogger();
}

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
void App::DbgCurrentTest(wxCommandEvent&)
{
    wxASSERT_MSG(false, "assert in MsgLogging");
}

#endif
