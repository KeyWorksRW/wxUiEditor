/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame event handlers
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-27-2026]

#include <format>
#include <wx/aboutdlg.h>     // declaration of wxAboutDialog class
#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/clipbrd.h>      // wxClipboard class and clipboard functions
#include <wx/config.h>       // wxConfig base header
#include <wx/filedlg.h>      // wxFileDialog base header
#include <wx/infobar.h>      // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/stc/stc.h>      // Scintilla

#include "wx/msw/toplevel.h"
#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "mainframe.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "version.h"          // Version numbers and other constants

#include "preferences_dlg.h"  // PreferencesDlg -- Dialog for setting user preferences
#include "ui/startup_dlg.h"  // StartupDlg -- Dialog to display if wxUE is launched with no arguments

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

void MainFrame::OnAbout([[maybe_unused]] wxCommandEvent& event)
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName(txtVersion);

    // Use trailing spaces to make the dialog width a bit wider
    wxue::string description;
    description
        << "wxUiEditor is a designer for wxWidgets\ngenerating C++, Python, Ruby and XRC code.\n";

    if (wxGetApp().isTestingMenuEnabled())
    {
        description << "\n" << Project.get_ProjectFile() << "  \n";
        description << "Original Project version: " << Project.get_OriginalProjectVersion() << "\n";
        description << "wxUiEditor Project version: " << curSupportedVer << "\n";
    }

    aboutInfo.SetDescription(description);
    aboutInfo.SetWebSite("https://github.com/KeyWorksRW/wxUiEditor");

    aboutInfo.SetCopyright(txtCopyRight);

    // Save this for when/if other people regularly contribute to the project
    // aboutInfo.AddDeveloper("Randalphwa");

#if defined(_DEBUG)
    aboutInfo.SetIcon(
        wxue_img::bundle_debug_logo_svg(64, 64).GetIconFor(static_cast<const wxWindow*>(this)));
#else
    aboutInfo.SetIcon(
        wxue_img::bundle_wxUiEditor_svg(64, 64).GetIconFor(static_cast<wxWindow*>(this)));
#endif  // _DEBUG

    wxAboutBox(aboutInfo);
}

void MainFrame::OnAppendCrafter(wxCommandEvent& /* event unused */)
{
    const wxue::SaveCwd saved_cwd(wxue::restore_cwd);
    wxFileDialog dialog(this, "Open or Import Project", saved_cwd.get_SavedCwd(), wxEmptyString,
                        "wxCrafter Project File (*.wxcp)|*.wxcp||",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dialog.GetPaths(files);
        Project.AppendCrafter(files);
    }
}

void MainFrame::OnAppendDialogBlocks(wxCommandEvent& /* event unused */)
{
    const wxue::SaveCwd saved_cwd(wxue::restore_cwd);
    wxFileDialog dialog(this, "Open or Import Project", saved_cwd.get_SavedCwd(), wxEmptyString,
                        "DialogBlocks Project File (*.pjd)|*.pjd||",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dialog.GetPaths(files);
        Project.AppendDialogBlocks(files);
    }
}

void MainFrame::OnAppendFormBuilder(wxCommandEvent& /* event unused */)
{
    const wxue::SaveCwd saved_cwd(wxue::restore_cwd);
    wxFileDialog dialog(this, "Open or Import Project", saved_cwd.get_SavedCwd(), wxEmptyString,
                        "wxFormBuilder Project File (*.fbp)|*.fbp||",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dialog.GetPaths(files);
        Project.AppendFormBuilder(files);
    }
}

void MainFrame::OnAppendGlade(wxCommandEvent& /* event unused */)
{
    const wxue::SaveCwd saved_cwd(wxue::restore_cwd);
    wxFileDialog dialog(this, "Open or Import Project", saved_cwd.get_SavedCwd(), wxEmptyString,
                        "wxGlade Project File (*.wxg)|*.wxg||",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dialog.GetPaths(files);
        Project.AppendGlade(files);
    }
}

