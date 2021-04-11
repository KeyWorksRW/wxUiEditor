/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/aboutdlg.h>          // declaration of wxAboutDialog class
#include <wx/config.h>            // wxConfig base header
#include <wx/filedlg.h>           // wxFileDialog base header
#include <wx/frame.h>             // wxFrame class interface
#include <wx/infobar.h>           // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/persist/splitter.h>  // persistence support for wxTLW
#include <wx/persist/toplevel.h>  // persistence support for wxTLW
#include <wx/splitter.h>          // Base header for wxSplitterWindow
#include <wx/stc/stc.h>           // Scintilla
#include <wx/toolbar.h>           // wxToolBar interface declaration

#include <ttcvector.h>   // cstrVector -- Vector of ttlib::cstr strings
#include <ttcwd.h>       // cwd -- Class for storing and optionally restoring the current directory
#include <ttmultistr.h>  // multistr -- Breaks a single string into multiple strings

// auto-generated: mainframe_base.h and mainframe_base.cpp

#include "mainframe.h"

#include "auto_freeze.h"   // AutoFreeze -- Automatically Freeze/Thaw a window
#include "bitmaps.h"       // Map of bitmaps accessed by name
#include "cstm_event.h"    // CustomEvent -- Custom Event class
#include "gen_base.h"      // Generate Base class
#include "gen_enums.h"     // Enumerations for generators
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class
#include "node_prop.h"     // NodeProperty -- NodeProperty class
#include "pjtsettings.h"   // ProjectSettings -- Hold data for currently loaded project
#include "uifuncs.h"       // Miscellaneous functions for displaying UI
#include "undo_cmds.h"     // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "utils.h"         // Utility functions that work with properties
#include "write_code.h"    // Write code to Scintilla or file

#include "panels/base_panel.h"      // BasePanel -- C++ panel
#include "panels/nav_panel.h"       // NavigationPanel -- Node tree class
#include "panels/propgrid_panel.h"  // PropGridPanel -- Node inspector class
#include "panels/ribbon_tools.h"    // RibbonPanel -- Displays component tools in a wxRibbonBar

#include "ui/importwinresdlg.h"  // ImportWinResDlg -- Dialog for Importing a Windows resource file
#include "ui/insertdialog.h"     // InsertDialog -- Dialog to lookup and insert a widget

#if defined(_DEBUG)
    #include "ui/debugsettings.h"  // DebugSettings -- Settings while running the Debug version of wxUiEditor
#endif

#include "mockup/mockup_parent.h"  // MockupParent -- Top-level MockUp Parent window

#include "../pugixml/pugixml.hpp"

#include "xpm/logo64.xpm"

using namespace GenEnum;

bool GenerateCodeFiles(wxWindow* parent, bool NeedsGenerateCheck);

enum
{
    IDM_IMPORT_WINRES = wxID_HIGHEST + 500,

    id_DebugCurrentTest,
    id_DebugPreferences,
    id_ShowLogger,
};

MainFrame::MainFrame() : MainFrameBase(nullptr), m_findData(wxFR_DOWN)
{
    wxIconBundle bundle;

    wxIcon ico16;
    ico16.CopyFromBitmap(GetXPMImage("logo16"));
    bundle.AddIcon(ico16);

    wxIcon ico32;
    ico32.CopyFromBitmap(GetXPMImage("logo32"));
    bundle.AddIcon(ico32);
    SetIcons(bundle);

    SetTitle("wxUiEditor");

#if defined(_DEBUG)
    auto menuDebug = new wxMenu;
    menuDebug->Append(id_ShowLogger, "Show Log Window", "Show window containing debug messages");
    menuDebug->Append(id_DebugPreferences, "Debug &Settings...", "Settings to use in Debug build");
    menuDebug->Append(id_DebugCurrentTest, "&Current Test", "Current debugging test");

    m_menubar->Append(menuDebug, "&Debug");
#endif  // _DEBUG

    auto config = wxConfig::Get();
    config->SetPath(txt_main_window_config);
    m_FileHistory.Load(*config);
    m_FileHistory.UseMenu(m_submenu_recent);
    m_FileHistory.AddFilesToMenu();
    config->SetPath("/");

    CreateStatusBar(StatusPanels);
    SetStatusBarPane(1);  // specifies where menu and toolbar help content is displayed

    wxPersistentRegisterAndRestore(this, "MainFrame");
    CreateSplitters();

    m_SecondarySplitter->Bind(wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED, [this](wxSplitterEvent&) { UpdateStatusWidths(); });
    m_MainSplitter->Bind(wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED, [this](wxSplitterEvent&) { UpdateStatusWidths(); });

    Bind(wxEVT_MENU, &MainFrame::OnImportWindowsResource, this, IDM_IMPORT_WINRES);
    Bind(wxEVT_MENU, &MainFrame::OnOpenRecentProject, this, wxID_FILE1, wxID_FILE9);

    Bind(wxEVT_FIND, &MainFrame::OnFind, this);
    Bind(wxEVT_FIND_NEXT, &MainFrame::OnFind, this);
    Bind(wxEVT_FIND_CLOSE, &MainFrame::OnFindClose, this);

    Bind(EVT_NodeSelected, &MainFrame::OnNodeSelected, this);

    Bind(EVT_NodeCreated, [this](CustomEvent&) { UpdateFrame(); });
    Bind(EVT_EventHandlerChanged, [this](CustomEvent&) { UpdateFrame(); });
    Bind(EVT_NodeDeleted, [this](CustomEvent&) { UpdateFrame(); });
    Bind(EVT_NodePropChange, [this](CustomEvent&) { UpdateFrame(); });

    Bind(
        wxEVT_MENU, [this](wxCommandEvent&) { Close(); }, wxID_EXIT);

    Bind(
        wxEVT_MENU, [this](wxCommandEvent& event) { m_mockupPanel->ShowHiddenControls(event.IsChecked()); }, id_ShowHidden);

    Bind(
        wxEVT_MENU, [this](wxCommandEvent& event) { m_mockupPanel->MagnifyWindow(event.IsChecked()); }, id_Magnify);

#if defined(_DEBUG)
    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&) {
            DebugSettings dlg(this);
            dlg.ShowModal();
        },
        id_DebugPreferences);

    Bind(
        wxEVT_MENU, [](wxCommandEvent&) { g_pMsgLogging->ShowLogger(); }, id_ShowLogger);

    Bind(wxEVT_MENU, &App::DbgCurrentTest, &wxGetApp(), id_DebugCurrentTest);
