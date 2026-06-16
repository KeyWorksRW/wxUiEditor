/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-27-2026]

// mainframe_events.cpp contains the event handlers for the MainFrame class.
// mainframe_updates.cpp contains the MainFrame::Update...() functions

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

#include "wxue_namespace/wxue_string.h"  // wxue::string

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
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties
#include "version.h"          // Version information for wxUiEditor and wxWidgets
#include "wxdocview_dlg.h"    // wxDocView -- Dialog for displaying wxWidgets documentation

#include "newdialogs/new_mdi.h"  // NewMdiForm -- Dialog for creating a new MDI application

#include "panels/base_panel.h"      // BasePanel -- C++ panel
#include "panels/doc_view.h"        // Panel for displaying docs in wxWebView
#include "panels/nav_panel.h"       // NavigationPanel -- Node tree class
#include "panels/propgrid_panel.h"  // PropGridPanel -- Node inspector class
#include "panels/ribbon_tools.h"    // RibbonPanel -- Displays component tools in a wxRibbonBar

#include "wxui/ui_images.h"  // This is generated from the Images List

#include "internal/import_panel.h"  // ImportPanel -- Panel to display original imported file
#include "internal/msg_logging.h"   // MsgLogging -- Message logging class
#include "internal/node_info.h"     // NodeInfo
#include "internal/undo_info.h"     // UndoInfo

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    #include "internal/debugsettings.h"  // DebugSettings -- Settings while running the Debug version of wxUiEditor
#endif

#include "mockup/mockup_parent.h"  // MockupParent -- Top-level MockUp Parent window

// Warning! This MUST be at least 3!
constexpr const size_t StatusPanels = 3;

using namespace wxue_img;
using namespace GenEnum;

// Comment out the following line to change the UI back to the way it was in 1.1.2 and all earlier
// versions.
constexpr int NEW_LAYOUT = 1;

enum class MenuIDs : int
{
    IDM_IMPORT_WINRES = START_TESTING_IDS,

    ID_EXPERIMENTAL_MDI_APP,

    id_TestSwitch,
    id_CodeDiffDlg,
    id_ConvertImage,
    id_DebugCurrentTest,
    id_DebugPreferences,
    id_DebugXrcDuplicate,
    id_DebugXrcImport,
    id_FindWidget,
    id_GenerateCpp,
    id_GeneratePerl,
    id_GeneratePython,
    id_GenerateRuby,
    id_GenerateXrc,
    id_GenSingleCpp,
    id_GenSinglePython,
    id_GenSingleRuby,
    id_GenSingleXrc,
    id_NodeMemory,
    id_ShowLogger,
    id_XrcPreviewDlg,
    id_UndoInfo,
    id_DocViewDlg,
    id_DebugPythonTest,
    id_DebugRubyTest,
    id_AssertionTest,
};

const std::string_view txtNewProject = "New Project";
constexpr int MAX_HISTORY_FILES = 9;

MainFrame::MainFrame() :
    MainFrameBase(nullptr),
    m_findData(wxFR_DOWN),
    m_ImportHistory(MAX_HISTORY_FILES, START_IMPORT_FILE_IDS)
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

    wxConfigBase* config = wxConfig::Get();

    // Normally, wxPersistentRegisterAndRestore(this, "MainFrame"); could be called to save/restore
    // the size and position. That works fine on Windows 10, but on Windows 11, a user can maximize
    // the height of a window by dragging the frame to the bottom of the screen. This does not
    // generate the normal size event, and therefore the main windows doesn't save or restore the
    // correct size and position. It's worth noting that even Windows apps like Notepad don't handle
    // this correctly either. However, by retrieving the dimensions when the app is closed, the
    // exact size and position can be saved and restored without relying on event messages.

#if defined(_DEBUG)
    config->SetPath("/debug_mainframe");
#else
    config->SetPath("/mainframe");