void MainFrame::OnAppendSmith(wxCommandEvent& /* event unused */)
{
    const wxue::SaveCwd saved_cwd(wxue::restore_cwd);
    wxFileDialog dialog(this, "Open or Import Project", saved_cwd.get_SavedCwd(), wxEmptyString,
                        "wxSmith File (*.wxs)|*.wxs||",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dialog.GetPaths(files);
        Project.AppendSmith(files);
    }
}

void MainFrame::OnAppendXRC(wxCommandEvent& /* event unused */)
{
    const wxue::SaveCwd saved_cwd(wxue::restore_cwd);
    wxFileDialog dialog(this, "Open or Import Project", saved_cwd.get_SavedCwd(), wxEmptyString,
                        "XRC File (*.xrc)|*.xrc||",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dialog.GetPaths(files);
        Project.AppendXRC(files);
    }
}

void MainFrame::OnAuiNotebookPageChanged(wxAuiNotebookEvent& /* event unused */)
{
    UpdateFrame();
    if (auto* page = m_notebook->GetCurrentPage(); page)
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
        else if (page != m_importPanel)
        {
            dynamic_cast<BasePanel*>(page)->GenerateBaseClass();
        }
    }
}

void MainFrame::OnBrowseDocs(wxCommandEvent& /* event unused */)
{
    wxString url;
    url = (Project.get_LangVersion(GenLang::cplusplus) < CPP_WIDGETS_VERSION_3_3_0) ?
              "https://docs.wxwidgets.org/3.2.8" :
              "https://docs.wxwidgets.org/latest";

    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            const wxue::string file = generator->GetHelpURL(m_selected_node.get());
            if (!file.empty())
            {
                url += "/class";
                if (file.starts_with("group"))
                {
                    url.RemoveLast(sizeof("class") - 1);
                }
                url << file;
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    wxLaunchDefaultBrowser(url);
}

void MainFrame::OnBrowsePython(wxCommandEvent& /* event unused */)
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            const wxue::string file = generator->GetPythonURL(m_selected_node.get());
            if (!file.empty())
            {
                wxString url("https://docs.wxpython.org/");
                url << file;
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    wxLaunchDefaultBrowser("https://docs.wxpython.org/index.html");
}

void MainFrame::OnBrowseRuby(wxCommandEvent& /* event unused */)
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            const wxue::string file = generator->GetRubyURL(m_selected_node.get());
            if (!file.empty())
            {
                wxString url("https://mcorino.github.io/wxRuby3/");
                url << file;
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    wxLaunchDefaultBrowser("https://mcorino.github.io/wxRuby3/");
}

void MainFrame::OnChangeAlignment(wxCommandEvent& event)
{
    if (!m_selected_node)
    {
        return;
    }

    int align = 0;
    const bool vertical = (event.GetId() == id_AlignTop || event.GetId() == id_AlignBottom ||
                           event.GetId() == id_AlignCenterVertical);

    switch (event.GetId())  // NOLINT (cppcheck-suppress)
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

    m_selected_node->ChangeAlignment(align, vertical);

    UpdateLayoutTools();
}

void MainFrame::OnChangeBorder(wxCommandEvent& event)
{
    if (!m_selected_node)
    {
        return;
    }

    int border = 0;

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

    m_selected_node->ToggleBorderFlag(border);

    UpdateLayoutTools();
}

void MainFrame::OnClose(wxCloseEvent& event)
{
    if (!SaveWarning())
    {
        return;
    }

    wxGetApp().setMainFrameClosing();

    wxConfigBase* config = wxConfig::Get();
#if defined(_DEBUG)
    config->SetPath("/debug_mainframe");
#else
    config->SetPath("/mainframe");
#endif

    const bool isIconized = IsIconized();
    const bool isMaximized = IsMaximized();

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

    if (wxGetApp().isTestingMenuEnabled())
    {
        config->SetPath("/debug_history");
        m_ImportHistory.Save(*config);
        config->SetPath("/");

        if (g_pMsgLogging)
        {
            g_pMsgLogging->CloseLogger();
        }
    }
    event.Skip();
}

void MainFrame::OnCopy(wxCommandEvent& /* event unused */)
{
    if (auto* focus_win = wxWindow::FindFocus();
        focus_win && focus_win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        wxStaticCast(focus_win, wxStyledTextCtrl)->Copy();
        return;
    }
    if (m_selected_node)
    {
        m_selected_node->CopyNode();
        UpdateFrame();
    }
}

void MainFrame::OnCut(wxCommandEvent& /* event unused */)
{
    if (auto* focus_win = wxWindow::FindFocus();
        focus_win && focus_win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        // This is a read-only control, so we don't allow Cut
        return;
    }
    if (m_selected_node)
    {
        m_selected_node->RemoveNode(true);
        UpdateFrame();
    }
}

void MainFrame::OnDelete(wxCommandEvent& /* event unused */)
{
    ASSERT(m_selected_node);
    m_selected_node->RemoveNode(false);
    UpdateFrame();
}

void MainFrame::OnDifferentProject(wxCommandEvent& /* event unused */)
{
    if (!SaveWarning())
    {
        return;
    }

    std::ignore = DisplayStartupDlg(m_nav_panel);
}

void MainFrame::OnDuplicate(wxCommandEvent& /* event unused */)
{
    ASSERT(m_selected_node);
    m_selected_node->DuplicateNode();
}

void MainFrame::OnFind(wxFindDialogEvent& event)
{
    if (auto* page = m_notebook->GetCurrentPage(); page)
    {
        event.StopPropagation();
        event.SetClientData(m_findDialog);
        page->GetEventHandler()->ProcessEvent(event);
    }
}

void MainFrame::OnFindClose([[maybe_unused]] wxFindDialogEvent& event)
{
    m_findDialog->Destroy();
    m_findDialog = nullptr;
}

void MainFrame::OnFindDialog([[maybe_unused]] wxCommandEvent& event)
{
    if (!m_findDialog)
    {
        if (auto* page = m_notebook->GetCurrentPage(); page)
        {
            if (wxGetApp().isTestingMenuEnabled() && page == m_importPanel)
            {
                m_findData.SetFindString(m_importPanel->GetTextCtrl()->GetSelectedText());
            }
            else if (page != m_mockupPanel && page != m_docviewPanel)
            {
                m_findData.SetFindString(dynamic_cast<BasePanel*>(page)->GetSelectedText());
            }
        }
        m_findDialog = new wxFindReplaceDialog(this, &m_findData, "Find");
        m_findDialog->Centre(wxCENTRE_ON_SCREEN | wxBOTH);
    }
    m_findDialog->Show(true);
}

void MainFrame::OnImportProject([[maybe_unused]] wxCommandEvent& event)
{
    if (!SaveWarning())
    {
        return;
    }

    if (g_pMsgLogging)
    {
        g_pMsgLogging->Clear();
    }

    Project.NewProject();
}

void MainFrame::OnImportRecent(wxCommandEvent& event)
{
    const wxue::string file =
        m_ImportHistory.GetHistoryFile(event.GetId() - (START_IMPORT_FILE_IDS)).utf8_string();

    if (!SaveWarning())
    {
        return;
    }

    if (file.file_exists())
    {
        if (g_pMsgLogging)
        {
            g_pMsgLogging->Clear();
        }
        Project.ImportProject(file);
    }
    else if (wxMessageBox(wxString::Format("The project file '%s' doesn't exist.\n\nWould you "
                                           "like to remove it from the recent files list?",
                                           file.c_str()),
                          "Open recent project", wxICON_WARNING | wxYES_NO) == wxYES)
    {
        m_ImportHistory.RemoveFileFromHistory(event.GetId() - (START_IMPORT_FILE_IDS));
    }
}

void MainFrame::OnNewProject(wxCommandEvent& /* event unused */)
{
    if (!SaveWarning())
    {
        return;
    }

    // true means create an empty project
    Project.NewProject(true);
}

void MainFrame::OnNodeSelected(CustomEvent& event)
{
    // This event is normally only fired if the current selection has changed. We dismiss any
    // previous infobar message, and check to see if the current selection has any kind of issue
    // that we should warn the user about.
    m_info_bar->Dismiss();

    Node* evt_flags = event.getNode();

    if (evt_flags->is_Gen(gen_wxToolBar))
    {
        if (evt_flags->get_Parent()->is_Sizer())
        {
            const Node* grandparent = evt_flags->get_Parent()->get_Parent();
            if (grandparent->is_Type(type_frame_form) ||
                grandparent->is_Gen(gen_wxAuiMDIChildFrame))
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
        if (g_pMsgLogging)
        {
            g_pMsgLogging->OnNodeSelected();
        }
        m_importPanel->OnNodeSelected(evt_flags);
    }

    UpdateFrame();
}

void MainFrame::OnOpenProject([[maybe_unused]] wxCommandEvent& event)
{
    if (!SaveWarning())
    {
        return;
    }

    const wxString path = ShowOpenProjectDialog(this);
    if (!path.IsEmpty())
    {
        const wxue::string filename = path.utf8_string();
        // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui
        // extension
        if (filename.extension().is_sameas(PROJECT_FILE_EXTENSION, wxue::CASE::either) ||
            filename.extension().is_sameas(PROJECT_LEGACY_FILE_EXTENSION, wxue::CASE::either))
        {
            std::ignore = Project.LoadProject(filename);
        }
        else
        {
            Project.ImportProject(filename);
        }
    }
}

void MainFrame::OnOpenRecentProject(wxCommandEvent& event)
{
    if (!SaveWarning())
    {
        return;
    }
    const wxue::string file =
        m_FileHistory.GetHistoryFile(event.GetId() - wxID_FILE1).utf8_string();

    if (file.file_exists())
    {
        std::ignore = Project.LoadProject(file);
    }
    else if (wxMessageBox(wxString::Format("The project file '%s' doesn't exist.\n\nWould you "
                                           "like to remove it from the recent files list?",
                                           file.c_str()),
                          "Open recent project", wxICON_WARNING | wxYES_NO) == wxYES)
    {
        m_FileHistory.RemoveFileFromHistory(event.GetId() - wxID_FILE1);
    }
}

void MainFrame::OnPaste(wxCommandEvent& /* event unused */)
{
    if (auto* focus_win = wxWindow::FindFocus();
        focus_win && focus_win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        // This is a read-only control, so we don't allow Paste
        return;  // we don't allow pasting into the code generation windows which are marked as
                 // read-only
    }
    if (m_selected_node)
    {
        PasteNode(m_selected_node.get());
        UpdateFrame();
    }
}

void MainFrame::OnPreferencesDlg(wxCommandEvent& /* event unused */)
{
    PreferencesDlg dialog(this);
    dialog.ShowModal();
}

void MainFrame::OnProjectLoaded()
{
    UpdateFrame();
}

void MainFrame::OnReloadProject(wxCommandEvent& /* event unused */)
{
    if (wxMessageBox(
            wxString() << "This will lose any changes you have made since the last save.\n\n"
                          "Are you sure you want to reload the project?",
            "Reload Project", wxICON_WARNING | wxYES_NO) == wxYES)
    {
        std::ignore = Project.LoadProject(Project.get_ProjectFile());
    }
}

void MainFrame::OnSaveAsProject(wxCommandEvent& /* event unused */)
{
    wxFileName filename(*Project.get_wxFileName());
    if (!filename.IsOk())
    {
        filename.Assign("MyProject");
    }

    // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui
    // extension
    wxFileDialog dialog(
        this, "Save Project As", wxFileName::GetCwd(),
        Project.is_NewProject() ? wxString() : filename.GetFullName(),
        wxString(std::format("wxUiEditor Project File (*{})|{};{}", PROJECT_FILE_EXTENSION,
                             PROJECT_FILE_EXTENSION, PROJECT_LEGACY_FILE_EXTENSION)),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        filename = dialog.GetPath();

        if (filename.GetName().CmpNoCase("New Project") == 0)
        {
            wxMessageBox("You cannot save the project as \"New Project\". Please choose a "
                         "different name.",
                         "Save Project As");
            return;
        }

        // Note that under Windows, any extension the user added will be followed with a .wxui
        // extension
        const wxString file_ext = filename.GetExt();
        if (file_ext.empty())
        {
            filename.SetExt("wxui");
        }

        // Don't allow the user to walk over existing project file types that are probably
        // associated with another designer tool

        else if (file_ext.CmpNoCase("fbp") == 0)
        {
            wxMessageBox("You cannot save the project as a wxFormBuilder project file",
                         "Save Project As");
            return;
        }
        else if (file_ext.CmpNoCase("fjd") == 0)
        {
            wxMessageBox("You cannot save the project as a DialogBlocks project file",
                         "Save Project As");
            return;
        }
        else if (file_ext.CmpNoCase("wxg") == 0)
        {
            wxMessageBox("You cannot save the project as a wxGlade file", "Save Project As");
            return;
        }
        else if (file_ext.CmpNoCase("wxs") == 0)
        {
            wxMessageBox("You cannot save the project as a wxSmith file", "Save Project As");
            return;
        }
        else if (file_ext.CmpNoCase("xrc") == 0)
        {
            wxMessageBox("You cannot save the project as a XRC file", "Save Project As");
            return;
        }
        else if (file_ext.CmpNoCase("rc") == 0 || file_ext.CmpNoCase("dlg") == 0)
        {
            wxMessageBox("You cannot save the project as a Windows Resource file",
                         "Save Project As");
            return;
        }

        pugi::xml_document document;
        Project.get_ProjectNode()->CreateDoc(document);
        if (document.save_file(filename.GetFullPath().utf8_string(), "  ",
                               pugi::format_indent_attributes))
        {
            m_isProject_modified = false;
            m_isImported = false;
            Project.clear_NewProject();
            m_FileHistory.AddFileToHistory(filename.GetFullPath());
            Project.set_ProjectPath(filename);
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
    if (m_isImported || Project.get_ProjectFile().empty() || Project.is_NewProject())
    {
        OnSaveAsProject(event);
    }
    else
    {
        if (Project.get_OriginalProjectVersion() != Project.get_ProjectVersion())
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
        pugi::xml_document document;
        Project.get_ProjectNode()->CreateDoc(document);
        if (document.save_file(Project.get_ProjectFile(), "  ", pugi::format_indent_attributes))
        {
            m_isProject_modified = false;
            ProjectSaved();
        }
        else
        {
            wxMessageBox(wxString("Unable to save the project: ") << Project.get_ProjectFile(),
                         "Save Project");
        }
    }
}

void MainFrame::OnToggleExpandLayout(wxCommandEvent& /* event unused */)
{
    if (!m_selected_node || !m_selected_node->get_Parent() ||
        !m_selected_node->get_Parent()->is_Sizer())
    {
        return;
    }

    NodeProperty* propFlag = m_selected_node->get_PropPtr(prop_flags);

    if (!propFlag)
    {
        return;
    }

    const wxue::string currentValue = propFlag->as_string();
    const bool wasExpanded = isPropFlagSet("wxEXPAND", currentValue);
    const wxue::string value = (wasExpanded ? ClearPropFlag("wxEXPAND", currentValue) :
                                              SetPropFlag("wxEXPAND", currentValue));

    if (!wasExpanded)
    {
        NodeProperty* alignment = m_selected_node->get_PropPtr(prop_alignment);
        if (alignment && !alignment->as_string().empty())
        {
            // All alignment flags are invalid if wxEXPAND is set
            ModifyProperty(alignment, "");
        }
    }

    ModifyProperty(propFlag, value);
}

void MainFrame::OnUpdateBrowseDocs(wxUpdateUIEvent& event)
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            wxString label = generator->GetHelpText(m_selected_node.get());
            if (label.empty())
            {
                label << "wxWidgets";
            }
            label << " Documentation";
            event.SetText(label);
            return;
        }
    }

    event.SetText("wxWidgets Documentation");
}

void MainFrame::OnUpdateBrowsePython(wxUpdateUIEvent& event)
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            wxString label = generator->GetPythonHelpText(m_selected_node.get());
            if (label.empty())
            {
                label << "wxPython";
            }
            label << " Documentation";
            event.SetText(label);
            return;
        }
    }

    event.SetText("wxPython Documentation");
}

void MainFrame::OnUpdateBrowseRuby(wxUpdateUIEvent& event)
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            wxString label = generator->GetRubyHelpText(m_selected_node.get());
            if (label.empty())
            {
                label << "wxRuby";
            }
            label << " Documentation";
            event.SetText(label);
            return;
        }
    }

    event.SetText("wxRuby Documentation");
}