#endif

    AddCustomEventHandler(GetEventHandler());

    m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &MainFrame::OnAuiNotebookPageChanged, this);
}

MainFrame::~MainFrame()
{
    delete m_findDialog;
}

void MainFrame::OnSaveProject(wxCommandEvent& event)
{
    if (m_isImported || wxGetApp().getProjectFileName().empty())
        OnSaveAsProject(event);
    else
    {
        pugi::xml_document doc;
        wxGetApp().GetProjectPtr()->CreateDoc(doc);
        if (doc.save_file(wxGetApp().getProjectFileName().c_str(), "  "))
        {
            m_isProject_modified = false;
            ProjectSaved();
        }
        else
        {
            appMsgBox(ttlib::cstr("Unable to save the project: ") << wxGetApp().getProjectFileName(), _tt("Save Project"));
        }
    }
}

void MainFrame::OnSaveAsProject(wxCommandEvent&)
{
    // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui extension
    wxFileDialog dialog(this, _tt(strIdTitleSaveAs), wxEmptyString, wxEmptyString,
                        "wxUiEditor Project File (*.wxui)|*.wxui;*.wxue", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        ttString filename = dialog.GetPath();
        if (filename.extension().empty())
        {
            filename.replace_extension(".wxui");
        }

        // Don't allow the user to walk over existing project file types that are probably associated with another designer
        // tool

        else if (filename.extension().is_sameas(".fbp", tt::CASE::either))
        {
            appMsgBox("You cannot save the project as a wxFormBuilder project file", _tt(strIdTitleSaveAs));
            return;
        }
        else if (filename.extension().is_sameas(".wxs", tt::CASE::either))
        {
            appMsgBox("You cannot save the project as a wxSmith file", _tt(strIdTitleSaveAs));
            return;
        }
        else if (filename.extension().is_sameas(".xrc", tt::CASE::either))
        {
            appMsgBox("You cannot save the project as a XRC file", _tt(strIdTitleSaveAs));
            return;
        }
        else if (filename.extension().is_sameas(".rc", tt::CASE::either) ||
                 filename.extension().is_sameas(".dlg", tt::CASE::either))
        {
            appMsgBox("You cannot save the project as a Windows Resource file", _tt(strIdTitleSaveAs));
            return;
        }

        pugi::xml_document doc;
        wxGetApp().GetProjectPtr()->CreateDoc(doc);
        if (doc.save_file(filename.sub_cstr().c_str(), "  "))
        {
            m_isProject_modified = false;
            m_FileHistory.AddFileToHistory(filename);
            wxGetApp().GetProjectSettings()->SetProjectFile(filename);
            wxGetApp().GetProjectSettings()->SetProjectPath(filename);
            m_isImported = false;
            ProjectSaved();
            FireProjectLoadedEvent();
        }
        else
        {
            appMsgBox(ttlib::cstr("Unable to save the project: ") << filename.wx_str(), _tt(strIdTitleSaveAs));
        }
    };
}

void MainFrame::OnOpenProject(wxCommandEvent&)
{
    if (!SaveWarning())
        return;

    // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui extension
    wxFileDialog dialog(this, _tt(strIdTitleOpenOrImport), wxEmptyString, wxEmptyString,
                        "wxUiEditor Project File (*.wxui)|*.wxui;*.wxue"
                        "|WxFormBuilder Project File (*.fbp)|*.fbp"
                        "|Windows Resource File (*.rc)|*.rc",
                        wxFD_OPEN);

    if (dialog.ShowModal() == wxID_OK)
    {
        ttString filename = dialog.GetPath();
        // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui extension
        if (filename.extension().is_sameas(".wxui", tt::CASE::either) ||
            filename.extension().is_sameas(".wxue", tt::CASE::either))
        {
            wxGetApp().LoadProject(filename);
        }
        else
        {
            wxGetApp().ImportProject(filename);
        }
    };
}