#endif

    if (const bool isMaximixed = config->ReadBool("IsMaximized", false); isMaximixed)
    {
        Maximize();
    }
    else
    {
        if (const bool isIconized = config->ReadBool("IsIconized", false); isIconized)
        {
            Iconize();
        }
        else
        {
            wxPoint point;
            point.x = config->ReadLong("PosX", -1);
            point.y = config->ReadLong("PosY", -1);
            SetPosition(point);

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

    CreateTestingMenuItems(this);

#if defined(_DEBUG)
    auto* menuInternal = new wxMenu;

    menuInternal->AppendSeparator();
    menuInternal->Append(std::to_underlying(MenuIDs::id_DebugPreferences), "Test &Settings...",
                         "Settings to use in testing builds");
    menuInternal->Append(std::to_underlying(MenuIDs::id_DebugCurrentTest), "&Current Test",
                         "Current debugging test");

    menuInternal->AppendSeparator();
    menuInternal->Append(std::to_underlying(MenuIDs::id_AssertionTest), "&Assertion Test...",
                         "Run assertion test");

    menuInternal->AppendSeparator();
    menuInternal->Append(std::to_underlying(MenuIDs::id_ConvertImage), "&Convert Image...",
                         "Image conversion testing...");

    m_menubar->Append(menuInternal, "&Internal");

#endif  // defined(_DEBUG)

    if (!wxGetApp().isTestingMenuEnabled())
    {
        // For version 1.1.0.0, preview isn't reliable enough to be included in the release version
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

    Bind(wxEVT_MENU, &MainFrame::OnImportWindowsResource, this,
         std::to_underlying(MenuIDs::IDM_IMPORT_WINRES));
    Bind(wxEVT_MENU, &MainFrame::OnOpenRecentProject, this, wxID_FILE1, wxID_FILE9);

    Bind(wxEVT_FIND, &MainFrame::OnFind, this);
    Bind(wxEVT_FIND_NEXT, &MainFrame::OnFind, this);
    Bind(wxEVT_FIND_CLOSE, &MainFrame::OnFindClose, this);
    Bind(wxEVT_TIMER, &MainFrame::OnGenerationTimer, this, m_generation_timer.GetId());

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
            std::ignore = CreateViaNewDlg(static_cast<GenName>(event.GetId()));
        },
        CreateNewDialog, MdiMenuBar - 1);

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent& event)
        {
            CreateToolNode(static_cast<GenName>(event.GetId()));
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

#if defined(_DEBUG)
    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            DebugSettings dialog(this);
            dialog.ShowModal();
        },
        std::to_underlying(MenuIDs::id_DebugPreferences));

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            ASSERT_MSG(false, wxue::string() << "Assertion test triggered");
        },
        std::to_underlying(MenuIDs::id_AssertionTest));

    Bind(wxEVT_MENU, &App::DbgCurrentTest, &wxGetApp(),
         std::to_underlying(MenuIDs::id_DebugCurrentTest));
#endif

    AddCustomEventHandler(GetEventHandler());

    m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &MainFrame::OnAuiNotebookPageChanged, this);
}

MainFrame::~MainFrame()
{
    delete m_findDialog;
}

