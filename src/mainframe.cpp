/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// mainframe_events.cpp contains the event handlers for the MainFrame class.
// mainframe_updates.cpp contains the MainFrame::Update...() functions

#include <sstream>

#include <wx/aui/auibook.h>       // wxaui: wx advanced user interface - notebook
#include <wx/config.h>            // wxConfig base header
#include <wx/fdrepdlg.h>          // wxFindReplaceDialog class
#include <wx/filedlg.h>           // wxFileDialog base header
#include <wx/filehistory.h>       // wxFileHistory class
#include <wx/frame.h>             // wxFrame class interface
#include <wx/infobar.h>           // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/persist/splitter.h>  // persistence support for wxTLW
#include <wx/persist/toplevel.h>  // persistence support for wxTLW
#include <wx/splitter.h>          // Base header for wxSplitterWindow
#include <wx/toolbar.h>           // wxToolBar interface declaration
#include <wx/utils.h>             // Miscellaneous utilities
#include <wx/wupdlock.h>          // wxWindowUpdateLocker prevents window redrawing

// auto-generated: wxui/mainframe_base.h and wxui/mainframe_base.cpp

#include "mainframe.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "clipboard.h"        // wxUiEditorData -- Handles reading and writing OS clipboard data
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "gen_base.h"         // Generate Base class
#include "gen_common.h"       // Common component functions
#include "gen_enums.h"        // Enumerations for generators
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator class
#include "node_gridbag.h"     // GridBag -- Create and modify a node containing a wxGridBagSizer
#include "node_prop.h"        // NodeProperty -- NodeProperty class
#include "preferences.h"      // Preferences -- Stores user preferences
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties

#include "panels/base_panel.h"      // BasePanel -- C++ panel
#include "panels/doc_view.h"        // Panel for displaying docs in wxWebView
#include "panels/nav_panel.h"       // NavigationPanel -- Node tree class
#include "panels/propgrid_panel.h"  // PropGridPanel -- Node inspector class
#include "panels/ribbon_tools.h"    // RibbonPanel -- Displays component tools in a wxRibbonBar

#include "wxui/ui_images.h"  // This is generated from the Images List

#include "internal/code_compare.h"  // CodeCompare
#include "internal/import_panel.h"  // ImportPanel -- Panel to display original imported file
#include "internal/msg_logging.h"   // MsgLogging -- Message logging class
#include "internal/node_info.h"     // NodeInfo
#include "internal/undo_info.h"     // UndoInfo

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    #include "internal/debugsettings.h"  // DebugSettings -- Settings while running the Debug version of wxUiEditor
#endif

#include "mockup/mockup_parent.h"  // MockupParent -- Top-level MockUp Parent window

#include "pugixml.hpp"

// Warning! This MUST be at least 3!
constexpr const size_t StatusPanels = 3;

using namespace wxue_img;
using namespace GenEnum;

// Comment out the following line to change the UI back to the way it was in 1.1.2 and all earlier versions.
#define NEW_LAYOUT 1

enum
{
    IDM_IMPORT_WINRES = START_TESTING_IDS,

    id_TestSwitch,
    id_CodeDiffDlg,
    id_ConvertImage,
    id_DebugCurrentTest,
    id_DebugPreferences,
    id_DebugXrcDuplicate,
    id_DebugXrcImport,
    id_FindWidget,
    id_GenerateCpp,
    id_GeneratePython,
    id_GenerateRuby,
    id_GenerateFortran,
    id_GenerateHaskell,
    id_GenerateLua,
    id_GeneratePerl,
    id_GenerateRust,
    id_GenerateXrc,
    id_GenSingleCpp,
    id_GenSinglePython,
    id_GenSingleRuby,
    id_GenSingleFortran,
    id_GenSingleHaskell,
    id_GenSingleLua,
    id_GenSinglePerl,
    id_GenSingleRust,
    id_GenSingleXrc,
    id_NodeMemory,
    id_ShowLogger,
    id_XrcPreviewDlg,
    id_UndoInfo,
    id_DebugPythonTest,
    id_DebugRubyTest,

};

const char* txtEmptyProject = "Empty Project";