void MainFrame::OnImportFormBuilder(wxCommandEvent&)
{
    ttSaveCwd cwd;
    wxFileDialog dlg(this, _tt(strIdTitleOpenOrImport), cwd, wxEmptyString, "WxFormBuilder Project File (*.fbp)|*.fbp||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        wxGetApp().ImportFormBuilder(files);
    }
}

void MainFrame::OnImportWindowsResource(wxCommandEvent&)
{
    ImportWinResDlg dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxGetApp().ImportWinRes(dlg.GetRcFilename(), dlg.GetDlgNames());
    }
}

void MainFrame::OnInsertWidget(wxCommandEvent&)
{
    InsertDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        CreateToolNode(dlg.GetWidget());
    }
}

void MainFrame::OnOpenRecentProject(wxCommandEvent& event)
{
    if (!SaveWarning())
        return;

    ttString file = m_FileHistory.GetHistoryFile(event.GetId() - wxID_FILE1);

    if (file.file_exists())
    {
        wxGetApp().LoadProject(file);
    }
    else if (appMsgBox(ttlib::cstr().Format(_tt(strIdProjectMRUQuery), file.wx_str()), _tt(strIdTitleOpenMRU),
                       wxICON_WARNING | wxYES_NO) == wxYES)
    {
        m_FileHistory.RemoveFileFromHistory(event.GetId() - wxID_FILE1);
    }
}

void MainFrame::OnNewProject(wxCommandEvent&)
{
    if (!SaveWarning())
        return;

    wxGetApp().NewProject();
}

void MainFrame::OnGenerateCode(wxCommandEvent&)
{
    GenerateCodeFiles(this);
    m_isProject_generated = true;

    m_menuTools->Enable(id_GenerateCode, !m_isProject_generated);
    m_toolbar->EnableTool(id_GenerateCode, !m_isProject_generated);
}

void MainFrame::OnAbout(wxCommandEvent&)
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName(txtVersion);

    // Use trailing spaces to make the dialog width a bit wider
    aboutInfo.SetDescription("wxWidgets GUI designer for C++ applications  ");
    aboutInfo.SetCopyright(txtCopyRight);
    aboutInfo.SetWebSite("https://github.com/KeyWorksRW/wxUiEditor");
    aboutInfo.AddDeveloper("Ralph Walden");

    wxIcon icon;
    icon.CopyFromBitmap(wxImage(logo64_xpm));
    aboutInfo.SetIcon(icon);

    wxAboutBox(aboutInfo);
}

void MainFrame::OnClose(wxCloseEvent& event)
{
    if (!SaveWarning())
        return;

    wxGetApp().SetMainFrameClosing();

    auto config = wxConfig::Get();
    config->SetPath(txt_main_window_config);
    m_FileHistory.Save(*config);
    m_property_panel->SaveDescBoxHeight();

#if defined(_DEBUG)
    g_pMsgLogging->CloseLogger();
#endif

    event.Skip();
}

void MainFrame::ProjectLoaded()
{
    ttlib::ChangeDir(wxGetApp().getProjectPath());
    setStatusText(_tt(strIdStatusProjectLoaded));
    if (!m_isImported)
    {
        m_isProject_generated = !GenerateCodeFiles(this, true);
        m_FileHistory.AddFileToHistory(wxGetApp().GetProjectFileName());
        m_isProject_modified = false;
    }
    else
    {
        m_isProject_generated = false;
    }

    m_undo_stack.clear();
    m_undo_stack_size = 0;

    m_selected_node = wxGetApp().GetProjectPtr();

    UpdateFrame();
}

void MainFrame::ProjectSaved()
{
    ttlib::cstr str(wxGetApp().getProjectFileName().filename() + _ttc(strIdSaved));
    setStatusText(str);
    UpdateFrame();
}

void MainFrame::OnNodeSelected(CustomEvent& event)
{
    // This event is normally only fired if the current selection has changed. We dismiss any previous infobar message, and
    // check to see if the current selection has any kind of issu that we should warn the user about.
    m_info_bar->Dismiss();

    auto sel_node = event.GetNode();

    if (sel_node->isGen(gen_wxToolBar))
    {
        if (sel_node->GetParent()->IsSizer())
        {
            auto grandparent = sel_node->GetParent()->GetParent();
            if (grandparent->isGen(gen_wxFrame) || grandparent->isGen(gen_wxAuiMDIChildFrame))
            {
                // Caution! In wxWidgets 3.1.3 The info bar will wrap the first word if it starts with "If".
                GetPropInfoBar()->ShowMessage(_tt(strId_tb_in_sizer), wxICON_INFORMATION);
            }
        }
    }

    // If a code generation panel is open, then attempt to locate the node's name in that panel
    FindItemName(sel_node);

    UpdateFrame();
}