void MainFrame::CreateTestingMenuItems(MainFrame* frame)
{
    if (!wxGetApp().isTestingMenuEnabled())
    {
        return;
    }

    wxMenuItem* item = nullptr;

    auto* menuExperimental = new wxMenu;
    menuExperimental->Append(std::to_underlying(MenuIDs::ID_EXPERIMENTAL_MDI_APP),
                             "Experimental MDI App", "Create a base Document/View MDI application");

    auto* menuTesting = new wxMenu;

    menuTesting->Append(std::to_underlying(MenuIDs::id_FindWidget), "&Find Widget...",
                        "Search for a widget starting with the current selected node");
    menuTesting->Append(std::to_underlying(MenuIDs::id_NodeMemory), "Node &Information...",
                        "Show node memory usage");
    menuTesting->Append(std::to_underlying(MenuIDs::id_UndoInfo), "Undo &Stack Information...",
                        "Show undo/redo stack memory usage");
    menuTesting->AppendSeparator();
    menuTesting->Append(std::to_underlying(MenuIDs::id_GeneratePython), "&Generate Python",
                        "Generate all python files from current project.");
    menuTesting->Append(std::to_underlying(MenuIDs::id_GenerateRuby), "&Generate Ruby",
                        "Generate all ruby files from current project.");

    auto* submenu_xrc = new wxMenu();
    item = submenu_xrc->Append(std::to_underlying(MenuIDs::id_XrcPreviewDlg), "&XRC Tests...",
                               "Dialog with multiple XRC tests");
    item->SetBitmap(bundle_xrc_tests_svg(16, 16));
    item = submenu_xrc->Append(std::to_underlying(MenuIDs::id_DebugXrcImport), "&Test XRC import",
                               "Export the current form, then verify importing it");
    item->SetBitmap(bundle_import_svg(16, 16));
    submenu_xrc->Append(std::to_underlying(MenuIDs::id_DebugXrcDuplicate), "&Test XRC duplication",
                        "Duplicate the current form via Export and Import XRC");
    menuTesting->AppendSubMenu(submenu_xrc, "&XRC");

    menuTesting->AppendSeparator();
    menuTesting->Append(std::to_underlying(MenuIDs::id_ShowLogger), "Show &Log Window",
                        "Show window containing debug messages");
    wxMenuItem* menuItem =
        menuTesting->Append(std::to_underlying(MenuIDs::id_TestSwitch), "Testing Switch",
                            "Toggle test switch", wxITEM_CHECK);
    menuItem->Check(wxGetApp().isTestingSwitch());
    frame->Bind(
        wxEVT_MENU,
        [](wxCommandEvent& event)
        {
            if (wxGetApp().isTestingSwitch())
            {
                wxGetApp().setTestingSwitch(false);
                if (wxMenuItem* menuItemTestSwitch =
                        wxStaticCast(event.GetEventObject(), wxMenu)
                            ->FindItem(std::to_underlying(MenuIDs::id_TestSwitch)))
                {
                    menuItemTestSwitch->Check(false);
                }
            }
            else
            {
                wxGetApp().setTestingSwitch(true);
                if (wxMenuItem* menuItemTestSwitch =
                        wxStaticCast(event.GetEventObject(), wxMenu)
                            ->FindItem(std::to_underlying(MenuIDs::id_TestSwitch)))
                {
                    menuItemTestSwitch->Check(true);
                }
            }
        },
        std::to_underlying(MenuIDs::id_TestSwitch));

    frame->m_menubar->Append(menuExperimental, "Experimental");
    frame->m_menubar->Append(menuTesting, "Testing");

    frame->m_submenu_import_recent = new wxMenu();
    frame->m_menuFile->AppendSeparator();
    frame->m_menuFile->AppendSubMenu(frame->m_submenu_import_recent, "Import &Recent");

    wxConfigBase* config = wxConfig::Get();
    config->SetPath("/debug_history");
    frame->m_ImportHistory.Load(*config);
    frame->m_ImportHistory.UseMenu(frame->m_submenu_import_recent);
    frame->m_ImportHistory.AddFilesToMenu();
    config->SetPath("/");

    frame->Bind(wxEVT_MENU, &MainFrame::OnImportRecent, frame, START_IMPORT_FILE_IDS,
                START_IMPORT_FILE_IDS + MAX_HISTORY_FILES);

    // Experimental menu items
    frame->Bind(
        wxEVT_MENU,
        [frame](wxCommandEvent&)
        {
            wxGetMainFrame()->SelectNode(Project.get_ProjectNode(), evt_flags::force_selection);
            NewMdiForm dialog(frame);
            if (dialog.ShowModal() == wxID_OK)
            {
                dialog.CreateNode();
            }
        },
        std::to_underlying(MenuIDs::ID_EXPERIMENTAL_MDI_APP));

    frame->Bind(
        wxEVT_MENU,
        [frame](wxCommandEvent&)
        {
            NodeInfo dialog(frame);
            dialog.ShowModal();
        },
        std::to_underlying(MenuIDs::id_NodeMemory));

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            UndoInfo dialog(this);
            dialog.ShowModal();
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
            OnGenerateSingleLanguage(GenLang::python);
        },
        id_GenSinglePython);

    frame->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            OnGenerateSingleLanguage(GenLang::ruby);
        },
        std::to_underlying(MenuIDs::id_GenSingleRuby));

    frame->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            OnGenerateSingleLanguage(GenLang::xrc);
        },
        std::to_underlying(MenuIDs::id_GenSingleXrc));

    frame->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            OnGenerateLanguage(GenLang::ruby);
        },
        std::to_underlying(MenuIDs::id_GenerateRuby));

    frame->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            OnGenerateLanguage(GenLang::xrc);
        },
        std::to_underlying(MenuIDs::id_GenerateXrc));

    frame->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            g_pMsgLogging->ShowLogger();
        },
        std::to_underlying(MenuIDs::id_ShowLogger));
    frame->Bind(wxEVT_MENU, &MainFrame::OnXrcPreview, frame,
                std::to_underlying(MenuIDs::id_XrcPreviewDlg));
    frame->Bind(wxEVT_MENU, &MainFrame::OnTestXrcImport, frame,
                std::to_underlying(MenuIDs::id_DebugXrcImport));
    frame->Bind(wxEVT_MENU, &MainFrame::OnTestXrcDuplicate, frame,
                std::to_underlying(MenuIDs::id_DebugXrcDuplicate));

    frame->m_toolbar->AddTool(std::to_underlying(MenuIDs::id_XrcPreviewDlg), "XRC Tests",
                              bundle_xrc_tests_svg(24, 24), "Dialog with multiple XRC tests");
}