MainFrame::MainFrame() : MainFrameBase(nullptr), m_findData(wxFR_DOWN), m_ImportHistory(9, START_IMPORT_FILE_IDS)
{
    m_dpi_menu_size = FromDIP(wxSize(16, 16));
    m_dpi_toolbar_size = FromDIP(wxSize(16, 16));
    m_dpi_ribbon_size = FromDIP(wxSize(24, 24));

    wxIconBundle bundle;

#if defined(_DEBUG)
    bundle.AddIcon(bundle_debug_logo_svg(16, 16).GetIcon(wxSize(16, 16)));
    bundle.AddIcon(bundle_debug_logo_svg(32, 32).GetIcon(wxSize(32, 32)));
    bundle.AddIcon(bundle_debug_logo_svg(48, 48).GetIcon(wxSize(48, 48)));
    bundle.AddIcon(bundle_debug_logo_svg(64, 64).GetIcon(wxSize(64, 64)));
    bundle.AddIcon(bundle_debug_logo_svg(128, 128).GetIcon(wxSize(128, 128)));
#else
    bundle.AddIcon(bundle_wxUiEditor_svg(16, 16).GetIcon(wxSize(16, 16)));
    bundle.AddIcon(bundle_wxUiEditor_svg(32, 32).GetIcon(wxSize(32, 32)));
    bundle.AddIcon(bundle_wxUiEditor_svg(48, 48).GetIcon(wxSize(48, 48)));
    bundle.AddIcon(bundle_wxUiEditor_svg(64, 64).GetIcon(wxSize(64, 64)));
    bundle.AddIcon(bundle_wxUiEditor_svg(128, 128).GetIcon(wxSize(128, 128)));
#endif  // _DEBUG

    SetIcons(bundle);

    SetTitle("wxUiEditor");

    if (WakaTime::IsWakaTimeAvailable())
    {
        m_wakatime = std::make_unique<WakaTime>();
    }

    auto config = wxConfig::Get();

    // Normally, wxPersistentRegisterAndRestore(this, "MainFrame"); could be called to save/restore the size and position.
    // That works fine on Windows 10, but on Windows 11, a user can maximize the height of a window by dragging the frame to
    // the bottom of the screen. This does not generate the normal size event, and therefore the main windows doesn't save or
    // restore the correct size and position. It's worth noting that even Windows apps like Notepad don't handle this
    // correctly either. However, by retrieving the dimensions when the app is closed, the exact size and position can be
    // saved and restored without relying on event messages.

#if defined(_DEBUG)
    config->SetPath("/debug_mainframe");
#else
    config->SetPath("/mainframe");
#endif

    if (auto isMaximixed = config->ReadBool("IsMaximized", false); isMaximixed)
    {
        Maximize();
    }
    else
    {
        if (auto isIconized = config->ReadBool("IsIconized", false); isIconized)
        {
            Iconize();
        }
        else
        {
            wxPoint pt;
            pt.x = config->ReadLong("PosX", -1);
            pt.y = config->ReadLong("PosY", -1);
            SetPosition(pt);

            wxSize config_size;
            config_size.x = config->ReadLong("SizeW", 500);
            config_size.y = config->ReadLong("SizeH", 400);
            SetSize(config_size);
        }
    }

    config->SetPath(txt_main_window_config);
    m_FileHistory.Load(*config);
    m_FileHistory.UseMenu(m_submenu_recent);
    m_FileHistory.AddFilesToMenu();

    if (wxGetApp().isTestingMenuEnabled())
    {
        auto menuTesting = new wxMenu;

        menuTesting->Append(id_CodeDiffDlg, "Compare Code &Generation...",
                            "Dialog showing what class have changed, and optional viewing in WinMerge");
        menuTesting->Append(id_FindWidget, "&Find Widget...", "Search for a widget starting with the current selected node");
        menuTesting->Append(id_NodeMemory, "Node &Information...", "Show node memory usage");
        menuTesting->Append(id_UndoInfo, "Undo &Stack Information...", "Show undo/redo stack memory usage");
        menuTesting->AppendSeparator();
        menuTesting->Append(id_GeneratePython, "&Generate Python", "Generate all python files from current project.");
        menuTesting->Append(id_GenerateRuby, "&Generate Ruby", "Generate all ruby files from current project.");

        auto* submenu_xrc = new wxMenu();
        wxMenuItem* item;
        item = submenu_xrc->Append(id_XrcPreviewDlg, "&XRC Tests...", "Dialog with multiple XRC tests");
        item->SetBitmap(bundle_xrc_tests_svg(16, 16));
        item =
            submenu_xrc->Append(id_DebugXrcImport, "&Test XRC import", "Export the current form, then verify importing it");
        item->SetBitmap(bundle_import_svg(16, 16));
        submenu_xrc->Append(id_DebugXrcDuplicate, "&Test XRC duplication",
                            "Duplicate the current form via Export and Import XRC");
        menuTesting->AppendSubMenu(submenu_xrc, "&XRC");

        menuTesting->AppendSeparator();
        menuTesting->Append(id_ShowLogger, "Show &Log Window", "Show window containing debug messages");
        auto menuItem = menuTesting->Append(id_TestSwitch, "Testing Switch", "Toggle test switch", wxITEM_CHECK);
        menuItem->Check(wxGetApp().isTestingSwitch());
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent& event)
            {
                if (wxGetApp().isTestingSwitch())
                {
                    wxGetApp().setTestingSwitch(false);
                    wxStaticCast(event.GetEventObject(), wxMenu)->FindItem(id_TestSwitch)->Check(false);
                }
                else
                {
                    wxGetApp().setTestingSwitch(true);
                    wxStaticCast(event.GetEventObject(), wxMenu)->FindItem(id_TestSwitch)->Check(true);
                }
            },
            id_TestSwitch);

        m_menubar->Append(menuTesting, "Testing");

        m_submenu_import_recent = new wxMenu();
        m_menuFile->AppendSeparator();
        m_menuFile->AppendSubMenu(m_submenu_import_recent, "Import &Recent");

        config = wxConfig::Get();
        config->SetPath("/debug_history");
        m_ImportHistory.Load(*config);
        m_ImportHistory.UseMenu(m_submenu_import_recent);
        m_ImportHistory.AddFilesToMenu();
        config->SetPath("/");

        Bind(wxEVT_MENU, &MainFrame::OnImportRecent, this, START_IMPORT_FILE_IDS, START_IMPORT_FILE_IDS + 9);
    }