void MainFrame::UpdateLayoutTools()
{
    int option = -1;
    int border = 0;
    int flag = 0;
    int orient = 0;

    bool gotLayoutSettings = GetLayoutSettings(&flag, &option, &border, &orient);

    bool enableHorizontalTools = (orient != wxHORIZONTAL) && gotLayoutSettings;
    m_menuEdit->Enable(id_AlignLeft, enableHorizontalTools);
    m_toolbar->EnableTool(id_AlignLeft, enableHorizontalTools);
    m_menuEdit->Check(id_AlignLeft, ((flag & (wxALIGN_RIGHT | wxALIGN_CENTER_HORIZONTAL)) == 0) && enableHorizontalTools);
    m_toolbar->ToggleTool(id_AlignLeft,
                          ((flag & (wxALIGN_RIGHT | wxALIGN_CENTER_HORIZONTAL)) == 0) && enableHorizontalTools);

    m_menuEdit->Enable(id_AlignCenterHorizontal, enableHorizontalTools);
    m_toolbar->EnableTool(id_AlignCenterHorizontal, enableHorizontalTools);
    m_menuEdit->Check(id_AlignCenterHorizontal, (flag & wxALIGN_CENTER_HORIZONTAL) && enableHorizontalTools);
    m_toolbar->ToggleTool(id_AlignCenterHorizontal, (flag & wxALIGN_CENTER_HORIZONTAL) && enableHorizontalTools);

    m_menuEdit->Enable(id_AlignRight, enableHorizontalTools);
    m_toolbar->EnableTool(id_AlignRight, enableHorizontalTools);
    m_menuEdit->Check(id_AlignRight, (flag & wxALIGN_RIGHT) && enableHorizontalTools);
    m_toolbar->ToggleTool(id_AlignRight, (flag & wxALIGN_RIGHT) && enableHorizontalTools);

    bool enableVerticalTools = (orient != wxVERTICAL) && gotLayoutSettings;
    m_menuEdit->Enable(id_AlignTop, enableVerticalTools);
    m_toolbar->EnableTool(id_AlignTop, enableVerticalTools);
    m_menuEdit->Check(id_AlignTop, ((flag & (wxALIGN_BOTTOM | wxALIGN_CENTER_VERTICAL)) == 0) && enableVerticalTools);
    m_toolbar->ToggleTool(id_AlignTop, ((flag & (wxALIGN_BOTTOM | wxALIGN_CENTER_VERTICAL)) == 0) && enableVerticalTools);

    m_menuEdit->Enable(id_AlignCenterVertical, enableVerticalTools);
    m_toolbar->EnableTool(id_AlignCenterVertical, enableVerticalTools);
    m_menuEdit->Check(id_AlignCenterVertical, (flag & wxALIGN_CENTER_VERTICAL) && enableVerticalTools);
    m_toolbar->ToggleTool(id_AlignCenterVertical, (flag & wxALIGN_CENTER_VERTICAL) && enableVerticalTools);

    m_menuEdit->Enable(id_AlignBottom, enableVerticalTools);
    m_toolbar->EnableTool(id_AlignBottom, enableVerticalTools);
    m_menuEdit->Check(id_AlignBottom, (flag & wxALIGN_BOTTOM) && enableVerticalTools);
    m_toolbar->ToggleTool(id_AlignBottom, (flag & wxALIGN_BOTTOM) && enableVerticalTools);

    m_menuEdit->Enable(id_BorderLeft, gotLayoutSettings);
    m_menuEdit->Enable(id_BorderRight, gotLayoutSettings);
    m_menuEdit->Enable(id_BorderTop, gotLayoutSettings);
    m_menuEdit->Enable(id_BorderBottom, gotLayoutSettings);

    m_toolbar->EnableTool(id_BorderLeft, gotLayoutSettings);
    m_toolbar->EnableTool(id_BorderRight, gotLayoutSettings);
    m_toolbar->EnableTool(id_BorderTop, gotLayoutSettings);
    m_toolbar->EnableTool(id_BorderBottom, gotLayoutSettings);

    m_menuEdit->Check(id_BorderTop, ((flag & wxTOP) != 0) && gotLayoutSettings);
    m_menuEdit->Check(id_BorderRight, ((flag & wxRIGHT) != 0) && gotLayoutSettings);
    m_menuEdit->Check(id_BorderLeft, ((flag & wxLEFT) != 0) && gotLayoutSettings);
    m_menuEdit->Check(id_BorderBottom, ((flag & wxBOTTOM) != 0) && gotLayoutSettings);

    m_toolbar->ToggleTool(id_BorderTop, ((flag & wxTOP) != 0) && gotLayoutSettings);
    m_toolbar->ToggleTool(id_BorderRight, ((flag & wxRIGHT) != 0) && gotLayoutSettings);
    m_toolbar->ToggleTool(id_BorderLeft, ((flag & wxLEFT) != 0) && gotLayoutSettings);
    m_toolbar->ToggleTool(id_BorderBottom, ((flag & wxBOTTOM) != 0) && gotLayoutSettings);

    m_menuEdit->Enable(id_Expand, gotLayoutSettings);
    m_toolbar->EnableTool(id_Expand, gotLayoutSettings);
    m_menuEdit->Check(id_Expand, ((flag & wxEXPAND) != 0) && gotLayoutSettings);
    m_toolbar->ToggleTool(id_Expand, ((flag & wxEXPAND) != 0) && gotLayoutSettings);
}