wxBitmapBundle wxueBundleSVG(const unsigned char* data, size_t size_data, size_t size_svg,
                             wxSize def_size);

#if defined(_DEBUG)
    #include "internal/debugsettings.h"
#endif

void MainFrame::ProjectLoaded()
{
    Project.ChangeDir();
    setStatusText("Project loaded");
    ResetDerivedCodeState();
    if (!m_isImported)
    {
        if (!Project.is_NewProject())
        {
            m_FileHistory.AddFileToHistory(Project.get_ProjectFile());
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
                 {
                     m_wakatime->ResetHeartbeat();
                 }
             });
    }

    if (!Project.HasValue(prop_wxWidgets_version))
    {
        std::ignore = Project.set_value(prop_wxWidgets_version, UserPrefs.get_CppWidgetsVersion());
    }
    else if (Project.as_string(prop_wxWidgets_version) == "3.2")
    {
        std::ignore = Project.set_value(prop_wxWidgets_version, WXWIDGETS_VERSION_3_2_0);
    }

    m_selected_node = Project.get_ProjectNode()->get_SharedPtr();

    UpdateLanguagePanels();
}

void MainFrame::ProjectSaved()
{
    setStatusText(wxue::string(Project.get_ProjectFile().filename()) << " saved");
    UpdateFrame();
}

