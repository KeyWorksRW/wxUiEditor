/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame event handlers
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aboutdlg.h>     // declaration of wxAboutDialog class
#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/clipbrd.h>      // wxClipboad class and clipboard functions
#include <wx/config.h>       // wxConfig base header
#include <wx/filedlg.h>      // wxFileDialog base header
#include <wx/infobar.h>      // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/stc/stc.h>      // Scintilla

#include "mainframe.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "version.h"          // Version numbers and other constants

#include "preferences_dlg.h"  // PreferencesDlg -- Dialog for setting user preferences
#include "startup_dlg.h"      // Dialog to display if wxUE is launched with no arguments

#include "panels/base_panel.h"      // BasePanel -- C++ panel
#include "panels/doc_view.h"        // Panel for displaying docs in wxWebView
#include "panels/nav_panel.h"       // NavigationPanel -- Node tree class
#include "panels/propgrid_panel.h"  // PropGridPanel -- Node inspector class

#include "mockup/mockup_parent.h"  // MockupParent -- Top-level MockUp Parent window

#include "internal/import_panel.h"  // ImportPanel -- Panel to display original imported file
#include "internal/msg_logging.h"   // MsgLogging -- Message logging class

#include "pugixml.hpp"

// The following is taken from wxui/ui_images.h
namespace wxue_img
{
    wxBitmapBundle bundle_debug_logo_svg(int width, int height);
    wxBitmapBundle bundle_wxUiEditor_svg(int width, int height);
}  // namespace wxue_img

void MainFrame::OnAbout(wxCommandEvent&)
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName(txtVersion);

    // Use trailing spaces to make the dialog width a bit wider
    tt_string description;
    description
        << "wxUiEditor is a designer for wxWidgets\ngenerating C++, Python, Ruby and XRC code.\n";

    if (wxGetApp().isTestingMenuEnabled())
    {
        description << "\n" << Project.getProjectFile() << "  \n";
        description << "Original Project version: " << Project.getOriginalProjectVersion() << "\n";
        description << "wxUiEditor Project version: " << curSupportedVer << "\n";
    }

    aboutInfo.SetDescription(description);
    aboutInfo.SetWebSite("https://github.com/KeyWorksRW/wxUiEditor");

    aboutInfo.SetCopyright(txtCopyRight);

#if 0
    // Save this for when/if other people contribute to the project
    aboutInfo.AddDeveloper("Randalphwa");
#endif

#if defined(_DEBUG)
    aboutInfo.SetIcon(wxue_img::bundle_debug_logo_svg(64, 64).GetIconFor(this));
#else
    aboutInfo.SetIcon(wxue_img::bundle_wxUiEditor_svg(64, 64).GetIconFor(this));
#endif  // _DEBUG

    wxAboutBox(aboutInfo);
}

void MainFrame::OnAppendCrafter(wxCommandEvent&)
{
    tt_cwd cwd(true);
    wxFileDialog dlg(this, "Open or Import Project", cwd.make_wxString(), wxEmptyString,
                     "wxCrafter Project File (*.wxcp)|*.wxcp||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.appendCrafter(files);
    }
}

void MainFrame::OnAppendDialogBlocks(wxCommandEvent&)
{
    tt_cwd cwd(true);
    wxFileDialog dlg(this, "Open or Import Project", cwd, wxEmptyString,
                     "DialogBlocks Project File (*.pjd)|*.pjd||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.appendDialogBlocks(files);
    }
}

void MainFrame::OnAppendFormBuilder(wxCommandEvent&)
{
    tt_cwd cwd(true);
    wxFileDialog dlg(this, "Open or Import Project", cwd.make_wxString(), wxEmptyString,
                     "wxFormBuilder Project File (*.fbp)|*.fbp||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.appendFormBuilder(files);
    }
}

void MainFrame::OnAppendGlade(wxCommandEvent&)
{
    tt_cwd cwd(true);
    wxFileDialog dlg(this, "Open or Import Project", cwd.make_wxString(), wxEmptyString,
                     "wxGlade Project File (*.wxg)|*.wxg||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.appendGlade(files);
    }
}

void MainFrame::OnAppendSmith(wxCommandEvent&)
{
    tt_cwd cwd(true);
    wxFileDialog dlg(this, "Open or Import Project", cwd.make_wxString(), wxEmptyString,
                     "wxSmith File (*.wxs)|*.wxs||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.appendSmith(files);
    }
}