void MainFrame::UpdateFrame()
{
    ttString filename = wxGetApp().GetProjectFileName().filename();

    if (filename.empty())
    {
        filename = wxT("untitled");
    }
    filename.remove_extension();

    if (m_isProject_modified)
    {
        filename.insert(0, wxT("*"));
        m_isProject_generated = false;
    }
    SetTitle(filename);

    m_menuTools->Enable(id_GenerateCode, !m_isProject_generated);
    m_toolbar->EnableTool(id_GenerateCode, !m_isProject_generated);

    wxString menu_text = _ttwx(strIdUndo);
    if (m_undo_stack.IsUndoAvailable())
    {
        if (m_undo_stack.GetUndoString().size())
        {
            menu_text << ' ' << m_undo_stack.GetUndoString();
        }
    }
    menu_text << "\tCtrl+Z";
    m_menuEdit->FindChildItem(wxID_UNDO)->SetItemLabel(menu_text);

    menu_text = _ttwx(strIdRedo);
    if (m_undo_stack.IsRedoAvailable())
    {
        if (m_undo_stack.GetUndoString().size())
        {
            menu_text << ' ' << m_undo_stack.GetUndoString();
        }
    }
    menu_text << "\tCtrl+Y";
    m_menuEdit->FindChildItem(wxID_REDO)->SetItemLabel(menu_text);

    if (m_clipboard)
    {
        menu_text = _ttwx(strIdPaste);
        menu_text << ' ' << m_clipboard->GetClassName();
        menu_text << "\tCtrl+V";
        m_menuEdit->FindChildItem(wxID_PASTE)->SetItemLabel(menu_text);
    }

    bool isMockup = (m_notebook->GetPageText(m_notebook->GetSelection()) == _ttwx(strIdMockupTabTitle));
    m_menuEdit->Enable(wxID_FIND, !isMockup);
    m_menuEdit->Enable(id_insert_widget, m_selected_node && m_selected_node->GetClassName() != "Project");

    UpdateMoveMenu();
    UpdateLayoutTools();
}

void MainFrame::OnCopy(wxCommandEvent&)
{
    if (auto win = wxWindow::FindFocus(); win && win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        wxStaticCast(win, wxStyledTextCtrl)->Copy();
        return;
    }
    else if (m_selected_node)
    {
        m_clipboard = g_NodeCreator.MakeCopy(m_selected_node);
        UpdateFrame();
    }
}

void MainFrame::OnCut(wxCommandEvent&)
{
    if (auto win = wxWindow::FindFocus(); win && win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        // This is a read-only control, so we don't allow Cut
        return;
    }
    else
    {
        RemoveNode(m_selected_node.get(), true);
        UpdateFrame();
    }
}

void MainFrame::OnDelete(wxCommandEvent&)
{
    ASSERT(m_selected_node);
    RemoveNode(m_selected_node.get(), false);
    UpdateFrame();
}

void MainFrame::OnPaste(wxCommandEvent&)
{
    if (auto win = wxWindow::FindFocus(); win && win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        // This is a read-only control, so we don't allow Paste
        return;  // we don't allow pasting into the code generation windows which are marked as read-only
    }
    else if (m_selected_node)
    {
        PasteNode(m_selected_node.get());
        UpdateFrame();
    }
}

void MainFrame::OnChangeAlignment(wxCommandEvent& event)
{
    int align = 0;
    bool vertical =
        (event.GetId() == id_AlignTop || event.GetId() == id_AlignBottom || event.GetId() == id_AlignCenterVertical);

    switch (event.GetId())
    {
        case id_AlignRight:
            align = wxALIGN_RIGHT;

            break;

        case id_AlignCenterHorizontal:
            align = wxALIGN_CENTER_HORIZONTAL;

            break;

        case id_AlignBottom:
            align = wxALIGN_BOTTOM;

            break;

        case id_AlignCenterVertical:
            align = wxALIGN_CENTER_VERTICAL;

            break;
    }

    ChangeAlignment(m_selected_node.get(), align, vertical);

    UpdateLayoutTools();
}

void MainFrame::OnChangeBorder(wxCommandEvent& event)
{
    int border;

    switch (event.GetId())
    {
        case id_BorderLeft:
            border = wxLEFT;
            break;

        case id_BorderRight:
            border = wxRIGHT;
            break;

        case id_BorderTop:
            border = wxTOP;
            break;

        case id_BorderBottom:
            border = wxBOTTOM;
            break;

        default:
            border = 0;
            break;
    }

    ToggleBorderFlag(m_selected_node.get(), border);

    UpdateLayoutTools();
}

bool MainFrame::SaveWarning()
{
    int result = wxYES;

    if (m_isProject_modified)
    {
        result = ::wxMessageBox("Current project file has been modified...\n"
                                "Do you want to save the changes?",
                                "Save project", wxYES | wxNO | wxCANCEL, this);

        if (result == wxYES)
        {
            wxCommandEvent dummy;
            OnSaveProject(dummy);
        }
    }

    return (result != wxCANCEL);
}