bool MainFrame::SaveWarning()
{
    int result = wxYES;

    if (m_isProject_modified)
    {
        // Testing often requires importing multiple projects to verify they work, so there is
        // no reason to save them.
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
    m_notebook =
        new wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook->SetArtProvider(new wxAuiSimpleTabArt());

    m_mockupPanel = new MockupParent(m_notebook, this);
    m_notebook->AddPage(m_mockupPanel, "Mock Up", false, wxWithImages::NO_IMAGE);

#if wxUSE_WEBVIEW
    m_docviewPanel = new DocViewPanel(m_notebook, this);
    m_notebook->AddPage(m_docviewPanel, "Docs", false, wxWithImages::NO_IMAGE);
#endif

    if (wxGetApp().isTestingMenuEnabled())
    {
        // Shows original import file if project is imported, otherwise it shows the project
        // file
        m_importPanel = new ImportPanel(m_notebook);
        m_notebook->AddPage(m_importPanel, "Import", false, wxWithImages::NO_IMAGE);
    }

    return m_notebook;
}

void MainFrame::CreateSplitters()
{
    // The main splitter contains the navigation tree control and it's toolbar on the left. On
    // the right is a panel containing the Ribbon toolbar at the top, and a splitter window
    // containing the property grid and notebook with mockup and code windows below it.

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

    m_SecondarySplitter = new wxSplitterWindow(m_panel_right, wxID_ANY, wxDefaultPosition,
                                               wxDefaultSize, wxSP_LIVE_UPDATE);
    m_right_panel_sizer->Add(m_SecondarySplitter, wxSizerFlags(1).Expand());

    m_property_panel = new PropGridPanel(m_SecondarySplitter, this);
    wxWindow* notebook = CreateNoteBook(m_SecondarySplitter);

    if (UserPrefs.is_RightPropGrid())
    {
        m_SecondarySplitter->SplitVertically(notebook, m_property_panel, m_SecondarySashPosition);
    }
    else
    {
        m_SecondarySplitter->SplitVertically(m_property_panel, notebook, m_SecondarySashPosition);
    }

    m_MainSplitter->SplitVertically(m_nav_panel, m_panel_right);
    m_MainSplitter->SetName("Navigation");
    wxPersistenceManager::Get().RegisterAndRestore(m_MainSplitter);

    // Need to update the left splitter so the right one is drawn correctly
    wxSizeEvent update(GetSize(), GetId());
    ProcessEvent(update);
    m_MainSplitter->UpdateSize();
    m_MainSplitter->SetMinimumPaneSize(2);

    // Set to zero because we don't need this to change relative size when the main window is
    // resized. Fixes issue #90
    m_SecondarySplitter->SetSashGravity(0);
    m_SecondarySplitter->SetMinimumPaneSize(2);

    m_SecondarySplitter->SetName("Properties");
    wxPersistenceManager::Get().RegisterAndRestore(m_SecondarySplitter);

    // UpdateStatusWidths();
    m_MainSashPosition = m_MainSplitter->GetSashPosition();
    m_SecondarySashPosition = m_SecondarySplitter->GetSashPosition();
    int widths[StatusPanels] = { 1,
                                 (m_MainSashPosition + m_SecondarySashPosition) - STATUS_SASH_INSET,
                                 -1 };
    SetStatusWidths(sizeof(widths) / sizeof(int), widths);

    // SetMinSize(wxSize(700, 380));
}

void MainFrame::setStatusField(const wxString& text, int position)
{
    if (position == -1)
    {
        position = m_posPropGridStatusField;
    }
    setStatusText(text, position);
}

Node* MainFrame::getSelectedForm()
{
    if (!m_selected_node || m_selected_node->is_Gen(gen_Project))
    {
        return nullptr;
    }

    return m_selected_node->get_Form();
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

    m_selected_node = node->get_SharedPtr();

    if (flags & evt_flags::queue_event)
    {
        const CustomEvent node_event(EVT_NodeSelected, m_selected_node.get());
        for (auto* handler: m_custom_event_handlers)
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

void MainFrame::PasteNode(Node* parent)
{
    if (const bool result = isClipboardDataAvailable(); result)
    {
        const NodeSharedPtr new_node = GetClipboardNode();
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
        {
            return;
        }
        parent = m_selected_node.get();
    }

    if (parent->is_Gen(gen_wxSplitterWindow) && parent->get_ChildCount() > 1)
    {
        wxMessageBox("A wxSplitterWindow can't have more than two windows.");
        return;
    }

    // This will insert the node using an Undo command labeled Paste + clipboard string name.
    //
    // It's a lambda because it can be called within a nested conditional -- this avoids code
    // duplication.
    auto create_undo_event = [&](const NodeSharedPtr& created_node) -> void
    {
        wxue::string undo_str("Paste ");
        undo_str << m_clipboard->get_DeclName();

        const ptrdiff_t pos = parent->FindInsertionPos(m_selected_node);
        PushUndoAction(
            std::make_shared<InsertNodeAction>(created_node.get(), parent, undo_str, pos));
        FireCreatedEvent(created_node);
        SelectNode(created_node, evt_flags::fire_event | evt_flags::force_selection);
    };

    const NodeSharedPtr new_node = NodeCreation.MakeCopy(m_clipboard.get(), parent);

    // This makes it possible to switch from a normal child toolbar to a form toolbar and vice
    // versa. Both wxToolBar and wxAuiToolbar are supported
    if ((parent->is_Gen(gen_ToolBar) && new_node->is_Gen(gen_wxToolBar)) ||
        (parent->is_Gen(gen_AuiToolBar) && new_node->is_Gen(gen_wxAuiToolBar)) ||
        (parent->is_Gen(gen_wxToolBar) && new_node->is_Gen(gen_ToolBar)) ||
        (parent->is_Gen(gen_wxAuiToolBar) && new_node->is_Gen(gen_AuiToolBar)))
    {
        auto group = std::make_shared<GroupUndoActions>("Paste children", parent);

        for (auto& child_node: new_node->get_ChildNodePtrs())
        {
            auto insert_action =
                std::make_shared<InsertNodeAction>(child_node.get(), parent, "paste");
            insert_action->SetFireCreatedEvent(true);
            group->Add(insert_action);
        }
        wxGetFrame().PushUndoAction(group);
        return;
    }
    // This makes it possible to paste between a wxToolBar and a wxAuiToolBar and vice versa.
    // Both a normal child and a form toolbar are supported.
    if ((parent->is_Gen(gen_AuiToolBar) && new_node->is_Gen(gen_wxToolBar)) ||
        (parent->is_Gen(gen_wxAuiToolBar) && new_node->is_Gen(gen_wxToolBar)) ||
        (parent->is_Gen(gen_ToolBar) && new_node->is_Gen(gen_wxAuiToolBar)) ||
        (parent->is_Gen(gen_wxToolBar) && new_node->is_Gen(gen_wxAuiToolBar)))
    {
        auto group = std::make_shared<GroupUndoActions>("Paste children", parent);

        for (auto& child_node: new_node->get_ChildNodePtrs())
        {
            // We are changing from a wxToolBar to a wxAuiToolBar, so we need to change the node
            // type
            const NodeSharedPtr new_child = NodeCreation.MakeCopy(child_node.get(), parent);
            auto insert_action =
                std::make_shared<InsertNodeAction>(new_child.get(), parent, "paste");
            insert_action->SetFireCreatedEvent(true);
            group->Add(insert_action);
        }
        wxGetFrame().PushUndoAction(group);
        return;
    }

    if (new_node->is_Form())
    {
        Project.FixupDuplicatedNode(new_node.get());
    }

    if (!parent->is_ChildAllowed(new_node))
    {
        Node* grandparent = parent->get_Parent();

        if (new_node->is_Gen(gen_wxMenuItem))
        {
            if (grandparent && (parent->is_ToolBar() || grandparent->is_ToolBar()))
            {
                if (!parent->is_ToolBar())
                {
                    parent = grandparent;
                }
                const GenName tool_generator =
                    (parent->is_Type(type_toolbar) || parent->is_Type(type_toolbar_form)) ?
                        gen_tool :
                        gen_auitool;
                auto [tool_node_ptr, tool_node_validity] =
                    NodeCreation.CreateNode(tool_generator, parent);
                ASSERT(tool_node_validity == Node::Validity::valid_node);
                // REVIEW: [Randalphwa - 04-28-2025] Not being able to create a tool node with a
                // valid toolbar parent is extremely unlikely. Simply returning prevents a
                // crash, but tells the user nothing...
                if (tool_node_validity != Node::Validity::valid_node)
                {
                    return;
                }

                for (const auto& iter: new_node->get_NodeDeclaration()->GetPropInfoMap())
                {
                    // Walk through all the properties in the menu item and copy any of them
                    // that exist in the tool node.
                    if (tool_node_ptr->HasProp(iter.second->get_name()))
                    {
                        tool_node_ptr->set_value(
                            iter.second->get_name(),
                            new_node->get_PropPtr(iter.second->get_name())->get_value());
                    }
                }
                create_undo_event(tool_node_ptr);
                return;
            }
        }
        else if (new_node->is_Gen(gen_tool) || new_node->is_Gen(gen_auitool))
        {
            if (grandparent && (parent->is_Menu() || grandparent->is_Menu()))
            {
                if (!parent->is_Menu())
                {
                    parent = grandparent;
                }
                auto [menu_node_ptr, menu_node_validity] =
                    NodeCreation.CreateNode(gen_wxMenuItem, parent);
                ASSERT(menu_node_validity == Node::Validity::valid_node);
                if (menu_node_validity != Node::Validity::valid_node)
                {
                    return;
                }

                for (const auto& iter: new_node->get_NodeDeclaration()->GetPropInfoMap())
                {
                    // Walk through all the properties in the menu item and copy any of them
                    // that exist in the tool node.
                    if (menu_node_ptr->HasProp(iter.second->get_name()))
                    {
                        menu_node_ptr->set_value(
                            iter.second->get_name(),
                            new_node->get_PropPtr(iter.second->get_name())->get_value());
                    }
                }
                create_undo_event(menu_node_ptr);
                return;
            }
        }

        if (!grandparent || !grandparent->is_ChildAllowed(new_node))
        {
            wxMessageBox(wxue::string() << "You cannot paste " << new_node->get_DeclName()
                                        << " into " << parent->get_DeclName());
            return;
        }
        parent = grandparent;
    }

    if (parent->is_Gen(gen_wxGridBagSizer))
    {
        GridBag grid_bag(parent);
        [[maybe_unused]] const bool result = grid_bag.InsertNode(parent, new_node.get());
        return;
    }

    create_undo_event(new_node);
}

bool MainFrame::CanCopyNode()
{
    return (m_selected_node.get() && !m_selected_node->is_Gen(gen_Project));
}

bool MainFrame::CanPasteNode()
{
    return (m_selected_node.get() && (m_clipboard.get() || isClipboardDataAvailable()));
}

void MainFrame::Undo()
{
    const wxWindowUpdateLocker freeze(this);

    m_undo_stack.Undo();
    m_isProject_modified = (m_undo_stack_size != m_undo_stack.size());
    if (!m_undo_stack.wasUndoEventGenerated())
    {
        FireProjectUpdatedEvent();
    }
    if (!m_undo_stack.wasUndoSelectEventGenerated())
    {
        FireSelectedEvent(m_selected_node.get());
    }
}

void MainFrame::Redo()
{
    const wxWindowUpdateLocker freeze(this);

    m_undo_stack.Redo();
    m_isProject_modified = (m_undo_stack_size != m_undo_stack.size());
    if (!m_undo_stack.wasRedoEventGenerated())
    {
        FireProjectUpdatedEvent();
    }
    if (!m_undo_stack.wasRedoSelectEventGenerated())
    {
        FireSelectedEvent(getSelectedNode());
    }
}

void MainFrame::ModifyProperty(NodeProperty* prop, std::string_view value)
{
    if (prop && value != prop->as_string())
    {
        if (BaseGenerator* generator = prop->getNode()->get_Generator();
            !generator || !generator->ModifyProperty(prop, value))
        {
            PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
        }
    }
}

bool MainFrame::GetLayoutSettings(int* flag, int* option, int* border, int* orient)
{
    if (!m_selected_node || !m_selected_node->get_Parent() ||
        !m_selected_node->get_Parent()->is_Sizer())
    {
        return false;
    }

    const wxSizerFlags prop_flags = m_selected_node->getSizerFlags();

    const NodeProperty* propOption = m_selected_node->get_PropPtr(prop_proportion);
    if (propOption)
    {
        *option = prop_flags.GetProportion();
    }

    *flag = prop_flags.GetFlags();
    *border = prop_flags.GetBorderInPixels();

    Node* sizer = m_selected_node->get_Parent();
    if (sizer)
    {
        if (sizer->is_Gen(gen_wxBoxSizer) || m_selected_node->is_StaticBoxSizer())
        {
            const NodeProperty* propOrient = sizer->get_PropPtr(prop_orientation);
            if (propOrient)
            {
                *orient = propOrient->as_int();
            }
        }
    }
    return true;
}

void MainFrame::ChangeEventHandler(NodeEvent* event, std::string_view value)
{
    if (event && value != event->get_value())
    {
        PushUndoAction(std::make_shared<ModifyEventAction>(event, value));
        UpdateWakaTime();
    }
}

void MainFrame::RemoveFileFromHistory(std::string_view file)
{
    if (file.empty())
    {
        return;
    }

    for (size_t idx = 0; idx < m_FileHistory.GetCount(); ++idx)
    {
        if (file == m_FileHistory.GetHistoryFile(idx).utf8_string())
        {
            m_FileHistory.RemoveFileFromHistory(idx);
            break;
        }
    }
}

void MainFrame::PushUndoAction(const UndoActionPtr& cmd, bool add_to_stack)
{
    m_isProject_modified = true;
    if (!add_to_stack)
    {
        cmd->Change();
    }
    else
    {
        m_undo_stack.Push(cmd);
    }
}

void MainFrame::ShowInfoBarMsg(std::string_view msg, int icon)
{
    m_info_bar->ShowMessage(wxString(msg.data(), msg.size()), icon);
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
    auto* panel = dynamic_cast<BasePanel*>(m_notebook->GetPage(1));
    ASSERT_MSG(panel, "Page 1 is not a BasePanel");
    return panel;
}

void MainFrame::RemoveCustomEventHandler(wxEvtHandler* handler)
{
    for (std::vector<wxEvtHandler*>::iterator iter = m_custom_event_handlers.begin();
         iter != m_custom_event_handlers.end(); ++iter)
    {
        if (*iter == handler)
        {
            m_custom_event_handlers.erase(iter);
            return;
        }
    }
}