#if defined(_DEBUG)
    auto menuInternal = new wxMenu;

    // We want these available in internal Release builds

    menuInternal->AppendSeparator();
    menuInternal->Append(id_DebugPreferences, "Test &Settings...", "Settings to use in testing builds");
    menuInternal->Append(id_DebugCurrentTest, "&Current Test", "Current debugging test");

    menuInternal->AppendSeparator();
    menuInternal->Append(id_ConvertImage, "&Convert Image...", "Image conversion testing...");

    m_menubar->Append(menuInternal, "&Internal");

#endif  // defined(_DEBUG) || defined(INTERNAL_TESTING)

    if (wxGetApp().isTestingMenuEnabled())
    {
        m_toolbar->AddTool(id_XrcPreviewDlg, "XRC Tests", bundle_xrc_tests_svg(24, 24), "Dialog with multiple XRC tests");
    }

    // For version 1.1.0.0, preview isn't reliable enough to be included in the release version
    else
    {
        m_menuTools->Delete(m_mi_preview);
        m_toolbar->DeleteTool(id_PreviewForm);
    }
    m_toolbar->Realize();

    CreateStatusBar(StatusPanels);
    SetStatusBarPane(1);  // specifies where menu and toolbar help content is displayed

    // auto* box_sizer = new wxBoxSizer(wxVERTICAL);
    m_ribbon_panel = new RibbonPanel(this);
    m_mainframe_sizer->Insert(0, m_ribbon_panel, wxSizerFlags(0).Expand());

    CreateSplitters();

    m_nav_panel->SetMainFrame(this);

    m_SecondarySplitter->Bind(wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,
                              [this](wxSplitterEvent&)
                              {
                                  UpdateStatusWidths();
                              });
    m_MainSplitter->Bind(wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,
                         [this](wxSplitterEvent&)
                         {
                             UpdateStatusWidths();
                         });

    Bind(wxEVT_MENU, &MainFrame::OnImportWindowsResource, this, IDM_IMPORT_WINRES);
    Bind(wxEVT_MENU, &MainFrame::OnOpenRecentProject, this, wxID_FILE1, wxID_FILE9);

    Bind(wxEVT_FIND, &MainFrame::OnFind, this);
    Bind(wxEVT_FIND_NEXT, &MainFrame::OnFind, this);
    Bind(wxEVT_FIND_CLOSE, &MainFrame::OnFindClose, this);

    Bind(EVT_NodeSelected, &MainFrame::OnNodeSelected, this);

    Bind(EVT_EventHandlerChanged,
         [this](CustomEvent&)
         {
             UpdateFrame();
         });
    Bind(EVT_GridBagAction,
         [this](CustomEvent&)
         {
             UpdateFrame();
         });
    Bind(EVT_NodeCreated,
         [this](CustomEvent&)
         {
             UpdateFrame();
         });
    Bind(EVT_NodeDeleted,
         [this](CustomEvent&)
         {
             UpdateFrame();
         });
    Bind(EVT_NodePropChange,
         [this](CustomEvent&)
         {
             UpdateFrame();
         });
    Bind(EVT_ParentChanged,
         [this](CustomEvent&)
         {
             UpdateFrame();
         });
    Bind(EVT_PositionChanged,
         [this](CustomEvent&)
         {
             UpdateFrame();
         });
    Bind(EVT_ProjectUpdated,
         [this](CustomEvent&)
         {
             UpdateFrame();
         });

    // Create bindings for the range of IDs in the Add menu

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent& event)
        {
            CreateViaNewDlg(static_cast<GenName>(event.GetId()));
        },
        CreateNewDialog, MdiMenuBar - 1);

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent& event)
        {
            createToolNode(static_cast<GenName>(event.GetId()));
        },
        gen_wxMdiWindow, gen_name_array_size - 1);

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            Close();
        },
        wxID_EXIT);

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent& event)
        {
            m_mockupPanel->ShowHiddenControls(event.IsChecked());
        },
        id_ShowHidden);

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent& event)
        {
            m_mockupPanel->MagnifyWindow(event.IsChecked());
        },
        id_Magnify);

    if (wxGetApp().isTestingMenuEnabled())
    {
        Bind(
            wxEVT_MENU,
            [this](wxCommandEvent&)
            {
                CodeCompare dlg(this);
                dlg.ShowModal();
            },
            id_CodeDiffDlg);

        Bind(
            wxEVT_MENU,
            [this](wxCommandEvent&)
            {
                NodeInfo dlg(this);
                dlg.ShowModal();
            },
            id_NodeMemory);

        Bind(
            wxEVT_MENU,
            [this](wxCommandEvent&)
            {
                UndoInfo dlg(this);
                dlg.ShowModal();
            },
            id_UndoInfo);
        Bind(wxEVT_MENU, &MainFrame::OnFindWidget, this, id_FindWidget);
    }
    if (wxGetApp().isTestingMenuEnabled())
    {
        Bind(wxEVT_MENU, &MainFrame::OnGenSingleCpp, this, id_GenSingleCpp);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateSingleLanguage(GEN_LANG_PYTHON);
            },
            id_GenSinglePython);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateSingleLanguage(GEN_LANG_RUBY);
            },
            id_GenSingleRuby);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateSingleLanguage(GEN_LANG_PERL);
            },
            id_GenSinglePerl);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateSingleLanguage(GEN_LANG_RUST);
            },
            id_GenSingleRust);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateSingleLanguage(GEN_LANG_XRC);
            },
            id_GenSingleXrc);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateLanguage(GEN_LANG_RUBY);
            },
            id_GenerateRuby);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateLanguage(GEN_LANG_PERL);
            },
            id_GeneratePerl);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateLanguage(GEN_LANG_RUST);
            },
            id_GenerateRust);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateLanguage(GEN_LANG_XRC);
            },
            id_GenerateXrc);