void MainFrame::OnAuiNotebookPageChanged(wxAuiNotebookEvent&)
{
    UpdateFrame();
    if (auto page = m_notebook->GetCurrentPage(); page)
    {
        if (page == m_mockupPanel)
        {
            m_mockupPanel->CreateContent();
        }
        else if (page == m_generatedPanel)
        {
            m_generatedPanel->GenerateBaseClass();
        }
        else if (page == m_derivedPanel)
        {
            m_derivedPanel->GenerateBaseClass();
        }
    }
}

void MainFrame::OnFindDialog(wxCommandEvent&)
{
    if (!m_findDialog)
    {
        m_findDialog = new wxFindReplaceDialog(this, &m_findData, "Find");
        m_findDialog->Centre(wxCENTRE_ON_SCREEN | wxBOTH);
    }
    m_findDialog->Show(true);
}

void MainFrame::OnFindClose(wxFindDialogEvent&)
{
    m_findDialog->Destroy();
    m_findDialog = nullptr;
}

void MainFrame::OnFind(wxFindDialogEvent& event)
{
    if (auto page = m_notebook->GetCurrentPage(); page)
    {
        event.StopPropagation();
        event.SetClientData(m_findDialog);
        page->GetEventHandler()->ProcessEvent(event);
    }
}

wxWindow* MainFrame::CreateNoteBook(wxWindow* parent)
{
    m_notebook = new wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);
    m_notebook->SetArtProvider(new wxAuiSimpleTabArt());

    m_mockupPanel = new MockupParent(m_notebook, this);

    m_notebook->AddPage(m_mockupPanel, _ttwx(strIdMockupTabTitle), false, 0);

    m_generatedPanel = new BasePanel(m_notebook, this, false);
    m_notebook->AddPage(m_generatedPanel, "Generated", false, 1);

    m_derivedPanel = new BasePanel(m_notebook, this, true);
    m_notebook->AddPage(m_derivedPanel, "Derived", false, 1);

    return m_notebook;
}