void MainFrame::OnAppendXRC(wxCommandEvent&)
{
    tt_cwd cwd(true);
    wxFileDialog dlg(this, "Open or Import Project", cwd.make_wxString(), wxEmptyString,
                     "XRC File (*.xrc)|*.xrc||", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.appendXRC(files);
    }
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
#if wxUSE_WEBVIEW
        else if (page == m_docviewPanel)
        {
            m_docviewPanel->ActivatePage();
        }
#endif
        else if (page != m_imnportPanel)
        {
            static_cast<BasePanel*>(page)->GenerateBaseClass();
        }
    }
}

void MainFrame::OnBrowseDocs(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_node)
    {
        if (auto generator = m_selected_node->getGenerator(); generator)
        {
            auto file = generator->GetHelpURL(m_selected_node.get());
            if (file.size())
            {
                // wxString url("https://docs.wxwidgets.org/trunk/class");
                wxString url("https://docs.wxwidgets.org/3.2.0/class");
                if (file.starts_with("group"))
                    url.RemoveLast(sizeof("class") - 1);
                url << file.make_wxString();
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    // wxLaunchDefaultBrowser("https://docs.wxwidgets.org/trunk/");
    wxLaunchDefaultBrowser("https://docs.wxwidgets.org/3.2.0/");
}

void MainFrame::OnBrowsePython(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_node)
    {
        if (auto generator = m_selected_node->getGenerator(); generator)
        {
            auto file = generator->GetPythonURL(m_selected_node.get());
            if (file.size())
            {
                wxString url("https://docs.wxpython.org/");
                url << file.make_wxString();
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    wxLaunchDefaultBrowser("https://docs.wxpython.org/index.html");
}

void MainFrame::OnBrowseRuby(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_node)
    {
        if (auto generator = m_selected_node->getGenerator(); generator)
        {
            auto file = generator->GetRubyURL(m_selected_node.get());
            if (file.size())
            {
                wxString url("https://mcorino.github.io/wxRuby3/");
                url << file.make_wxString();
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    wxLaunchDefaultBrowser("https://mcorino.github.io/wxRuby3/");
}

void MainFrame::OnChangeAlignment(wxCommandEvent& event)
{
    int align = 0;
    bool vertical = (event.GetId() == id_AlignTop || event.GetId() == id_AlignBottom ||
                     event.GetId() == id_AlignCenterVertical);

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

void MainFrame::OnClose(wxCloseEvent& event)
{
    if (!SaveWarning())
        return;

    wxGetApp().setMainFrameClosing();

    auto config = wxConfig::Get();
#if defined(_DEBUG)
    config->SetPath("/debug_mainframe");
#else
    config->SetPath("/mainframe");
#endif

    bool isIconized = IsIconized();
    bool isMaximized = IsMaximized();

    if (!isMaximized)
    {
        config->Write("PosX", isIconized ? -1 : GetPosition().x);
        config->Write("PosY", isIconized ? -1 : GetPosition().y);
        config->Write("SizeW", isIconized ? -1 : GetSize().GetWidth());
        config->Write("SizeH", isIconized ? -1 : GetSize().GetHeight());
    }

    config->Write("IsMaximized", isMaximized);
    config->Write("IsIconized", isIconized);

    config->SetPath(txt_main_window_config);
    m_FileHistory.Save(*config);
    m_property_panel->SaveDescBoxHeight();

    // BUGBUG: [KeyWorks - 01-24-2022] m_has_clipboard_data is never set to true

    // If we have clipboard data, ensure it persists after we exit
    if (m_has_clipboard_data)
        wxTheClipboard->Flush();

    if (wxGetApp().isTestingMenuEnabled())
    {
        config->SetPath("/debug_history");
        m_ImportHistory.Save(*config);
        config->SetPath("/");

        if (g_pMsgLogging)
            g_pMsgLogging->CloseLogger();
    }
    event.Skip();
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
        CopyNode(m_selected_node.get());
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

void MainFrame::OnDifferentProject(wxCommandEvent& WXUNUSED(event))
{
    if (!SaveWarning())
        return;

    StartupDlg start_dlg(m_nav_panel);
    if (auto result = start_dlg.ShowModal(); result == wxID_OK)
    {
        switch (start_dlg.GetCommandType())
        {
            case StartupDlg::START_MRU:
                if (!start_dlg.GetProjectFile().extension().is_sameas(".wxui", tt::CASE::either) &&
                    !start_dlg.GetProjectFile().extension().is_sameas(".wxue", tt::CASE::either))
                {
                    Project.ImportProject(start_dlg.GetProjectFile());
                }
                else
                {
                    Project.LoadProject(start_dlg.GetProjectFile());
                }
                break;

            case StartupDlg::START_EMPTY:
                Project.NewProject(true);
                break;

            case StartupDlg::START_CONVERT:
                Project.NewProject(false);
                break;

            case StartupDlg::START_OPEN:
                {
                    // TODO: [KeyWorks - 02-21-2021] A CodeBlocks file will contain all of the
                    // wxSmith resources -- so it would actually make sense to process it since we
                    // can combine all of those resources into our single project file.

                    wxFileDialog dialog(nullptr, "Open or Import Project", wxEmptyString,
                                        wxEmptyString,
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
                            Project.ImportProject(filename);
                        }
                        else
                        {
                            Project.LoadProject(dialog.GetPath());
                        }
                    }
                }
                break;
        }
    }
}

void MainFrame::OnDuplicate(wxCommandEvent& WXUNUSED(event))
{
    ASSERT(m_selected_node);
    DuplicateNode(m_selected_node.get());
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

void MainFrame::OnFindClose(wxFindDialogEvent&)
{
    m_findDialog->Destroy();
    m_findDialog = nullptr;
}

void MainFrame::OnFindDialog(wxCommandEvent&)
{
    if (!m_findDialog)
    {
        if (auto page = m_notebook->GetCurrentPage(); page)
        {
            if (wxGetApp().isTestingMenuEnabled() && page == m_imnportPanel)
                m_findData.SetFindString(m_imnportPanel->GetTextCtrl()->GetSelectedText());
            else if (page != m_mockupPanel && page != m_docviewPanel)
            {
                m_findData.SetFindString(static_cast<BasePanel*>(page)->GetSelectedText());
            }
        }
        m_findDialog = new wxFindReplaceDialog(this, &m_findData, "Find");
        m_findDialog->Centre(wxCENTRE_ON_SCREEN | wxBOTH);
    }
    m_findDialog->Show(true);
}

void MainFrame::OnImportProject(wxCommandEvent&)
{
    if (!SaveWarning())
        return;

    if (g_pMsgLogging)
        g_pMsgLogging->Clear();

    Project.NewProject();
}

void MainFrame::OnImportRecent(wxCommandEvent& event)
{
    tt_string file =
        m_ImportHistory.GetHistoryFile(event.GetId() - (START_IMPORT_FILE_IDS)).utf8_string();

    if (!SaveWarning())
        return;

    if (file.file_exists())
    {
        g_pMsgLogging->Clear();
        Project.ImportProject(file);
    }
    else if (wxMessageBox(wxString().Format("The project file '%s' doesn't exist.\n\nWould you "
                                            "like to remove it from the recent files list?",
                                            file.c_str()),
                          "Open recent project", wxICON_WARNING | wxYES_NO) == wxYES)
    {
        m_ImportHistory.RemoveFileFromHistory(event.GetId() - wxID_FILE1);
    }
}

void MainFrame::OnNewProject(wxCommandEvent&)
{
    if (!SaveWarning())
        return;

    // true means create an empty project
    Project.NewProject(true);
}

void MainFrame::OnNodeSelected(CustomEvent& event)
{
    // This event is normally only fired if the current selection has changed. We dismiss any
    // previous infobar message, and check to see if the current selection has any kind of issue
    // that we should warn the user about.
    m_info_bar->Dismiss();

    auto evt_flags = event.getNode();

    if (evt_flags->isGen(gen_wxToolBar))
    {
        if (evt_flags->getParent()->isSizer())
        {
            auto grandparent = evt_flags->getParent()->getParent();
            if (grandparent->isType(type_frame_form) || grandparent->isGen(gen_wxAuiMDIChildFrame))
            {
                GetPropInfoBar()->ShowMessage("For the toolbar to be owned by the frame window, it "
                                              "should be placed directly under the frame, "
                                              "not under a sizer. Use Cut and Paste to move it.",
                                              wxICON_INFORMATION);
            }
        }
    }

    if (wxGetApp().isTestingMenuEnabled())
    {
        g_pMsgLogging->OnNodeSelected();
        m_imnportPanel->OnNodeSelected(evt_flags);
    }

    UpdateFrame();
}

void MainFrame::OnOpenProject(wxCommandEvent&)
{
    if (!SaveWarning())
        return;

    // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui
    // extension
    wxFileDialog dialog(this, "Open or Import Project", wxEmptyString, wxEmptyString,
                        "wxUiEditor Project File (*.wxui)|*.wxui;*.wxue"
                        "|Windows Resource File (*.rc)|*.rc"
                        "|wxCrafter Project File (*.wxcp)|*.wxcp"
                        "|DialogBlocks Project File (*.fjd)|*.fjd"
                        "|wxFormBuilder Project File (*.fbp)|*.fbp"
                        "|wxGlade File (*.wxg)|*.wxg"
                        "|wxSmith File (*.wxs)|*.wxs"
                        "|XRC File (*.xrc)|*.xrc||",
                        wxFD_OPEN);

    if (dialog.ShowModal() == wxID_OK)
    {
        tt_string filename = dialog.GetPath().utf8_string();
        // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui
        // extension
        if (filename.extension().is_sameas(".wxui", tt::CASE::either) ||
            filename.extension().is_sameas(".wxue", tt::CASE::either))
        {
            Project.LoadProject(filename);
        }
        else
        {
            Project.ImportProject(filename);
        }
    };
}

void MainFrame::OnOpenRecentProject(wxCommandEvent& event)
{
    if (!SaveWarning())
        return;
    tt_string file = m_FileHistory.GetHistoryFile(event.GetId() - wxID_FILE1).utf8_string();

    if (file.file_exists())
    {
        Project.LoadProject(file);
    }
    else if (wxMessageBox(wxString().Format("The project file '%s' doesn't exist.\n\nWould you "
                                            "like to remove it from the recent files list?",
                                            file.c_str()),
                          "Open recent project", wxICON_WARNING | wxYES_NO) == wxYES)
    {
        m_FileHistory.RemoveFileFromHistory(event.GetId() - wxID_FILE1);
    }
}

void MainFrame::OnPaste(wxCommandEvent&)
{
    if (auto win = wxWindow::FindFocus(); win && win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        // This is a read-only control, so we don't allow Paste
        return;  // we don't allow pasting into the code generation windows which are marked as
                 // read-only
    }
    else if (m_selected_node)
    {
        PasteNode(m_selected_node.get());
        UpdateFrame();
    }
}

void MainFrame::OnPreferencesDlg(wxCommandEvent& WXUNUSED(event))
{
    PreferencesDlg dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnProjectLoaded()
{
    UpdateFrame();
}

void MainFrame::OnReloadProject(wxCommandEvent& WXUNUSED(event))
{
    if (wxMessageBox(
            wxString() << "This will lose any changes you have made since the last save.\n\n"
                          "Are you sure you want to reload the project?",
            "Reload Project", wxICON_WARNING | wxYES_NO) == wxYES)
    {
        Project.LoadProject(Project.getProjectFile());
    }
}

void MainFrame::OnSaveAsProject(wxCommandEvent&)
{
    wxFileName filename(*Project.get_wxFileName());
    if (!filename.IsOk())
    {
        filename.Assign("MyProject");
    }

    // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui
    // extension
    wxFileDialog dialog(this, "Save Project As", wxFileName::GetCwd(), filename.GetFullName(),
                        "wxUiEditor Project File (*.wxui)|*.wxui;*.wxue",
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        filename = dialog.GetPath();
        // Note that under Windows, any extension the user added will be followed with a .wxui
        // extension
        auto ext = filename.GetExt();
        if (ext.empty())
        {
            filename.SetExt("wxui");
        }

        // Don't allow the user to walk over existing project file types that are probably
        // associated with another designer tool

        else if (ext.CmpNoCase("fbp") == 0)
        {
            wxMessageBox("You cannot save the project as a wxFormBuilder project file",
                         "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("fjd") == 0)
        {
            wxMessageBox("You cannot save the project as a DialogBlocks project file",
                         "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("wxg") == 0)
        {
            wxMessageBox("You cannot save the project as a wxGlade file", "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("wxs") == 0)
        {
            wxMessageBox("You cannot save the project as a wxSmith file", "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("xrc") == 0)
        {
            wxMessageBox("You cannot save the project as a XRC file", "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("rc") == 0 || ext.CmpNoCase("dlg") == 0)
        {
            wxMessageBox("You cannot save the project as a Windows Resource file",
                         "Save Project As");
            return;
        }

        pugi::xml_document doc;
        Project.getProjectNode()->createDoc(doc);
        if (doc.save_file(filename.GetFullPath().utf8_string(), "  ",
                          pugi::format_indent_attributes))
        {
            m_isProject_modified = false;
            m_isImported = false;
            m_FileHistory.AddFileToHistory(filename.GetFullPath());
            Project.setProjectPath(&filename);
            ProjectSaved();
            FireProjectLoadedEvent();
        }
        else
        {
            wxMessageBox(wxString("Unable to save the project: ") << filename.GetFullPath(),
                         "Save Project As");
        }
    };
}

void MainFrame::OnSaveProject(wxCommandEvent& event)
{
    if (m_isImported || Project.getProjectFile().empty() ||
        Project.getProjectFile().filename().is_sameas(txtEmptyProject))
        OnSaveAsProject(event);
    else
    {
        if (Project.getOriginalProjectVersion() != Project.getProjectVersion())
        {
            if (wxMessageBox("A project saved with this version of wxUiEditor is not compatible "
                             "with older versions of "
                             "wxUiEditor.\n"
                             "Continue with save?",
                             "Save Project", wxYES_NO) == wxNO)
            {
                return;
            }
            Project.UpdateOriginalProjectVersion();  // Don't ask again
        }
        pugi::xml_document doc;
        Project.getProjectNode()->createDoc(doc);
        if (doc.save_file(Project.getProjectFile(), "  ", pugi::format_indent_attributes))
        {
            m_isProject_modified = false;
            ProjectSaved();
        }
        else
        {
            wxMessageBox(wxString("Unable to save the project: ") << Project.getProjectFile(),
                         "Save Project");
        }
    }
}

void MainFrame::OnToggleExpandLayout(wxCommandEvent&)
{
    if (!m_selected_node || !m_selected_node->getParent() ||
        !m_selected_node->getParent()->isSizer())
    {
        return;
    }

    auto propFlag = m_selected_node->getPropPtr(prop_flags);

    if (!propFlag)
    {
        return;
    }

    auto currentValue = propFlag->as_string();
    auto wasExpanded = isPropFlagSet("wxEXPAND", currentValue);
    auto value = (wasExpanded ? ClearPropFlag("wxEXPAND", currentValue) :
                                SetPropFlag("wxEXPAND", currentValue));

    if (!wasExpanded)
    {
        auto alignment = m_selected_node->getPropPtr(prop_alignment);
        if (alignment && alignment->as_string().size())
        {
            // All alignment flags are invalid if wxEXPAND is set
            modifyProperty(alignment, "");
        }
    }

    modifyProperty(propFlag, value);
}

void MainFrame::OnUpdateBrowseDocs(wxUpdateUIEvent& event)
{
    if (m_selected_node)
    {
        if (auto generator = m_selected_node->getGenerator(); generator)
        {
            auto label = generator->GetHelpText(m_selected_node.get());
            if (label.empty())
            {
                label << "wxWidgets";
            }
            label << " Documentation";
            event.SetText(label.make_wxString());
            return;
        }
    }

    event.SetText("wxWidgets Documentation");
}

void MainFrame::OnUpdateBrowsePython(wxUpdateUIEvent& event)
{
    if (m_selected_node)
    {
        if (auto generator = m_selected_node->getGenerator(); generator)
        {
            auto label = generator->GetPythonHelpText(m_selected_node.get());
            if (label.empty())
            {
                label << "wxPython";
            }
            label << " Documentation";
            event.SetText(label.make_wxString());
            return;
        }
    }

    event.SetText("wxPython Documentation");
}

void MainFrame::OnUpdateBrowseRuby(wxUpdateUIEvent& event)
{
    if (m_selected_node)
    {
        if (auto generator = m_selected_node->getGenerator(); generator)
        {
            auto label = generator->GetRubyHelpText(m_selected_node.get());
            if (label.empty())
            {
                label << "wxRuby";
            }
            label << " Documentation";
            event.SetText(label.make_wxString());
            return;
        }
    }

    event.SetText("wxRuby Documentation");
}
