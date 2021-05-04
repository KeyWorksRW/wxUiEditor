/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/config.h>   // wxConfig base header
#include <wx/cshelp.h>   // Context-sensitive help support classes
#include <wx/filedlg.h>  // wxFileDialog base header
#include <wx/sysopt.h>   // wxSystemOptions

#include <ttparser.h>  // cmd -- Command line parser

#include "mainapp.h"

#include "appoptions.h"    // AppOptions -- Application-wide options
#include "bitmaps.h"       // Contains various images handling functions
#include "mainframe.h"     // MainFrame -- Main window frame
#include "node.h"          // Node -- Node class
#include "node_creator.h"  // NodeCreator class
#include "pjtsettings.h"   // ProjectSettings -- Hold data for currently loaded project
#include "startup.h"       // CStartup -- Dialog to display is wxUE is launched with no arguments
#include "uifuncs.h"       // Miscellaneous functions for displaying UI

#include "../pugixml/pugixml.hpp"

wxIMPLEMENT_APP(App);

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

    // Add all image handlers so that the EmbedImage class can be used to convert any type of image that wxWidgets supports.
    wxInitAllImageHandlers();

    // The name is sort of a standard. More importantly, it is sometimes used as the mask in Windows bitmaps for toolbar
    // images.
    wxTheColourDatabase->AddColour("Grey94", wxColour(240, 240, 240));

#if defined(wxUSE_ON_FATAL_EXCEPTION)
    ::wxHandleFatalExceptions(true);
#endif

#if defined(_WIN32)
    // Docs say we need this since our toolbar bitmaps >16 colors
    wxSystemOptions::SetOption("msw.remap", 0);  // only remap background of toolbar bitmaps, not the actual content
#endif

    // If we're just providing text-popups for help, then this is all we need.
    wxHelpProvider::Set(new wxSimpleHelpProvider);

#if defined(_DEBUG)
    g_pMsgLogging = new MsgLogging();
    wxLog::SetActiveTarget(g_pMsgLogging);
#endif

    SetVendorName("KeyWorks");

    auto config = wxConfig::Get();
    config->SetPath("/preferences");

    m_prefs.flags = config->ReadLong("flags", PREFS_MSG_WINDOW | PREFS_MSG_INFO | PREFS_MSG_EVENT | PREFS_MSG_WARNING);
    m_prefs.project_flags = config->ReadLong("project_flags", PREFS_PJT_MEMBER_PREFIX);

#if defined(_DEBUG)
    config->Read("DebugLanguage", &m_prefs.language);
#endif  // _DEBUG

    GetAppOptions().ReadConfig();
    config->SetPath("/");

    return true;
}