#if GENERATE_NEW_LANG_CODE
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateLanguage(GEN_LANG_FORTRAN);
            },
            id_GenerateFortran);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateLanguage(GEN_LANG_HASKELL);
            },
            id_GenerateHaskell);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateLanguage(GEN_LANG_LUA);
            },
            id_GenerateLua);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateSingleLanguage(GEN_LANG_FORTRAN);
            },
            id_GenSingleFortran);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateSingleLanguage(GEN_LANG_HASKELL);
            },
            id_GenSingleHaskell);

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                OnGenerateSingleLanguage(GEN_LANG_LUA);
            },
            id_GenSingleLua);
#endif

        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                g_pMsgLogging->ShowLogger();
            },
            id_ShowLogger);
        Bind(wxEVT_MENU, &MainFrame::OnXrcPreview, this, id_XrcPreviewDlg);
        Bind(wxEVT_MENU, &MainFrame::OnTestXrcImport, this, id_DebugXrcImport);
        Bind(wxEVT_MENU, &MainFrame::OnTestXrcDuplicate, this, id_DebugXrcDuplicate);
    }

#if defined(_DEBUG)
    Bind(wxEVT_MENU, &MainFrame::OnConvertImageDlg, this, id_ConvertImage);
    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            DebugSettings dlg(this);
            dlg.ShowModal();
        },
        id_DebugPreferences);

    Bind(wxEVT_MENU, &App::DbgCurrentTest, &wxGetApp(), id_DebugCurrentTest);
#endif

    AddCustomEventHandler(GetEventHandler());

    m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &MainFrame::OnAuiNotebookPageChanged, this);
}

MainFrame::~MainFrame()
{
    delete m_findDialog;
}

wxBitmapBundle wxueBundleSVG(const unsigned char* data, size_t size_data, size_t size_svg, wxSize def_size);

#if defined(_DEBUG)
    #include "internal/debugsettings.h"
#endif

void MainFrame::ProjectLoaded()
{
    Project.ChangeDir();
    setStatusText("Project loaded");
    if (!m_isImported)
    {
        if (!Project.getProjectFile().filename().is_sameas(txtEmptyProject))
        {
            m_FileHistory.AddFileToHistory(Project.getProjectFile());
        }
        m_isProject_modified = false;
    }

    m_undo_stack.clear();
    m_undo_stack_size = 0;

    if (!m_iswakatime_bound)
    {
        m_iswakatime_bound = true;
        Bind(wxEVT_ACTIVATE,
             [this](wxActivateEvent&)
             {
                 if (m_wakatime)
                     m_wakatime->ResetHeartbeat();
             });
    }

    if (!Project.hasValue(prop_wxWidgets_version))
    {
        Project.set_value(prop_wxWidgets_version, UserPrefs.get_CppWidgetsVersion());
    }
    else if (Project.as_string(prop_wxWidgets_version) == "3.2")
    {
        Project.set_value(prop_wxWidgets_version, "3.2.0");
    }

    m_selected_node = Project.getProjectNode()->getSharedPtr();

    UpdateLanguagePanels();
}

void MainFrame::ProjectSaved()
{
    setStatusText(tt_string(Project.getProjectFile().filename()) << " saved");
    UpdateFrame();
}

bool MainFrame::SaveWarning()
{
    int result = wxYES;

    if (m_isProject_modified)
    {
        // Testing often requires importing multiple projects to verify they work, so there is no
        // reason to save them.
        if (wxGetApp().isTestingMenuEnabled() && m_isImported)
        {
            result = wxNO;
        }
        else
        {
            result = ::wxMessageBox("Current project file has been modified...\n"
                                    "Do you want to save the changes?",
                                    "Save project", wxYES | wxNO | wxCANCEL, this);
        }
        if (result == wxYES)
        {
            wxCommandEvent dummy;
            OnSaveProject(dummy);
        }
    }

    return (result != wxCANCEL);
}

wxWindow* MainFrame::CreateNoteBook(wxWindow* parent)
{
    m_notebook = new wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook->SetArtProvider(new wxAuiSimpleTabArt());

    m_mockupPanel = new MockupParent(m_notebook, this);
    m_notebook->AddPage(m_mockupPanel, "Mock Up", false, wxWithImages::NO_IMAGE);

    m_xrcPanel = new BasePanel(m_notebook, this, GEN_LANG_XRC);
    m_notebook->AddPage(m_xrcPanel, "XRC", false, wxWithImages::NO_IMAGE);

#if wxUSE_WEBVIEW
    m_docviewPanel = new DocViewPanel(m_notebook, this);
    m_notebook->AddPage(m_docviewPanel, "Docs", false, wxWithImages::NO_IMAGE);
#endif

    if (wxGetApp().isTestingMenuEnabled())
    {
        // Shows original import file if project is imported, otherwise it shows the project file
        m_imnportPanel = new ImportPanel(m_notebook);
        m_notebook->AddPage(m_imnportPanel, "Import", false, wxWithImages::NO_IMAGE);
    }

    return m_notebook;
}

