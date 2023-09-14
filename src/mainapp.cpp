/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
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

#include "bitmaps.h"          // Contains various images handling functions
#include "gen_results.h"      // Code generation file writing functions
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node -- Node class
#include "node_creator.h"     // NodeCreator class
#include "preferences.h"      // Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

#include "ui/startup_dlg.h"  // StartupDlg -- Dialog to display if wxUE is launched with no arguments

#include "pugixml.hpp"

#ifdef _MSC_VER
    #if defined(_WIN32)

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
#endif

#if defined(_DEBUG)
tt_string widgets_build_signature = WX_BUILD_OPTIONS_SIGNATURE;
#endif  // _DEBUG

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

tt_string tt_empty_cstr;

#if wxCHECK_VERSION(3, 3, 0) && defined(_WIN32)
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
#if defined(INTERNAL_TESTING) || defined(_DEBUG)
    m_TestingMenuEnabled = true;
#endif

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

#if wxCHECK_VERSION(3, 3, 0) && defined(_WIN32)

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

    // [Randalphwa - 02-08-2023] This probably works, but will remain hidden until it is
    // tested. That said, I'm doubtful that it has any actual value other than for testing -- I
    // just don't see a reason for a user to want to use the command line to generate XRC
    // files.
    parser.AddLongSwitch("gen_xrc", "generate XRC files and exit", wxCMD_LINE_HIDDEN);

    // The "test" options will not write any files, it simply runs the code generation skipping
    // the part where files get written, and generates the log file.

    parser.AddLongSwitch("test_cpp", "generate C++ files and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_python", "generate python files and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_ruby", "generate python files and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_xrc", "generate XRC files and exit", wxCMD_LINE_HIDDEN);

    parser.AddLongSwitch("test_menu", "create test menu to the right of the Help menu",
                         wxCMD_LINE_HIDDEN | wxCMD_LINE_SWITCH_NEGATABLE);

    parser.Parse();
    if (auto result = parser.FoundSwitch("test_menu"); result != wxCMD_SWITCH_NOT_FOUND)
    {
        m_TestingMenuEnabled = (result == wxCMD_SWITCH_ON ? true : false);
    }

    if (parser.GetParamCount() || parser.GetArguments().size())
    {
        wxString filename;
        if (parser.GetParamCount())
        {
            filename = parser.GetParam(0);
        }

        tt_string log_file;
        auto generate_type = GEN_LANG_NONE;
        bool test_only = false;
        if (parser.Found("gen_cpp", &filename))
        {
            generate_type = GEN_LANG_CPLUSPLUS;
        }
        else if (parser.Found("gen_python", &filename))
        {
            generate_type = GEN_LANG_PYTHON;
        }
        else if (parser.Found("gen_ruby", &filename))
        {
            generate_type = GEN_LANG_RUBY;
        }
        else if (parser.Found("gen_xrc", &filename))
        {
            generate_type = GEN_LANG_XRC;
        }

        else if (parser.Found("test_cpp", &filename))
        {
            generate_type = GEN_LANG_CPLUSPLUS;
            test_only = true;
        }
        else if (parser.Found("test_python", &filename))
        {
            generate_type = GEN_LANG_PYTHON;
            test_only = true;
        }
        else if (parser.Found("test_ruby", &filename))
        {
            generate_type = GEN_LANG_PYTHON;
            test_only = true;
        }
        else if (parser.Found("test_xrc", &filename))
        {
            generate_type = GEN_LANG_XRC;
            test_only = true;
        }

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

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
            g_pMsgLogging = new MsgLogging();
#endif

#if defined(_DEBUG)
            // wxLog only exists in _DEBUG builds
            wxLog::SetActiveTarget(g_pMsgLogging);
#endif  // _DEBUG
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
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
                results.StartClock();
#endif
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

            std::vector<tt_string> class_list;
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
            results.StartClock();
#endif

            // Passing a class_list reference will cause the code generator to process all the
            // top-level forms, but only populate class_list with the names of the forms that
            // would be changed if the file was written. If test_only is set, then we use this
            // mechanism and write any special messages that code generation caused (warnings,
            // errors, timing, etc.) to a log file.

            switch (generate_type)
            {
                case GEN_LANG_CPLUSPLUS:
                    GenerateCodeFiles(results, test_only ? &class_list : nullptr);
                    break;

                case GEN_LANG_PYTHON:
                    GeneratePythonFiles(results, test_only ? &class_list : nullptr);
                    break;

                case GEN_LANG_RUBY:
                    GenerateRubyFiles(results, test_only ? &class_list : nullptr);
                    break;

                case GEN_LANG_XRC:
                    GenerateXrcFiles(results, {}, test_only ? &class_list : nullptr);
                    break;

                default:
                    break;
            }

            tt_string_vector log;

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
            log.WriteFile(log_file);

            return 0;
        }
    }

    if (!m_frame)  // nothing passed on the command line, so frame not created yet
    {
        m_frame = new MainFrame();

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
        g_pMsgLogging = new MsgLogging();
#endif

#if defined(_DEBUG)
        // wxLog only exists in _DEBUG builds
        wxLog::SetActiveTarget(g_pMsgLogging);
#endif  // _DEBUG
    }

    if (UserPrefs.is_LoadLastProject() && !is_project_loaded)
    {
        auto& file_history = m_frame->getFileHistory();
        tt_string file = file_history.GetHistoryFile(0).utf8_string();
        is_project_loaded = Project.LoadProject(file);
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

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
bool App::AutoMsgWindow() const
{
    return (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW);
}
#endif

#if defined(_DEBUG) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)

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
    #if defined(_DEBUG)

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

#if defined(_DEBUG) || defined(INTERNAL_TESTING)

void App::ShowMsgWindow()
{
    g_pMsgLogging->ShowLogger();
}

void App::DbgCurrentTest(wxCommandEvent&)
{
    if (tt::file_exists("c:\\rwCode\\wxTest\\src\\ruby\\rb_main.rb"))
    {
        wxExecuteEnv env;
        env.cwd = "c:\\rwCode\\wxTest\\src\\ruby";

        wxExecute("ruby rb_main.rb", wxEXEC_SYNC, nullptr, &env);
        return;
    }

    wxMessageBox("Add code you want to test to (mainapp.cpp) App::DbgCurrentTest()", txtVersion);
}

#endif

#if defined(_DEBUG)
void App::DbgPythonTest(wxCommandEvent&)
{
    if (tt::file_exists("python\\py_main.py"))
    {
        wxExecuteEnv env;
        env.cwd = wxGetCwd() + "\\python";

        wxExecute("python py_main.py", wxEXEC_SYNC, nullptr, &env);
        return;
    }

    wxMessageBox("Debug Python test not currently available", txtVersion);
}

void App::DbgRubyTest(wxCommandEvent&)
{
    if (tt::file_exists("ruby\\rb_main.rb"))
    {
        wxExecuteEnv env;
        env.cwd = wxGetCwd() + "\\ruby";

        wxExecute("ruby rb_main.rb", wxEXEC_SYNC, nullptr, &env);
        return;
    }

    wxMessageBox("Debug Ruby test not currently available", txtVersion);
}
#endif
