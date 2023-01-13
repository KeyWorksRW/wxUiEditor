/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <wx/cmdline.h>  // wxCmdLineParser and related classes for parsing the command
#include <wx/config.h>   // wxConfig base header
#include <wx/cshelp.h>   // Context-sensitive help support classes
#include <wx/filedlg.h>  // wxFileDialog base header
#include <wx/sysopt.h>   // wxSystemOptions
#include <wx/utils.h>    // Miscellaneous utilities

#include <tttextfile_wx.h>  // textfile -- Classes for reading and writing line-oriented files

#include "mainapp.h"

#include "bitmaps.h"          // Contains various images handling functions
#include "gen_results.h"      // Code generation file writing functions
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node -- Node class
#include "node_creator.h"     // NodeCreator class
#include "preferences.h"      // Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

#include "wxui/startupdlg_base.h"  // CStartup -- Dialog to display is wxUE is launched with no arguments

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
ttlib::cstr widgets_build_signature = WX_BUILD_OPTIONS_SIGNATURE;
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

ttlib::cstr tt_empty_cstr;

App::App()
{
    m_lang = wxLANGUAGE_DEFAULT;
}

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
    Preferences().ReadConfig();

    return true;
}

int App::OnRun()
{
    NodeCreation.Initialize();
    bool is_project_loaded = false;

    wxCmdLineParser parser(argc, argv);
    OnInitCmdLine(parser);
    parser.AddParam("Filename", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

    parser.AddLongOption("gen_python", "generate python files and exit", wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);
    parser.AddLongOption("gen_cpp", "generate C++ files and exit", wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);
    parser.AddLongOption("gen_xrc", "generate XRC files and exit", wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);

    parser.AddLongOption("test_python", "generate python files and exit", wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);
    parser.AddLongOption("test_cpp", "generate C++ files and exit", wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);
    parser.AddLongOption("test_xrc", "generate XRC files and exit", wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);

    parser.Parse();
    if (parser.GetParamCount() || parser.GetArguments().size())
    {
        ttString filename;
        ttString log_file;
        auto generate_type = GEN_LANG_NONE;
        bool test_only = false;
        if (parser.Found("gen_python", &filename))
        {
            generate_type = GEN_LANG_PYTHON;
        }
        else if (parser.Found("gen_cpp", &filename))
        {
            generate_type = GEN_LANG_CPLUSPLUS;
        }
        else if (parser.Found("gen_xrc", &filename))
        {
            generate_type = GEN_LANG_XRC;
        }
        else if (parser.Found("test_python", &filename))
        {
            generate_type = GEN_LANG_PYTHON;
            test_only = true;
        }
        else if (parser.Found("test_cpp", &filename))
        {
            generate_type = GEN_LANG_CPLUSPLUS;
            test_only = true;
        }
        else if (parser.Found("test_xrc", &filename))
        {
            generate_type = GEN_LANG_XRC;
            test_only = true;
        }
        else
        {
            filename = parser.GetParam(0);
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

        filename.make_absolute();
        GenResults results;
        if (filename.file_exists())
        {
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
            if (generate_type != GEN_LANG_NONE)
            {
                log_file = filename;
                log_file.replace_extension(".log");
                results.StartClock();
            }
#endif
            if (!filename.extension().is_sameas(".wxui", tt::CASE::either) &&
                !filename.extension().is_sameas(".wxue", tt::CASE::either))
            {
                is_project_loaded = Project.ImportProject(filename, generate_type == GEN_LANG_NONE);
            }
            else
            {
                is_project_loaded = Project.LoadProject(filename, generate_type == GEN_LANG_NONE);
            }
        }
        else
        {
            if (generate_type != GEN_LANG_NONE)
            {
                std::cerr << "Unable to find project file: " << filename << std::endl;
                return 1;
            }
        }

        if (generate_type != GEN_LANG_NONE)
        {
            if (!is_project_loaded)
            {
                std::cerr << "Unable to load project file: " << filename << std::endl;
                return 1;
            }

            std::vector<ttlib::cstr> class_list;
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
            results.StartClock();
#endif
            switch (generate_type)
            {
                case GEN_LANG_PYTHON:
                    GeneratePythonFiles(results, test_only ? &class_list : nullptr);
                    break;

                case GEN_LANG_CPLUSPLUS:
                    GenerateCodeFiles(results, test_only ? &class_list : nullptr);
                    break;

                case GEN_LANG_XRC:
                    GenerateXrcFiles(results, {}, test_only ? &class_list : nullptr);
                    break;

                default:
                    break;
            }
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
            results.EndClock();
#endif

            ttlib::textfile log;

            if (results.updated_files.size() || class_list.size())
            {
                if (test_only)
                {
                    for (auto& iter: class_list)
                    {
                        auto& msg = log.emplace_back();
                        msg << "Needs updating: " << iter;
                    }
                }
                else
                {
                    for (auto& iter: results.updated_files)
                    {
                        auto& msg = log.emplace_back();
                        msg << "Updated: " << iter;
                    }
                }
            }
            else
            {
                auto& msg = log.emplace_back();
                msg << "All " << results.file_count << " generated files are current";
            }

            for (auto& iter: results.msgs)
            {
                auto& msg = log.emplace_back();
                msg << iter;
            }
            log.WriteFile(log_file.utf8_string());

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
                                            "|wxFormBuilder Project File (*.fbp)|*.fbp"
                                            "|wxGlade File (*.wxg)|*.wxg"
                                            "|wxSmith File (*.wxs)|*.wxs"
                                            "|XRC File (*.xrc)|*.xrc"
                                            "|Windows Resource File (*.rc)|*.rc||",
                                            wxFD_OPEN);

                        if (dialog.ShowModal() == wxID_OK)
                        {
                            ttString filename = dialog.GetPath();
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
    return (Preferences().GetDebugFlags() & PREFS::PREFS_CREATION_MSG);
}

bool App::IsPjtMemberPrefix() const
{
    return (Preferences().GetProjectFlags() & PREFS::PREFS_PJT_MEMBER_PREFIX);
}

bool App::AutoMsgWindow() const
{
    return (Preferences().GetDebugFlags() & PREFS::PREFS_MSG_WINDOW);
}

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
            ttlib::cstr source;
            source << frame.GetFileName().wx_str() << ':' << (to_int) frame.GetLine();

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

            m_calls.emplace_back() << (to_int) frame.GetLevel() << ' ' << frame.GetName().wx_str() << params.wx_str() << ' '
                                   << source;
        }
        else
        {
            m_calls.emplace_back() << (to_int) frame.GetLevel() << ' ' << frame.GetName().wx_str();
        }
    }

    std::vector<ttlib::cstr> m_calls;
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
    FAIL_MSG("test FAIL_MSG() macro")

    wxMessageBox("Add code you want to test to (mainapp.cpp) App::DbgCurrentTest()", txtVersion);
}

#endif