void MainFrame::CreateSplitters()
{
    // The main splitter contains the navigation tree control and it's toolbar on the left. On the right is a panel
    // containing the Ribbon toolbar at the top, and a splitter window containing the property grid and notebook with
    // mockup and code windows below it.

    m_MainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

    m_nav_panel = new NavigationPanel(m_MainSplitter, this);
    auto panel_right = new wxPanel(m_MainSplitter);
    panel_right->SetWindowStyle(wxBORDER_RAISED);

    auto parent_sizer = new wxBoxSizer(wxVERTICAL);
    auto ribbon_panel = new RibbonPanel(panel_right);
    parent_sizer->Add(ribbon_panel, wxSizerFlags(0).Expand());

    m_SecondarySplitter = new wxSplitterWindow(panel_right, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
    parent_sizer->Add(m_SecondarySplitter, wxSizerFlags(1).Expand());

    m_info_bar = new wxInfoBar(panel_right);
    parent_sizer->Add(m_info_bar, wxSizerFlags().Expand());

    panel_right->SetSizer(parent_sizer);

    m_property_panel = new PropGridPanel(m_SecondarySplitter, this);
    auto notebook = CreateNoteBook(m_SecondarySplitter);

    m_SecondarySplitter->SplitVertically(m_property_panel, notebook, m_SecondarySashPosition);

    m_MainSplitter->SplitVertically(m_nav_panel, panel_right);
    m_MainSplitter->SetName("Navigation");
    wxPersistenceManager::Get().RegisterAndRestore(m_MainSplitter);

    // Need to update the left splitter so the right one is drawn correctly
    wxSizeEvent update(GetSize(), GetId());
    ProcessEvent(update);
    m_MainSplitter->UpdateSize();
    m_MainSplitter->SetMinimumPaneSize(2);

    // Set to zero because we don't need this to change relative size when the main window is resized. Fixes issue #90
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

void MainFrame::SetStatusField(const ttlib::cstr text, int position)
{
    if (position == -1)
        position = m_posPropGridStatusField;
    setStatusText(text, position);
}

void MainFrame::UpdateStatusWidths()
{
    if (m_MainSplitter)
    {
        m_MainSashPosition = m_MainSplitter->GetSashPosition();
    }
    if (m_SecondarySplitter)
    {
        m_SecondarySashPosition = m_SecondarySplitter->GetSashPosition();
    }

    int widths[StatusPanels] = { 1, (m_MainSashPosition + m_SecondarySashPosition - 16), -1 };
    SetStatusWidths(sizeof(widths) / sizeof(int), widths);
}

void MainFrame::UpdateMoveMenu()
{
    auto node = m_selected_node.get();
    Node* parent = nullptr;
    if (node)
        parent = node->GetParent();
    if (!node || !parent)
    {
        m_menuEdit->Enable(id_MoveUp, false);
        m_menuEdit->Enable(id_MoveDown, false);
        m_menuEdit->Enable(id_MoveLeft, false);
        m_menuEdit->Enable(id_MoveRight, false);
        return;
    }

    m_menuEdit->Enable(id_MoveUp, MoveNode(node, MoveDirection::Up, true));
    m_menuEdit->Enable(id_MoveDown, MoveNode(node, MoveDirection::Down, true));
    m_menuEdit->Enable(id_MoveLeft, MoveNode(node, MoveDirection::Left, true));
    m_menuEdit->Enable(id_MoveRight, MoveNode(node, MoveDirection::Right, true));
}

void MainFrame::FindItemName(Node* node)
{
    if (auto value = node->get_value_ptr("var_name"); value && value->size())
    {
        m_generatedPanel->FindItemName(*value);
        return;
    }

    if (node->GetClassName().is_sameprefix("ribbon"))
    {
        if (auto value = node->get_value_ptr("id"); value && value->size())
        {
            m_generatedPanel->FindItemName(*value);
        }
    }
}

Node* MainFrame::GetSelectedForm()
{
    if (!m_selected_node)
        return nullptr;

    if (m_selected_node->isType(type_form) || m_selected_node->isType(type_wizard) ||
        m_selected_node->isType(type_menubar_form) || m_selected_node->isType(type_toolbar_form))
    {
        return m_selected_node.get();
    }
    else
    {
        return m_selected_node->FindParentForm();
    }
}

bool MainFrame::SelectNode(Node* node, bool force, bool notify)
{
    if (!node)
    {
        m_selected_node = NodeSharedPtr();
        return false;
    }

    if (node == m_selected_node.get() && !force)
    {
        return false;  // already selected
    }

    m_selected_node = node->GetSharedPtr();
    if (notify)
    {
        FireSelectedEvent(node);
    }
    return true;
}

void MainFrame::CreateToolNode(const ttlib::cstr& name)
{
    if (!m_selected_node)
    {
        appMsgBox("You need to select something first in order to properly place this widget.");
        return;
    }

    if (!m_selected_node->CreateToolNode(name))
    {
        appMsgBox(ttlib::cstr() << "Unable to create " << name << " as a child of " << m_selected_node->GetClassName());
    }
}

void MainFrame::CopyNode(Node* node)
{
    ASSERT(node);
    m_clipboard = g_NodeCreator.MakeCopy(node);
}

void MainFrame::PasteNode(Node* parent)
{
    ASSERT(m_clipboard);

    if (!parent)
    {
        ASSERT(m_selected_node)
        if (!m_selected_node)
            return;
        parent = m_selected_node.get();
    }

    ttlib::cstr undo_str = "paste " + m_clipboard->GetClassName();
    auto new_node = g_NodeCreator.MakeCopy(m_clipboard);

    if (!parent->IsChildAllowed(new_node))
    {
        auto grandparent = parent->GetParent();
        if (!grandparent || !grandparent->IsChildAllowed(new_node))
        {
            appMsgBox(ttlib::cstr() << _tt("You cannot paste ") << new_node->GetClassName() << _tt(" into ")
                                    << parent->GetClassName());
            return;
        }
        parent = grandparent;
    }

    auto pos = parent->FindInsertionPos(m_selected_node);
    PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
    new_node->FixPastedNames();

    FireCreatedEvent(new_node);
    SelectNode(new_node, true, true);
}

void MainFrame::DuplicateNode(Node* node)
{
    ASSERT(node);

    auto parent = node->GetParent();
    ASSERT(parent);

    ttlib::cstr undo_str = "duplicate " + node->GetClassName();
    auto pos = parent->FindInsertionPos(m_selected_node);
    auto new_node = g_NodeCreator.MakeCopy(node);
    PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
    new_node->FixPastedNames();

    FireCreatedEvent(new_node);
    SelectNode(new_node, true, true);
}

bool MainFrame::CanCopyNode()
{
    return (m_selected_node.get() && m_selected_node->GetClassName() != "Project");
}

bool MainFrame::CanPasteNode()
{
    return (m_selected_node.get() && m_clipboard.get());
}

void MainFrame::Undo()
{
    AutoFreeze freeze(this);

    m_undo_stack.Undo();
    m_isProject_modified = (m_undo_stack_size != m_undo_stack.size());
    FireProjectUpdatedEvent();
    ASSERT(m_selected_node)
    FireSelectedEvent(m_selected_node.get());
}

void MainFrame::Redo()
{
    AutoFreeze freeze(this);

    m_undo_stack.Redo();
    m_isProject_modified = (m_undo_stack_size != m_undo_stack.size());
    FireProjectUpdatedEvent();
    FireSelectedEvent(GetSelectedNode());
}

void MainFrame::OnToggleExpandLayout(wxCommandEvent&)
{
    if (!m_selected_node || !m_selected_node->GetParent() || !m_selected_node->GetParent()->IsSizer())
    {
        return;
    }

    auto propFlag = m_selected_node->get_prop_ptr(prop_flags);

    if (!propFlag)
    {
        return;
    }

    auto currentValue = propFlag->as_string();
    auto value = (isPropFlagSet("wxEXPAND", currentValue) ? ClearPropFlag("wxEXPAND", currentValue) :
                                                            SetPropFlag("wxEXPAND", currentValue));

    ModifyProperty(propFlag, value);
}

void MainFrame::ToggleBorderFlag(Node* node, int border)
{
    if (!node)
        return;

    auto propFlag = node->get_prop_ptr(prop_borders);

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

    ModifyProperty(propFlag, value);
}

void MainFrame::ModifyProperty(NodeProperty* prop, ttlib::cview value)
{
    if (prop && value != prop->as_cview())
    {
        PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
        FirePropChangeEvent(prop);
    }
}

void MainFrame::ChangeAlignment(Node* node, int align, bool vertical)
{
    if (!node)
        return;

    auto propFlag = node->get_prop_ptr(prop_alignment);

    if (!propFlag)
        return;

    ttlib::cstr value;

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

    ModifyProperty(propFlag, SetPropFlag(alignStr, value));
}

bool MainFrame::GetLayoutSettings(int* flag, int* option, int* border, int* orient)
{
    if (!m_selected_node || !m_selected_node->GetParent() || !m_selected_node->GetParent()->IsSizer())
    {
        return false;
    }

    auto prop_flags = m_selected_node->GetSizerFlags();

    auto propOption = m_selected_node->get_prop_ptr(prop_proportion);
    if (propOption)
    {
        *option = prop_flags.GetProportion();
    }

    *flag = prop_flags.GetFlags();
    *border = prop_flags.GetBorderInPixels();

    auto sizer = m_selected_node->GetParent();
    if (sizer)
    {
        auto parentName = sizer->GetClassName();
        if (parentName == "wxBoxSizer" || m_selected_node->IsStaticBoxSizer())
        {
            auto propOrient = sizer->get_prop_ptr(prop_orientation);
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
    auto parent = node->GetParent();
    ASSERT(parent || node->GetClassName() == "Project");
    if (!parent)
        return false;

    if (where == MoveDirection::Left)
    {
        auto grandparent = parent->GetParent();
        while (grandparent && !grandparent->IsSizer())
        {
            grandparent = grandparent->GetParent();
        }

        if (check_only)
            return (grandparent ? true : false);

        if (grandparent)
        {
            PushUndoAction(std::make_shared<ChangeParentAction>(node, grandparent));
            FireProjectUpdatedEvent();
            SelectNode(node, true);
            return true;
        }
        appMsgBox(_tt("There is no sizer to the left of this item that it can be moved into."), _tt(strIdMoveTitle));
    }
    else if (where == MoveDirection::Right)
    {
        auto pos = parent->GetChildPosition(node) - 1;
        if (pos < parent->GetChildCount())
        {
            parent = FindChildSizerItem(parent->GetChild(pos));

            if (check_only)
                return (parent ? true : false);

            if (parent)
            {
                PushUndoAction(std::make_shared<ChangeParentAction>(node, parent));
                FireProjectUpdatedEvent();
                SelectNode(node, true);
                return true;
            }
        }
        if (!check_only)
            appMsgBox(_tt("There is nothing above this item that it can be moved into."), _tt(strIdMoveTitle));
    }
    else if (where == MoveDirection::Up)
    {
        auto pos = parent->GetChildPosition(node);
        if (check_only)
            return (pos > 0);
        if (pos > 0)
        {
            PushUndoAction(std::make_shared<ChangePositionAction>(node, pos - 1));
            AutoFreeze freeze(this);
            FireProjectUpdatedEvent();
            SelectNode(node, true);
            return true;
        }
        appMsgBox(_tt(strIdCantMoveUp), _tt(strIdMoveTitle));
    }
    else if (where == MoveDirection::Down)
    {
        auto pos = parent->GetChildPosition(node) + 1;
        if (check_only)
            return (pos < parent->GetChildCount());
        if (pos < parent->GetChildCount())
        {
            PushUndoAction(std::make_shared<ChangePositionAction>(node, pos));
            AutoFreeze freeze(this);
            FireProjectUpdatedEvent();
            SelectNode(node, true);
            return true;
        }
        appMsgBox(node->GetClassName() + _tt(" cannot be moved down any lower."), _tt(strIdMoveTitle));
    }

    return false;
}

void MainFrame::RemoveNode(Node* node, bool isCutMode)
{
    ASSERT(node->isType(type_project));
    ASSERT(node->GetParentPtr());

    auto parent = node->GetParent();
    if (!parent)
        return;

    // We need to make a copy in order to pass to FireDeletedEvent
    auto deleted_node = node;
    Node* node_copy = node;

    if (isCutMode)
    {
        ttlib::cstr undo_str = "cut " + node->GetClassName();
        PushUndoAction(std::make_shared<RemoveNodeAction>(node_copy, undo_str, true));
    }
    else
    {
        ttlib::cstr undo_str = "delete " + node->GetClassName();
        PushUndoAction(std::make_shared<RemoveNodeAction>(node_copy, undo_str));
    }

    FireDeletedEvent(deleted_node);
    ASSERT(GetSelectedNodePtr());  // RemoveNodeAction should have selected something
    SelectNode(GetSelectedNode(), true, true);
}

Node* MainFrame::FindChildSizerItem(Node* node)
{
    if (node->GetNodeDeclaration()->isSubclassOf(gen_sizer_dimension))
        return node;
    else
    {
        for (size_t i = 0; i < node->GetChildCount(); ++i)
        {
            auto result = FindChildSizerItem(node->GetChild(i));
            if (result)
                return result;
        }
    }

    return nullptr;
}