int App::OnRun()
{
    g_NodeCreator.Initialize();

    ttlib::cmd cmdLine;
#if defined(_WIN32)
    // This will call Windows API to get utf16 version of command line, then convert it to utf8 strings
    cmdLine.parse();
#else
    cmdLine.parse(argc, argv);
#endif  // _WIN32

    ttlib::cstr projectFile;
    if (cmdLine.getExtras().size())
    {
        projectFile.assign(cmdLine.getExtras().at(0));
        projectFile.make_absolute();
        if (!projectFile.file_exists())
        {
            appMsgBox(_tt(strIdCantOpen) + projectFile, _tt(strIdTitleLoadProject));
            projectFile.clear();
        }
    }

    // Create the frame before the dialog so that file history is initialized.
    m_frame = new MainFrame();

    if (projectFile.empty())
    {
        CStartup dlg;
        if (auto result = dlg.ShowModal(); result != wxID_OK)
        {
            return 1;
        }

        switch (dlg.GetCommandType())
        {
            case CStartup::START_MRU:
                projectFile = dlg.GetMruFilename();
                break;

            case CStartup::START_CONVERT:
                {
                    wxFileDialog dialog(nullptr, "Open Project to Convert", wxEmptyString, wxEmptyString,
                                        "All Importable Files|*.rc;*.dlg;*.fbp;*.wxg;*.wxs;*.xrc"
                                        "|Windows Resource File (*.rc)|*.rc;*.dlg"
                                        "|wxFormBuilder Project File (*.fbp)|*.fbp"
                                        "|wxGlade File (*.wxg)|*.wxg"
                                        "|wxSmith File (*.wxs)|*.wxs"
                                        "|XRC File (*.xrc)|*.xrc||",
                                        wxFD_OPEN);

                    if (dialog.ShowModal() == wxID_OK)
                    {
                        projectFile.utf(dialog.GetPath().wx_str());
                    }
                    else
                    {
                        return 1;
                    }
                }
                break;

            case CStartup::START_OPEN:
                {
                    // TODO: [KeyWorks - 02-21-2021] A CodeBlocks file will contain all of the wxSmith resources -- so it
                    // would actually make sense to process it since we can combine all of those resources into our single
                    // project file.

                    wxFileDialog dialog(nullptr, _tt(strIdTitleOpenOrImport), wxEmptyString, wxEmptyString,
                                        "wxUiEditor Project File (*.wxui)|*.wxui"
                                        "|wxFormBuilder Project File (*.fbp)|*.fbp"
                                        "|wxGlade File (*.wxg)|*.wxg"
                                        "|wxSmith File (*.wxs)|*.wxs"
                                        "|XRC File (*.xrc)|*.xrc"
                                        "|Windows Resource File (*.rc)|*.rc||",
                                        wxFD_OPEN);

                    if (dialog.ShowModal() == wxID_OK)
                    {
                        projectFile.utf(dialog.GetPath().wx_str());
                    }
                    else
                    {
                        return 1;
                    }
                }
                break;

            case CStartup::START_EMPTY:
            default:
                break;
        }
    }

    if (projectFile.size())
    {
        m_frame->Show();
        SetTopWindow(m_frame);
        if (!projectFile.extension().is_sameas(".wxui", tt::CASE::either) &&
            !projectFile.extension().is_sameas(".wxue", tt::CASE::either))
        {
            // TODO: [KeyWorks - 10-23-2020] Temp until projectFile gets changed to ttString
            ttString Temp(projectFile.wx_str());
            if (ImportProject(Temp))
            {
                return wxApp::OnRun();
            }
        }

        else if (LoadProject(wxString::FromUTF8(projectFile)))
        {
            return wxApp::OnRun();
        }

        auto answer = appMsgBox(ttlib::cstr() << _tt(strIdCannotLoadProjectFile) << projectFile
                                              << "\n\nDo you want to create an empty project?",
                                _tt(strIdTitleLoadProject), wxYES_NO);
        if (answer != wxYES)
        {
            m_frame->Close();
            wxApp::OnRun();  // Make sure all events get handled
            return 2;
        }
    }
    else
    {
        if (!NewProject())
        {
            m_frame->Close();
            wxApp::OnRun();  // Make sure all events get handled
            return 2;
        }

        m_frame->Show();
        SetTopWindow(m_frame);
    }

    return wxApp::OnRun();
}

int App::OnExit()
{
#if defined(_WIN32) && defined(_DEBUG) && defined(USE_CRT_MEMORY_DUMP)
    // This isn't really necessary, it just makes it easier to track down memory leaks.

    m_project.reset();

#endif  // defined(_WIN32) && defined(_DEBUG) && defined(USE_CRT_MEMORY_DUMP)

    // Probably not necessary to delete this -- presumably any wxBitmaps that it stores will get freed without calling
    // the dtor, but this ensures we don't leave any operating system resource handles lying around.
    delete m_pjtSettings;

    return wxApp::OnExit();
}

wxImage App::GetImage(const ttlib::cstr& description)
{
    if (description.is_sameprefix("XPM;") || description.is_sameprefix("Header;") || description.is_sameprefix("Art;"))
        return m_pjtSettings->GetPropertyBitmap(description);
    else
        return GetXPMImage("unknown");
}

#if defined(_DEBUG) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)

    #include <wx/stackwalk.h>

class StackLogger : public wxStackWalker
{
public:
    ttlib::cstrVector& GetCalls() { return m_calls; }

protected:
    void OnStackFrame(const wxStackFrame& frame) override
    {
        if (frame.HasSourceLocation())
        {
            ttlib::cstr source;
            source << frame.GetFileName().wx_str() << ':' << static_cast<int>(frame.GetLine());

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

            m_calls.emplace_back() << static_cast<int>(frame.GetLevel()) << ' ' << frame.GetName().wx_str()
                                   << params.wx_str() << ' ' << source;
        }
        else
        {
            m_calls.emplace_back() << static_cast<int>(frame.GetLevel()) << ' ' << frame.GetName().wx_str();
        }
    }

    ttlib::cstrVector m_calls;
};

#endif  // defined(_DEBUG) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)

#if defined(wxUSE_ON_FATAL_EXCEPTION)

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
    appMsgBox(_tt(strIdInternalError), txtVersion);
}

#endif

#if defined(_DEBUG)

void App::ShowMsgWindow()
{
    g_pMsgLogging->ShowLogger();
}

void App::DbgCurrentTest(wxCommandEvent&)
{
    appMsgBox("Add code you want to test to App::DbgCurrentTest()", "Debug Testing");
    wxLogMessage("This is a wxWidgets message!");
}

#endif  // defined(_DEBUG)