void MainFrame::CreateSplitters()
{
    // The main splitter contains the navigation tree control and it's toolbar on the left. On the right is a panel
    // containing the Ribbon toolbar at the top, and a splitter window containing the property grid and notebook with
    // mockup and code windows below it.

    m_panel_right->SetWindowStyle(wxBORDER_RAISED);

    // auto parent_sizer = new wxBoxSizer(wxVERTICAL);

#if !defined(NEW_LAYOUT)
    m_ribbon_panel = new RibbonPanel(m_panel_right);
    m_right_panel_sizer->Add(m_ribbon_panel, wxSizerFlags(0).Expand());
#else
    // auto main_toolbar = new MainToolBar(m_panel_right);
    // m_right_panel_sizer->Add(main_toolbar, wxSizerFlags(0).Expand());
#endif

    m_info_bar = new wxInfoBar(m_panel_right);
    m_right_panel_sizer->Add(m_info_bar, wxSizerFlags().Expand());

    m_SecondarySplitter = new wxSplitterWindow(m_panel_right, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
    m_right_panel_sizer->Add(m_SecondarySplitter, wxSizerFlags(1).Expand());

    m_property_panel = new PropGridPanel(m_SecondarySplitter, this);
    auto notebook = CreateNoteBook(m_SecondarySplitter);

    if (UserPrefs.is_RightPropGrid())
        m_SecondarySplitter->SplitVertically(notebook, m_property_panel, m_SecondarySashPosition);
    else
        m_SecondarySplitter->SplitVertically(m_property_panel, notebook, m_SecondarySashPosition);

    m_MainSplitter->SplitVertically(m_nav_panel, m_panel_right);
    m_MainSplitter->SetName("Navigation");
    wxPersistenceManager::Get().RegisterAndRestore(m_MainSplitter);

    // Need to update the left splitter so the right one is drawn correctly
    wxSizeEvent update(GetSize(), GetId());
    ProcessEvent(update);
    m_MainSplitter->UpdateSize();
    m_MainSplitter->SetMinimumPaneSize(2);

    // Set to zero because we don't need this to change relative size when the main window is resized. Fixes issue
    // #90
    m_SecondarySplitter->SetSashGravity(0);
    m_SecondarySplitter->SetMinimumPaneSize(2);

    m_SecondarySplitter->SetName("Properties");
    wxPersistenceManager::Get().RegisterAndRestore(m_SecondarySplitter);

    // UpdateStatusWidths();
    m_MainSashPosition = m_MainSplitter->GetSashPosition();
    m_SecondarySashPosition = m_SecondarySplitter->GetSashPosition();
    int widths[StatusPanels] = { 1, (m_MainSashPosition + m_SecondarySashPosition) - 12, -1 };
    SetStatusWidths(sizeof(widths) / sizeof(int), widths);

    // SetMinSize(wxSize(700, 380));
}

void MainFrame::setStatusField(const tt_string text, int position)
{
    if (position == -1)
        position = m_posPropGridStatusField;
    setStatusText(text, position);
}

Node* MainFrame::getSelectedForm()
{
    if (!m_selected_node || m_selected_node->isGen(gen_Project))
        return nullptr;

    return m_selected_node->getForm();
}

bool MainFrame::SelectNode(Node* node, size_t flags)
{
    if (!node)
    {
        m_selected_node = NodeSharedPtr();
        return false;
    }

    if (node == m_selected_node.get() && !(flags & evt_flags::force_selection))
    {
        return false;  // already selected
    }

    m_selected_node = node->getSharedPtr();

    if (flags & evt_flags::queue_event)
    {
        CustomEvent node_event(EVT_NodeSelected, m_selected_node.get());
        for (auto handler: m_custom_event_handlers)
        {
            handler->QueueEvent(node_event.Clone());
        }
    }
    else if (flags & evt_flags::fire_event)
    {
        FireSelectedEvent(node);
    }

    return true;
}

void MainFrame::CopyNode(Node* node)
{
    ASSERT(node);
    m_clipboard = NodeCreation.makeCopy(node);
    if (m_clipboard)
    {
        SmartClipboard clip;
        if (clip.IsOpened())
        {
            pugi::xml_document doc;
            auto clip_node = doc.append_child("node");
            int project_version = minRequiredVer;
            m_clipboard->addNodeToDoc(clip_node, project_version);
            // REVIEW: [Randalphwa - 08-24-2022] project_version is ignored, assuming that the same version of
            // wxClipboard will be used to paste the clipboard node.
            auto* u8_data = new wxUtf8DataObject();
            std::stringstream strm;
            doc.save(strm, "", pugi::format_raw);

            // Skip over the XML header
            auto begin = strm.str().find("<node");
            if (tt::is_found(begin))
            {
                u8_data->GetText() = strm.str().c_str() + begin;
                auto hash_data = new wxUEDataObject();

                m_clip_hash = 0;
                m_clipboard->calcNodeHash(m_clip_hash);
                hash_data->GetHash() = m_clip_hash;

                wxDataObjectComposite* data = new wxDataObjectComposite();
                data->Add(u8_data, true);
                data->Add(hash_data, false);
                wxTheClipboard->SetData(data);
            }
            else
            {
                // If it wasn't passed to wxDataObjectComposite, then we need to delete it
                // ourselves.
                delete u8_data;
            }
        }
    }
}

void MainFrame::PasteNode(Node* parent)
{
    if (auto result = isClipboardDataAvailable(); result)
    {
        auto new_node = GetClipboardNode();
        if (new_node)
        {
            m_clipboard = new_node;
        }
    }

    if (!m_clipboard)
    {
        wxMessageBox("There is nothing in the clipboard that can be pasted!", "Paste Clipboard");
        return;
    }

    if (!parent)
    {
        ASSERT(m_selected_node)
        if (!m_selected_node)
            return;
        parent = m_selected_node.get();
    }

    if (parent->isGen(gen_wxSplitterWindow) && parent->getChildCount() > 1)
    {
        wxMessageBox("A wxSplitterWindow can't have more than two windows.");
        return;
    }

    auto new_node = NodeCreation.makeCopy(m_clipboard.get(), parent);

    // This makes it possible to switch from a normal child toolbar to a form toolbar and vice versa.
    // Both wxToolBar and wxAuiToolbar are supported
    if ((parent->isGen(gen_ToolBar) && new_node->isGen(gen_wxToolBar)) ||
        (parent->isGen(gen_AuiToolBar) && new_node->isGen(gen_wxAuiToolBar)) ||
        (parent->isGen(gen_wxToolBar) && new_node->isGen(gen_ToolBar)) ||
        (parent->isGen(gen_wxAuiToolBar) && new_node->isGen(gen_AuiToolBar)))
    {
        auto group = std::make_shared<GroupUndoActions>("Paste children", parent);

        for (auto& child_node: new_node->getChildNodePtrs())
        {
            auto insert_action = std::make_shared<InsertNodeAction>(child_node.get(), parent, "paste");
            insert_action->SetFireCreatedEvent(true);
            group->Add(insert_action);
        }
        wxGetFrame().PushUndoAction(group);
        return;
    }
    // This makes it possible to paste between a wxToolBar and a wxAuiToolBar and vice versa.
    // Both a normal child and a form toolbar are supported.
    else if ((parent->isGen(gen_AuiToolBar) && new_node->isGen(gen_wxToolBar)) ||
             (parent->isGen(gen_wxAuiToolBar) && new_node->isGen(gen_wxToolBar)) ||
             (parent->isGen(gen_ToolBar) && new_node->isGen(gen_wxAuiToolBar)) ||
             (parent->isGen(gen_wxToolBar) && new_node->isGen(gen_wxAuiToolBar)))
    {
        auto group = std::make_shared<GroupUndoActions>("Paste children", parent);

        for (auto& child_node: new_node->getChildNodePtrs())
        {
            // We are changing from a wxToolBar to a wxAuiToolBar, so we need to change the node type
            auto new_child = NodeCreation.makeCopy(child_node.get(), parent);
            auto insert_action = std::make_shared<InsertNodeAction>(new_child, parent->getSharedPtr(), "paste");
            insert_action->SetFireCreatedEvent(true);
            group->Add(insert_action);
        }
        wxGetFrame().PushUndoAction(group);
        return;
    }

    if (new_node->isForm())
        Project.FixupDuplicatedNode(new_node.get());

    if (!parent->isChildAllowed(new_node))
    {
        auto grandparent = parent->getParent();
        if (!grandparent || !grandparent->isChildAllowed(new_node))
        {
            wxMessageBox(tt_string() << "You cannot paste " << new_node->declName() << " into " << parent->declName());
            return;
        }
        parent = grandparent;
    }

    if (parent->isGen(gen_wxGridBagSizer))
    {
        GridBag grid_bag(parent);
        grid_bag.InsertNode(parent, new_node.get());
        return;
    }

    tt_string undo_str("paste ");
    undo_str << m_clipboard->declName();

    auto pos = parent->findInsertionPos(m_selected_node);
    PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
    FireCreatedEvent(new_node);
    SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
}

void MainFrame::DuplicateNode(Node* node)
{
    ASSERT(node);
    ASSERT(node->getParent());

    auto new_node = NodeCreation.makeCopy(node);
    if (new_node->isForm())
        Project.FixupDuplicatedNode(new_node.get());
    auto* parent = node->getParent();
    if (parent->isGen(gen_wxGridBagSizer))
    {
        GridBag grid_bag(parent);
        grid_bag.InsertNode(parent, new_node.get());
        // GridBag::InsertNode() will have already fired events
    }
    else
    {
        tt_string undo_str("duplicate ");
        undo_str << node->declName();
        auto pos = parent->findInsertionPos(m_selected_node);
        PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
        m_selected_node = new_node;
        FireCreatedEvent(new_node);
        SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
    }
}

bool MainFrame::CanCopyNode()
{
    return (m_selected_node.get() && !m_selected_node->isGen(gen_Project));
}

bool MainFrame::CanPasteNode()
{
    return (m_selected_node.get() && (m_clipboard.get() || isClipboardDataAvailable()));
}

void MainFrame::Undo()
{
    wxWindowUpdateLocker freeze(this);

    m_undo_stack.Undo();
    m_isProject_modified = (m_undo_stack_size != m_undo_stack.size());
    if (!m_undo_stack.wasUndoEventGenerated())
        FireProjectUpdatedEvent();
    if (!m_undo_stack.wasUndoSelectEventGenerated())
        FireSelectedEvent(m_selected_node.get());
}

void MainFrame::Redo()
{
    wxWindowUpdateLocker freeze(this);

    m_undo_stack.Redo();
    m_isProject_modified = (m_undo_stack_size != m_undo_stack.size());
    if (!m_undo_stack.wasRedoEventGenerated())
        FireProjectUpdatedEvent();
    if (!m_undo_stack.wasRedoSelectEventGenerated())
        FireSelectedEvent(getSelectedNode());
}

void MainFrame::ToggleBorderFlag(Node* node, int border)
{
    if (!node)
        return;

    auto propFlag = node->getPropPtr(prop_borders);

    if (!propFlag)
        return;

    auto value = ClearMultiplePropFlags("wxALL|wxTOP|wxBOTTOM|wxRIGHT|wxLEFT", propFlag->as_string());

    auto intVal = propFlag->as_int();
    intVal ^= border;

    if ((intVal & wxALL) == wxALL)
        value = value << "|wxALL";
    else
    {
        if ((intVal & wxTOP))
            value << "|wxTOP";

        if ((intVal & wxBOTTOM))
            value << "|wxBOTTOM";

        if ((intVal & wxRIGHT))
            value << "|wxRIGHT";

        if ((intVal & wxLEFT))
            value << "|wxLEFT";
    }

    if (value[0] == '|')
        value.erase(0, 1);

    modifyProperty(propFlag, value);
}

void MainFrame::modifyProperty(NodeProperty* prop, tt_string_view value)
{
    if (prop && value != prop->as_string())
    {
        if (auto* gen = prop->getNode()->getGenerator(); !gen || !gen->modifyProperty(prop, value))
        {
            PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
        }
    }
}

void MainFrame::ChangeAlignment(Node* node, int align, bool vertical)
{
    if (!node)
        return;

    auto propFlag = node->getPropPtr(prop_alignment);

    if (!propFlag)
        return;

    tt_string value;

    // First we delete the flags from the previous configuration, in order to avoid alignment conflicts.

    if (vertical)
    {
        value = ClearMultiplePropFlags("wxALIGN_TOP|wxALIGN_BOTTOM|wxALIGN_CENTER_VERTICAL", propFlag->as_string());
    }
    else
    {
        value = ClearMultiplePropFlags("wxALIGN_LEFT|wxALIGN_RIGHT|wxALIGN_CENTER_HORIZONTAL", propFlag->as_string());
    }

    const char* alignStr;
    switch (align)
    {
        case wxALIGN_RIGHT:
            alignStr = "wxALIGN_RIGHT";
            break;

        case wxALIGN_CENTER_HORIZONTAL:
            alignStr = "wxALIGN_CENTER_HORIZONTAL";
            break;

        case wxALIGN_BOTTOM:
            alignStr = "wxALIGN_BOTTOM";
            break;

        case wxALIGN_CENTER_VERTICAL:
            alignStr = "wxALIGN_CENTER_VERTICAL";
            break;

        default:
            alignStr = "";
            break;
    }

    modifyProperty(propFlag, SetPropFlag(alignStr, value));
}

bool MainFrame::GetLayoutSettings(int* flag, int* option, int* border, int* orient)
{
    if (!m_selected_node || !m_selected_node->getParent() || !m_selected_node->getParent()->isSizer())
    {
        return false;
    }

    auto prop_flags = m_selected_node->getSizerFlags();

    auto propOption = m_selected_node->getPropPtr(prop_proportion);
    if (propOption)
    {
        *option = prop_flags.GetProportion();
    }

    *flag = prop_flags.GetFlags();
    *border = prop_flags.GetBorderInPixels();

    auto sizer = m_selected_node->getParent();
    if (sizer)
    {
        if (sizer->isGen(gen_wxBoxSizer) || m_selected_node->isStaticBoxSizer())
        {
            auto propOrient = sizer->getPropPtr(prop_orientation);
            if (propOrient)
            {
                *orient = propOrient->as_int();
            }
        }
    }
    return true;
}

bool MainFrame::MoveNode(Node* node, MoveDirection where, bool check_only)
{
    auto parent = node->getParent();

    ASSERT(parent || node->isGen(gen_Project));
    if (!parent)
        return false;

    if (node->isGen(gen_Images) || parent->isGen(gen_Images))
    {
        if (!check_only)
        {
            wxMessageBox("You can't move images within Images List", "Error", wxICON_ERROR);
        }
        return false;
    }
    else if (node->isGen(gen_Data) || (parent->isGen(gen_Data) && !node->isGen(gen_data_folder)))
    {
        if (!check_only)
        {
            wxMessageBox("You can't move data strings within Data List", "Error", wxICON_ERROR);
        }
        return false;
    }

    if (parent->isGen(gen_wxGridBagSizer))
    {
        return GridBag::MoveNode(node, where, check_only);
    }

    switch (where)
    {
        case MoveDirection::Left:
            if (node->isGen(gen_folder) || node->isGen(gen_data_folder))
                return false;
            else if (node->isGen(gen_sub_folder) && parent->isGen(gen_folder))
                return false;  // You can't have Project as the parent of a sub_folder

            if (parent->isGen(gen_folder) || parent->isGen(gen_sub_folder))
            {
                if (!check_only)
                {
                    wxWindowUpdateLocker freeze(this);
                    auto grandparent = parent->getParent();
                    int pos = (to_int) grandparent->getChildPosition(parent) + 1;
                    PushUndoAction(std::make_shared<ChangeParentAction>(node, parent->getParent(), pos));
                }
                return true;
            }

            if (auto grandparent = parent->getParent(); grandparent)
            {
                if (auto valid_parent = NodeCreation.isValidCreateParent(node->getGenName(), grandparent); valid_parent)
                {
                    if (!check_only)
                    {
                        wxWindowUpdateLocker freeze(this);
                        int pos = -1;
                        if (grandparent == valid_parent)
                            pos = (to_int) grandparent->getChildPosition(parent) + 1;
                        PushUndoAction(std::make_shared<ChangeParentAction>(node, grandparent, pos));
                    }
                    return true;
                }
            }
            return false;

        case MoveDirection::Right:
            if (node->isGen(gen_folder) || node->isGen(gen_sub_folder) || node->isGen(gen_data_folder) ||
                node->isGen(gen_Images) || node->isGen(gen_Data))
            {
                return false;
            }

            if (auto pos = parent->getChildPosition(node) - 1; pos < parent->getChildCount())
            {
                if (node->isForm() && pos >= 0)
                {
                    auto* new_parent = parent->getChild(pos);
                    if (new_parent->isForm())
                    {
                        ASSERT_MSG(check_only, tt_string()
                                                   << "MoveDirection::Right called even though check would have failed.");
                        return false;
                    }
                    else if (new_parent->isGen(gen_folder) || new_parent->isGen(gen_sub_folder))
                    {
                        if (!check_only)
                        {
                            wxWindowUpdateLocker freeze(this);
                            PushUndoAction(std::make_shared<ChangeParentAction>(node, new_parent));
                        }
                        return true;
                    }
                }
                else if (node->isGen(gen_sub_folder) && pos >= 0)
                {
                    auto* new_parent = parent->getChild(pos);
                    while (new_parent->isForm())
                    {
                        if (pos == 0)
                        {
                            ASSERT_MSG(check_only,
                                       tt_string() << "MoveDirection::Right called even though check would have failed.");
                            return false;
                        }
                    }
                    if (new_parent->isGen(gen_folder) || new_parent->isGen(gen_sub_folder))
                    {
                        if (!check_only)
                        {
                            wxWindowUpdateLocker freeze(this);
                            PushUndoAction(std::make_shared<ChangeParentAction>(node, new_parent));
                        }
                        return true;
                    }
                }

                auto possible_parent = parent->getChild(pos);
                if (auto valid_parent = NodeCreation.isValidCreateParent(node->getGenName(), possible_parent, false);
                    valid_parent)
                {
                    if (!check_only)
                    {
                        wxWindowUpdateLocker freeze(this);
                        PushUndoAction(std::make_shared<ChangeParentAction>(node, valid_parent));
                    }
                    return true;
                }
            }
            return false;

        case MoveDirection::Up:
            if (auto pos = parent->getChildPosition(node); pos > 0)
            {
                if (!check_only)
                {
                    wxWindowUpdateLocker freeze(this);
                    PushUndoAction(std::make_shared<ChangePositionAction>(node, pos - 1));
                }
                return true;
            }
            return false;

        case MoveDirection::Down:
            if (auto pos = parent->getChildPosition(node) + 1; pos < parent->getChildCount())
            {
                if (!check_only)
                {
                    wxWindowUpdateLocker freeze(this);
                    PushUndoAction(std::make_shared<ChangePositionAction>(node, pos));
                }
                return true;
            }
            return false;
    }

    return false;
}

void MainFrame::RemoveNode(Node* node, bool isCutMode)
{
    ASSERT_MSG(!node->isType(type_project), "Don't call RemoveNode to remove the entire project.");
    ASSERT_MSG(node->getParent(), "The node being removed has no parent -- that should be impossible.");

    auto parent = node->getParent();
    if (!parent)
        return;

    if (isCutMode)
    {
        tt_string undo_str;
        undo_str << "cut " << node->declName();
        PushUndoAction(std::make_shared<RemoveNodeAction>(node, undo_str, true));
    }
    else
    {
        tt_string undo_str;
        undo_str << "delete " << node->declName();
        PushUndoAction(std::make_shared<RemoveNodeAction>(node, undo_str, false));
    }
    UpdateWakaTime();
}

void MainFrame::ChangeEventHandler(NodeEvent* event, const tt_string& value)
{
    if (event && value != event->get_value())
    {
        PushUndoAction(std::make_shared<ModifyEventAction>(event, value));
        UpdateWakaTime();
    }
}

void MainFrame::RemoveFileFromHistory(tt_string file)
{
    if (file.empty())
        return;

    for (size_t idx = 0; idx < m_FileHistory.GetCount(); ++idx)
    {
        if (file == m_FileHistory.GetHistoryFile(idx).utf8_string())
        {
            m_FileHistory.RemoveFileFromHistory(idx);
            break;
        }
    }
}

void MainFrame::PushUndoAction(UndoActionPtr cmd, bool add_to_stack)
{
    m_isProject_modified = true;
    if (!add_to_stack)
        cmd->Change();
    else
        m_undo_stack.Push(cmd);
}

void MainFrame::ShowInfoBarMsg(const tt_string& msg, int icon)
{
    m_info_bar->ShowMessage(msg, icon);
    m_info_bar_dismissed = false;
}

void MainFrame::DismissInfoBar()
{
    if (!m_info_bar_dismissed)
    {
        m_info_bar->Dismiss();
        m_info_bar_dismissed = true;
    }
}

BasePanel* MainFrame::GetFirstCodePanel()
{
    auto page = m_notebook->GetPage(1);
    return static_cast<BasePanel*>(page);
}

void MainFrame::RemoveCustomEventHandler(wxEvtHandler* handler)
{
    for (auto iter = m_custom_event_handlers.begin(); iter != m_custom_event_handlers.end(); ++iter)
    {
        if (*iter == handler)
        {
            m_custom_event_handlers.erase(iter);
            return;
        }
    }
}
